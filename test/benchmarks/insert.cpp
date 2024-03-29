
#include "locked_slot_map.h"
#include "optimized_locked_slot_map.h"
#include "dynamic_slot_map.h"

#include <benchmark/benchmark.h>

#include <string>
#include <vector>
#include <array>
#include <cmath>
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


static void insert_int64_1000_vector_push_back(benchmark::State& state) 
{
    for (auto _ : state)
    {
        std::vector<int64_t> vec;
        vec.reserve(1000);
        benchmark::DoNotOptimize(pushBackArr<std::vector<int64_t>, 1000, false>(vec));
    }
}
BENCHMARK(insert_int64_1000_vector_push_back);


static void insert_int64_1000_unordered_set(benchmark::State& state) 
{
    for (auto _ : state)
    {
        std::unordered_set<int64_t> set;
        set.reserve(1000);
        benchmark::DoNotOptimize(insertMap<std::unordered_set<int64_t>, 1000, false>(set));
    }
}
BENCHMARK(insert_int64_1000_unordered_set);


static void insert_int64_1000_sg14_slotMap(benchmark::State& state) 
{
    for (auto _ : state)
    {
        stdext::slot_map<int64_t> sg14SlotMap;
        sg14SlotMap.reserve(1000);
        benchmark::DoNotOptimize(insertMap<stdext::slot_map<int64_t>, 1000, false>(sg14SlotMap));
    }
}
BENCHMARK(insert_int64_1000_sg14_slotMap);


static void insert_int64_1000_lockedSlotMap(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::locked_slot_map<int64_t> slotMap;
        slotMap.reserve(1000);
        benchmark::DoNotOptimize(insertMap<gby::locked_slot_map<int64_t>, 1000, false>(slotMap));
    }
}
BENCHMARK(insert_int64_1000_lockedSlotMap);


static void insert_int64_1000_optimizedLockedSlotMap(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::optimized_locked_slot_map<int64_t, 1000> optimizedSlotMap;
        benchmark::DoNotOptimize(insertMap<gby::optimized_locked_slot_map<int64_t, 1000>, 1000, false>(optimizedSlotMap));
    }
}
BENCHMARK(insert_int64_1000_optimizedLockedSlotMap);


static void insert_int64_1000_dynamicSlotMap(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::dynamic_slot_map<int64_t> dynamicSlotMap(1000);
        benchmark::DoNotOptimize(insertMap<gby::dynamic_slot_map<int64_t>, 1000, false>(dynamicSlotMap));
    }
}
BENCHMARK(insert_int64_1000_dynamicSlotMap);


static void insert_string_1000_vector_push_back(benchmark::State& state) 
{
    for (auto _ : state)
    {
        std::vector<std::string> vec;
        vec.reserve(1000);
        benchmark::DoNotOptimize(pushBackArr<std::vector<std::string>, 1000, true>(vec));
    }
}
BENCHMARK(insert_string_1000_vector_push_back);


static void insert_string_1000_unordered_set(benchmark::State& state) 
{
    for (auto _ : state)
    {
        std::unordered_set<std::string> set;
        set.reserve(1000);
        benchmark::DoNotOptimize(insertMap<std::unordered_set<std::string>, 1000, true>(set));
    }
}
BENCHMARK(insert_string_1000_unordered_set);


static void insert_string_1000_sg14SlotMap(benchmark::State& state) 
{
    for (auto _ : state)
    {
        stdext::slot_map<std::string> sg14SlotMap;
        sg14SlotMap.reserve(1000);
        benchmark::DoNotOptimize(insertMap<stdext::slot_map<std::string>, 1000, true>(sg14SlotMap));
    }
}
BENCHMARK(insert_string_1000_sg14SlotMap);


static void insert_string_1000_lockedSlotMap(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::locked_slot_map<std::string> slotMap;
        slotMap.reserve(1000);
        benchmark::DoNotOptimize(insertMap<gby::locked_slot_map<std::string>, 1000, true>(slotMap));
    }
}
BENCHMARK(insert_string_1000_lockedSlotMap);


static void insert_string_1000_optimizedLockedSlotMap(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::optimized_locked_slot_map<std::string, 1000> optimizedSlotMap;
        benchmark::DoNotOptimize(insertMap<gby::optimized_locked_slot_map<std::string, 1000>, 1000, true>(optimizedSlotMap));
    }
}
BENCHMARK(insert_string_1000_optimizedLockedSlotMap);


static void insert_string_1000_dynamicSlotMap(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::dynamic_slot_map<std::string> dynamicSlotMap(1000);
        benchmark::DoNotOptimize(insertMap<gby::dynamic_slot_map<std::string>, 1000, true>(dynamicSlotMap));
    }
}
BENCHMARK(insert_string_1000_dynamicSlotMap);


