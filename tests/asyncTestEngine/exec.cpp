#include <gtest/gtest.h>
#include "ThreadsafeHashTable.hpp"
#include <gtest/gtest.h>
#include "../../extern/sigslot/signal.hpp"
#include <iostream>

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

}

void f()
{
    std::cout << "XDXXDXD" << std::endl;
}

int main(int argc, char **argv)
{
    sigslot::signal<> sig;
    sig.connect(f);
    sig();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}