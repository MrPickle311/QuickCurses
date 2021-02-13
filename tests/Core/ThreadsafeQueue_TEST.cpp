#include "../../include/Core/ThreadsafeQueue.hpp"
#include <gtest/gtest.h>
#include <set>
#include "TestingGoodies.hpp"
#include "AsyncTestsEngine.hpp"

//This code is simple, not fast because its test code

template<typename T>
class QueuePusher:
    public TestedOperation<ThreadsafeQueue<T>,void>
{
    using Operation = TestedOperation<ThreadsafeQueue<T>, void>;
private:
    std::vector<T> values_to_insert_;
    size_t pushes_nmbr_;
    std::atomic_bool verbose_;
protected:
    virtual void operation(size_t i)
    {
        Operation::testedObject().push(values_to_insert_[i]);
    }
    virtual void printData() const
    {
        if(verbose_)
        {
            Printer<T> printer;
            std::cout << "Values push oto queue\n";
            for(auto& e : values_to_insert_)
                printer(e);
            std::cout << std::endl;
        }
    }
public:
    QueuePusher(ObjectWrapper<ThreadsafeQueue<T>>& wrapper,
        std::shared_future<void>& ready):
        Operation{wrapper,ready},
        values_to_insert_{},
        pushes_nmbr_{0},
        verbose_{true}
    {}
    virtual ~QueuePusher() {}
    void addValues(std::vector<T> values)
    {
        std::vector<T> new_values{mergeVectors(values,values_to_insert_)};
        pushes_nmbr_ = new_values.size();
        clearValues();
        values_to_insert_ = std::move(new_values);
        Operation::setInvocationsCount(values_to_insert_.size());
    } 
    void clearValues()
    {
        values_to_insert_.clear();
        values_to_insert_.shrink_to_fit();
    }
    void showData(bool p)
    {
        verbose_ = p;
    }
};

template<typename T>
class QueueTryPopPtr:
    public TestedOperation<ThreadsafeQueue<T>,std::shared_ptr<T>>
{
    using Operation = TestedOperation<ThreadsafeQueue<T>, std::shared_ptr<T>>;
protected:
    virtual std::shared_ptr<T> operation(size_t i)
    {
        return Operation::testedObject().tryPop();
    }
public:
    QueueTryPopPtr(ObjectWrapper<ThreadsafeQueue<T>>& wrapper,
        std::shared_future<void>& ready):
        Operation{wrapper,ready}
    {}
    virtual ~QueueTryPopPtr() {}
};

template<typename T>
class QueueTryPopRef:
    public TestedOperation<ThreadsafeQueue<T>, bool>
{
    using Operation = TestedOperation<ThreadsafeQueue<T>, bool>;
private:
    std::set<T> values_from_queue_;
    std::mutex mutex_;
protected:
    virtual bool operation(size_t i)
    {
        Printer<T> printer;
        T x{};
        bool pred {Operation::testedObject().tryPop(x)};
        std::lock_guard<std::mutex> lock{mutex_};
        if(pred) values_from_queue_.insert(std::move(x));
        return pred;
    }
public:
    QueueTryPopRef(ObjectWrapper<ThreadsafeQueue<T>>& wrapper,
        std::shared_future<void>& ready):
        Operation{wrapper,ready},
        values_from_queue_{},
        mutex_{}
    {}   
    virtual ~QueueTryPopRef() {}
    void printValuesFromQueue() const
    {
        std::cout << "Values grabbed from queue\n";
        Printer<T> printer;
        for(auto& e: values_from_queue_)
            printer(e);
    }
};

template<typename T>
class QueueClear:
    public TestedOperation<ThreadsafeQueue<T>, void>
{
    using Operation = TestedOperation<ThreadsafeQueue<T>, void>;
protected:
    virtual void operation(size_t i)
    {
        Operation::testedObject().clear();
    } 
    virtual void workload()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(4));
    }
public:
    QueueClear(ObjectWrapper<ThreadsafeQueue<T>>& wrapper,
        std::shared_future<void>& ready):
        Operation{wrapper,ready}
    {}
    virtual ~QueueClear() {}
};

