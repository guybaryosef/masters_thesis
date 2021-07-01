/*
 * locked_slot_map.h - A thread-safe wrapper arround SG14's slot_map 
 * implementation, implemented using locks.
 */

#pragma once


#include "slot_map.h"

#include <algorithm>
#include <mutex>
#include <shared_mutex>

namespace gby
{

template<
    class T,
    class Key = std::pair<unsigned, unsigned>,
    template<class...> class Container = std::vector
>
class locked_slot_map
{
    static constexpr auto get_index(const Key& k) { const auto& [idx, gen] = k; return idx; }
    static constexpr auto get_generation(const Key& k) { const auto& [idx, gen] = k; return gen; }

public:
    
    using key_type = Key;
    using mapped_type = T;

    using key_index_type = decltype(locked_slot_map::get_index(std::declval<Key>()));
    using key_generation_type = decltype(locked_slot_map::get_generation(std::declval<Key>()));

    using container_type = Container<mapped_type>;
    using reference = typename container_type::reference;
    using const_reference = typename container_type::const_reference;
    using pointer = typename container_type::pointer;
    using const_pointer = typename container_type::const_pointer;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;
    using reverse_iterator = typename container_type::reverse_iterator;
    using const_reverse_iterator = typename container_type::const_reverse_iterator;

    using size_type = typename container_type::size_type;
    using value_type = typename container_type::value_type;


    constexpr locked_slot_map() = default;
    constexpr locked_slot_map(const locked_slot_map&) = default;
    constexpr locked_slot_map(locked_slot_map&&) = default;
    constexpr locked_slot_map& operator=(const locked_slot_map&) = default;
    constexpr locked_slot_map& operator=(locked_slot_map&&) = default;
    ~locked_slot_map() = default;

    locked_slot_map(const stdext::slot_map<T,Key,Container>& map)
            : slot_map{map} {}

    locked_slot_map(stdext::slot_map<T,Key,Container>&& map)
            : slot_map{map} {}

    // at() methods
    constexpr reference at(const key_type& key)
    {
        std::shared_lock sl(m);
        return slot_map.at(key);
    }

    constexpr const_reference at(const key_type& key) const
    {
        std::shared_lock sl(m);
        return slot_map.at(key);
    }

    // [] operators
    constexpr reference operator[](const key_type& key)              
    {
        std::shared_lock sl{m};
        return slot_map[key]; 
    }

    constexpr const_reference operator[](const key_type& key) const  
    {
        return *find_unchecked(key); 
    }

    // find() methods
    constexpr iterator find(const key_type& key) 
    {
        std::shared_lock sl{m}; // TODO: enters mutex multiple times - have scoped_shared_mutex?555
        return slot_map.find(key);
    }

    constexpr const_iterator find(const key_type& key) const
    {
        std::shared_lock sl{m};
        return slot_map.find(key);
    }

    // find_unchecked() methods
    constexpr iterator find_unchecked(const key_type& key) 
    {
        std::shared_lock sl{m};
        return slot_map.find_unchecked(key);
    }

    constexpr const_iterator find_unchecked(const key_type& key) const 
    {
        std::shared_lock sl{m};
        return slot_map.find_unchecked(key);
    }

    // replace iterators with iterate_map function
    template <class P>
    constexpr void iterate_map(P pred) {
        std::shared_lock sl{m};
        std::for_each(slot_map.begin(), slot_map.end(), pred);
    }

    constexpr bool empty() const                      
    {
        std::shared_lock sl{m};
        return slot_map.size() == 0; 
    }
    constexpr size_type size() const                  
    { 
        std::shared_lock sl{m};
        return slot_map.size(); 
    }

    constexpr void reserve(size_type n) 
    {
        std::lock_guard sl{m};
        return slot_map.reserve(n);
    }

    template<class C = Container<T>, class = decltype(std::declval<const C&>().capacity())>
    constexpr size_type capacity() const 
    {
        std::shared_lock sl{m};
        return slot_map.capacity();
    }
    
    constexpr void reserve_slots(size_type n) 
    {
        std::lock_guard lg{m};
        return slot_map.reserve_slots(n);
    }

    constexpr size_type slot_count() const 
    { 
        std::shared_lock sl{m};
        return slot_map.slot_count(); 
    }

    constexpr key_type insert(const mapped_type& value)   
    { 
        std::lock_guard lg{m};
        return slot_map.insert(value); 
    }

    constexpr key_type insert(mapped_type&& value)        
    { 
        std::lock_guard lg{m};
        return slot_map.insert(std::move(value)); 
    }

    template<class... Args> 
    constexpr key_type emplace(Args&&... args) 
    {
        std::lock_guard lg{m};
        return slot_map.emplace(std::forward<Args>(args)...); 
    }


    constexpr iterator erase(iterator pos) 
    { 
        return this->erase(const_iterator(pos)); 
    }
    
    constexpr iterator erase(iterator first, iterator last) 
    { 
        return this->erase(const_iterator(first), const_iterator(last)); 
    }

    constexpr iterator erase(const_iterator pos) 
    {
        std::lock_guard lg{m};
        return slot_map.erase(pos);
    }
    
    constexpr iterator erase(const_iterator first, const_iterator last) 
    {
        std::lock_guard lg{m};
        return slot_map.erase(first, last);        
    }

    constexpr size_type erase(const key_type& key) 
    {
        std::lock_guard lg{m};
        return slot_map.erase(key);        
    }

    constexpr void clear() 
    {
        std::lock_guard lg{m};
        slot_map.clear();
    }

    constexpr void swap(locked_slot_map& rhs) 
    {
        std::scoped_lock sl{m, rhs.m};
        slot_map.swap(rhs.slot_map);
    }

    constexpr iterator begin()                         { return slot_map.begin(); }
    constexpr iterator end()                           { return slot_map.end(); }
    constexpr const_iterator begin() const             { return slot_map.begin(); }
    constexpr const_iterator end() const               { return slot_map.end(); }
    constexpr const_iterator cbegin() const            { return slot_map.begin(); }
    constexpr const_iterator cend() const              { return slot_map.end(); }
    constexpr reverse_iterator rbegin()                { return slot_map.rbegin(); }
    constexpr reverse_iterator rend()                  { return slot_map.rend(); }
    constexpr const_reverse_iterator rbegin() const    { return slot_map.rbegin(); }
    constexpr const_reverse_iterator rend() const      { return slot_map.rend(); }
    constexpr const_reverse_iterator crbegin() const   { return slot_map.rbegin(); }
    constexpr const_reverse_iterator crend() const     { return slot_map.rend(); }
private:
    mutable std::shared_mutex m;
    stdext::slot_map<T, Key, Container> slot_map;
};

} // namespace gby
