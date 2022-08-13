/*
 * slot_map.h - A lock-free impelemntation of a slot_map.
 */

#pragma once

#include "internal_vector.h"

#include <utility>
#include <vector>
#include <mutex>
#include <optional>
#include <assert.h>
#include <shared_mutex>


namespace gby
{

template<
    typename T,
    typename Key = std::pair<unsigned, unsigned>
>
class slot_map
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

    slot_map() = default;

    slot_map(size_t initial_size, float reserve_factor = 2)
            : _capacity{initial_size}
            , _conservative_size {}
            , _size {}
            , _reserve_factor {(reserve_factor > 1) ? reserve_factor : 2}
    {
        _slots.reserve(_capacity + 1); // +1 for sentinel node
        _reverse_array.reserve(_capacity + 1); // +1 for sentinel node
        _erase_array.reserve(_capacity + 1); // +1 for sentinel node


        _next_available_slot_index.store(0); // first element of slot container
        for (slot_index_type slot_idx {}; slot_idx < _capacity; ++slot_idx)
        {
            _slots.push_back(std::make_pair(slot_idx+1, key_generation_type{}));
        }
        
        _slots.push_back(std::make_pair(_capacity, key_generation_type{}));
        _sentinel_last_slot_index.store(_capacity);
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
                {}
            }
        }
        while (!_next_available_slot_index.compare_exchange_strong(cur_slot_idx, get_index(_slots[cur_slot_idx]))); 

        slot_type* cur_slot {};
        {
            std::shared_lock lg {_eraseMut};
            slot_index_type cur_value_idx = _size.fetch_add(1, std::memory_order_acq_rel);

            _data[cur_value_idx] = std::forward<Args...>(args...);

            cur_slot = &_slots[cur_slot_idx]; 
            set_index(*cur_slot, cur_value_idx);
            _reverse_array[cur_value_idx] = cur_slot_idx;            

            slot_index_type conservSize{};
            do 
            {
                conservSize = _conservative_size.load(std::memory_order_acquire);
                if (get_index(_slots[_reverse_array[conservSize]]) != conservSize)
                    break;
            }
            while (conservSize < _size.load(std::memory_order_acquire) && 
                   _conservative_size.compare_exchange_strong(conservSize, conservSize+1));
        }
        
        return {cur_slot_idx, get_generation(*cur_slot).load(std::memory_order_acquire)};       
    }

    constexpr void reserve(float new_capacity)
    {
        slot_index_type requested_capacity = static_cast<slot_index_type>(new_capacity);
        slot_index_type previous_capacity = _capacity.load(std::memory_order_acquire);
        if (requested_capacity <= previous_capacity)
            return;

        if (!_capacity.compare_exchange_strong(previous_capacity, requested_capacity))
            return;

        // +1 for the sentinel node
        _data.reserve(requested_capacity+1);
        _reverse_array.reserve(requested_capacity+1);
        _erase_array.reserve(requested_capacity+1);        
        _slots.reserve(requested_capacity + 1); 

        for (int i = previous_capacity+1; i < requested_capacity; ++i)
        {
            _slots.push_back(std::make_pair(i+1, key_generation_type{}));
        }
        _slots.push_back(std::make_pair(requested_capacity, key_generation_type{}));

        std::shared_lock lg {_eraseMut};
        auto previousSentinel = _sentinel_last_slot_index.load(std::memory_order_relaxed); 
        set_index(_slots[previousSentinel], previous_capacity+1);
        _sentinel_last_slot_index.store(requested_capacity, std::memory_order_relaxed);
    }

    template<bool Block=false>
    constexpr bool erase(const key_type& key) 
    {
        if (addToEraseQueue(key))
        {
            drainEraseQueue<Block>();                
            return true;
        }
        return false;
    }

    template <class P>
    constexpr void iterate_map(P pred) 
    {
        {
            std::shared_lock sl {_eraseMut};

            size_t i {};
            size_t size {};
            do 
            {
                size = _conservative_size.load(std::memory_order_acquire);
                for ( ; i < size; ++i)
                    pred(_data[i]);
            } 
            while (size != _conservative_size.load(std::memory_order_acquire));
        }
        
        drainEraseQueue();
    }

    constexpr void set_reserve_factor(const float val_)
    {
        if (val_ > 1)
            _reserve_factor = val_;
    }

    constexpr size_t size()     const { return _size.load(std::memory_order_relaxed); }
    constexpr size_t capacity() const { return _capacity.load(std::memory_order_acquire); }
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

    template<bool Block=false>
    void drainEraseQueue()
    {
        if constexpr (Block)
        {
            std::unique_lock ul {_eraseMut};
            drainEraseQueueImpl();
        }
        else
        {
            std::unique_lock ul {_eraseMut, std::try_to_lock};
            if (ul.owns_lock())
            {
                drainEraseQueueImpl();                
            }
        }
    }  

