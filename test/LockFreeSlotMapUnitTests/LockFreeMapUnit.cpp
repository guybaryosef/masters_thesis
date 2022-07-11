
#include "../UnitTestHelpers.h"

#include "lock_free_slot_map.h"

#include <gtest/gtest.h>
#include <string>
#include <deque>


TEST(LockFreeUnit, IntElement)
{
    gby::lock_free_slot_map<int> intMap;
    std::array<int, 3> vals {48, 0, -9823};

    addQueryAndRemoveElement(intMap, vals);
}

TEST(LockFreeUnit, StringElement)
{
    gby::lock_free_slot_map<std::string> stringMap;
    std::array<std::string, 3> vals {"this is a string", {}, "ABC."};

    addQueryAndRemoveElement(stringMap, vals);
}

TEST(LockFreeUnit, TestObjElement)
{
    gby::lock_free_slot_map<TestObj, std::pair<int32_t, uint64_t>> testObjMap;
    std::array<TestObj, 3> vals { TestObj{156, 'b', "this is a string"}, 
                                  TestObj{}, 
                                  TestObj{-124, 'Q', "anotherSTRING"} }; 

    addQueryAndRemoveElement(testObjMap, vals);
}

TEST(LockFreeUnit, IterateOverTestObj)
{
    gby::lock_free_slot_map<TestObj, std::pair<int32_t, uint64_t>> testObjMap;

    std::vector<decltype(testObjMap)::key_type> keys{};
    std::vector<TestObj> values {
        {-5, 25, "a magnificent String"},
        {9999, 'a', "a v029843y51O'AAFJAHSDG'AJA';LKAS A'23!#!#!ADSF"},
        {1024, 'Z', "dog"}
    };

    for (auto& v : values)
        keys.push_back(testObjMap.insert(v));
    
    testObjMap.iterate_map([](TestObj& testObj) {
                                                    testObj._a -= 15; 
                                                    testObj._c.append("_Appended.");
                                                });

    for (int i = 0; i < keys.size(); ++i)
    {
        auto [k,v] = std::tie(keys[i], values[i]);
        v._a -= 15;
        v._c += "_Appended.";
        EXPECT_EQ(v, testObjMap.find(k));
    }

}
