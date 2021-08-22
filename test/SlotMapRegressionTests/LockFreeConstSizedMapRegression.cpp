

#include "RegressionTestHelpers.h"
#include "lock_free_const_sized_slot_map.h"

#include <gtest/gtest.h>

#include <string>
#include <deque>

constexpr size_t iterationCount {100000};
constexpr size_t maxStrLen {50};

char alphaNum2[] = "0123456789"
                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                  "abcdefghijklmnopqrstuvwxyz";


TEST(LockFreeConstSizedSlotMapRegression, IntElement)
{
    gby::lock_free_const_sized_slot_map<int, iterationCount> map;
    test_SCMP<iterationCount, 3>(map, []() { return rand();}, false);

    gby::lock_free_const_sized_slot_map<int, iterationCount> map2;
    test_SCMP<iterationCount, 3>(map2, []() { return rand();}, true);
}

TEST(LockFreeConstSizedSlotMapRegression, StringElement)
{
    constexpr size_t strCount {iterationCount};
    std::array<std::string, strCount> strInput {};

    auto genStr = []
        {
            auto len {rand()%maxStrLen};

            std::string s{};
            s.reserve(len);
            for (int i = 0; i < len; ++i)
                s += alphaNum2[rand() % sizeof(alphaNum2)];
            return s;
        };
    std::generate_n(strInput.begin(), strCount, genStr);

    gby::lock_free_const_sized_slot_map<std::string, iterationCount> map;
    test_SCMP<strCount, 3>(map, [&strInput]() { return strInput[rand()%strCount];}, false);

    gby::lock_free_const_sized_slot_map<std::string, iterationCount> map2;
    test_SCMP<strCount, 3>(map2, [&strInput]() { return strInput[rand()%strCount];}, true);
}

TEST(LockFreeConstSizedSlotMapRegression, TestObjElement)
{
    constexpr size_t testObjCount {iterationCount};
    std::array<TestObj, testObjCount> testObjInput {};

    auto genTestObj = []
        {
            auto len {rand()%maxStrLen};

            std::string s{};
            s.reserve(len);
            for (int i = 0; i < len; ++i)
                s += alphaNum2[rand() % sizeof(alphaNum2)];
            
            return TestObj{rand(), alphaNum2[rand() % sizeof(alphaNum2)], s};
        };
    std::generate_n(testObjInput.begin(), testObjCount, genTestObj);

    gby::lock_free_const_sized_slot_map<TestObj, iterationCount, std::pair<int32_t, uint64_t>> map;
    test_SCMP<testObjCount, 3>(map, [&testObjInput]() { return testObjInput[rand()%testObjCount];}, false);

    gby::lock_free_const_sized_slot_map<TestObj, iterationCount, std::pair<int32_t, uint64_t>> map2;
    test_SCMP<testObjCount, 3>(map2, [&testObjInput]() { return testObjInput[rand()%testObjCount];}, true);
}