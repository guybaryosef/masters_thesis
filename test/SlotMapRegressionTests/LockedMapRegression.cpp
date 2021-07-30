

#include "RegressionTestHelpers.h"
#include "locked_slot_map.h"

#include <gtest/gtest.h>

#include <string>
#include <deque>


TEST(LockedSlotMapRegression, IntElement)
{
    gby::locked_slot_map<int> intMap;

    test_SCMP<100000, 5>(intMap, []() { return rand();}, false);
}

// TEST(LockedSlotMap, StringElement)
// {
//     gby::locked_slot_map<std::string> stringMap;
//     std::array<std::string, 3> vals {"this is a string", {}, "ABC."};

//     addQueryAndRemoveElement(stringMap, vals);
// }

// TEST(LockedSlotMap, TestObjElement)
// {
//     gby::locked_slot_map<TestObj, std::pair<int32_t, uint64_t>, std::deque> testObjMap;
//     std::array<TestObj, 3> vals { TestObj{156, 'b', "this is a string"}, 
//                                   TestObj{}, 
//                                   TestObj{-124, 'Q', "anotherSTRING"} }; 

//     addQueryAndRemoveElement(testObjMap, vals);
// }
