

#include "../RegressionTestHelpers.h"
#include "lock_free_const_sized_slot_map.h"

#include <gtest/gtest.h>

#include <string>
#include <deque>

constexpr size_t iterationCount {50000};

/////////// SCMP ///////////
TEST(LockFreeConstSized, SCMPIntElement)
{
    gby::lock_free_const_sized_slot_map<int, iterationCount> map;
    test_SCMP<iterationCount, 3>(map, []() { return rand();}, false);

    gby::lock_free_const_sized_slot_map<int, iterationCount> map2;
    test_SCMP<iterationCount, 3>(map2, []() { return rand();}, true);
}

TEST(LockFreeConstSized, SCMPStringElement)
{
    constexpr size_t strCount {iterationCount};
    auto strInput = genStrInput<strCount>();

    gby::lock_free_const_sized_slot_map<std::string, iterationCount> map;
    test_SCMP<strCount, 3>(map, [&strInput]() { return strInput[rand()%strCount];}, false);

    gby::lock_free_const_sized_slot_map<std::string, iterationCount> map2;
    test_SCMP<strCount, 3>(map2, [&strInput]() { return strInput[rand()%strCount];}, true);
}

TEST(LockFreeConstSized, SCMPTestObjElement)
{
    constexpr size_t testObjCount {iterationCount};
    auto testObjInput = genTestObj<testObjCount>();

    gby::lock_free_const_sized_slot_map<TestObj, iterationCount, std::pair<int32_t, uint64_t>> map;
    test_SCMP<testObjCount, 3>(map, [&testObjInput]() { return testObjInput[rand()%testObjCount];}, false);

    gby::lock_free_const_sized_slot_map<TestObj, iterationCount, std::pair<int32_t, uint64_t>> map2;
    test_SCMP<testObjCount, 3>(map2, [&testObjInput]() { return testObjInput[rand()%testObjCount];}, true);
}

/////////// MCMP ///////////
constexpr size_t WriterCount {2};
constexpr size_t MCMP_writesPerWriter {iterationCount/WriterCount};

TEST(LockFreeConstSized, MCMPIntElement)
{
    gby::lock_free_const_sized_slot_map<int, iterationCount> map;
    test_MCMP<WriterCount, MCMP_writesPerWriter, 2, 3>(map, [] { return rand();});
}

TEST(LockFreeConstSized, MCMPStringElement)
{
    constexpr size_t strCount {iterationCount};
    auto strInput = genStrInput<strCount>();

    gby::lock_free_const_sized_slot_map<std::string, iterationCount> map;
    test_MCMP<WriterCount, MCMP_writesPerWriter, 1, 5>(map, [&strInput] { return strInput[rand()%strCount];});
}

TEST(LockFreeConstSized, MCMPTestObjElement)
{
    constexpr size_t testObjCount {iterationCount};
    auto testObjInput = genTestObj<testObjCount>();

    gby::lock_free_const_sized_slot_map<TestObj, iterationCount, std::pair<int32_t, uint64_t>> map;
    test_MCMP<WriterCount, MCMP_writesPerWriter, 0, 3>(map, [&testObjInput] { return testObjInput[rand()%testObjCount];});
}