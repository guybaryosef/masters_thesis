

#include "../RegressionTestHelpers.h"
#include "optimized_locked_slot_map.h"

#include <gtest/gtest.h>

#include <string>
#include <deque>

constexpr size_t iterationCount {50000};
    
TEST(OptimizedLockedSlotMap, IntElement)
{
    gby::optimized_locked_slot_map<int, 1000000> map;
    test_SPMC<iterationCount, 3>(map, []() { return rand();}, false);

    gby::optimized_locked_slot_map<int, 1000000> map2;
    test_SPMC<iterationCount, 3>(map2, []() { return rand();}, true);
}

TEST(OptimizedLockedSlotMap, StringElement)
{
    constexpr size_t strCount {25};
    std::array<std::string, strCount> strInput {};

    auto genStr = []
    {
        auto len {rand()%maxStrLen};

        std::string s{};
        s.reserve(len);
        for (int i = 0; i < len; ++i)
            s += alphaNum[rand() % sizeof(alphaNum)];
        return s;
    };
    std::generate_n(strInput.begin(), strCount, genStr);

    gby::optimized_locked_slot_map<std::string, 1000000> map;
    test_SPMC<strCount, 3>(map, [&strInput]() { return strInput[rand()%strCount];}, false);

    gby::optimized_locked_slot_map<std::string, 1000000> map2;
    test_SPMC<strCount, 3>(map2, [&strInput]() { return strInput[rand()%strCount];}, true);
}

TEST(OptimizedLockedSlotMap, TestObjElement)
{
    constexpr size_t testObjCount {25};
    std::array<TestObj, testObjCount> testObjInput {};

    auto genTestObj = []
     {
         auto len {rand()%maxStrLen};

         std::string s{};
         s.reserve(len);
         for (int i = 0; i < len; ++i)
             s += alphaNum[rand() % sizeof(alphaNum)];

         return TestObj{rand(), alphaNum[rand() % sizeof(alphaNum)], s};
     };
    std::generate_n(testObjInput.begin(), testObjCount, genTestObj);

    gby::optimized_locked_slot_map<TestObj, 1000000, std::pair<int32_t, uint64_t>> map;
    test_SPMC<testObjCount, 3>(map, [&testObjInput]() { return testObjInput[rand()%testObjCount];}, false);

    gby::optimized_locked_slot_map<TestObj, 1000000, std::pair<int32_t, uint64_t>> map2;
    test_SPMC<testObjCount, 3>(map2, [&testObjInput]() { return testObjInput[rand()%testObjCount];}, true);
}

/////////// MCMP ///////////
constexpr size_t WriterCount {2};
constexpr size_t MCMP_writesPerWriter {iterationCount/WriterCount};

TEST(OptimizedLockedSlotMap, MCMPIntElement)
{
    gby::optimized_locked_slot_map<int, 1000000> map;
    test_MPMC<WriterCount, MCMP_writesPerWriter, 2, 3>(map, [] { return rand();});
}

 TEST(OptimizedLockedSlotMap, MCMPStringElement)
 {
     constexpr size_t strCount {25};
     auto strInput = genStrInput<strCount>();

     gby::optimized_locked_slot_map<std::string, 1000000> map;
     test_MPMC<WriterCount, MCMP_writesPerWriter, 1, 5>(map, [&strInput] { return strInput[rand()%strCount];});
 }

 TEST(OptimizedLockedSlotMap, MCMPTestObjElement)
 {
     constexpr size_t testObjCount {25};
     auto testObjInput = genTestObj<testObjCount>();

     gby::optimized_locked_slot_map<TestObj, 1000000, std::pair<int32_t, uint64_t>> map;
     test_MPMC<WriterCount, MCMP_writesPerWriter, 0, 3>(map, [&testObjInput] { return testObjInput[rand()%testObjCount];});
 }