private:
   constexpr bool validate_and_increment_slot(const key_type &key) noexcept
    {
        try
        {
            const auto &[idx, gen] = key;

            auto &slot = _slots[idx];

            key_generation_type genCpy = gen;
            auto& slotGen = const_cast<slot_generation_type&>(get_generation(slot));
            if (slotGen.compare_exchange_strong(genCpy, genCpy+1))
                return true;
        }
        catch(const std::out_of_range &e) 
        {}
        
        return false;
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

    bool addToEraseQueue(const key_type &key)
    {
        if (validate_and_increment_slot(key))
        {
            size_t index = _erase_array_length.fetch_add(1, std::memory_order_acq_rel);
            _erase_array[index] = get_index(key);
            _conservative_erase_array_length.store(index+1, std::memory_order_release);
            return true;
        }
        return false;
    }

    // should only ever be called by drainEraseQueue - don't call this directly.
    void drainEraseQueueImpl()
    {
        size_t cur_erase_array_length {};
        size_t erase_idx {};
        do
        {
            cur_erase_array_length = _conservative_erase_array_length.load(std::memory_order_acquire);

            for ( ; erase_idx < cur_erase_array_length; ++erase_idx)
            {
                const size_t slot_to_erase_idx = _erase_array[erase_idx];
                slot_type &slot_to_erase = _slots[slot_to_erase_idx];
                size_t data_idx_to_free = get_index(slot_to_erase);

                // replace object with the current last object in values array (actually erasing it from slot map)
                slot_index_type data_arr_len = _size.fetch_sub(1, std::memory_order_acq_rel);
                _data[data_idx_to_free] = _data[data_arr_len-1];

                size_t slot_to_update_idx = _reverse_array[data_arr_len-1];
                slot_type &slot_to_update = _slots[slot_to_update_idx];
                set_index(slot_to_update, data_idx_to_free);
                _reverse_array[data_idx_to_free] = slot_to_update_idx;

                _conservative_size.store(data_arr_len-1, std::memory_order_release);

                // add 'erased' slot back to free slot list
                key_index_type previous_sentinel = _sentinel_last_slot_index.load(std::memory_order_acquire);
                set_index(_slots[previous_sentinel], slot_to_erase_idx);
                _sentinel_last_slot_index.store(slot_to_erase_idx, std::memory_order_release);                    _sentinel_last_slot_index.store(slot_to_erase_idx, std::memory_order_release);
            }
        }
        while (!_erase_array_length.compare_exchange_strong(cur_erase_array_length, 0));
        
        assert(cur_erase_array_length == erase_idx);
    }

    gby::internal_vector<slot_type> _slots;
    container_type                  _data;
    gby::internal_vector<slot_index_type> _reverse_array;    

    std::atomic<key_index_type> _next_available_slot_index;
    std::atomic<key_index_type> _sentinel_last_slot_index;

    std::atomic<slot_index_type> _size;
    std::atomic<slot_index_type> _conservative_size;
    std::atomic<slot_index_type> _capacity;

    float _reserve_factor;

    // stack used to store elements to be deleted. This is only used if trying
    // to delete while iterating- otherwise the elemnt gets deleted on the spot)
    gby::internal_vector<slot_index_type> _erase_array;
    std::atomic<size_t> _erase_array_length;
    std::atomic<size_t> _conservative_erase_array_length;

    // enforces that we can't iterate & delete at the same time
    std::shared_mutex _eraseMut;
};

} // namespace gby
