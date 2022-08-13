
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
#include <iostream>

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

template<typename SlotMap, typename KeyVec, size_t Size, bool Str>
KeyVec insertSlotMap(SlotMap& map_)
{
    KeyVec keyVec;
    keyVec.reserve(Size);

    if constexpr (Str == true)
    {
        std::array<std::string, Size> data = genDataStr<Size>();
        for (size_t i = 0; i < Size; ++i)
            keyVec.push_back(map_.insert(data[i]));
    }
    else
    {
        std::array<int64_t, Size> data = genDataInt64<Size>();        
        for (size_t i = 0; i < Size; ++i)
            keyVec.push_back(map_.insert(data[i]));
    }

    return keyVec;
}

template<typename Container>
bool eraseIterators(Container& container_)
{
    for (auto it = container_.begin(); it != container_.end(); )
        it = container_.erase(it);
    return true;
}

template<typename SlotMap, typename KeyVector>
bool eraseSlotMap(SlotMap& map_, const KeyVector& keyVec_)
{
    for (int i=0; i<keyVec_.size(); ++i)
    {
        map_.erase(keyVec_[i]);
    }
    return true;
}


static void erase_int64_1000_vector(benchmark::State& state) 
{
    for (auto _ : state)
    {
        std::vector<int64_t> vec;
        vec.reserve(1000);
        pushBackArr<std::vector<int64_t>, 1000, false>(vec);
        benchmark::DoNotOptimize(eraseIterators(vec));
    }
}
BENCHMARK(erase_int64_1000_vector);


static void erase_int64_1000_unordered_set(benchmark::State& state) 
{
    for (auto _ : state)
    {
        std::unordered_set<int64_t> set;
        set.reserve(1000);
        insertMap<std::unordered_set<int64_t>, 1000, false>(set);
        benchmark::DoNotOptimize(eraseIterators(set));
    }
}
BENCHMARK(erase_int64_1000_unordered_set);


static void erase_int64_1000_sg14_slotMap(benchmark::State& state) 
{
    for (auto _ : state)
    {
        stdext::slot_map<int64_t> sg14SlotMap;
        sg14SlotMap.reserve(1000);
        insertMap<stdext::slot_map<int64_t>, 1000, false>(sg14SlotMap);
        benchmark::DoNotOptimize(eraseIterators(sg14SlotMap));
    }
}
BENCHMARK(erase_int64_1000_sg14_slotMap);


static void erase_int64_1000_lockedSlotMap_insertOnly(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::locked_slot_map<int64_t> slotMap;
        slotMap.reserve(1000);
        benchmark::DoNotOptimize(insertSlotMap<gby::locked_slot_map<int64_t>, std::vector<gby::locked_slot_map<int64_t>::key_type>, 1000, false>(slotMap));
    }
}
BENCHMARK(erase_int64_1000_lockedSlotMap_insertOnly);


static void erase_int64_1000_lockedSlotMap_insertAndErase(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::locked_slot_map<int64_t> slotMap;
        slotMap.reserve(1000);
        auto vec = insertSlotMap<gby::locked_slot_map<int64_t>, std::vector<gby::locked_slot_map<int64_t>::key_type>, 1000, false>(slotMap);
        benchmark::DoNotOptimize(eraseSlotMap(slotMap, vec));
    }
}
BENCHMARK(erase_int64_1000_lockedSlotMap_insertAndErase);


static void erase_int64_1000_optimizedLockedSlotMap_insertOnly(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::optimized_locked_slot_map<int64_t, 1000> optimizedSlotMap;
        benchmark::DoNotOptimize(insertSlotMap<gby::optimized_locked_slot_map<int64_t, 1000>, std::vector<gby::optimized_locked_slot_map<int64_t, 1000>::key_type>, 1000, false>(optimizedSlotMap));
    }
}
BENCHMARK(erase_int64_1000_optimizedLockedSlotMap_insertOnly);


static void erase_int64_1000_optimizedLockedSlotMap_insertAndErase(benchmark::State& state) 
{

    std::vector<gby::optimized_locked_slot_map<int64_t, 1000>::key_type> vec;
    for (auto _ : state)
    {
        gby::optimized_locked_slot_map<int64_t, 1000> optimizedSlotMap;
        auto vec = insertSlotMap<gby::optimized_locked_slot_map<int64_t, 1000>, std::vector<gby::optimized_locked_slot_map<int64_t, 1000>::key_type>, 1000, false>(optimizedSlotMap);
        benchmark::DoNotOptimize(eraseSlotMap(optimizedSlotMap, vec));
    }
}
BENCHMARK(erase_int64_1000_optimizedLockedSlotMap_insertAndErase);


static void erase_int64_1000_dynamicSlotMap_insertOnly(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::dynamic_slot_map<int64_t> dynamicSlotMap(1000);
        benchmark::DoNotOptimize(insertSlotMap<gby::dynamic_slot_map<int64_t>, std::vector<gby::dynamic_slot_map<int64_t>::key_type>, 1000, false>(dynamicSlotMap));
    }
}
BENCHMARK(erase_int64_1000_dynamicSlotMap_insertOnly);