static void insert_int64_10000_vector_push_back(benchmark::State& state) 
{
    for (auto _ : state)
    {
        std::vector<int64_t> vec;
        vec.reserve(10000);
        benchmark::DoNotOptimize(pushBackArr<std::vector<int64_t>, 10000, false>(vec));
    }
}
BENCHMARK(insert_int64_10000_vector_push_back);


static void insert_int64_10000_unordered_set(benchmark::State& state) 
{
    for (auto _ : state)
    {
        std::unordered_set<int64_t> set;
        set.reserve(10000);
        benchmark::DoNotOptimize(insertMap<std::unordered_set<int64_t>, 10000, false>(set));
    }
}
BENCHMARK(insert_int64_10000_unordered_set);


static void insert_int64_10000_sg14_slotMap(benchmark::State& state) 
{
    for (auto _ : state)
    {
        stdext::slot_map<int64_t> sg14SlotMap;
        sg14SlotMap.reserve(10000);
        benchmark::DoNotOptimize(insertMap<stdext::slot_map<int64_t>, 10000, false>(sg14SlotMap));
    }
}
BENCHMARK(insert_int64_10000_sg14_slotMap);


static void insert_int64_10000_lockedSlotMap(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::locked_slot_map<int64_t> slotMap;
        slotMap.reserve(10000);
        benchmark::DoNotOptimize(insertMap<gby::locked_slot_map<int64_t>, 10000, false>(slotMap));
    }
}
BENCHMARK(insert_int64_10000_lockedSlotMap);


static void insert_int64_10000_optimizedLockedSlotMap(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::optimized_locked_slot_map<int64_t, 10000> optimizedSlotMap;
        benchmark::DoNotOptimize(insertMap<gby::optimized_locked_slot_map<int64_t, 10000>, 10000, false>(optimizedSlotMap));
    }
}
BENCHMARK(insert_int64_10000_optimizedLockedSlotMap);


static void insert_int64_10000_dynamicSlotMap(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::dynamic_slot_map<int64_t> dynamicSlotMap(10000);
        benchmark::DoNotOptimize(insertMap<gby::dynamic_slot_map<int64_t>, 10000, false>(dynamicSlotMap));
    }
}
BENCHMARK(insert_int64_10000_dynamicSlotMap);


static void insert_string_10000_vector_push_back(benchmark::State& state) 
{
    for (auto _ : state)
    {
        std::vector<std::string> vec;
        vec.reserve(10000);
        benchmark::DoNotOptimize(pushBackArr<std::vector<std::string>, 10000, true>(vec));
    }
}
BENCHMARK(insert_string_10000_vector_push_back);


static void insert_string_10000_unordered_set(benchmark::State& state) 
{
    for (auto _ : state)
    {
        std::unordered_set<std::string> set;
        set.reserve(10000);
        benchmark::DoNotOptimize(insertMap<std::unordered_set<std::string>, 10000, true>(set));
    }
}
BENCHMARK(insert_string_10000_unordered_set);


static void insert_string_10000_sg14SlotMap(benchmark::State& state) 
{
    for (auto _ : state)
    {
        stdext::slot_map<std::string> sg14SlotMap;
        sg14SlotMap.reserve(10000);
        benchmark::DoNotOptimize(insertMap<stdext::slot_map<std::string>, 10000, true>(sg14SlotMap));
    }
}
BENCHMARK(insert_string_10000_sg14SlotMap);


static void insert_string_10000_lockedSlotMap(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::locked_slot_map<std::string> slotMap;
        slotMap.reserve(10000);
        benchmark::DoNotOptimize(insertMap<gby::locked_slot_map<std::string>, 10000, true>(slotMap));
    }
}
BENCHMARK(insert_string_10000_lockedSlotMap);


static void insert_string_10000_optimizedLockedSlotMap(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::optimized_locked_slot_map<std::string, 10000> optimizedSlotMap;
        benchmark::DoNotOptimize(insertMap<gby::optimized_locked_slot_map<std::string, 10000>, 10000, true>(optimizedSlotMap));
    }
}
BENCHMARK(insert_string_10000_optimizedLockedSlotMap);


static void insert_string_10000_dynamicSlotMap(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::dynamic_slot_map<std::string> dynamicSlotMap(10000);
        benchmark::DoNotOptimize(insertMap<gby::dynamic_slot_map<std::string>, 10000, true>(dynamicSlotMap));
    }
}
BENCHMARK(insert_string_10000_dynamicSlotMap);
