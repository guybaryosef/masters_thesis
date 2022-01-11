
#include <gtest/gtest.h>
#include "lock_free_vector.h"


TEST(LockFreeVector, SingleThreadedIntPushBack)
{
    gby::lock_free_vector<int> vec;

    std::vector<int> vals {5, -12, 0, 4};

    for (auto& i : vals)
        vec.push_back(i);

    for (int i = 0; i < vals.size(); ++i)
        ASSERT_EQ(vec[i], vals[i]);

    ASSERT_EQ(vals.size(), vec.size());
}

TEST(LockFreeVector, SingleThreadedIntPushBack2)
{
    gby::lock_free_vector<int, 2> vec;

    std::vector<int> vals {5, -12, 0, 4};

    for (auto& i : vals)
        vec.push_back(i);

    for (int i = 0; i < vals.size(); ++i)
        ASSERT_EQ(vec[i], vals[i]);
    ASSERT_EQ(vals.size(), vec.size());
}

TEST(LockFreeVector, SingleThreadedIntPushBack3)
{
    gby::lock_free_vector<int, 2, 3> vec;

    std::vector<int> vals {5, -12, 0, 4};

    for (auto& i : vals)
        vec.push_back(i);

    for (int i = 0; i < vals.size(); ++i)
        ASSERT_EQ(vec[i], vals[i]);
    ASSERT_EQ(vals.size(), vec.size());
}

TEST(LockFreeVector, SingleThreadedIntUpdate1)

{
    gby::lock_free_vector<int, 2> vec;

    std::vector<int> vals {5, -12, 0, 4};

    vec.reserve(vals.size());

    for (auto& i : vals)
        vec.push_back(i);

    vals[2] = 981541;
    vec.update(2, vals[2]);

    vals.push_back(-991182);
    vec.push_back(vals.back());

    for (int i = 0; i < vals.size(); ++i)
        ASSERT_EQ(vec[i], vals[i]);

    ASSERT_EQ(vals.size(), vec.size());
}

TEST(LockFreeVector, SingleThreadedIntUpdate2)
{
    gby::lock_free_vector<int, 4> vec;

    std::vector<int> vals {5, -12, 0, 4};

    vec.reserve(vals.size());

    for (auto& i : vals)
        vec.push_back(i);

    vals[2] = 981541;
    ASSERT_TRUE(vec.update(2, vals[2]));

    vals.push_back(-991182);
    vec.push_back(vals.back());

    vals[5] = 1;
    vec[5] = vals.back();

    for (int i = 0; i < vals.size(); ++i)
        ASSERT_EQ(vec[i], vals[i]);

    ASSERT_EQ(vals.size(), vec.size());
}

TEST(LockFreeVector, SingleThreadedIntUpdate3)
{
    gby::lock_free_vector<int, 4> vec;

    ASSERT_FALSE(vec.update(0, 1));
    ASSERT_FALSE(vec.update(1, 2));
    
    std::vector<int> vals {5, -12, 0, 4};

    vec.reserve(vals.size());

    for (auto& i : vals)
        vec.push_back(i);

    ASSERT_FALSE(vec.update(5, vals[2]));

    ASSERT_EQ(vals.size(), vec.size());
}

TEST(LockFreeVector, SingleThreadedIntPop1)
{
    gby::lock_free_vector<int> vec;

    std::vector<int> vals {5};

    for (auto& i : vals)
        vec.push_back(i);

    ASSERT_EQ(1, vec.size());
    ASSERT_EQ(vals[0], vec.pop_back());
    ASSERT_EQ(0, vec.size());
}

TEST(LockFreeVector, SingleThreadedIntPop2)
{
    gby::lock_free_vector<int, 4> vec;

    std::vector<int> vals {5, -12, 0, 4};

    vec.reserve(vals.size());

    for (auto& i : vals)
        vec.push_back(i);

    ASSERT_EQ(4, vec.size());
    ASSERT_EQ(vals[3], vec.pop_back());
    ASSERT_EQ(3, vec.size());
    ASSERT_EQ(vals[2], vec.pop_back());
    ASSERT_EQ(2, vec.size());
    ASSERT_EQ(vals[1], vec.pop_back());
    ASSERT_EQ(1, vec.size());
    ASSERT_EQ(vals[0], vec.pop_back());
    ASSERT_EQ(0, vec.size());

    vals.clear();

    int newVal = -991182;
    vals.push_back(newVal);
    vec.push_back(newVal);

    ASSERT_FALSE(vec.update(2, 1));

    ASSERT_EQ(1, vec.size());
    ASSERT_EQ(newVal, vec[0]);
}