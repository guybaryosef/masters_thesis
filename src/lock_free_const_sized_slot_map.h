
#pragma once

#include <utility>
#include <array>
#include <deque>
#include <chrono>
#include <atomic>
#include <mutex>
#include <stdexcept>
#include <tuple>
#include <limits>


namespace gby
{

template<
    typename T,
    size_t Size,
    typename Key = std::pair<unsigned, unsigned>,
    typename Container = std::vector<T>
>
class lock_free_const_sized_slot_map
{
    static constexpr auto get_index(const Key& k) { const auto& [idx, gen] = k; return idx; }
    static constexpr auto get_generation(const Key& k) { const auto& [idx, gen] = k; return gen; }
    template<class Integral> static constexpr void set_index(Key& k, Integral value) { auto& [idx, gen] = k; idx = static_cast<key_index_type>(value); }
    static constexpr void increment_generation(Key& k) { auto& [idx, gen] = k; ++gen; }

public:
    using key_type   = Key;
    using slot_type  = key_type;
    using key_index_type       = decltype(lock_free_const_sized_slot_map::get_index(std::declval<Key>()));
    using key_generation_type  = decltype(lock_free_const_sized_slot_map::get_generation(std::declval<Key>()));
    using slot_index_type      = key_index_type;
    using slot_generation_type = key_generation_type;

    using container_type   = Container;
    using size_type        = typename container_type::size_type;
    using reference        = typename container_type::reference;
    using const_reference  = typename container_type::const_reference;
    using iterator         = typename container_type::iterator;
    using const_iterator   = typename container_type::const_iterator;
    using reverse_iterator = typename container_type::reverse_iterator;
    using const_reverse_iterator = typename container_type::const_reverse_iterator;

    static constexpr size_t null_key_index = std::numeric_limits<key_index_type>::max();

    constexpr iterator begin()                         { return _values.begin(); }
    constexpr iterator end()                           { return _values.end();   }
    constexpr const_iterator begin() const             { return _values.begin(); }
    constexpr const_iterator end() const               { return _values.end();   }
    constexpr const_iterator cbegin() const            { return _values.begin(); }
    constexpr const_iterator cend() const              { return _values.end();   }
    constexpr reverse_iterator rbegin()                { return _values.rbegin();}
    constexpr reverse_iterator rend()                  { return _values.rend();  }
    constexpr const_reverse_iterator rbegin() const    { return _values.rbegin();}
    constexpr const_reverse_iterator rend() const      { return _values.rend();  }
    constexpr const_reverse_iterator crbegin() const   { return _values.rbegin();}
    constexpr const_reverse_iterator crend() const     { return _values.rend();  }

    lock_free_const_sized_slot_map() : _erase_array_length {}, _size{}
    {
        _next_available_slot_index.store(0);
        for (slot_index_type slot_idx {}; slot_idx < _slots.size(); ++slot_idx)
        {
            _slots[slot_idx] = std::make_pair(slot_idx+1, key_generation_type{});
        }
        
        _sentinel_last_slot_index.store(_slots.size()-1);
    }

    constexpr key_type insert(const T& value)   { return this->emplace(value); }
    constexpr key_type insert(T&& value)        { return this->emplace(std::move(value)); }

    template<class... Args> 
    constexpr key_type emplace(Args&&... args) 
    {
        /*
        if slots or values array is full, return error/exception/etc

        increment values size atomically
        emplace element into value spot
        insert into slots array the new value slot
            - CAS on next_available_slot_index, whether it has changed, and if not switch it with the next in line.
            - edit new slot object
            - increment slots size
        */
        slot_index_type cur_slot_idx {};
        do
        {
            cur_slot_idx = _next_available_slot_index.load(std::memory_order_relaxed);
            if (cur_slot_idx == _sentinel_last_slot_index.load(std::memory_order_relaxed))
                throw std::length_error("Slot Map is at max capacity.");
        }
        while (!_next_available_slot_index.compare_exchange_strong(cur_slot_idx, get_index(_slots[cur_slot_idx]))); 

        slot_index_type cur_value_idx = _size.fetch_add(1); 
        _values[cur_value_idx] = std::forward<Args...>(args...);
        
        slot_type& cur_slot = _slots[cur_slot_idx]; 
        set_index(cur_slot, cur_value_idx);
        _reverse_array[cur_value_idx] = cur_slot_idx;

        return {cur_slot_idx, get_generation(cur_slot)};       
    }

    constexpr void erase(const key_type& key) 
    {
        /*
            add to erase queue
            if not iterating
                drain erase queue
        */
        addToEraseQueue(key);

        std::unique_lock<std::mutex> ul (_iterationLock, std::try_to_lock);
        if (ul.owns_lock())
            drainEraseQueue();
    }


