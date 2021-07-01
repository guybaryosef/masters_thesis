

#include <gtest/gtest.h>
#include <string>
#include <deque>
#include "locked_slot_map.h"


struct TestObj
{
    int         _a;
    char        _b;
    std::string _c;
};

bool operator== (const TestObj &lhs, const TestObj &rhs)
{
    return lhs._a == rhs._a &&
           lhs._b == rhs._b &&
           lhs._c == rhs._c;
}



template <typename T, typename... U>
void addAndRemoveElement(gby::locked_slot_map<T, U...> map, std::array<T, 3> vals)
{
    EXPECT_TRUE(map.empty());
    
    auto key1 = map.insert(vals[0]);
    auto key2 = map.insert(vals[1]);
    auto key3 = map.insert(vals[2]);

    EXPECT_EQ(3, map.size());

    EXPECT_EQ(vals[0], map[key1]);
    EXPECT_EQ(vals[1], *map.find(key2));
    EXPECT_EQ(vals[2], *map.find_unchecked(key3));

    map.erase(key2);
    EXPECT_EQ(vals[0], *map.find(key1));
    EXPECT_EQ(map.end(), map.find(key2));
    EXPECT_EQ(vals[2], *map.find(key3));
    EXPECT_EQ(2, map.size());

    map.erase(key1);
    EXPECT_EQ(map.end(), map.find(key1));
    EXPECT_EQ(map.end(), map.find(key2));
    EXPECT_EQ(vals[2], *map.find(key3));
    EXPECT_EQ(1, map.size());

    map.erase(key3);
    EXPECT_EQ(map.end(), map.find(key1));
    EXPECT_EQ(map.end(), map.find(key2));
    EXPECT_EQ(map.end(), map.find(key3));
    EXPECT_EQ(0, map.size());
}


TEST(LockedSlotMap, AddAndRemoveIntElement)
{
    gby::locked_slot_map<int> intMap;
    // std::array<int, 3> vals ;

    addAndRemoveElement<int>(intMap, {48, 0, -9823});
}

TEST(LockedSlotMap, AddAndRemoveStringElement)
{
    gby::locked_slot_map<std::string> stringMap;
    std::array<std::string, 3> vals {"this is a string", {}, "ABC."};

    addAndRemoveElement<std::string>(stringMap, vals);
}

TEST(LockedSlotMap, AddTestObjElement)
{
    gby::locked_slot_map<TestObj, std::pair<int32_t, uint64_t>, std::deque> testObjMap;

    std::array<TestObj, 3> vals { }; 
    
    // { {156, 'b', "this is a string"},
    //                               {},
    //                               {-124, 'Q', "anotherSTRING"} };

    addAndRemoveElement<TestObj, std::pair<int32_t, uint64_t>, std::deque>(testObjMap, vals);
}


int main(int argc, char **argv)
{
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
}