static void erase_string_1000_dynamicSlotMap_insertAndErase(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::dynamic_slot_map<int64_t> dynamicSlotMap(1000);
        auto vec = insertSlotMap<gby::dynamic_slot_map<int64_t>, std::vector<gby::dynamic_slot_map<int64_t>::key_type>, 1000, false>(dynamicSlotMap);
        eraseSlotMap(dynamicSlotMap, vec);
    }
}
BENCHMARK(erase_string_1000_dynamicSlotMap_insertAndErase);

static void erase_string_1000_vector_idx(benchmark::State& state) 
{

    for (auto _ : state)
    {
        std::vector<std::string> vec;
        vec.reserve(1000);
        pushBackArr<std::vector<std::string>, 1000, true>(vec);
        benchmark::DoNotOptimize(eraseIterators(vec));
    }
}
BENCHMARK(erase_string_1000_vector_idx);


static void erase_string_1000_unordered_set(benchmark::State& state) 
{
    for (auto _ : state)
    {
        std::unordered_set<std::string> set;
        set.reserve(1000);
        insertMap<std::unordered_set<std::string>, 1000, true>(set);
        benchmark::DoNotOptimize(eraseIterators(set));
    }
}
BENCHMARK(erase_string_1000_unordered_set);


static void erase_string_1000_sg14_slotMap(benchmark::State& state) 
{
    for (auto _ : state)
    {
        stdext::slot_map<std::string> sg14SlotMap;
        sg14SlotMap.reserve(1000);
        insertMap<stdext::slot_map<std::string>, 1000, true>(sg14SlotMap);
        benchmark::DoNotOptimize(eraseIterators(sg14SlotMap));
    }
}
BENCHMARK(erase_string_1000_sg14_slotMap);

static void erase_string_1000_lockedSlotMap_onlyInsert(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::locked_slot_map<std::string> slotMap;
        slotMap.reserve(1000);
        benchmark::DoNotOptimize(insertSlotMap<gby::locked_slot_map<std::string>, std::vector<gby::locked_slot_map<std::string>::key_type>, 1000, true>(slotMap));
    }
}
BENCHMARK(erase_string_1000_lockedSlotMap_onlyInsert);


static void erase_string_1000_lockedSlotMap_insertAndErase(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::locked_slot_map<std::string> slotMap;
        slotMap.reserve(1000);
        auto vec = insertSlotMap<gby::locked_slot_map<std::string>, std::vector<gby::locked_slot_map<std::string>::key_type>, 1000, true>(slotMap);
        benchmark::DoNotOptimize(eraseSlotMap(slotMap, vec));
    }
}
BENCHMARK(erase_string_1000_lockedSlotMap_insertAndErase);


static void erase_string_1000_optimizedLockedSlotMap_insertOnly(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::optimized_locked_slot_map<std::string, 1000> optimizedSlotMap;
        benchmark::DoNotOptimize(insertSlotMap<gby::optimized_locked_slot_map<std::string, 1000>, std::vector<gby::optimized_locked_slot_map<std::string, 1000>::key_type>, 1000, true>(optimizedSlotMap));
    }
}
BENCHMARK(erase_string_1000_optimizedLockedSlotMap_insertOnly);


static void erase_string_1000_optimizedLockedSlotMap_insertAndErase(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::optimized_locked_slot_map<std::string, 1000> optimizedSlotMap;
        auto vec = insertSlotMap<gby::optimized_locked_slot_map<std::string, 1000>, std::vector<gby::optimized_locked_slot_map<std::string, 1000>::key_type>, 1000, true>(optimizedSlotMap);
        eraseSlotMap(optimizedSlotMap, vec);
    }
}
BENCHMARK(erase_string_1000_optimizedLockedSlotMap_insertAndErase);


static void erase_string_1000_dynamicSlotMap_insertOnly(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::dynamic_slot_map<std::string> dynamicSlotMap(1000);
        benchmark::DoNotOptimize(insertSlotMap<gby::dynamic_slot_map<std::string>, std::vector<gby::dynamic_slot_map<std::string>::key_type>, 1000, true>(dynamicSlotMap));
    }
}
BENCHMARK(erase_string_1000_dynamicSlotMap_insertOnly);


static void erase_string_1000_dynamicSlotMap_insertAndErase(benchmark::State& state) 
{
    for (auto _ : state)
    {
        gby::dynamic_slot_map<std::string> dynamicSlotMap(1000);
        auto vec = insertSlotMap<gby::dynamic_slot_map<std::string>, std::vector<gby::dynamic_slot_map<std::string>::key_type>, 1000, true>(dynamicSlotMap);
        eraseSlotMap(dynamicSlotMap, vec);
    }
}
BENCHMARK(erase_string_1000_dynamicSlotMap_insertAndErase);

