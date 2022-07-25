
#pragma once

#include <atomic>

namespace gby
{

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

template<typename T> 
auto constexpr is_atomic = false;

template<typename T> 
auto constexpr is_atomic<std::atomic<T>> = true;

template<> 
auto constexpr is_atomic<std::atomic_flag> = true;

template<typename T>
auto constexpr is_pair_atomic = false; 

template<typename T, typename U>
auto constexpr is_pair_atomic<std::pair<T, U>> = is_atomic<T> || is_atomic<U>; 

template<typename T>
auto constexpr is_pair_first_atomic = false;

template<typename T, typename U>
auto constexpr is_pair_first_atomic<std::pair<T, U>> = is_atomic<T>;

template<typename T>
auto constexpr is_pair_second_atomic = false;

template<typename T, typename U>
auto constexpr is_pair_second_atomic<std::pair<T, U>> = is_atomic<U>;

template<typename T>
auto constexpr is_not_atomic = true;

template<typename T>
auto constexpr is_not_atomic<std::atomic<T>> = false;

} // namespace gby