template<typename T>
class QueueEmpty:
    public TestedOperation<ThreadsafeQueue<T>, bool>
{
    using Operation = TestedOperation<ThreadsafeQueue<T>, bool>;
protected:
    virtual void operation(size_t i)
    {
        Operation::testedObject().empty();
    } 
public:
    QueueEmpty(ObjectWrapper<ThreadsafeQueue<T>>& wrapper,
        std::shared_future<void>& ready):
        Operation{wrapper,ready}
    {}
    virtual ~QueueEmpty() {}
};

template<typename T>
class QueueWaitPopPtr:
    public TestedOperation<ThreadsafeQueue<T>,std::shared_ptr<T>>
{
    using Operation = TestedOperation<ThreadsafeQueue<T>, std::shared_ptr<T>>;
protected:
    virtual std::shared_ptr<T> operation(size_t i)
    {
        return Operation::testedObject().waitAndPop();
    }
public:
    QueueWaitPopPtr(ObjectWrapper<ThreadsafeQueue<T>>& wrapper,
        std::shared_future<void>& ready):
        Operation{wrapper,ready}
    {}
    virtual ~QueueWaitPopPtr() {}
};

template<typename T>
class QueueWaitPopRef:
    public TestedOperation<ThreadsafeQueue<T>,void>
{
    using Operation = TestedOperation<ThreadsafeQueue<T>, void>;
protected:
    virtual void operation(size_t i)
    {
        T x{};
        return Operation::testedObject().waitAndPop(x);
    }
public:
    QueueWaitPopRef(ObjectWrapper<ThreadsafeQueue<T>>& wrapper,
        std::shared_future<void>& ready):
        Operation{wrapper,ready}
    {}
    virtual ~QueueWaitPopRef() {}
};

template<typename T>
class AsyncThreadsafeQueueTest:
    public testing::Test,
    public AsyncTest<ThreadsafeQueue<T>>
{
    using AsyncBase = AsyncTest<ThreadsafeQueue<T>>;
protected:
    QueuePusher<T> pusher_;
    QueueTryPopPtr<T> try_pop_ptr_;
    QueueTryPopRef<T> try_pop_ref_;
    QueueClear<T> clear_;
    QueueWaitPopPtr<T> wait_pop_ptr_;
    QueueWaitPopRef<T> wait_pop_ref_;

    AsyncThreadsafeQueueTest():
        AsyncTest<ThreadsafeQueue<T>>{},
        pusher_{AsyncBase::getWrapper(),AsyncBase::getReadyIndicator()},
        try_pop_ptr_{AsyncBase::getWrapper(),AsyncBase::getReadyIndicator()},
        try_pop_ref_{AsyncBase::getWrapper(),AsyncBase::getReadyIndicator()},
        wait_pop_ptr_{AsyncBase::getWrapper(),AsyncBase::getReadyIndicator()},
        wait_pop_ref_{AsyncBase::getWrapper(),AsyncBase::getReadyIndicator()},
        clear_{AsyncBase::getWrapper(),AsyncBase::getReadyIndicator()}
    {}
    virtual ~AsyncThreadsafeQueueTest() {}
    virtual void wait() 
    {
        pusher_.wait();
        try_pop_ptr_.wait();
        try_pop_ref_.wait();
        wait_pop_ref_.wait();
        wait_pop_ptr_.wait();
        clear_.wait();
    }
    virtual void getFutures() 
    {
        pusher_.getFuteres();
    }
    void addValues(std::vector<T> values)
    {
        pusher_.addValues(values);
    }
    void tryPopPtr(size_t count)
    {
        try_pop_ptr_.setInvocationsCount(count);
    }
    void tryPopRef(size_t count)
    {
        try_pop_ref_.setInvocationsCount(count);
    }
    void clear(size_t count)
    {
        clear_.setInvocationsCount(count);
    }
    void waitPopPtr(size_t count)
    {
        wait_pop_ptr_.setInvocationsCount(count);
    }
    void waitPopRef(size_t count)
    {
        wait_pop_ref_.setInvocationsCount(count);
    }
    void setVerbose(bool verbose)
    {
        pusher_.showData(verbose);
    }
}; 



