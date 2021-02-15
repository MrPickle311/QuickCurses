#include <gtest/gtest.h>
#include "ThreadsafeHashTable.hpp"
#include <gtest/gtest.h>
#include "../../extern/sigslot/signal.hpp"
#include <iostream>
#include <future>
#include <ratio>
#include <chrono>

//SILNIK SYGNAŁÓW NA BAZIE TEGO KONTENERA
template<typename Key,typename Value>
class TableWrapper
{
private:
    ThreadsafeHashTable<Key,Value> table_;
public:
    TableWrapper(size_t buckets_number):
        table_{buckets_number}
    {}
    Value getValue(Key const& key) const
    {
        return table_.valueFor(key);
    }
    void tryAddEntry(Key const& key,Value const& value)
    {
        table_.addOrUpdateMapping(key,value);
    }
    void tryRemoveEntry(const Key& key)
    {
        table_.removeMapping(key);
    }
};

TEST(TableTest,InvariantTest)
{
    TableWrapper<int,int> tw{50};
    EXPECT_NO_THROW(tw.tryRemoveEntry(6));
    EXPECT_NO_THROW(tw.getValue(6));
    EXPECT_NO_THROW(tw.getValue(7));
    EXPECT_NO_THROW(tw.tryRemoveEntry(66));
    EXPECT_NO_THROW(tw.tryAddEntry(66,76));
    EXPECT_NO_THROW(tw.tryAddEntry(0,0));
    EXPECT_NO_THROW(tw.tryAddEntry(66,76));
    EXPECT_NO_THROW(tw.tryAddEntry(0,0));
    EXPECT_NO_THROW(tw.tryAddEntry(66,76));
    EXPECT_EQ(tw.getValue(66),76);
    EXPECT_NO_THROW(tw.tryRemoveEntry(66));
    EXPECT_EQ(tw.getValue(0),0);
    EXPECT_NO_THROW(tw.tryRemoveEntry(0));
    EXPECT_EQ(tw.getValue(66),0);
    EXPECT_EQ(tw.getValue(0),0);
}

void push(std::shared_future<void>& future,
          ThreadsafeHashTable<int,int>& table,
          int key)
{
    future.wait();
    table.addOrUpdateMapping(key,6);
    table.removeMapping(key-1);
    table.valueFor(key+1);
}

void testDataRacePresence()
{
    ThreadsafeHashTable<int,int> table{40};
    std::vector<std::thread> v;
    v.reserve(10);
    std::promise<void> promise;
    std::shared_future<void> future{promise.get_future()};

    for(size_t i{0};i < 10;++i)
        v.emplace_back(push,std::ref(future),std::ref(table),i);
    std::this_thread::sleep_for(std::chrono::seconds{1});
    promise.set_value();
    for(auto& t : v)
        t.join();
}

TEST(TableTest,AsyncSimpleTest)
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