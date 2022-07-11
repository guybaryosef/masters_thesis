
#pragma once

#include <gtest/gtest.h>

#include "locked_slot_map.h"

#include <string>
#include <array>


struct TestObj
{
    int         _a;
    char        _b;
    std::string _c;
};

inline bool operator==(const TestObj &lhs, const TestObj &rhs)
{
    return lhs._a == rhs._a &&
           lhs._b == rhs._b &&
           lhs._c == rhs._c;
}

template <typename T, typename U>
void addQueryAndRemoveElement(T& map, std::array<U, 3>& vals)
{
    EXPECT_TRUE(map.empty());
    
    auto key1 = map.insert(vals[0]);
    auto key2 = map.insert(vals[1]);
    auto key3 = map.insert(vals[2]);

    EXPECT_EQ(3, map.size());

    EXPECT_EQ(vals[0], map[key1]);
    EXPECT_EQ(vals[1], (*map.find(key2)).get());
    EXPECT_EQ(vals[2], map.find_unchecked(key3));

    map.erase(key2);
    EXPECT_EQ(vals[0], (*map.find(key1)).get());
    EXPECT_FALSE(map.find(key2).has_value());
    EXPECT_EQ(vals[2], (*map.find(key3)).get());
    EXPECT_EQ(2, map.size());

    map.erase(key1);
    EXPECT_FALSE(map.find(key1).has_value());
    EXPECT_FALSE(map.find(key2).has_value());
    EXPECT_EQ(vals[2], (*map.find(key3)).get());
    EXPECT_EQ(1, map.size());

    map.erase(key3);
    EXPECT_FALSE(map.find(key1).has_value());
    EXPECT_FALSE(map.find(key2).has_value());
    EXPECT_FALSE(map.find(key3).has_value());
    EXPECT_EQ(0, map.size());

    EXPECT_TRUE(map.empty());
}


template <typename T, typename U>
void addQueryAndRemoveElement_Locked (T& map, std::array<U, 3>& vals)
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

    EXPECT_TRUE(map.empty());
}