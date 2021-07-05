
#include "TestHelpers.h"

#include "locked_slot_map.h"

#include <gtest/gtest.h>
#include <string>
#include <deque>


TEST(LockedSlotMap, IntElement)
{
    gby::locked_slot_map<int> intMap;
    std::array<int, 3> vals {48, 0, -9823};

    addQueryAndRemoveElement(intMap, vals);
}

TEST(LockedSlotMap, StringElement)
{
    gby::locked_slot_map<std::string> stringMap;
    std::array<std::string, 3> vals {"this is a string", {}, "ABC."};

    addQueryAndRemoveElement(stringMap, vals);
}

TEST(LockedSlotMap, TestObjElement)
{
    gby::locked_slot_map<TestObj, std::pair<int32_t, uint64_t>, std::deque> testObjMap;
    std::array<TestObj, 3> vals { TestObj{156, 'b', "this is a string"}, 
                                  TestObj{}, 
                                  TestObj{-124, 'Q', "anotherSTRING"} }; 

    addQueryAndRemoveElement(testObjMap, vals);
}
