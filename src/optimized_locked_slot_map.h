/*
 * optimized_locked_slot_map.h - A thread-safe constant sized implementation 
 * of a slot map. It's called optimized relative to the locked_slot_map 
 * implementation which is a wrapper around a non-thread safe slot map.
 * 
 * This optimized implementation utilizes elements of lock-free programming
 * to avoid blocking in most scenarios.
 * 
 */

#pragma once

#include "utils.h"

#include <utility>
#include <vector>
#include <deque>
#include <chrono>
#include <assert.h>
#include <atomic>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <stdexcept>
#include <tuple>
#include <limits>
#include <iostream>

namespace gby
{

template<
    typename T,
    size_t Size,
    typename Key = std::pair<unsigned, unsigned>,
    typename Container = std::vector<T>
>
class optimized_locked_slot_map
{
    template<class KeySlot> 
    static constexpr auto get_index(const KeySlot& k)      { const auto& [idx, gen] = k; return idx; }
    
    template<class KeySlot> 
    static constexpr auto& get_generation(const KeySlot& k)  { auto& [idx, gen] = k; return gen; }
    
    template<class KeySlot, class Integral> 
    static constexpr void set_index(KeySlot& k, Integral value) { auto& [idx, gen] = k; idx = static_cast<key_index_type>(value); }

public:
    using key_type             = Key;
    using key_index_type       = decltype(std::declval<Key>().first);
    using key_generation_type  = decltype(std::declval<Key>().second);
    using slot_index_type      = key_index_type;
    using slot_generation_type = std::atomic<key_generation_type>;
    using slot_type            = std::pair<slot_index_type, slot_generation_type>;

    using container_type   = Container;
    using value_type       = T;
    using size_type        = typename container_type::size_type;
    using reference        = typename container_type::reference;
    using const_reference  = typename container_type::const_reference;

    static constexpr size_t null_key_index = std::numeric_limits<key_index_type>::max();

    optimized_locked_slot_map() 
            : _erase_array_length {0}
            , _conservative_size {0}
            , _size {0}
    {
        _next_available_slot_index.store(0); // first element of slot container
         
        for (slot_index_type slot_idx {}; slot_idx < Size; ++slot_idx)
        {
            _slots[slot_idx] = std::make_pair(slot_idx+1, key_generation_type{});
        }
        
        _slots[Size] = std::make_pair(Size, key_generation_type{});
        _sentinel_last_slot_index.store(Size);
    }

    constexpr key_type insert(const T& value)   { return this->emplace(value);            }
    constexpr key_type insert(T&& value)        { return this->emplace(std::move(value)); }

    template<class ... Args> 
    constexpr key_type emplace(Args&& ... args) 
    {
        slot_index_type cur_slot_idx {};
        do 
        {
            cur_slot_idx = _next_available_slot_index.load(std::memory_order_acquire);

            if (unlikely(cur_slot_idx == _sentinel_last_slot_index.load(std::memory_order_acquire)))
                throw std::length_error("Slot Map is at max capacity.");
        }
        while (!_next_available_slot_index.compare_exchange_strong(cur_slot_idx, get_index<slot_type>(_slots[cur_slot_idx]))); 

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
            while (conservSize < _size.load(std::memory_order_acquire) && _conservative_size.compare_exchange_strong(conservSize, conservSize+1));
        }        
        
        return {cur_slot_idx, get_generation(*cur_slot).load(std::memory_order_acquire)};       
    }

    // this is non blocking. if another thread is currently iterating,
    // add to erase queue and return. 
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

    // due to the iterationLock, size can only increase inside this method
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

    constexpr size_t size()     const { return _size.load(std::memory_order_relaxed); }
    constexpr size_t capacity() const { return _data.capacity(); }
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
        if (const auto&[idx,gen] = key; idx >=Size)
            throw std::out_of_range("Index " + std::to_string(idx) + 
                    " is larger than Slot Map size of " + std::to_string(Size));
        
        return find(key);
    }

    constexpr std::optional<std::reference_wrapper<const value_type>> at(const key_type& key) const
    {
        if (const auto&[idx, gen] = key; idx >= Size)
            throw std::out_of_range("Index " + std::to_string(idx) + 
                    " is larger than Slot Map size of " + std::to_string(Size));
        
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

public:
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
            size_t idx {};
            do
            {
                idx = _erase_array_length.load(std::memory_order_acquire);
                _erase_array[idx] = get_index(key);
            }
            while (!_erase_array_length.compare_exchange_strong(idx, idx+1));
            _erase_array[idx] = get_index(key);

            return true;
        }
        return false;
    }

    // should only ever be called by drainEraseQueue - don't call this directly.
    void drainEraseQueueImpl()
    {
        if (_erase_array_length.load(std::memory_order_acquire) == 0)
            return;

        size_t cur_erase_array_length {};
        size_t erase_idx {};
        do
        {
            cur_erase_array_length = _erase_array_length.load(std::memory_order_acquire);

            for ( ; erase_idx < cur_erase_array_length; ++erase_idx)
            {
                const size_t slot_to_erase_idx = _erase_array[erase_idx];
                slot_type &slot_to_erase = _slots[slot_to_erase_idx];
                size_t data_idx_to_free = get_index(slot_to_erase);

                // replace object with the current last object in data array
                slot_index_type data_arr_len = _size.fetch_sub(1, std::memory_order_acq_rel);
                _data[data_idx_to_free] = _data[data_arr_len-1];

                size_t slot_to_update_idx = _reverse_array[data_arr_len-1];
                slot_type &slot_to_update = _slots[slot_to_update_idx];
                set_index(slot_to_update, data_idx_to_free);
                _reverse_array[data_idx_to_free] = slot_to_update_idx;

                _conservative_size.store(data_arr_len-1, std::memory_order_release);

                // add 'erased' slot back to free slot list
                // NOTE: this is the only place that _sentinel_last_slot_index can change, 
                // and because of the lock we don't need to worry about data races. 
                key_index_type previous_sentinel = _sentinel_last_slot_index.load(std::memory_order_acquire);
                set_index(_slots[previous_sentinel], slot_to_erase_idx);
                _sentinel_last_slot_index.store(slot_to_erase_idx, std::memory_order_release);
            }
        }
        while (!_erase_array_length.compare_exchange_strong(cur_erase_array_length, 0));
        assert(cur_erase_array_length == erase_idx);
    }


    std::vector<slot_type> _slots = std::vector<slot_type>(Size+1); // +1 for sentinel
    container_type         _data = container_type(Size+1);
    std::vector<size_t>    _reverse_array = std::vector<size_t>(Size+1);

    // free list used to keep track of the available slots in the slot array
    std::atomic<key_index_type> _next_available_slot_index;
    std::atomic<key_index_type> _sentinel_last_slot_index;
    
    // stack used to store elements to be deleted.
    std::vector<slot_index_type> _erase_array = std::vector<slot_index_type>(Size);
    std::atomic<size_t>  _erase_array_length;


    // number of elements in the values container. Unless caught in the middle 
    // of an insertion/deletion, this will also be the size of used slots
    // in the slots array.
    std::atomic<slot_index_type> _size;
    std::atomic<slot_index_type> _conservative_size;

    std::shared_mutex _eraseMut;
};

} // namespace gby