// TEST(SingleCoreThreadsafeQueueTest,InvariantQueueTest)
// {
//     ThreadsafeQueue<int> queue;
//     std::cout << "single start\n";
//     EXPECT_TRUE(queue.empty());
//     std::cout << "single start1\n";
//     EXPECT_NO_THROW(queue.tryPop());
//     std::cout << "single start2\n";
//     EXPECT_NO_THROW(queue.clear());
//     std::cout << "single start3\n";
//     queue.push(5);
//     queue.push(6);
//     queue.push(8);
//     queue.push(67);
//     int a;
//     queue.tryPop(a);
//     EXPECT_EQ(a,5);
//     queue.waitAndPop(a);
//     EXPECT_EQ(a,6);
//     queue.waitAndPop(a);
//     EXPECT_EQ(a,8);
//     queue.tryPop(a);
//     EXPECT_EQ(a,67);
//     std::cout << "single end\n";
// }

// using MyTypes = testing::Types<int,std::string>;
// TYPED_TEST_SUITE(AsyncThreadsafeQueueTest,MyTypes);

// // TYPED_TEST(AsyncThreadsafeQueueTest,QueueTestWithClear)
// // {
// //     this->setVerbose(false);
// //     DataGenerator<TypeParam> g;
// //     this->addValues(g.generate(1));

// //     EXPECT_NO_THROW(this->tryPopPtr(3));
// //     EXPECT_NO_THROW(this->tryPopRef(1));
// //     EXPECT_NO_THROW(this->clear(30));

// //     EXPECT_NO_THROW(this->runTest());
// // }

// TYPED_TEST(AsyncThreadsafeQueueTest,QueueTestWithoutClear)
// {
//     this->setVerbose(false);
//     DataGenerator<TypeParam> g;
//     this->addValues(g.generate(1));

//     //EXPECT_NO_THROW(this->tryPopPtr(600));//dodaj jesze 600x5000 w pętli
//     EXPECT_NO_THROW(this->tryPopRef(1));
//     //EXPECT_NO_THROW(this->waitPopPtr(60));
//     //EXPECT_NO_THROW(this->waitPopRef(60));

//     EXPECT_NO_THROW(this->runTest());
// }

int main(int argc, char **argv)
{
    ThreadsafeQueue<int> queue;
    
    std::promise<void> go;
    std::shared_future<void> ready{go.get_future()};

    std::vector<std::promise<void>> push_ready_list_;
    std::vector<std::future<void>> push_done_list_;

    std::vector<std::promise<std::string>> pop_ready_list_;
    std::vector<std::future<bool>> pop_done_list_;

    push_ready_list_.resize(5);
    push_done_list_.resize(5);
    pop_ready_list_.resize(5);
    pop_done_list_.resize(5);

    for (size_t i = 0; i < 5; i++)
    {
        push_done_list_[i] = std::async(std::launch::async,[&]
        {
            //wektory nie są atomowe ani wielowątkowe,więc wątki napierdalają się o to 
            //by tam na chama wjebać dane,szczególnie gdy wektorki zasuwają na
            //iteratorach
            push_ready_list_[i].set_value();
            ready.wait();
            queue.push(5);
        });
        pop_done_list_[i] = std::async(std::launch::async,[&]
        {
            int a;
            pop_ready_list_[i].set_value("xd");
            ready.wait();
            return queue.tryPop(a);
        });
    }


    for (size_t i = 0; i < 5; i++)
    {
        push_ready_list_[i].get_future().wait();
        pop_ready_list_[i].get_future().wait();
    }
    go.set_value();
    for (size_t i = 0; i < 5; i++)
    {
        push_done_list_[i].get();
        pop_done_list_[i].get();
    }
    //::testing::InitGoogleTest(&argc, argv);
    //return RUN_ALL_TESTS();
}