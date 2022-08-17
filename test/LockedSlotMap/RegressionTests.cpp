

#include "../RegressionTestHelpers.h"
#include "locked_slot_map.h"

#include <gtest/gtest.h>

#include <string>
#include <deque>

constexpr size_t iterationCount {50000};

char alphanum[] = "0123456789"
                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                  "abcdefghijklmnopqrstuvwxyz";

    
 TEST(LockedSlotMap, IntElement)
 {
     gby::locked_slot_map<int> map;
     map.reserve(iterationCount);
     test_SPMC<iterationCount, 3>(map, []() { return rand();}, false);

     gby::locked_slot_map<int> map2;
     map2.reserve(iterationCount);
     test_SPMC<iterationCount, 3>(map2, []() { return rand();}, true);
 }

 TEST(LockedSlotMap, StringElement)
 {
     constexpr size_t strCount {25};
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
     map.reserve(iterationCount);
     test_SPMC<strCount, 3>(map, [&strInput]() { return strInput[rand()%strCount];}, false);

     gby::locked_slot_map<std::string> map2;
     map2.reserve(iterationCount);
     test_SPMC<strCount, 3>(map2, [&strInput]() { return strInput[rand()%strCount];}, true);
 }

 TEST(LockedSlotMap, TestObjElement)
 {
     constexpr size_t testObjCount {25};
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
     map.reserve(iterationCount);
     test_SPMC<testObjCount, 3>(map, [&testObjInput]() { return testObjInput[rand()%testObjCount];}, false);

     gby::locked_slot_map<TestObj, std::pair<int32_t, uint64_t>> map2;
     map2.reserve(iterationCount);
     test_SPMC<testObjCount, 3>(map2, [&testObjInput]() { return testObjInput[rand()%testObjCount];}, true);
 }

/////////// MCMP ///////////
constexpr size_t WriterCount {2};
constexpr size_t MCMP_writesPerWriter {iterationCount/WriterCount};

TEST(LockedSlotMap, MCMPIntElement)
{
    gby::locked_slot_map<int> map;
    map.reserve(iterationCount);
    test_MPMC<WriterCount, MCMP_writesPerWriter, 2, 3>(map, [] { return rand();});
}

 TEST(LockedSlotMap, MCMPStringElement)
 {
     constexpr size_t strCount {25};
     auto strInput = genStrInput<strCount>();

     gby::locked_slot_map<std::string> map;
     map.reserve(iterationCount);
     test_MPMC<WriterCount, MCMP_writesPerWriter, 1, 5>(map, [&strInput] { return strInput[rand()%strCount];});
 }

 TEST(LockedSlotMap, MCMPTestObjElement)
 {
     constexpr size_t testObjCount {25};
     auto testObjInput = genTestObj<testObjCount>();

     gby::locked_slot_map<TestObj, std::pair<int32_t, uint64_t>> map;
     map.reserve(iterationCount);
     test_MPMC<WriterCount, MCMP_writesPerWriter, 0, 3>(map, [&testObjInput] { return testObjInput[rand()%testObjCount];});
 }
 