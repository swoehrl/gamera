#include <gtest/gtest.h>
#include "mergesort.hpp"

TEST(MergeSortTest, CorrectSort) {
    externalsort("test.num", "testout.num", 100*1024);
    EXPECT_TRUE(checksort("testout.num", 100*1024));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
