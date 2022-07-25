/*
 * locked_slot_map.h - A lock-free impelemntation of a slot_map.
 */

#pragma once

#include "internal_vector.h"

#include <utility>
#include <vector>
#include <mutex>


namespace gby
{

template<
    typename T,
    typename Key = std::pair<unsigned, unsigned>
>
class lock_free_slot_map
{
    template<class KeySlot> 
    static constexpr auto get_index(const KeySlot& k)      { const auto& [idx, gen] = k; return idx; }
    
    template<class KeySlot> 
    static constexpr auto& get_generation(const KeySlot& k) { const auto& [idx, gen] = k; return gen; }

   
    template<class KeySlot, class Integral> 
    static constexpr void set_index(KeySlot& k, Integral value) { auto& [idx, gen] = k; idx = static_cast<key_index_type>(value); }

public:
    using key_type             = Key;
    using key_index_type       = decltype(std::declval<Key>().first);
    using key_generation_type  = decltype(std::declval<Key>().second);
    using slot_index_type      = key_index_type;
    using slot_generation_type = std::atomic<key_generation_type>;
    using slot_type            = std::pair<slot_index_type, slot_generation_type>;

    using container_type   = internal_vector<T>;
    using value_type       = T;
    using size_type        = typename container_type::size_type;
    using reference        = typename container_type::reference;
    using const_reference  = typename container_type::const_reference;
    using iterator         = typename container_type::iterator;
    using const_iterator   = typename container_type::const_iterator;

    static constexpr size_t null_key_index = std::numeric_limits<key_index_type>::max();

    lock_free_slot_map() = default;

    lock_free_slot_map(size_t initial_size, float reserve_factor = 2)
            : _capacity{initial_size}
            , _conservative_size {}
            , _size {}
            , _reserve_factor {(reserve_factor > 1) ? reserve_factor : 2}
    {
        _slots.reserve(_capacity + 1); // +1 for sentinel node

        _next_available_slot_index.store(0); // first element of slot container
        for (int i = 0; i < _slots.capacity(); ++i)
        {
            _slots.push_back(std::make_pair(i+1, key_generation_type{}));
        }
        
        _sentinel_last_slot_index.store(_slots.capacity()-1);
    }
    
    constexpr key_type insert(const T& value)   { return this->emplace(value);            }
    constexpr key_type insert(T&& value)        { return this->emplace(std::move(value)); }

    template<class... Args> 
    constexpr key_type emplace(Args&&... args) 
    {
        slot_index_type cur_slot_idx {};
        do
        {
            cur_slot_idx = _next_available_slot_index.load(std::memory_order_acquire);


            if (unlikely(cur_slot_idx == _sentinel_last_slot_index.load(std::memory_order_acquire)))
            {
                reserve(_reserve_factor*_capacity);
                while (cur_slot_idx == _sentinel_last_slot_index.load(std::memory_order_acquire))
                    ;
            }
        }
        while (!_next_available_slot_index.compare_exchange_strong(cur_slot_idx, get_index(_slots[cur_slot_idx]))); 

        // replace object with the current last object in data array
        size_type cur_value_idx {};
        do 
        {
            cur_value_idx = _conservative_size.load(std::memory_order_acquire);
        }
        while (!_size.compare_exchange_strong(cur_value_idx, cur_value_idx+1));

        _data[cur_value_idx] = std::forward<Args...>(args...);

        slot_type& cur_slot = _slots[cur_slot_idx]; 
        set_index(cur_slot, cur_value_idx);
        _reverse_array[cur_value_idx] = cur_slot_idx;

        _conservative_size.store(cur_value_idx+1, std::memory_order_release);

        return {cur_slot_idx, get_generation(cur_slot).load(std::memory_order_relaxed)};
    }

