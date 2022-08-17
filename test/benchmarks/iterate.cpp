
#include "locked_slot_map.h"
#include "optimized_locked_slot_map.h"
#include "dynamic_slot_map.h"

#include <benchmark/benchmark.h>

#include <string>
#include <vector>
#include <array>
#include <cmath>
#include <functional>
#include <unordered_set>

template<size_t Size> 
std::array<std::string, Size> genDataStr()
{
    std::array<std::string, Size> arr;
    for (int i=0; i<Size; ++i)
        
        arr[i] = "identifier_" + std::to_string(i);
    return arr;
}

template<size_t Size> 
std::array<int64_t, Size> genDataInt64()
{
    std::array<int64_t, Size> arr;
    for (int i=0; i<Size; ++i)
        arr[i] = 916713*pow(i,5);
    return arr;
}

template<typename Container, size_t Size, bool Str>
bool pushBackArr(Container& container_)
{
    if constexpr (Str == true)
    {
        std::array<std::string, Size> data = genDataStr<Size>();
        for (size_t i = 0; i < Size; ++i)
            container_.push_back(data[i]);
    }
    else
    {
        std::array<int64_t, Size> data = genDataInt64<Size>();        
        for (size_t i = 0; i < Size; ++i)
            container_.push_back(data[i]);
    }

    return true;
}

template<typename Container, size_t Size, bool Str>
bool insertMap(Container& container_)
{
    if constexpr (Str == true)
    {
        std::array<std::string, Size> data = genDataStr<Size>();
        for (size_t i = 0; i < Size; ++i)
            container_.insert(data[i]);
    }
    else
    {
        std::array<int64_t, Size> data = genDataInt64<Size>();        
        for (size_t i = 0; i < Size; ++i)
            container_.insert(data[i]);
    }

    return true;
}

template<typename T>
void pretendFnc(T element_)
{
    std::hash<T>{}(element_);
}

template<typename Container, size_t Size, typename Fnc>
bool iterateVectorIdx(Container& container_, Fnc fnc_)
{
    for (size_t i = 0; i < Size; ++i)
        fnc_(container_[i]);
    return true;
}

template<typename Container, typename Fnc>
bool iterateIterator(Container& container_, Fnc fnc_)
{
    for(auto& i : container_)
        fnc_(i);
    return true;
}

template<typename SlotMap, typename Fnc>
bool iterateMap(SlotMap& map_, Fnc fnc_)
{
    map_.iterate_map(fnc_);
    return true;
}


static void iterate_int64_1000_vector_idx(benchmark::State& state) 
{
    std::vector<int64_t> vec;
    vec.reserve(1000);
    pushBackArr<std::vector<int64_t>, 1000, false>(vec);

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateVectorIdx<std::vector<int64_t>, 1000, std::function<void (int64_t)>>(vec, pretendFnc<int64_t>));
    }
}
BENCHMARK(iterate_int64_1000_vector_idx);


static void iterate_int64_1000_vector_iterator(benchmark::State& state) 
{
    std::vector<int64_t> vec;
    vec.reserve(1000);
    pushBackArr<std::vector<int64_t>, 1000, false>(vec);

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateIterator<std::vector<int64_t>, std::function<void (int64_t)>>(vec, pretendFnc<int64_t>));
    }
}
BENCHMARK(iterate_int64_1000_vector_iterator);


static void iterate_int64_1000_unordered_set(benchmark::State& state) 
{
    std::unordered_set<int64_t> set;
    set.reserve(1000);
    insertMap<std::unordered_set<int64_t>, 1000, false>(set);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateIterator<std::unordered_set<int64_t>, std::function<void (int64_t)>>(set, pretendFnc<int64_t>));
    }
}
BENCHMARK(iterate_int64_1000_unordered_set);


