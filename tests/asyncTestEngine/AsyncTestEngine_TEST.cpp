#include <gtest/gtest.h>
#include <future>
#include "AsyncTestEngine.hpp"
#include "../../include/Core/ThreadsafeQueue.hpp"


template<typename T>
class DataStorage
{
private:
    std::vector<T> data_;
public:
    T getDataAtIndex(size_t index) const
    {
        if(index >= data_.size())
            return data_[index];
        else throw std::out_of_range{"Vector index out of range"};
    }
    std::vector<T> getRangeOfData() const
    {
        return data_;
    }
    void addData(std::vector<T> const & data) //casual setter
    {
        data_ = data;
    }
};

template<typename T>
class QueueLittlePushTest:
    public TestedOperation<ThreadsafeQueue<T>>,
    public DataStorage<T>
{
protected:
    virtual void operation(size_t operation_current_iteration) // now operation_number it's even handy
    {
        T data_to_push = getDataAtIndex(operation_current_iteration);
        System.getTestedObject().push(data_to_push);
    }
public:
    QueueLittlePushTest(System& system_ref):
        TestedOperation<ThreadsafeQueue<T>>{system_ref},
        DataStorage<T>{}
    {}
};

template<typename T>
class QueueBigPushTest:
    public TestedOperation<ThreadsafeQueue<T>>,
    public DataStorage<T>
{
protected:
    virtual void operation(size_t operation_current_iteration) //but not here, im pushing all data at once
    {
        for(auto& element : getRangeOfData())
            System::getTestedObject().push(element);
    }
    virtual void delay() const
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
public:
    QueueBigPushTest(System& system_ref):
        TestedOperation<ThreadsafeQueue<T>>{system_ref}
    {}
};

template<typename T>
class QueueTryPopRefTest:
    public TestedOperation<ThreadsafeQueue<T>>
{
private:
    std::vector<T> data_from_queue_;
protected:
    virtual void operation(size_t operation_current_iteration) 
    {
        T temp;
        System::getTestedObject().tryPop(temp);
        data_from_queue_.push_back(temp);
    }
    virtual void workload()//before wait(), some initialization actions
    {
        size_t count {TestedOperation::getIterationsCount()};
        data_from_queue_.resize(count);
    }
public:
    QueueTryPopRefTest(System& system_ref):
        TestedOperation<ThreadsafeQueue<T>>{system_ref}
    {}
};

template<typename T>
class QueueClearWithWaitAndPopTest:
    public TestedOperation<ThreadsafeQueue<T>>
{
protected:
    virtual void operation(size_t operation_current_iteration) 
    {
        System::getTestedObject().clear();
        std::cout << "Current repetition: " << TestedOperation::getCurrentRepetition() << std::endl;
        System::getTestedObject().waitAndPop();
    }
public:
    QueueClearWithWaitAndPopTest(System& system_ref):
        TestedOperation<ThreadsafeQueue<T>>{system_ref}
    {}
};

template<typename T>
class QueueEmptyTest:
    public TestedOperation<ThreadsafeQueue<T>>
{
protected:
    virtual void operation(size_t operation_current_iteration) 
    {
        System::getTestedObject().empty();
    }
public:
    QueueEmptyTest(System& system_ref):
        TestedOperation<ThreadsafeQueue<T>>{system_ref}
    {}
};

template<typename T>
class ThreadsafeQueueTest:
    public AsyncTest<ThreadsafeQueue<T>>,
    public testing::Test
{
    using AsyncTest = AsyncTest<ThreadsafeQueue<T>>;
private:
    QueueLittlePushTest<T> little_push_test_;
    QueueBigPushTest<T> big_push_test_;
    QueueTryPopRefTest<T> try_pop_ref_test_;
    QueueClearWithWaitAndPopTest<T> clear_with_wait_and_pop_test_;
    QueueEmptyTest<T> empty_test_; // default in-loop iterations count is 1 
private:
    void setPushIterations(size_t iterations)
    {
        little_push_test_.setIterations(iterations);
        big_push_test_.setIterations(iterations);
    }
protected:
    ThreadsafeQueueTest():
        little_push_test_{AsyncTest::getAccessToSystem()},
        big_push_test_{AsyncTest::getAccessToSystem()},
        try_pop_ref_test_{AsyncTest::getAccessToSystem()},
        clear_with_wait_and_pop_test_{AsyncTest::getAccessToSystem()},
        empty_test_{AsyncTest::getAccessToSystem()}
    {}
    void setPushRepetitionsCount(size_t count)
    {
        little_push_test_.setRepetitionsCount(count);
        big_push_test_.setRepetitionsCount(count);
    }
    void setEmptyRepetitionsCount(size_t count)
    {
        empty_test_.setRepetitionsCount(count);
    }
    void setClearWithWaitAndPopRepetitionsCount(size_t count)
    {
        clear_with_wait_and_pop_test_.setRepetitionsCount(count);
    }
    void setTryPopRepetitionsCount(size_t count)
    {
        try_pop_ref_test_.setRepetitionsCount(count);
    }
    void setTryPopIterationsCount(size_t count)
    {
        try_pop_ref_test_.setIterationsCount(count);
    }
    void addDataset(std::vector<T> new_data)
    {
        little_push_test_.addData(new_data);
        big_push_test_.addData(new_data);
        setPushIterations(new_data.size());
    }
    virtual void runTest()//but by default it will not pure virtual
    {
        System::run();
    };
};

using MyTypes = testing::Types<int,std::string>;
TYPED_TEST_SUITE(ThreadsafeQueueTest,MyTypes);

TYPED_TEST(ThreadsafeQueueTest,FirstTest)
{
    std::vector<T> data_;
    for (size_t i = 0; i < 1000; ++i)
        data_.push_back(T{});
    
    EXPECT_NO_THROW(this->addDataset(data_));
    EXPECT_NO_THROW(this->setPushRepetitionsCount(2));
    EXPECT_NO_THROW(this->setEmptyRepetitionsCount(3));
    EXPECT_NO_THROW(this->setClearWithWaitAndPopRepetitionsCount(2));
    EXPECT_NO_THROW(this->setTryPopRepetitionsCount(3));
    EXPECT_NO_THROW(this->setTryPopIterationsCount(300));

    EXPECT_NO_THROW(this->runTest());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
