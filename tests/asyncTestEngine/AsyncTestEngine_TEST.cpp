#include <gtest/gtest.h>
#include <future>
#include "AsyncTestEngine.hpp"

void f(SystemReadyIndicator& indicator)
{
    indicator.waitForSystemReady();
    std::cout << "Test string" << std::endl;
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


struct Wrapper
{
private:
    ThreadsafeHashTable<int,int> table_;
public:
    Wrapper(size_t buckets_number):
        table_{buckets_number}
    {}
    ThreadsafeHashTable<int,int>& getTable()
    {
        return table_;
    }
};

void pushWithWrapper(std::shared_future<void>& future,
          Wrapper& wrapper,
          int key)
{
    future.wait();
    wrapper.getTable().addOrUpdateMapping(key,6);
    wrapper.getTable().removeMapping(key-1);
    wrapper.getTable().valueFor(key+1);
}

void testDataRacePresenceWithWrapper()
{
    Wrapper wrapper{40};
    std::vector<std::thread> v;
    v.reserve(10);
    std::promise<void> promise;
    std::shared_future<void> future{promise.get_future()};

    for(size_t i{0};i < 10;++i)
        v.emplace_back(pushWithWrapper,std::ref(future),std::ref(wrapper),i);
    std::this_thread::sleep_for(std::chrono::seconds{1});
    promise.set_value();
    for(auto& t : v)
        t.join();
}

TEST(TableTest,WrapperTest)
{
    EXPECT_NO_THROW(testDataRacePresenceWithWrapper());    
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}