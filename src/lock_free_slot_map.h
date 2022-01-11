/*
 * locked_slot_map.h - A lock-free impelemntation of a slot_map.
 */

#pragma once

#include "lock_free_vector.h"

#include <utility>
#include <vector>

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

    using container_type   = std::vector<T>;
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

    lock_free_slot_map() = default;

    lock_free_slot_map(size_t initial_size)
            : _capacity{initial_size}
            , _size {0}
    {
        // _next_available_slot_index.store(0); // first element of slot container
         
        // for (slot_index_type slot_idx {}; slot_idx < _slots.size(); ++slot_idx)
        // {
        //     _slots[slot_idx] = std::make_pair(slot_idx+1, key_generation_type{});
        // }
        
        // _sentinel_last_slot_index.store(_slots.size()-1);

    }
    
    
    constexpr Key insert(const T& value)
    {
        //insert to back of 
        return {};
    }

    constexpr void erase(const key_type& key) 
    {
    }

    template <class P>
    constexpr bool iterate_map(P pred) 
    {
        return true;
    }

    constexpr size_t size() const
    {
        return 0;
    }

    constexpr size_t capacity() const
    {
        return 0;
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
        return end();
    }
    
    constexpr const_iterator find(const key_type& key) const
    {
        return end();
    }

    constexpr iterator find_unchecked(const key_type& key) 
    {
        return end();
    }
    
    constexpr const_iterator find_unchecked(const key_type& key) const
    {
        return end();
    }

    constexpr bool empty() const
    {
        return false;
    }

private:
    std::vector<Key> _slots;
    std::vector<T>   _values;

    std::atomic<uint64_t> _size;
    std::atomic<uint64_t> _capacity;
};


/*
insert
1. atomically get next available stop (similar to const sized map)
2. if size == capacity, prepare new block of memory and CAS.
3. get the next available slot. if generation is odd, skip to next value.
4. get empty slot with even generation, and increment the generation
5. build out the  
........
*/

/*
remove
1. if iteration lock then add key to to_be_removed list, 
and return future.
2. else:
3. 
.......
*/


/*
at/[]/find
1. return a reference- it i sup to the user to not delete that object while holding it.
*/


/*
iterate through - this should lock the values container.
1. get iteration lock.
2. get length of values array (not the values array).
2. iterate through the container and apply the function object.
3. release iteration lock.

- should we process to_be_removed list in the iteration lock, or outside?
*/


// --------------------------------------------------------------------

// Start small- Lets make a constant sized slot map lock free
// - no resize operation (capacity is a constant).
// - all memory is allocated ahead of time.
// - no memory to get destructed.

















} // namespace gby
