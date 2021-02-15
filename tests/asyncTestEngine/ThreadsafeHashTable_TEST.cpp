#include <gtest/gtest.h>
#include "ThreadsafeHashTable.hpp"
#include <gtest/gtest.h>
#include "../../extern/sigslot/signal.hpp"
#include <iostream>
#include <future>

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

void testDataRacePresence()
{
    ThreadsafeHashTable<int,int> table{40};
    std::thread th1{&ThreadsafeHashTable<int,int>::addOrUpdateMapping,&table,56,54};
    std::thread th2{&ThreadsafeHashTable<int,int>::addOrUpdateMapping,&table,565,54};
    th1.join();
    th2.join();
}

void f(int h)
{
    std::cout << "XDXXDXD 1 " << h << std::endl;
}

void j(int h)
{
    std::cout << "XDXXDXD " << h << std::endl;
}

void g(sigslot::signal<int>& sig)
{
    sig(6);
}

template<typename... Args>
using Signal = sigslot::signal<Args...>;

int main(int argc, char **argv)
{
    Signal<int> sig;
    auto c = sig.connect(j);
    c.block();
    sig(6);
    c.unblock();
    sig(7);
    c.connected();
    c.valid();
    testDataRacePresence();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}