static void iterate_int64_1000_sg14_slotMap(benchmark::State& state) 
{
    stdext::slot_map<int64_t> sg14SlotMap;
    sg14SlotMap.reserve(1000);
    insertMap<stdext::slot_map<int64_t>, 1000, false>(sg14SlotMap);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateIterator<stdext::slot_map<int64_t>, std::function<void (int64_t)>>(sg14SlotMap, pretendFnc<int64_t>));
    }
}
BENCHMARK(iterate_int64_1000_sg14_slotMap);


static void iterate_int64_1000_lockedSlotMap(benchmark::State& state) 
{
    gby::locked_slot_map<int64_t> slotMap;
    slotMap.reserve(1000);
    insertMap<gby::locked_slot_map<int64_t>, 1000, false>(slotMap);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateMap<gby::locked_slot_map<int64_t>, std::function<void (int64_t)>>(slotMap, pretendFnc<int64_t>));
    }
}
BENCHMARK(iterate_int64_1000_lockedSlotMap);


static void iterate_int64_1000_optimizedLockedSlotMap(benchmark::State& state) 
{
    gby::optimized_locked_slot_map<int64_t, 1000> optimizedSlotMap;
    insertMap<gby::optimized_locked_slot_map<int64_t, 1000>, 1000, false>(optimizedSlotMap);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateMap<gby::optimized_locked_slot_map<int64_t, 1000>, std::function<void (int64_t)>>(optimizedSlotMap, pretendFnc<int64_t>));
    }
}
BENCHMARK(iterate_int64_1000_optimizedLockedSlotMap);


static void iterate_int64_1000_dynamicSlotMap(benchmark::State& state) 
{
    gby::dynamic_slot_map<int64_t> dynamicSlotMap (1000);
    insertMap<gby::dynamic_slot_map<int64_t>, 1000, false>(dynamicSlotMap);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateMap<gby::dynamic_slot_map<int64_t>, std::function<void (int64_t)>>(dynamicSlotMap, pretendFnc<int64_t>));
    }
}
BENCHMARK(iterate_int64_1000_dynamicSlotMap);


static void iterate_string_1000_vector_idx(benchmark::State& state) 
{
    std::vector<std::string> vec;
    vec.reserve(1000);
    pushBackArr<std::vector<std::string>, 1000, true>(vec);

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateVectorIdx<std::vector<std::string>, 1000, std::function<void (std::string)>>(vec, pretendFnc<std::string>));
    }
}
BENCHMARK(iterate_string_1000_vector_idx);


static void iterate_string_1000_vector_iterator(benchmark::State& state) 
{
    std::vector<std::string> vec;
    vec.reserve(1000);
    pushBackArr<std::vector<std::string>, 1000, true>(vec);

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateIterator<std::vector<std::string>, std::function<void (std::string)>>(vec, pretendFnc<std::string>));
    }
}
BENCHMARK(iterate_string_1000_vector_iterator);


static void iterate_string_1000_unordered_set(benchmark::State& state) 
{
    std::unordered_set<std::string> set;
    set.reserve(1000);
    insertMap<std::unordered_set<std::string>, 1000, true>(set);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateIterator<std::unordered_set<std::string>, std::function<void (std::string)>>(set, pretendFnc<std::string>));
    }
}
BENCHMARK(iterate_string_1000_unordered_set);


static void iterate_string_1000_sg14_slotMap(benchmark::State& state) 
{
    stdext::slot_map<std::string> sg14SlotMap;
    sg14SlotMap.reserve(1000);
    insertMap<stdext::slot_map<std::string>, 1000, true>(sg14SlotMap);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateIterator<stdext::slot_map<std::string>, std::function<void (std::string)>>(sg14SlotMap, pretendFnc<std::string>));
    }
}
BENCHMARK(iterate_string_1000_sg14_slotMap);


static void iterate_string_1000_lockedSlotMap(benchmark::State& state) 
{
    gby::locked_slot_map<std::string> slotMap;
    slotMap.reserve(1000);
    insertMap<gby::locked_slot_map<std::string>, 1000, true>(slotMap);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateMap<gby::locked_slot_map<std::string>, std::function<void (std::string)>>(slotMap, pretendFnc<std::string>));
    }
}
BENCHMARK(iterate_string_1000_lockedSlotMap);


