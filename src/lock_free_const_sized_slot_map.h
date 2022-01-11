
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
    static constexpr auto get_index(const Key& k)      { const auto& [idx, gen] = k; return idx; }
    static constexpr auto get_generation(const Key& k) { const auto& [idx, gen] = k; return gen; }
    static constexpr void increment_generation(Key& k) { auto& [idx, gen] = k; ++gen; }
    
    template<class Integral> 
    static constexpr void set_index(Key& k, Integral value) { auto& [idx, gen] = k; idx = static_cast<key_index_type>(value); }

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

    lock_free_const_sized_slot_map() 
            : _erase_array_length {}
            , _size {}
    {
        _next_available_slot_index.store(0); // first element of slot container
         
        for (slot_index_type slot_idx {}; slot_idx < _slots.size(); ++slot_idx)
        {
            _slots[slot_idx] = std::make_pair(slot_idx+1, key_generation_type{});
        }
        
        _sentinel_last_slot_index.store(_slots.size()-1);
    }

    constexpr key_type insert(const T& value)   { return this->emplace(value);            }
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

    // this is non blocking. if another thread is currently iterating,
    // add to erase queue and return. 
    // TODO: perhaps we want to increment generation to disable the key from
    // further indexing?
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


    // due to the iterationLock, 
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
        assert(ul.owns_lock());

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

    constexpr size_t capacity() const
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

    // TODO: increment generation to make the object unreachable from here forward.
    void addToEraseQueue(const key_type &key)
    {
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
        
        assert(cur_erase_array_length == erase_idx);
    }


    std::vector<slot_type> _slots         = std::vector<slot_type>(Size+1); // +1 for sentinel
    container_type         _values        = container_type(Size);
    std::vector<size_t>    _reverse_array = std::vector<size_t>(Size);

    // linked list used to keep track of the available slots in the slot array
    std::atomic<key_index_type> _next_available_slot_index;
    std::atomic<key_index_type> _sentinel_last_slot_index;
    
    // stack used to store elements to be deleted. This is only used if trying
    // to delete while iterating- otherwise the elemnt gets deleted on the spot)
    std::vector<key_type> _erase_array = std::vector<key_type>(Size);
    std::atomic<size_t>  _erase_array_length;

    // number of elements in the values container. Unless caught in the middle 
    // of an insertion/deletion, this will also be the size of used slots
    // in the slots array.
    std::atomic<size_t> _size;

    // enforces that we can't iterate & delete at the same time
    std::mutex _iterationLock;
};

} // namespace gby