    constexpr void reserve(float new_capacity)
    {
        size_type requested_capacity = static_cast<size_type>(new_capacity);
        size_type previous_capacity = _capacity.load(std::memory_order_relaxed);
        if (requested_capacity <= previous_capacity)
            return;

        if (!_capacity.compare_exchange_strong(previous_capacity, requested_capacity))
            return;


        _data.reserve(requested_capacity);
        _reverse_array.reserve(requested_capacity);

        _slots.reserve(requested_capacity + 1); // +1 for the sentinel node
        // set up the new slots that have been allocated 
        for (int i = previous_capacity; i < _slots.capacity(); ++i)
        {
            _slots.push_back(std::make_pair(i+1, key_generation_type{}));
        }
    
        // set_index(_slots[_slots.capacity()-1], _sentinel_last_slot_index.load()); 
        // TODO: it would help if we had a way to signify from multiple places that this is the sentinel node.
        // perhaps in addition to the sentinel atomic pointer, we just have a sentinel value that we can check for.
        
        // Both the drainEraseQueue and this method update _sentinel_last_slot_index pointer, and it requires 
        // 2 operaitons to happen together.
        {
            std::scoped_lock sl {_sentinelLock};
            set_index(_slots[_sentinel_last_slot_index.load(std::memory_order_relaxed)], previous_capacity);
            _sentinel_last_slot_index.store(_slots.capacity()-1, std::memory_order_relaxed);
        }
    }

    constexpr void erase(const key_type& key) 
    {
        if (addToEraseQueue(key))
        {
            std::unique_lock ul (_iterationLock, std::try_to_lock);
            if (ul.owns_lock())
                drainEraseQueue();
        }
    }

    template<bool Block>
    constexpr void flushEraseQueue()
    {
        if constexpr(Block) 
        {
            std::unique_lock ul (_iterationLock);
            assert(ul.owns_lock());
            drainEraseQueue();
        }
        else 
        {
            std::unique_lock ul (_iterationLock, std::try_to_lock);
            if (ul.owns_lock())
                drainEraseQueue();
        }
    }

    template <class P>
    constexpr void iterate_map(P pred) 
    {
        std::scoped_lock sl (_iterationLock);

        int i {};
        size_t size {};
        do 
        {
            size = _conservative_size.load(std::memory_order_acquire);
            for ( ; i < size; ++i)
                pred(_data[i]);
        } 
        while (size != _conservative_size.load(std::memory_order_relaxed));

        drainEraseQueue();
    }

    constexpr void set_reserve_factor(const float val_)
    {
        if (val_ > 1)
            _reserve_factor = val_;
    }

    constexpr size_t size()     const { return _size.load(std::memory_order_relaxed); }
    constexpr size_t capacity() const { return _capacity.load(std::memory_order_relaxed); }
    constexpr bool   empty()    const { return size() == 0; }

    constexpr reference operator[](const key_type& key)              
    { 
        return find_unchecked(key);
    }

    constexpr const_reference operator[](const key_type& key) const  
    { 
        return find_unchecked(key);
    }

    constexpr std::optional<std::reference_wrapper<value_type>> at(const key_type& key)
    {
        if (const auto&[idx,gen] = key; idx >= size())
            throw std::out_of_range("Index " + std::to_string(idx) + 
                    " is larger than Slot Map size of " + std::to_string(size()));
        
        return find(key);
    }

    constexpr std::optional<std::reference_wrapper<const value_type>> at(const key_type& key) const
    {
        if (const auto&[idx, gen] = key; idx >= size())
            throw std::out_of_range("Index " + std::to_string(idx) + 
                    " is larger than Slot Map size of " + std::to_string(size()));
        
        return find(key);
    }

    constexpr std::optional<std::reference_wrapper<value_type>> find(const key_type& key) 
    {
        if (auto slot = get_slot(key))
            return _data[get_index<slot_type>(*slot)];
        else
            return {};
    }
    
    constexpr std::optional<std::reference_wrapper<const value_type>> find(const key_type& key) const
    {
        if (auto slot = get_slot(key))
            return _data[get_index<slot_type>(*slot)];
        else
            return {};
    }

    constexpr reference find_unchecked(const key_type& key) 
    {
        const auto& slot {_slots[get_index<key_type>(key)]};
        return _data[get_index<slot_type>(slot)];
    }
    
    constexpr const_reference find_unchecked(const key_type& key) const
    {
        const auto& slot {_slots[get_index<key_type>(key)]};
        return _data[get_index<slot_type>(slot)];
    }


private:
   constexpr std::optional<std::reference_wrapper<slot_type>> get_and_increment_slot(const key_type &key) noexcept
    {
        try
        {
            const auto &[idx, gen] = key;

            auto &slot = _slots[idx];

            key_generation_type genCpy = gen;
            auto& slotGen = const_cast<slot_generation_type&>(get_generation(slot));
            if (slotGen.compare_exchange_strong(genCpy, genCpy+1))
                return slot;
        }
        catch(const std::out_of_range &e) 
        {}
        
        return {};
    }

