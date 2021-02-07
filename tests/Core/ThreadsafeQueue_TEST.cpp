#include "../../include/Core/ThreadsafeQueue.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

TEST(TEST,FirstTest)
{
    ThreadsafeQueue<int> queue;
    queue.push(5);
    queue.push(6);
    int a;
    queue.tryPop(a);
    EXPECT_EQ(a,6);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}