

#include "RegressionTestHelpers.h"
#include "locked_slot_map.h"

#include <gtest/gtest.h>

#include <string>
#include <deque>

constexpr size_t iterationCount {100000};
constexpr size_t maxStrLen {50};

char alphanum[] = "0123456789"
                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                  "abcdefghijklmnopqrstuvwxyz";

    
TEST(LockedSlotMapRegression, IntElement)
{
    gby::locked_slot_map<int> map;
    test_SCMP<iterationCount, 3>(map, []() { return rand();}, false);

    gby::locked_slot_map<int> map2;
    test_SCMP<iterationCount, 3>(map2, []() { return rand();}, true);
}

TEST(LockedSlotMapRegression, StringElement)
{
    constexpr size_t strCount {iterationCount};
    std::array<std::string, strCount> strInput {};

    auto genStr = []
        {
            auto len {rand()%maxStrLen};

            std::string s{};
            s.reserve(len);
            for (int i = 0; i < len; ++i)
                s += alphanum[rand() % sizeof(alphanum)];
            return s;
        };
    std::generate_n(strInput.begin(), strCount, genStr);

    gby::locked_slot_map<std::string> map;
    test_SCMP<strCount, 3>(map, [&strInput]() { return strInput[rand()%strCount];}, false);

    gby::locked_slot_map<std::string> map2;
    test_SCMP<strCount, 3>(map2, [&strInput]() { return strInput[rand()%strCount];}, true);
}

TEST(LockedSlotMapRegression, TestObjElement)
{
    constexpr size_t testObjCount {iterationCount};
    std::array<TestObj, testObjCount> testObjInput {};

    auto genTestObj = []
        {
            auto len {rand()%maxStrLen};

            std::string s{};
            s.reserve(len);
            for (int i = 0; i < len; ++i)
                s += alphanum[rand() % sizeof(alphanum)];
            
            return TestObj{rand(), alphanum[rand() % sizeof(alphanum)], s};
        };    
    std::generate_n(testObjInput.begin(), testObjCount, genTestObj);

    gby::locked_slot_map<TestObj, std::pair<int32_t, uint64_t>> map;
    test_SCMP<testObjCount, 3>(map, [&testObjInput]() { return testObjInput[rand()%testObjCount];}, false);

    gby::locked_slot_map<TestObj, std::pair<int32_t, uint64_t>> map2;
    test_SCMP<testObjCount, 3>(map2, [&testObjInput]() { return testObjInput[rand()%testObjCount];}, true);
}