    constexpr std::optional<std::reference_wrapper<const slot_type>> get_slot(const key_type &key) const noexcept
    {
        try
        {
            const auto &[idx, gen] = key;

            auto &slot = _slots[idx];
            if (get_generation(slot).load(std::memory_order_relaxed) == gen)
                return slot;
        }
        catch(const std::out_of_range &e) 
        {}
        
        return {};
    }

    constexpr std::optional<std::reference_wrapper<slot_type>> get_slot(const key_type &key) noexcept
    {
        try
        {
            const auto &[idx, gen] = key;

            auto &slot = _slots[idx];
            if (get_generation(slot).load(std::memory_order_relaxed) == gen)
                return slot;
        }
        catch(const std::out_of_range &e) 
        {}

        return {};
    }

    // return true if the value was found
    // and successfully put in the erase queue,
    // otherwise false if the slot is
    // not found.
    bool addToEraseQueue(const key_type &key)
    {
        auto slot = get_and_increment_slot(key);
        if (slot)
        {
            size_t index = _erase_array_length.fetch_add(1);
            _erase_array[index] = key;
            return true;
        }
        return false;
    }

    // only one thread can be inside this function at a time
    void drainEraseQueue()
    {  
        /*
            get size of array queue
            go one by one:
                - validate key
                - increment generator
                - copy end of values to current value
                - switch end-of-values slot to this index & decrement values size
                - add current slot to free slots list
        */
        size_t cur_erase_array_length {};
        
        size_t erase_idx {};
        do
        {
            cur_erase_array_length = _erase_array_length.load(std::memory_order_acquire);

            for ( ; erase_idx < cur_erase_array_length; ++erase_idx)
            {
                const size_t slot_to_erase_idx = get_index(_erase_array[erase_idx]);
                slot_type &slot_to_erase = _slots[slot_to_erase_idx];
                size_t data_idx_to_free = get_index(slot_to_erase);

                // replace object with the current last object in values array (actually erasing it from slot map)
                size_type data_arr_len {};
                do
                {
                    data_arr_len = _size.load(std::memory_order_acquire);
                    _data[data_idx_to_free] = _data[data_arr_len-1];
                }
                while (!_size.compare_exchange_strong(data_arr_len, data_arr_len-1));

                size_t slot_to_update_idx = _reverse_array[data_arr_len-1];
                slot_type &slot_to_update = _slots[slot_to_update_idx];
                set_index(slot_to_update, data_idx_to_free);
                _reverse_array[data_idx_to_free] = slot_to_update_idx;

                _conservative_size.store(data_arr_len-1, std::memory_order_release);

                // add 'erased' slot back to free slot list
                {
                    std::scoped_lock sl {_sentinelLock};
                    set_index(_slots[_sentinel_last_slot_index.load(std::memory_order_relaxed)], slot_to_erase_idx);
                    _sentinel_last_slot_index.store(slot_to_erase_idx, std::memory_order_release);
                }
            }
        }
        while (!_erase_array_length.compare_exchange_strong(cur_erase_array_length, 0));
        
        assert(cur_erase_array_length == erase_idx);
    }

    gby::internal_vector<slot_type> _slots;
    container_type                  _data;
    std::vector<size_t>             _reverse_array = std::vector<size_t>(_capacity);    

    std::atomic<key_index_type> _next_available_slot_index;
    std::atomic<key_index_type> _sentinel_last_slot_index;

    std::atomic<size_type> _size;
    std::atomic<size_type> _conservative_size;
    std::atomic<size_type> _capacity;

    float _reserve_factor;

    // stack used to store elements to be deleted. This is only used if trying
    // to delete while iterating- otherwise the elemnt gets deleted on the spot)
    std::vector<key_type> _erase_array = std::vector<key_type>(_capacity);
    std::atomic<size_t>   _erase_array_length;

    // enforces that we can't iterate & delete at the same time
    std::mutex _iterationLock;

    // enforces that only one place can move the sentinel node at a time.
    // this is because moving the sentinel node requires 2 operations.
    // would be really nice to get rid of this, down the line.
    std::mutex _sentinelLock;
};

} // namespace gby
