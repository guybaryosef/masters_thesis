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
    static constexpr auto get_index(const Key& k)      { const auto& [idx, gen] = k; return idx; }
    static constexpr auto get_generation(const Key& k) { const auto& [idx, gen] = k; return gen; }
    static constexpr void increment_generation(Key& k) { auto& [idx, gen] = k; ++gen; }
    
    template<class Integral> 
    static constexpr void set_index(Key& k, Integral value) { auto& [idx, gen] = k; idx = static_cast<key_index_type>(value); }

public:
    using key_type             = Key;
    using slot_type            = key_type;
    using key_index_type       = decltype(lock_free_slot_map::get_index(std::declval<Key>()));
    using key_generation_type  = decltype(lock_free_slot_map::get_generation(std::declval<Key>()));
    using slot_index_type      = key_index_type;
    using slot_generation_type = key_generation_type;

    using container_type   = internal_vector<T>;
    using size_type        = typename container_type::size_type;
    using reference        = typename container_type::reference;
    using const_reference  = typename container_type::const_reference;
    using iterator         = typename container_type::iterator;
    using const_iterator   = typename container_type::const_iterator;

    static constexpr size_t null_key_index = std::numeric_limits<key_index_type>::max();

    constexpr iterator begin()                         { return _values.begin(); }
    constexpr iterator end()                           { return _values.end();   }
    constexpr const_iterator begin() const             { return _values.begin(); }
    constexpr const_iterator end() const               { return _values.end();   }
    constexpr const_iterator cbegin() const            { return _values.begin(); }
    constexpr const_iterator cend() const              { return _values.end();   }

    lock_free_slot_map() = default;

    lock_free_slot_map(size_t initial_size, float reserve_factor = 2)
            : _capacity{initial_size}
            , _size {0}
            , _reserve_factor {reserve_factor}
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
            cur_slot_idx = _next_available_slot_index.load(std::memory_order_relaxed);


            if (unlikely(cur_slot_idx == _sentinel_last_slot_index.load(std::memory_order_relaxed)))
            {
                reserve(_reserve_factor*_capacity);
                while (cur_slot_idx == _sentinel_last_slot_index.load(std::memory_order_relaxed))
                    ;
            }
        }
        while (!_next_available_slot_index.compare_exchange_strong(cur_slot_idx, get_index(_slots[cur_slot_idx]))); 

        slot_index_type cur_value_idx = _size.fetch_add(1); 
        _values[cur_value_idx] = std::forward<Args...>(args...);
        
        slot_type& cur_slot = _slots[cur_slot_idx]; 
        set_index(cur_slot, cur_value_idx);
        _reverse_array[cur_value_idx] = cur_slot_idx;

        return {cur_slot_idx, get_generation(cur_slot)};
    }

    constexpr void reserve(float new_capacity)
    {
        size_type requested_capacity = static_cast<size_type>(new_capacity);
        size_type previous_capacity = _capacity.load(std::memory_order_relaxed);
        if (requested_capacity <= previous_capacity)
            return;

        if (!_capacity.compare_exchange_strong(previous_capacity, requested_capacity))
            return;


        _values.reserve(requested_capacity);
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
        addToEraseQueue(key);

        std::unique_lock ul (_iterationLock, std::try_to_lock);
        if (ul.owns_lock())
            drainEraseQueue();
    }

    template <class P>
    constexpr void iterate_map(P pred) 
    {
        std::scoped_lock sl (_iterationLock);

        int i{};
        size_t size{};
        do
        {
            // the size can only increase while we own the _iterationLock.
            // therefor we reach its current value and then check if it 
            // increased.
            size = this->size();

            for ( ; i < size; ++i)
                pred(_values[i]);

        } 
        while (size != this->size());

        drainEraseQueue();
    }

    constexpr void set_reserve_factor(const float val_)
    {
        _reserve_factor = val_;
    }

    constexpr size_t size() const
    {
        return _size.load(std::memory_order_relaxed);
    }

    constexpr size_t capacity() const
    {
        return _capacity.load(std::memory_order_relaxed);
    }

    constexpr reference operator[](const key_type& key)              
    { 
        return *find_unchecked(key);

    }

    constexpr const_reference operator[](const key_type& key) const  
    { 
        return *find_unchecked(key);
    }

    constexpr iterator find(const key_type& key) 
    {
        if (auto slot = get_slot(key))
            return std::next(_values.begin(), get_index(*slot));
        else
            return end();
    }
    
    constexpr const_iterator find(const key_type& key) const
    {
        if (auto slot = get_slot(key))
            return std::next(_values.begin(), get_index(*slot));
        else
            return end();
    }

    constexpr iterator find_unchecked(const key_type& key) 
    {
        const auto& slot = _slots[get_index(key)];
        return std::next(_values.begin(), get_index(slot));
    }
    
    constexpr const_iterator find_unchecked(const key_type& key) const
    {
        const auto& slot = _slots[get_index(key)];
        return std::next(_values.begin(), get_index(slot));
    }

    constexpr bool empty() const
    {
        return size() == 0;
    }

private:
    constexpr std::optional<std::reference_wrapper<const slot_type>> get_slot(const key_type &key) const noexcept
    {
        try
        {
            const auto &[idx, gen] = key;

            auto &slot = _slots[idx];
            if (get_generation(slot) == gen)
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
            if (get_generation(slot) == gen)
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
        auto slot = get_slot(key);
        if (slot.has_value())
        {
            increment_generation(*slot);
            size_t index = _erase_array_length.fetch_add(1);
            _erase_array.at(index) = key;
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
            cur_erase_array_length = _erase_array_length.load(std::memory_order_relaxed);

            for ( ; erase_idx < cur_erase_array_length; ++erase_idx)
            {
                const key_type &key_to_slot_to_erase = _erase_array[erase_idx];

                auto op_slot = get_slot(key_to_slot_to_erase);
                if (op_slot)
                {
                    slot_type &cur_slot = op_slot.value();
                    increment_generation(cur_slot);    // makes the object unreachable from this point onwards from its key

                    // replace object with the current last object in values array (actually erasing it from slot map)
                    size_t values_length {};
                    do
                    {
                        values_length = _size.load(std::memory_order_relaxed);
                        _values[get_index(cur_slot)] = _values[values_length-1];
                    }
                    while (!_size.compare_exchange_strong(values_length, values_length-1));

                    slot_type &slot_to_update = _slots[ _reverse_array[values_length-1] ];
                    set_index(slot_to_update, get_index(cur_slot));

                    // add 'erased' slot back to free slot list
                    {
                        std::scoped_lock sl {_sentinelLock};
                        set_index(_slots[_sentinel_last_slot_index.load(std::memory_order_relaxed)], get_index(key_to_slot_to_erase));
                        _sentinel_last_slot_index.store(get_index(key_to_slot_to_erase), std::memory_order_relaxed);
                    }
                }
            }
        }
        while (!_erase_array_length.compare_exchange_strong(cur_erase_array_length, 0));
        
        assert(cur_erase_array_length == erase_idx);
    }

    gby::internal_vector<Key>   _slots;
    std::atomic<key_index_type> _next_available_slot_index;
    std::atomic<key_index_type> _sentinel_last_slot_index;

    container_type        _values;
    std::atomic<uint64_t> _size;
    std::atomic<uint64_t> _capacity;

    float _reserve_factor;

    std::vector<size_t> _reverse_array = std::vector<size_t>(_capacity);

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
