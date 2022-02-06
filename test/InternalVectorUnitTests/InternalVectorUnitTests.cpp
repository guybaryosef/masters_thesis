
#include <gtest/gtest.h>
#include <thread>
#include <algorithm>

#include "internal_vector.h"


const std::vector<std::string> strVals {"first", "second two three", "THIRD", "fourth and Final!!"};
const std::vector<int> intVals {5, -12, 0, 4};


TEST(InternalVector, SingleThreadedIntPushBack)
{
    gby::internal_vector<int> vec;

    for (auto& i : intVals)
        vec.push_back(i);

    for (int i = 0; i < intVals.size(); ++i)
        ASSERT_EQ(vec[i], intVals[i]);

    ASSERT_EQ(intVals.size(), vec.size());
}

TEST(InternalVector, SingleThreadedIntPushBack2)
{
    gby::internal_vector<int, 2> vec;

    for (auto& i : intVals)
        vec.push_back(i);

    for (int i = 0; i < intVals.size(); ++i)
        ASSERT_EQ(vec[i], intVals[i]);
    ASSERT_EQ(intVals.size(), vec.size());
}

TEST(InternalVector, SingleThreadedIntPushBack3)
{
    gby::internal_vector<int, 2, 3> vec;

    for (auto& i : intVals)
        vec.push_back(i);

    for (int i = 0; i < intVals.size(); ++i)
        ASSERT_EQ(vec[i], intVals[i]);
    ASSERT_EQ(intVals.size(), vec.size());
}

TEST(InternalVector, SingleThreadedIntUpdate1)

