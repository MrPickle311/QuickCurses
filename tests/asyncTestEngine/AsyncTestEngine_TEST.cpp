#include <gtest/gtest.h>
#include <future>
#include "AsyncTestEngine.hpp"

void f(SystemReadyIndicator& indicator)
{
    indicator.wait();
    std::cout << "xd" << std::endl;
}

void testDataRacePresence()
{
    std::promise<void> promise;
    SystemReadyIndicator indicator{promise};
    std::vector<std::thread> v;
    v.reserve(10);

    for(size_t i{0};i < 10;++i)
        v.emplace_back(f,std::ref(indicator));
    std::this_thread::sleep_for(std::chrono::seconds{1});
    promise.set_value();
    for(auto& t : v)
        t.join();
}

TEST(SystemReadyIndicatorTest,DataRaceTest)
{
    EXPECT_NO_THROW(testDataRacePresence());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}