#include <gtest/gtest.h>
#include "ThreadsafeHashTable.hpp"
#include <gtest/gtest.h>
#include "../../extern/sigslot/signal.hpp"
#include <iostream>
#include <future>
#include <ratio>
#include <chrono>
#include <functional>

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

void singleAsyncEqualityTest(std::shared_future<void>& future,
          ThreadsafeHashTable<int,int>& table1,
          ThreadsafeHashTable<int,int>& table2)
{
    future.wait();
    if(table1 == table2)
        std::cout << "Tables are equal\n";
}

void testDataRacePresence(size_t iterations_nmbr)
{
    ThreadsafeHashTable<int,int> table1{40};
    ThreadsafeHashTable<int,int> table2{40};

    std::vector<std::thread> threads1;
    std::vector<std::thread> threads2;

    threads1.reserve(iterations_nmbr);
    threads2.reserve(iterations_nmbr);

    std::promise<void> promise;
    std::shared_future<void> future{promise.get_future()};

    for(size_t i{0};i < iterations_nmbr;++i)
    {
        threads1.emplace_back(push,std::ref(future),std::ref(table1),i);
        threads2.emplace_back(singleAsyncEqualityTest,std::ref(future),
                                                      std::ref(table1),
                                                      std::ref(table2));
    }
    promise.set_value();
    
    for(auto& t : threads1)
        t.join();
    for(auto& t : threads2)
        t.join();
}

TEST(TableTest,AsyncSimpleTest)
{
    EXPECT_NO_THROW(testDataRacePresence(10));    
    EXPECT_NO_THROW(testDataRacePresence(20));  
    EXPECT_NO_THROW(testDataRacePresence(30));  
}

TEST(TableTest,EqualityOperatorTest)
{
    ThreadsafeHashTable<size_t,size_t> table1;
    ThreadsafeHashTable<size_t,size_t> table2;

    EXPECT_EQ(table1,table2);
    
    table1.addOrUpdateMapping(1,1);
    table2.addOrUpdateMapping(1,1);
    EXPECT_EQ(table1,table2);

    table1.addOrUpdateMapping(2,2);
    EXPECT_NE(table1,table2) << "Testing NE ";

    table2.addOrUpdateMapping(2,2);
    EXPECT_EQ(table1,table2);

    table2.addOrUpdateMapping(3,2);
    EXPECT_NE(table1,table2);
}

using Pair = std::pair<size_t,size_t>;

std::list<Pair> getIntList(size_t count)
{
    std::list<Pair>  list;
    for(size_t i{0}; i < count;++i)
        list.push_back(std::make_pair(i,i));
    return list;
}

void pushToVector(std::vector<std::list<Pair>>& vector,
                  size_t count)
{
    vector.push_back(getIntList(count));
}

template<size_t... lists_sizes>
void fillVector(std::vector<std::list<Pair>>& vector)
{
    std::vector<size_t> slots_sizes{lists_sizes...};
    for(auto& slot_size: slots_sizes)
        pushToVector(vector,slot_size);
}

TEST(EqualityTests,ComplexVectorsEqualityTest)
{
    std::vector<std::list<Pair>> vector1;
    std::vector<std::list<Pair>> vector2;

    fillVector<4,5,6,3,1>(vector1);
    fillVector<4,5,6,3,1>(vector2);

    EXPECT_EQ(vector1,vector2);
}

void pushToVectorPtr(std::vector<std::unique_ptr<std::list<Pair>>>& vector,
                  size_t count)
{
    vector.emplace_back(std::make_unique<std::list<Pair>>(getIntList(count)));
}

template<size_t... lists_sizes>
void fillVectorPtr(std::vector<std::unique_ptr<std::list<Pair>>>& vector)
{
    std::vector<size_t> slots_sizes{lists_sizes...};
    for(auto& slot_size: slots_sizes)
        pushToVectorPtr(vector,slot_size);
}

TEST(EqualityTests,ComplexWithPointersVectorsEqualityTest)
{
    std::vector<std::unique_ptr<std::list<Pair>>> vector1;
    std::vector<std::unique_ptr<std::list<Pair>>> vector2;

    fillVectorPtr<4,5,6,3,1>(vector1);
    fillVectorPtr<4,5,6,3,1>(vector2);

    for (size_t i {0}; i < vector1.size(); ++i)
        EXPECT_EQ(*vector1[i],*vector2[i]);//comapring lists
    
    EXPECT_NE(vector1,vector2);//comapring all vectors
}

using PairList = std::list<Pair>;

TEST(EqualityTests,PointersEqualityTest)
{
    std::unique_ptr<PairList> ptr1;
    std::unique_ptr<PairList> ptr2;

    ptr1 = std::make_unique<PairList>(getIntList(10));
    ptr2 = std::make_unique<PairList>(getIntList(10));

    EXPECT_EQ(*ptr1, *ptr2);
    EXPECT_NE(ptr1,ptr2);
}

int main(int argc, char **argv)
{
    ::testing::FLAGS_gtest_repeat = 30;
    ::testing::FLAGS_gtest_shuffle = true;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}