{
    gby::internal_vector<int, 2> vec;

    std::vector<int> vals = intVals;

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

TEST(InternalVector, SingleThreadedIntUpdate2)
{
    gby::internal_vector<int, 4> vec;

    std::vector<int> vals = intVals;

    vec.reserve(vals.size());

    ASSERT_EQ(1, vec.bucket_count());

    for (auto& i : vals)
        vec.push_back(i);

    vals[2] = 981541;
    ASSERT_TRUE(vec.update(2, vals[2]));

    vals.push_back(-991182);
    vec.push_back(vals.back());

    vals[4] = 1;
    vec[4] = vals.back();

    for (int i = 0; i < vals.size(); ++i)
        ASSERT_EQ(vec[i], vals[i]);

    ASSERT_EQ(vals.size(), vec.size());
}

TEST(InternalVector, SingleThreadedIntUpdate3)
{
    gby::internal_vector<int, 4> vec;

    ASSERT_FALSE(vec.update(0, 1));
    ASSERT_FALSE(vec.update(1, 2));
    
    std::vector<int> vals = intVals;

    vec.reserve(vals.size());

    for (auto& i : vals)
        vec.push_back(i);

    ASSERT_FALSE(vec.update(5, vals[2]));

    ASSERT_EQ(vals.size(), vec.size());
}

TEST(InternalVector, SingleThreadedIntPop1)
{
    gby::internal_vector<int> vec;

    std::vector<int> vals {5};

    for (auto& i : vals)
        vec.push_back(i);

    ASSERT_EQ(1, vec.size());
    ASSERT_EQ(vals[0], vec.pop_back());
    ASSERT_EQ(0, vec.size());
}

TEST(InternalVector, SingleThreadedIntPop2)
{
    gby::internal_vector<int, 4> vec;

    std::vector<int> vals = intVals;

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

TEST(InternalVector, SingleThreadedStringPushBack)
{
    gby::internal_vector<std::string> vec;

    for (auto& i : strVals)
        vec.push_back(i);

    for (int i = 0; i < strVals.size(); ++i)
        ASSERT_EQ(vec[i], strVals[i]);

    ASSERT_EQ(strVals.size(), vec.size());
}

TEST(InternalVector, SingleThreadedStringPushBack2)
{
    gby::internal_vector<std::string, 2> vec;

    for (auto& i : strVals)
        vec.push_back(i);

    for (int i = 0; i < strVals.size(); ++i)
        ASSERT_EQ(vec[i], strVals[i]);
    ASSERT_EQ(strVals.size(), vec.size());
}

TEST(InternalVector, SingleThreadedStringPushBack3)
{
    gby::internal_vector<std::string, 2, 3> vec;

    for (auto& i : strVals)
        vec.push_back(i);

    for (int i = 0; i < strVals.size(); ++i)
        ASSERT_EQ(vec[i], strVals[i]);
    ASSERT_EQ(strVals.size(), vec.size());
}

TEST(InternalVector, SingleThreadedStringUpdate1)
{
    gby::internal_vector<std::string, 2> vec;

    std::vector<std::string> vals = strVals;

    vec.reserve(vals.size());

    for (auto& i : vals)
        vec.push_back(i);

    vals[2] = "125235";
    vec.update(2, vals[2]);

    vals.push_back("this is not a number");
    vec.push_back(vals.back());

    for (int i = 0; i < vals.size(); ++i)
        ASSERT_EQ(vec[i], vals[i]);

    ASSERT_EQ(vals.size(), vec.size());
}

TEST(InternalVector, SingleThreadedStringUpdate2)
{
    gby::internal_vector<std::string, 4> vec;

    std::vector<std::string> vals = strVals;

    vec.reserve(vals.size());

    ASSERT_EQ(1, vec.bucket_count());

    for (auto& i : vals)
        vec.push_back(i);

    ASSERT_EQ(1, vec.bucket_count());
    
    vals[2] = "981541";
    ASSERT_TRUE(vec.update(2, vals[2]));

    ASSERT_EQ(1, vec.bucket_count());
    
    vals.push_back("blah BLAH");
    vec.push_back(vals.back());

    ASSERT_EQ(2, vec.bucket_count());
    
    vals[4] = 1;
    vec[4] = vals.back();

    for (int i = 0; i < vals.size(); ++i)
        ASSERT_EQ(vec[i], vals[i]);

    ASSERT_EQ(vals.size(), vec.size());
}

TEST(InternalVector, SingleThreadedStringUpdate3)
{
    gby::internal_vector<std::string, 4> vec;

    ASSERT_FALSE(vec.update(0, "a"));
    ASSERT_FALSE(vec.update(1, "AAA BBB"));
    
    vec.reserve(strVals.size());

    for (auto& i : strVals)
        vec.push_back(i);

    ASSERT_FALSE(vec.update(5, strVals[2]));

    ASSERT_EQ(strVals.size(), vec.size());
}

TEST(InternalVector, SingleThreadedStringPop1)
{
    gby::internal_vector<std::string> vec;

    std::vector<std::string> vals {"abc"};

    for (auto& i : vals)
        vec.push_back(i);

    ASSERT_EQ(1, vec.size());
    ASSERT_EQ(vals[0], vec.pop_back());
    ASSERT_EQ(0, vec.size());
}

TEST(InternalVector, SingleThreadedStringPop2)
{
    gby::internal_vector<std::string, 4> vec;

    std::vector<std::string> vals = strVals;

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

    std::string newVal = "abcasda";
    vals.push_back(newVal);
    vec.push_back(newVal);

    ASSERT_FALSE(vec.update(2, "BAC"));

    ASSERT_EQ(1, vec.size());
    ASSERT_EQ(newVal, vec[0]);
}

TEST(InternalVector, MultiThreadedInt)
{
    gby::internal_vector<uint64_t> vec;

    const uint64_t maxVal = 99999;

    // construct 4 vectors
    std::vector<std::vector<uint64_t>> inputVecs (4);
    for (uint64_t i = 0; i < maxVal; i += 4)
    {
        inputVecs[0].push_back(i);
        inputVecs[1].push_back(i + 1);
        inputVecs[2].push_back(i + 2);
        inputVecs[3].push_back(i + 3);
    }
    uint64_t total {};
    std::for_each(inputVecs.begin(), inputVecs.end(), [&total](const std::vector<uint64_t>& vec) {total += vec.size();});

    // run the 4 write threads
    auto writeFnc = [&vec] (const std::vector<uint64_t>& input)
    {
        for (const auto& i : input)
            vec.push_back(i);
    };

    std::vector<std::thread> writeThreads;
    std::ranges::for_each(inputVecs, [&vec, &writeFnc, &writeThreads] (const std::vector<uint64_t>& in) 
                                    { writeThreads.push_back(std::thread(writeFnc, in)); });


    std::ranges::for_each(writeThreads, [](std::thread& t) {if (t.joinable()) t.join();});

    ASSERT_EQ(total, vec.size());

    std::vector<bool> checkVals(total, false);
    for (uint64_t i = 0; i < total; ++i)
        checkVals[vec[i]] = true;

    for (auto found : checkVals)
        ASSERT_TRUE(found);
}

TEST(InternalVector, iteratorOperators)
{
    gby::internal_vector<int, 2, 8> vec;
    ASSERT_EQ(vec.begin(), vec.end());
    ASSERT_EQ(vec.cbegin(), vec.cend());

    std::vector<int> vals {5, 6, 7, 8, 9};
    for (auto& i : vals)
        vec.push_back(i);
    
    ASSERT_NE(vec.begin(), vec.end());
    ASSERT_NE(vec.cbegin(), vec.cend());

    gby::internal_vector<int>::iterator it = vec.begin();

    ASSERT_EQ(vals[0], *it);
    for (int i = 1; i < vals.size(); ++i)
        ASSERT_EQ(vals[i], *(it+i));

    ++it;
    ASSERT_EQ(vals[1], *it);
    
    --it;
    ASSERT_EQ(vals[0], *it);

    // crossing a bucket boundary
    ++it;
    ++it;
    ASSERT_EQ(vals[2], *it);

    --it;
    ASSERT_EQ(vals[1], *it);

    --it;
    gby::internal_vector<int>::iterator it2 = vec.begin();

    ASSERT_EQ(it, it2);

    ASSERT_EQ(it+vals.size(), vec.end());
    
    for (auto it = vec.begin(); it != vec.end(); ++it)
    {
        *it += 2;
    }

    auto it3 = vec.begin();
    for (int i = 1; i < vals.size(); ++i)
        ASSERT_EQ(vals[i] + 2, *(it3+i));
}

TEST(InternalVector, reserveTest)
{
    gby::internal_vector<int, 2, 8> vec;

    vec.reserve(2);
    ASSERT_EQ(1, vec.bucket_count());

    vec.reserve(4);
    ASSERT_EQ(2, vec.bucket_count());

    vec.reserve(6);
    ASSERT_EQ(2, vec.bucket_count());

    vec.reserve(7);
    ASSERT_EQ(3, vec.bucket_count());

    vec.reserve(12);
    ASSERT_EQ(3, vec.bucket_count());

    vec.reserve(14);
    ASSERT_EQ(3, vec.bucket_count());

    vec.reserve(9);
    ASSERT_EQ(3, vec.bucket_count());

    vec.reserve(4);
    ASSERT_EQ(3, vec.bucket_count());

    vec.reserve(60);
    ASSERT_EQ(5, vec.bucket_count());

    vec.reserve(63);
    ASSERT_EQ(6, vec.bucket_count());

    vec.reserve(6);
    ASSERT_EQ(6, vec.bucket_count());
}
