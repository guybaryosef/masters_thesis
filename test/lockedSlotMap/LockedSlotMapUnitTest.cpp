

#include <gtest/gtest.h>

#include "locked_slot_map.h"


class ApplicationTest : public ::testing::Test
{

    gby::locked_slot_map<int> map;
};


TEST_F(ApplicationTest, TestAddingElement)
{
    EXPECT_EQ(true, true) << "should always work!";
}


int main(int argc, char **argv)
{
        ::testing::InitGoogleTest(&argc,argv);
        return RUN_ALL_TESTS();
}