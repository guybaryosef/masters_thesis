

#include "../RegressionTestHelpers.h"
#include "dynamic_slot_map.h"

#include <gtest/gtest.h>

#include <string>
#include <deque>

constexpr size_t iterationCount {50000};

   
TEST(DynamicSlotMap, IntElement)
{
    gby::dynamic_slot_map<int> map;
    map.reserve(iterationCount);
    test_SPMC<iterationCount, 3>(map, []() { return rand();}, false);

    gby::dynamic_slot_map<int> map2;
    map2.reserve(iterationCount);
    test_SPMC<iterationCount, 3>(map2, []() { return rand();}, true);
}

TEST(DynamicSlotMap, StringElement)
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

    gby::dynamic_slot_map<std::string> map;
    map.reserve(iterationCount);
    test_SPMC<strCount, 3>(map, [&strInput]() { return strInput[rand()%strCount];}, false);

    gby::dynamic_slot_map<std::string> map2;
    map2.reserve(iterationCount);
    test_SPMC<strCount, 3>(map2, [&strInput]() { return strInput[rand()%strCount];}, true);
}

TEST(DynamicSlotMap, TestObjElement)
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

    gby::dynamic_slot_map<TestObj, std::pair<int32_t, uint64_t>> map;
    map.reserve(iterationCount);
    test_SPMC<testObjCount, 3>(map, [&testObjInput]() { return testObjInput[rand()%testObjCount];}, false);

    gby::dynamic_slot_map<TestObj, std::pair<int32_t, uint64_t>> map2;
    map2.reserve(iterationCount);
    test_SPMC<testObjCount, 3>(map2, [&testObjInput]() { return testObjInput[rand()%testObjCount];}, true);
}

/////////// MCMP ///////////
constexpr size_t WriterCount {2};
constexpr size_t MCMP_writesPerWriter {iterationCount/WriterCount};

TEST(DynamicSlotMap, MCMPIntElement)
{
    gby::dynamic_slot_map<int> map;
    map.reserve(iterationCount);
    test_MPMC<WriterCount, MCMP_writesPerWriter, 2, 3>(map, [] { return rand();});
}

TEST(DynamicSlotMap, MCMPStringElement)
{
    constexpr size_t strCount {25};
    auto strInput = genStrInput<strCount>();

    gby::dynamic_slot_map<std::string> map;
    map.reserve(iterationCount);
    test_MPMC<WriterCount, MCMP_writesPerWriter, 1, 5>(map, [&strInput] { return strInput[rand()%strCount];});
}

TEST(DynamicSlotMap, MCMPTestObjElement)
{
    constexpr size_t testObjCount {25};
    auto testObjInput = genTestObj<testObjCount>();

    gby::dynamic_slot_map<TestObj, std::pair<int32_t, uint64_t>> map;
    map.reserve(iterationCount);
    test_MPMC<WriterCount, MCMP_writesPerWriter, 1, 3>(map, [&testObjInput] { return testObjInput[rand()%testObjCount];});
}
