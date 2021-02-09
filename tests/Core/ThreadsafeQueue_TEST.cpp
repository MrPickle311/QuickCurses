#include "../../include/Core/ThreadsafeQueue.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <type_traits>

//This code is simple, not fast because its test code

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
template<typename StoredType,typename MethodReturnedType>
class QueueTester_base
{
public:
    //Main flags,comman data
    ThreadsafeQueue<StoredType>& queue_;
    std::shared_future<void>& ready_;

    //Action specified lists
    std::vector<std::promise<void>> action_ready_list_;
    std::vector<std::future<MethodReturnedType>> action_done_list_;

    QueueTester_base(ThreadsafeQueue<StoredType>& queue,std::shared_future<void>& ready):
        queue_{queue},
        ready_{ready},
        action_ready_list_{},
        action_done_list_{}
    {}
    virtual void setFuturesNumber(size_t number)
    {
        action_ready_list_.resize(number);
        action_done_list_.resize(number);
    }
    virtual ~QueueTester_base() {}
};

//i need to know types returned by queue
//template-method
template<typename StoredType,typename MethodReturnedType>
class QueueTesterEnabler
{
private:
    QueueTester_base<StoredType,MethodReturnedType>& base_ref_;
protected:
    virtual void printData() const //hook
    {}
    virtual void prepareData() //hook
    {}
    virtual MethodReturnedType operation(size_t i) = 0; //required to implementation by the others
public:
    QueueTesterEnabler(QueueTester_base<StoredType,MethodReturnedType>& base_ref):
        base_ref_{base_ref}
    {}
    void enable(size_t iterations_nmbr)
    {
        try
        {
            printData(); 
            for(size_t i{0}; i < iterations_nmbr; ++i)
            {
                base_ref_.action_done_list_[i] = std::async(std::launch::async,
                                        [this,i]
                                        {
                                            prepareData();
                                            base_ref_.action_ready_list_[i].set_value(); 
                                            base_ref_.ready_.wait(); 
                                            return operation(i);
                                        }
                                    );
            }
        }
        catch(const std::exception& e)
        {
            throw;
        }
    }
};

//zastanów się nad metodą szablonową dla wszystkich try-catchy!
//ale to jak skończę najpierw to 

template<typename StoredType,typename MethodReturnedType>
class PendingObject
{
private:
    QueueTester_base<StoredType,MethodReturnedType>& base_ref_;
public:
    PendingObject(QueueTester_base<StoredType,MethodReturnedType>& base_ref):
        base_ref_{base_ref}
    {}
    void wait()
    {
        try
        {
            for(auto& x: base_ref_.action_ready_list_)
                x.get_future().wait();
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
            for(auto& x: base_ref_.action_done_list_)
                x.get();
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
    public QueueTesterEnabler<T,void>
{
    using PObject = PendingObject<T,void>;
    using Enabler = QueueTesterEnabler<T,void>;
private:
    std::vector<T> values_to_insert;
    size_t pushes_nmbr_;

    QueueTester_base<T,void> base_;
protected:
    virtual void operation(size_t i)
    {
        base_.queue_.push(values_to_insert[i]);
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
        base_.setFuturesNumber(pushes_nmbr_);
    } 
    void clearValues()
    {
        values_to_insert.clear();
    }
};

template<typename T>
class QueueTryPopPtr:
    public PendingObject<T,std::shared_ptr<T>>,
    public QueueTesterEnabler<T,std::shared_ptr<T>>
{
    using PObject = PendingObject<T,std::shared_ptr<T>>;
    using Enabler = QueueTesterEnabler<T,std::shared_ptr<T>>;
private:
    QueueTester_base<T,std::shared_ptr<T>> base_;
protected:
    virtual std::shared_ptr<T> operation(size_t i)
    {
        return base_.queue_.tryPop();
    }
public:
    QueueTryPopPtr(ThreadsafeQueue<T>& queue,std::shared_future<void>& ready):
        PObject{base_},
        Enabler{base_},
        base_{queue, ready}
    {}
};

template<typename T>
class QueueTryPopRef:
    public PendingObject<T,bool>,
    public QueueTesterEnabler<T,bool>
{
    using PObject = PendingObject<T,bool>;
    using Enabler = QueueTesterEnabler<T,bool>;
private:
    QueueTester_base<T,bool> base_;
protected:
    virtual bool operation(size_t i)
    {
        T x;//ZMIENIC! , przeciążyć hooka!
        return base_.queue_.tryPop(x);
    }
public:
    QueueTryPopRef(ThreadsafeQueue<T>& queue,std::shared_future<void>& ready):
        PObject{base_},
        Enabler{base_},
        base_{queue, ready}
    {}
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
        catch(const std::exception& e)
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
    std::vector<TypeParam> v;
    if constexpr (std::is_same<TypeParam,int>::value)
    {
        for (size_t i = 0; i < 1000; i++)
           v.push_back(i);
    }
    else if constexpr (std::is_same<TypeParam,std::string>::value)
    {
        for (size_t i = 0; i < 1000; i++)
           v.push_back("str");
    }
    else if constexpr (std::is_same<TypeParam,std::vector<std::pair<int,std::string>>>::value)
    {
        std::pair<int,std::string> p{50,"str"};
        std::vector<std::pair<int,std::string>> v1;
        for (size_t i = 0; i < 1000; i++)
            v1.push_back(p);
        for (size_t i = 0; i < 1000; i++)
           v.push_back(v1);
    }
    EXPECT_NO_THROW(this->setPushValues(v););
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