
#include "../UnitTestHelpers.h"

#include "dynamic_slot_map.h"

#include <gtest/gtest.h>
#include <string>
#include <deque>


TEST(DynamicallyResizable, IntElement)
{
    gby::dynamic_slot_map<int> intMap;
    std::array<int, 3> vals {48, 0, -9823};

    addQueryAndRemoveElement(intMap, vals);
}

TEST(DynamicallyResizable, StringElement)
{
    gby::dynamic_slot_map<std::string> stringMap;
    std::array<std::string, 3> vals {"this is a string", {}, "ABC."};

    addQueryAndRemoveElement(stringMap, vals);
}


TEST(DynamicallyResizable, StringMultipleDynamicResize)
{
    gby::dynamic_slot_map<std::string> stringMap(1, 2);
    std::array<std::string, 10> vals {
        "this is a string", {}, "ABC.",
        "asdf asd", "asdfsa", "a", "asdf",
        "this is a string", {}, "ABC."};

    std::vector<gby::dynamic_slot_map<std::string>::key_type> keys;
    for (auto v : vals)
        keys.push_back(stringMap.insert(v));

    ASSERT_EQ(10, stringMap.size());

    for (int i=0; i<10; ++i)
    {
        auto k = keys[i];
        auto v = vals[i];
        auto mapV = stringMap[k];
        ASSERT_EQ(v, mapV);
    }
}

TEST(DynamicallyResizable, TestObjElement)
{
    gby::dynamic_slot_map<TestObj, std::pair<int32_t, uint64_t>> testObjMap;
    std::array<TestObj, 3> vals { TestObj{156, 'b', "this is a string"}, 
                                  TestObj{}, 
                                  TestObj{-124, 'Q', "anotherSTRING"} }; 

    addQueryAndRemoveElement(testObjMap, vals);
}

TEST(DynamicallyResizable, IterateOverTestObj)
{
    gby::dynamic_slot_map<TestObj, std::pair<int32_t, uint64_t>> testObjMap;

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
        EXPECT_EQ(v, testObjMap.find(k)->get());
    }

}
