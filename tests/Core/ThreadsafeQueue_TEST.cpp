#include "../../include/Core/ThreadsafeQueue.hpp"
#include <gtest/gtest.h>
#include <set>
#include "TestingGoodies.hpp"
#include "AsyncTestsEngine.hpp"

//This code is simple, not fast because its test code

//queue.push() returns void
template<typename T>
class QueuePusher:
    public PendingObject<T,void>,
    public AsyncEnabler<T,void>
{
    using PObject = PendingObject<T,void>;
    using Enabler = AsyncEnabler<T,void>;
private:
    std::vector<T> values_to_insert;
    size_t pushes_nmbr_;

    TestingBase<T,void> base_;
protected:
    virtual void operation(size_t i)
    {
        base_.queue().push(values_to_insert[i]);
    }
    virtual void printData() const
    {
        Printer<T> printer;
        std::cout << "Values push oto queue\n";
        for(auto& e : values_to_insert)
            printer(e);
        std::cout << std::endl;
    }
public:
    QueuePusher(ThreadsafeQueue<T>& queue,std::shared_future<void>& ready):
        PObject{base_},
        Enabler{base_},
        base_{queue,ready},
        values_to_insert{},
        pushes_nmbr_{0}
    {}
    virtual ~QueuePusher() {}
    void addValues(std::vector<T> values)
    {
        std::vector<T> new_values{mergeVectors(values,values_to_insert)};
        pushes_nmbr_ = new_values.size();
        values_to_insert.clear();
        values_to_insert = std::move(new_values);
    } 
    void clearValues()
    {
        values_to_insert.clear();
    }
};

template<typename T>
class QueueTryPopPtr:
    public PendingObject<T,std::shared_ptr<T>>,
    public AsyncEnabler<T,std::shared_ptr<T>>
{
    using PObject = PendingObject<T,std::shared_ptr<T>>;
    using Enabler = AsyncEnabler<T,std::shared_ptr<T>>;
private:
    TestingBase<T,std::shared_ptr<T>> base_;
protected:
    virtual std::shared_ptr<T> operation(size_t i)
    {
        return base_.queue().tryPop();
    }
public:
    QueueTryPopPtr(ThreadsafeQueue<T>& queue,std::shared_future<void>& ready):
        PObject{base_},
        Enabler{base_},
        base_{queue, ready}
    {}
    virtual ~QueueTryPopPtr() {}
};

template<typename T>
class QueueTryPopRef:
    public PendingObject<T,bool>,
    public AsyncEnabler<T,bool>
{
    using PObject = PendingObject<T,bool>;
    using Enabler = AsyncEnabler<T,bool>;
private:
    TestingBase<T,bool> base_;
    std::set<T> values_from_queue_;
    std::mutex mutex_;
protected:
    virtual bool operation(size_t i)
    {
        Printer<T> printer;
        T x{};
        bool pred {base_.queue().tryPop(x)};
        std::lock_guard<std::mutex> lock{mutex_};
        if(pred) values_from_queue_.insert(std::move(x));
        return pred;
    }
public:
    QueueTryPopRef(ThreadsafeQueue<T>& queue,std::shared_future<void>& ready):
        PObject{base_},
        Enabler{base_},
        base_{queue, ready},
        values_from_queue_{},
        mutex_{}
    {}   
    virtual ~QueueTryPopRef() 
    {

    }
    void printValuesFromQueue() const
    {
        std::cout << "Values grabbed from queue\n";
        Printer<T> printer;
        for(auto& e: values_from_queue_)
            printer(e);
    }
};

template<typename T>
class ThreadsafeQueueTest:
     public testing::Test
{
protected:
    std::shared_ptr<ThreadsafeQueue<T>> queue_;
    std::promise<void> go_;
    std::shared_future<void> ready_;

    QueuePusher<T> pusher_;
    QueueTryPopPtr<T> try_pop_ptr_;
    QueueTryPopRef<T> try_pop_ref_;

    ThreadsafeQueueTest():
        queue_{new ThreadsafeQueue<T>},
        go_{},
        ready_{go_.get_future()},
        pusher_{*queue_,ready_},
        try_pop_ptr_{*queue_,ready_},
        try_pop_ref_{*queue_,ready_}
    {}
    virtual ~ThreadsafeQueueTest() {}
    void waitForSetAll()
    {
        pusher_.wait();
        try_pop_ptr_.wait();
        try_pop_ref_.wait();
    }
    void setPushValues(std::vector<T> values)
    {
        pusher_.addValues(values);
        pusher_.enable(values.size());
    }
    void setTryPopPtrInvocations(size_t n)
    {
        try_pop_ptr_.enable(n);
    }
    void setTryPopRefInvocations(size_t n)
    {
        try_pop_ref_.enable(n);
    }
    void runTest()
    {
        try
        {
            waitForSetAll();
            go_.set_value();
            pusher_.getFuteres();
            try_pop_ptr_.getFuteres();
            try_pop_ref_.getFuteres();
        }
        catch(...)
        {
            go_.set_value();
            throw;
        }
    }
    void clearQueue()
    {
        queue_->clear();
    }
};

TEST(TEST,SimpleSingleThreadTest)
{
    ThreadsafeQueue<int> queue;
    queue.push(5);
    queue.push(6);
    queue.push(8);
    queue.push(67);
    int a;
    queue.tryPop(a);
    EXPECT_EQ(a,5);
    queue.waitAndPop(a);
    EXPECT_EQ(a,6);
    queue.waitAndPop(a);
    EXPECT_EQ(a,8);
    queue.tryPop(a);
    EXPECT_EQ(a,67);
}

using MyTypes = testing::Types<int,std::string,std::vector<std::pair<int,std::string>>>;
TYPED_TEST_SUITE(ThreadsafeQueueTest,MyTypes);

TYPED_TEST(ThreadsafeQueueTest,TryPopTest)
{
    DataGenerator<TypeParam> g;
    EXPECT_NO_THROW(this->setPushValues(g.generate(500)););
    EXPECT_NO_THROW(this->setTryPopPtrInvocations(300));
    EXPECT_NO_THROW(this->setTryPopRefInvocations(400));

    EXPECT_NO_THROW(this->runTest());

    EXPECT_NO_THROW(this->clearQueue());
    EXPECT_TRUE(this->queue_->empty());
    TypeParam checker;
    EXPECT_FALSE(this->queue_->tryPop(checker));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}