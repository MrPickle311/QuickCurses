#include "../../include/Core/ThreadsafeQueue.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <set>
#include <functional>

//This code is simple, not fast because its test code

//make specializations to print various types 
template<typename T>
struct Printer
{
    void operator() (const T& element)
    {
        std::cout << element << " ";
    }
};

template<>
struct Printer<std::vector<std::pair<int,std::string>>>
{
    void operator() (const std::vector<std::pair<int,std::string>>& element)
    {
        for(auto& e: element)
            std::cout << "( " << e.first << " , " << e.second << " ) ";
    }
};

template <typename T>
class DataGenerator
{
public:
    std::vector<T> generate(size_t count) const;
};

template<>
class DataGenerator<int>
{
public:
    std::vector<int> generate(size_t count) const
    {
        std::vector<int> v;
        v.resize(count);
        for (size_t i = 0; i < count; ++i)
           v.push_back(i);
        return v;
    }
};

template<>
class DataGenerator<std::string>
{
public:
    std::vector<std::string> generate(size_t count) const
    {
        std::vector<std::string> v;
        v.reserve(count);
        for (size_t i = 0; i < count; ++i)
           v.push_back("string data");
        return v;
    }
};

template<>
class DataGenerator<std::vector<std::pair<int,std::string>>>
{
public:
    std::vector<std::vector<std::pair<int,std::string>>> generate(size_t count) const
    {
        std::vector<std::vector<std::pair<int,std::string>>> v;
        v.reserve(count);
        for (size_t i = 0; i < count; i++)
        {
            std::vector<std::pair<int,std::string>> v1;
            v1.reserve(count / 4);
            for (size_t i = 0; i < count / 4; i++)
                v1.push_back(std::make_pair(4,std::to_string(i)));
            v.push_back(v1);
        }
        return v;
    }
};




template<typename T>
std::vector<T> mergeVectors(const std::vector<T>& left_vector,const std::vector<T>& right_vector)
{
    std::vector<T> new_values;
    size_t const new_size {left_vector.size() + right_vector.size()};
    new_values.resize(new_size);

    auto ptr = std::copy(left_vector.begin(), left_vector.end(),new_values.begin());
    std::copy(right_vector.begin(), right_vector.end(),ptr);
    new_values.shrink_to_fit();
    return new_values;
}

//all raw-data, direct access
//ROZDZIELIĆ I UOGÓLNIĆ
template<typename StoredType,typename MethodReturnedType>
class TestingBase
{
private:
    //Main flags,comman data
    ThreadsafeQueue<StoredType>& queue_;
    std::shared_future<void>& ready_;

    //Action specified lists
    std::vector<std::promise<void>> action_ready_list_;
    std::vector<std::future<MethodReturnedType>> action_done_list_;
public:
    TestingBase(ThreadsafeQueue<StoredType>& queue,std::shared_future<void>& ready):
        queue_{queue},
        ready_{ready},
        action_ready_list_{},
        action_done_list_{}
    {}
    virtual ~TestingBase() {}
    virtual void setFuturesNumber(size_t number)
    {
        action_ready_list_.resize(number);
        action_done_list_.resize(number);
    }
    std::promise<void>& readyPromise(size_t number)
    {
        return action_ready_list_[number];
    }
    std::future<MethodReturnedType>& doneFuture(size_t number)
    {
        return action_done_list_[number];
    }
    ThreadsafeQueue<StoredType>& queue()
    {
        return queue_;
    }
    std::shared_future<void>& readyIndicator()
    {
        return ready_;
    }
    size_t iterationsCount() const
    {
        if(action_done_list_.size() != action_done_list_.size())
            throw std::runtime_error("action_done_list_.size() != action_done_list_.size()");
        return action_done_list_.size();
    }
};

//i should add some configuration flags in by structure

//template-method
template<typename StoredType,typename MethodReturnedType>
class AsyncEnabler
{
private:
    TestingBase<StoredType,MethodReturnedType>& base_ref_;
protected:
    virtual void printData() const //hook
    {}
    virtual void prepareData() //hook
    {}
    virtual void workload() //hook
    {}
    virtual MethodReturnedType operation(size_t i) = 0; //required to implementation by the others
public:
    AsyncEnabler(TestingBase<StoredType,MethodReturnedType>& base_ref):
        base_ref_{base_ref}
    {}
    virtual ~AsyncEnabler() {}
    void enable(size_t iterations_nmbr)
    {
        try
        {
            base_ref_.setFuturesNumber(iterations_nmbr);
            printData(); 
            for(size_t i{0}; i < iterations_nmbr; ++i)
            {
                base_ref_.doneFuture(i) = std::async(std::launch::async,
                                        [this,i]
                                        {
                                            prepareData();
                                            base_ref_.readyPromise(i).set_value(); 
                                            base_ref_.readyIndicator().wait();
                                            workload(); 
                                            return operation(i);
                                        }
                                    );
            }
        }
        catch(...)
        {
            throw;
        }
    }
};

template<typename StoredType,typename MethodReturnedType>
class PendingObject
{
private:
    TestingBase<StoredType,MethodReturnedType>& base_ref_;
public:
    PendingObject(TestingBase<StoredType,MethodReturnedType>& base_ref):
        base_ref_{base_ref}
    {}
    virtual ~PendingObject() {}
    void wait()
    {
        try
        {
            for(size_t i{0} ; i < base_ref_.iterationsCount() ; ++i)
                base_ref_.readyPromise(i).get_future().wait();
        }
        catch(...)
        {
            throw;
        }
    }
    void getFuteres()
    {
        try
        {
            for(size_t i{0} ; i < base_ref_.iterationsCount() ; ++i)
                base_ref_.doneFuture(i).get();
        }
        catch(...)
        {
            throw;
        }
    }
};

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
    std::vector<T> values_from_queue_;
    std::mutex mutex_;
protected:
    virtual bool operation(size_t i)
    {
        Printer<T> printer;
        T x{};
        bool pred {base_.queue().tryPop(x)};
        std::lock_guard<std::mutex> lock{mutex_};
        if(pred) values_from_queue_.push_back(std::move(x));
        return pred;
    }
public:
    QueueTryPopRef(ThreadsafeQueue<T>& queue,std::shared_future<void>& ready):
        PObject{base_},
        Enabler{base_},
        base_{queue, ready},
        values_from_queue_{},
        mutex_{}
    {
        for(size_t i = 0; i < 1000; i++)
            values_from_queue_.push_back(T{});
    }   
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