static void iterate_string_1000_optimizedLockedSlotMap(benchmark::State& state) 
{
    gby::optimized_locked_slot_map<std::string, 1000> optimizedSlotMap;
    insertMap<gby::optimized_locked_slot_map<std::string, 1000>, 1000, true>(optimizedSlotMap);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateMap<gby::optimized_locked_slot_map<std::string, 1000>, std::function<void (std::string)>>(optimizedSlotMap, pretendFnc<std::string>));
    }
}
BENCHMARK(iterate_string_1000_optimizedLockedSlotMap);


static void iterate_string_1000_dynamicSlotMap(benchmark::State& state) 
{
    gby::dynamic_slot_map<std::string> dynamicSlotMap (1000);
    insertMap<gby::dynamic_slot_map<std::string>, 1000, true>(dynamicSlotMap);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateMap<gby::dynamic_slot_map<std::string>, std::function<void (std::string)>>(dynamicSlotMap, pretendFnc<std::string>));
    }
}
BENCHMARK(iterate_string_1000_dynamicSlotMap);


static void iterate_int64_10000_vector_idx(benchmark::State& state) 
{
    std::vector<int64_t> vec;
    vec.reserve(10000);
    pushBackArr<std::vector<int64_t>, 10000, false>(vec);

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateVectorIdx<std::vector<int64_t>, 10000, std::function<void (int64_t)>>(vec, pretendFnc<int64_t>));
    }
}
BENCHMARK(iterate_int64_10000_vector_idx);


static void iterate_int64_10000_vector_iterator(benchmark::State& state) 
{
    std::vector<int64_t> vec;
    vec.reserve(10000);
    pushBackArr<std::vector<int64_t>, 10000, false>(vec);

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateIterator<std::vector<int64_t>, std::function<void (int64_t)>>(vec, pretendFnc<int64_t>));
    }
}
BENCHMARK(iterate_int64_10000_vector_iterator);


static void iterate_int64_10000_unordered_set(benchmark::State& state) 
{
    std::unordered_set<int64_t> set;
    set.reserve(10000);
    insertMap<std::unordered_set<int64_t>, 10000, false>(set);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateIterator<std::unordered_set<int64_t>, std::function<void (int64_t)>>(set, pretendFnc<int64_t>));
    }
}
BENCHMARK(iterate_int64_10000_unordered_set);


static void iterate_int64_10000_sg14_slotMap(benchmark::State& state) 
{
    stdext::slot_map<int64_t> sg14SlotMap;
    sg14SlotMap.reserve(10000);
    insertMap<stdext::slot_map<int64_t>, 10000, false>(sg14SlotMap);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateIterator<stdext::slot_map<int64_t>, std::function<void (int64_t)>>(sg14SlotMap, pretendFnc<int64_t>));
    }
}
BENCHMARK(iterate_int64_10000_sg14_slotMap);


static void iterate_int64_10000_lockedSlotMap(benchmark::State& state) 
{
    gby::locked_slot_map<int64_t> slotMap;
    slotMap.reserve(10000);
    insertMap<gby::locked_slot_map<int64_t>, 10000, false>(slotMap);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateMap<gby::locked_slot_map<int64_t>, std::function<void (int64_t)>>(slotMap, pretendFnc<int64_t>));
    }
}
BENCHMARK(iterate_int64_10000_lockedSlotMap);


static void iterate_int64_10000_optimizedLockedSlotMap(benchmark::State& state) 
{
    gby::optimized_locked_slot_map<int64_t, 10000> optimizedSlotMap;
    insertMap<gby::optimized_locked_slot_map<int64_t, 10000>, 10000, false>(optimizedSlotMap);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateMap<gby::optimized_locked_slot_map<int64_t, 10000>, std::function<void (int64_t)>>(optimizedSlotMap, pretendFnc<int64_t>));
    }
}
BENCHMARK(iterate_int64_10000_optimizedLockedSlotMap);