    // while performing this method, length can't decrease- only increase
    template <class P>
    constexpr bool iterate_map(P pred) 
    {
        // get iteration-remove lock
        // get length of values array (not the slots array).
        // iterate through that length of elements
        // once done, check if size increased and continue iterating. repeat while size increases.
        // release iteration-remove lock
        // remove elements in erase queue
        std::unique_lock ul (_iterationLock, std::try_to_lock);
        if (!ul.owns_lock())
            return false;

        int i{};
        size_t size{};
        do
        {
            size = this->size();

            for ( ; i < size; ++i)
                pred(_values[i]);

        } 
        while (size != this->size());

        drainEraseQueue();

        return true;
    }

    constexpr size_t size() const
    {
        return _size.load(std::memory_order_relaxed);
    }

    constexpr size_t capacity()
    {
        return _values.max_size();
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
        const slot_type& slot {_slots[get_index(key)]};
        return std::next(_values.begin(), get_index(slot));
    }
    
    constexpr const_iterator find_unchecked(const key_type& key) const
    {
        const slot_type& slot {_slots[get_index(key)]};
        return std::next(_values.begin(), get_index(slot));
    }

    constexpr bool empty() const
    {
        return size() == 0;
    }

private:
    constexpr std::optional<std::reference_wrapper<const slot_type>> get_slot(const key_type &key) const
    {
        try
        {
            const auto &[idx, gen] = key;

            if (auto &slot = _slots[idx]; get_generation(slot) == gen)
                return slot;
        }
        catch(const std::out_of_range &e) {}
        return {};
    }

    constexpr std::optional<std::reference_wrapper<slot_type>> get_slot(const key_type &key)
    {
        try
        {
            const auto &[idx, gen] = key;

            if (auto &slot = _slots[idx]; get_generation(slot) == gen)
                return slot;
        }
        catch(const std::out_of_range &e) {}
        return {};
    }

    void addToEraseQueue(const key_type &key)
    {
        /*
            if erase queue is empty
                have begEraseQueue point to this value
            else
                have end of erase queue point to this value
        */
    //    std::optional<slot_type&> op_slot = get_slot(key);
    //    if (op_slot.has_value())
    //    {
    //         auto &slot = op_slot.value();
    //         key_index_type last_erase_value_idx {_last_on_erase_queue.exchange(get_index(slot))};
    //         key_index_type last_erase_slot_idx = _reverse_array[last_erase_value_idx];
    //         set_index(_slots[last_erase_slot_idx], slot)
    //         if (last_erase_slot == null_key_index)
    //         {
    //             key_index_type beg_erase_queue {_next_on_erase_queue.load(std::memory_order_relaxed)};
    //             if (beg_erase_queue == null_key_index)
    //             {
    //                 _next_on_erase_queue.compare_exchange_strong(0, get_index(slot));
    //             }
    //         }
    //    }
        if (get_slot(key).has_value()) // validate key is valid
        {
            size_t index = _erase_array_length.fetch_add(1);
            _erase_array[index] = key;
        }
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
        
        do
        {
            cur_erase_array_length = _erase_array_length.load(std::memory_order_relaxed);
            for (auto erase_idx = 0; erase_idx < cur_erase_array_length; ++erase_idx)
            {
                const key_type &key_to_slot_to_erase = _erase_array[erase_idx];
                auto op_slot = get_slot(key_to_slot_to_erase);
                if (op_slot)
                {
                    slot_type &cur_slot = op_slot.value();
                    increment_generation(cur_slot);

                    // erase the element
                    size_t values_length {};
                    do
                    {
                        values_length = _size.load(std::memory_order_relaxed);
                        _values[get_index(cur_slot)] = _values[values_length-1];
                    }
                    while (!_size.compare_exchange_strong(values_length, values_length-1));

                    slot_type &slot_to_update = _slots[ _reverse_array[values_length-1] ];

                    set_index(slot_to_update, get_index(cur_slot));

                    // add erased slot back to free slot list
                    key_index_type previous_sentinel {};
                    do
                    {
                        previous_sentinel = _sentinel_last_slot_index.load(std::memory_order_relaxed);
                        set_index(_slots[previous_sentinel], get_index(key_to_slot_to_erase));
                    } 
                    while (!_sentinel_last_slot_index.compare_exchange_strong(previous_sentinel, get_index(key_to_slot_to_erase)));
                }
            }
        }
        while (!_erase_array_length.compare_exchange_strong(cur_erase_array_length, 0));
    }

    std::vector<slot_type> _slots  = std::vector<slot_type>(Size+1);
    container_type         _values = container_type(Size);
    std::vector<size_t>    _reverse_array = std::vector<size_t>(Size);

    std::atomic<key_index_type> _next_available_slot_index;
    std::atomic<key_index_type> _sentinel_last_slot_index;
    
    std::vector<key_type> _erase_array = std::vector<key_type>(Size);
    std::atomic<size_t> _erase_array_length;

    std::atomic<size_t> _size;

    std::mutex _iterationLock;
};


} // namespace gby
