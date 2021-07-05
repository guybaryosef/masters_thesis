/*
 * locked_slot_map.h - A lock-free impelemntation of a slot_map.
 */

#pragma once


#include <utility>
#include <vector>

namespace gby
{

template<
    typename T,
    typename Key = std::pair<unsigned, unsigned>,
    template<typename...> typename Container = std::vector
>
class lock_free_slot_map
{

public:
    lock_free_slot_map() = default;

    constexpr Key insert(const T& value)
    {
        //insert to back of 
    }


private:
    Container<Key, Args...> _slots;
    Container<T, Args...> _values;

};

/*
current idea:
have as a wrapper for each item in map:
template <typename T>
class wrapperT
{
    T data;
    bool isActive;
    int referenceCount;
    int indexOfSlotVec;
}
*/

/*
expand

*/

/*
insert
1. check capacity of slots container- perhaps expand.
2. check capacity of values container- perhaps expand.
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
1. return an atomic_shared_ptr, so that the memory can't be destructed.
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