static void iterate_int64_10000_dynamicSlotMap(benchmark::State& state) 
{
    gby::dynamic_slot_map<int64_t> dynamicSlotMap (10000);
    insertMap<gby::dynamic_slot_map<int64_t>, 10000, false>(dynamicSlotMap);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateMap<gby::dynamic_slot_map<int64_t>, std::function<void (int64_t)>>(dynamicSlotMap, pretendFnc<int64_t>));
    }
}
BENCHMARK(iterate_int64_10000_dynamicSlotMap);


static void iterate_string_10000_vector_idx(benchmark::State& state) 
{
    std::vector<std::string> vec;
    vec.reserve(10000);
    pushBackArr<std::vector<std::string>, 10000, true>(vec);

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateVectorIdx<std::vector<std::string>, 10000, std::function<void (std::string)>>(vec, pretendFnc<std::string>));
    }
}
BENCHMARK(iterate_string_10000_vector_idx);


static void iterate_string_10000_vector_iterator(benchmark::State& state) 
{
    std::vector<std::string> vec;
    vec.reserve(10000);
    pushBackArr<std::vector<std::string>, 10000, true>(vec);

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateIterator<std::vector<std::string>, std::function<void (std::string)>>(vec, pretendFnc<std::string>));
    }
}
BENCHMARK(iterate_string_10000_vector_iterator);


static void iterate_string_10000_unordered_set(benchmark::State& state) 
{
    std::unordered_set<std::string> set;
    set.reserve(10000);
    insertMap<std::unordered_set<std::string>, 10000, true>(set);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateIterator<std::unordered_set<std::string>, std::function<void (std::string)>>(set, pretendFnc<std::string>));
    }
}
BENCHMARK(iterate_string_10000_unordered_set);


static void iterate_string_10000_sg14_slotMap(benchmark::State& state) 
{
    stdext::slot_map<std::string> sg14SlotMap;
    sg14SlotMap.reserve(10000);
    insertMap<stdext::slot_map<std::string>, 10000, true>(sg14SlotMap);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateIterator<stdext::slot_map<std::string>, std::function<void (std::string)>>(sg14SlotMap, pretendFnc<std::string>));
    }
}
BENCHMARK(iterate_string_10000_sg14_slotMap);


static void iterate_string_10000_lockedSlotMap(benchmark::State& state) 
{
    gby::locked_slot_map<std::string> slotMap;
    slotMap.reserve(10000);
    insertMap<gby::locked_slot_map<std::string>, 10000, true>(slotMap);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateMap<gby::locked_slot_map<std::string>, std::function<void (std::string)>>(slotMap, pretendFnc<std::string>));
    }
}
BENCHMARK(iterate_string_10000_lockedSlotMap);


static void iterate_string_10000_optimizedLockedSlotMap(benchmark::State& state) 
{
    gby::optimized_locked_slot_map<std::string, 10000> optimizedSlotMap;
    insertMap<gby::optimized_locked_slot_map<std::string, 10000>, 10000, true>(optimizedSlotMap);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateMap<gby::optimized_locked_slot_map<std::string, 10000>, std::function<void (std::string)>>(optimizedSlotMap, pretendFnc<std::string>));
    }
}
BENCHMARK(iterate_string_10000_optimizedLockedSlotMap);


static void iterate_string_10000_dynamicSlotMap(benchmark::State& state) 
{
    gby::dynamic_slot_map<std::string> dynamicSlotMap (10000);
    insertMap<gby::dynamic_slot_map<std::string>, 10000, true>(dynamicSlotMap);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterateMap<gby::dynamic_slot_map<std::string>, std::function<void (std::string)>>(dynamicSlotMap, pretendFnc<std::string>));
    }
}
BENCHMARK(iterate_string_10000_dynamicSlotMap);
