
#include <utility>
#include <array>
#include <deque>

namespace gby
{

template<
    typename T,
    size_t Size,
    typename Key = std::pair<unsigned, unsigned>,
    typename Container = std::array<T, Size> 
>
class lock_free_const_sized_slot_map
{
public:
    static constexpr auto get_index(const Key& k) { const auto& [idx, gen] = k; return idx; }
    static constexpr auto get_generation(const Key& k) { const auto& [idx, gen] = k; return gen; }
    template<class Integral> static constexpr void set_index(Key& k, Integral value) { auto& [idx, gen] = k; idx = static_cast<key_index_type>(value); }
    static constexpr void increment_generation(Key& k) { auto& [idx, gen] = k; ++gen; }

    using container_type = Container;
    using size_type      = typename container_type::size_type;
    using key_type       = Key;
    using key_index_type = decltype(lock_free_const_sized_slot_map::get_index(std::declval<Key>()));
    using key_generation_type = decltype(lock_free_const_sized_slot_map::get_generation(std::declval<Key>()));


    constexpr key_type insert(const T& value)   { return this->emplace(value); }
    constexpr key_type insert(T&& value)        { return this->emplace(std::move(value)); }

    template<class... Args> constexpr key_type emplace(Args&&... args) 
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
    }

    constexpr size_type erase(const key_type& key) 
    {
        /*
            increment generator (can't be random-accessed anymore)
            if iterating
                add to erase queue
            else
                lock iterating-remove lock

                get values size
                copy last values element into old value spot
                switch slot for last element to point to old value spot
                CAS on old values size and current value size- try to decrement by 1
                if failed, return slot for last element to original 'last value' (but clearly not the last-last value)

        */
    }


    template <class P>
    constexpr void iterate_map(P pred) 
    {
        // get iteration-remove lock
        // get length of values array (not the slots array).
        // iterate through that length of elements
        // once done, check if size increased and continue iterating. repeat while size increases.
        // release iteration-remove lock
        // remove elements in erase queue
        
        std::lock_guard m {_iterationLock};

        
    }

    constexpr size_t size()
    {
        return _values.size();
    }

private:
    container_type _slots;
    container_type _values;

    std::mutex _iterationLock;
    container_type _eraseQueue;
};


} // namespace gby
