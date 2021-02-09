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

void errorInfo(const std::exception& e)
{
    std::cout << "\n" << __LINE__ << "\n" << e.what() << std::endl;
}

template<typename T>
class QueueRefs
{
protected:
    ThreadsafeQueue<T>& queue_;
    std::shared_future<void>& ready_;
    QueueRefs(ThreadsafeQueue<T>& queue,std::shared_future<void>& ready):
        queue_{queue},
        ready_{ready}
    {}
    virtual ~QueueRefs() {}
};

template<typename T>
class QueuePusher:
    public QueueRefs<T>
{
private:
    std::vector<T> values_to_insert;
    std::vector<std::promise<void>> push_ready_list_;
    std::vector<std::future<void>> push_done_list_;

    size_t pushes_nmbr_;
private:
    void increasePushFutures()
    {  
        push_ready_list_.resize(pushes_nmbr_); 
        push_done_list_.resize(pushes_nmbr_);   
    }
public:
    QueuePusher(ThreadsafeQueue<T>& queue,std::shared_future<void>& ready):
        QueueRefs<T>(queue,ready),
        values_to_insert{},
        push_ready_list_{},
        push_done_list_{},
        pushes_nmbr_{0}
    {}
    void addValues(std::vector<T> values)
    {
        std::vector<T> new_values{mergeVectors(values,values_to_insert)};
        pushes_nmbr_ = new_values.size();
        values_to_insert.clear();
        values_to_insert = std::move(new_values);
        increasePushFutures();
    } 
    void clearValues()
    {
        values_to_insert.clear();
    }
    void enablePushing()
    {
        try
        {
            for(size_t i{0}; i < pushes_nmbr_; ++i)
            {
                push_done_list_[i] = std::async(std::launch::async,
                                        [this,i]
                                        {
                                            push_ready_list_[i].set_value();
                                            QueueRefs<T>::ready_.wait();
                                            QueueRefs<T>::queue_.push(values_to_insert[i]);
                                        }
                                    );
            }
        }
        catch(const std::exception& e)
        {
            errorInfo(e);
            throw;
        }
    }
    void waitForPush()
    {
        try
        {
            for(auto& x: push_ready_list_)
                x.get_future().wait();
        }
        catch(const std::exception& e)
        {
            errorInfo(e);
            throw;
        }
    }
    void getFuteres()
    {
        try
        {
            for(auto& x: push_done_list_)
                x.get();
        }
        catch(const std::exception& e)
        {
            errorInfo(e);
            throw;
        }   
    }
};

template<typename T>
class QueueTryPopPtr:
    public QueueRefs<T>
{
private:
    std::vector<std::promise<void>> try_pop_ptr_ready_list_;
    std::vector<std::future<std::shared_ptr<T>>> try_pop_ptr_done_list_;
private:
void increaseTryPopPtrFutures(size_t n)
{
    try_pop_ptr_ready_list_.resize(n);
    try_pop_ptr_done_list_.resize(n);
}
public:
    QueueTryPopPtr(ThreadsafeQueue<T>& queue,std::shared_future<void>& ready):
        QueueRefs<T>(queue, ready),
        try_pop_ptr_ready_list_{},
        try_pop_ptr_done_list_{}
    {}
    void enableTryPopPtr(size_t n)
    {
        increaseTryPopPtrFutures(n);
        try
        {
            for(size_t i{0}; i < n; ++i)
            {
                
                try_pop_ptr_done_list_[i] = std::async(std::launch::async,
                                    [this,i]
                                    {
                                        try_pop_ptr_ready_list_[i].set_value();
                                        QueueRefs<T>::ready_.wait();
                                        return QueueRefs<T>::queue_.tryPop();
                                    }
                                );
            }
        }
        catch(const std::exception& e)
        {
            errorInfo(e);
            throw;
        }
    }
    void waitForTryPopPtr()
    {
        try
        {
            for(auto& x: try_pop_ptr_ready_list_)
                x.get_future().wait();
        }
        catch(const std::exception& e)
        {
            errorInfo(e);
            throw;
        } 
    }
    void getFuteres()
    {
        try
        {
            for(auto& x: try_pop_ptr_done_list_)
                x.get();
        }
        catch(const std::exception& e)
        {
            errorInfo(e);
            throw;
        }
    }
};

template<typename T>
class QueueTryPopRef:
    public QueueRefs<T>
{
private:
    std::vector<std::promise<void>> try_pop_ref_ready_list_;
    std::vector<std::future<bool>> try_pop_ref_done_list_;
private:
    void increaseTryPopRefFutures(size_t n)
    {
        try_pop_ref_ready_list_.resize(n);
        try_pop_ref_done_list_.resize(n);
    }
public:
    QueueTryPopRef(ThreadsafeQueue<T>& queue,std::shared_future<void>& ready):
        QueueRefs<T>(queue, ready),
        try_pop_ref_ready_list_{},
        try_pop_ref_done_list_{}
    {}
    void enableTryPopRef(size_t n)
    {
        increaseTryPopRefFutures(n);
        try
        {
            for(size_t i{0}; i < n; ++i)
            {
                
                try_pop_ref_done_list_[i] = std::async(std::launch::async,
                                    [this,i]
                                    {
                                        bool pred;
                                        T x;
                                        try_pop_ref_ready_list_[i].set_value();
                                        QueueRefs<T>::ready_.wait();
                                        pred =  QueueRefs<T>::queue_.tryPop(x);
                                        return pred;
                                    }
                                );
            }
        }
        catch(const std::exception& e)
        {
            errorInfo(e);
            throw;
        }
    }
    void waitForTryPopRef()
    {
        try
        {
            for(auto& x: try_pop_ref_ready_list_)
                x.get_future().wait();
        }
        catch(const std::exception& e)
        {
            errorInfo(e);
            throw;
        } 
    }
    void getFuteres()
    {
        try
        {
            for(auto& x: try_pop_ref_done_list_)
                x.get();
        }
        catch(const std::exception& e)
        {
            errorInfo(e);
            throw;
        }
    }
};

//Facade
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
        pusher_.waitForPush();
        try_pop_ptr_.waitForTryPopPtr();
        try_pop_ref_.waitForTryPopRef();
    }
    void setPushValues(std::vector<T> values)
    {
        pusher_.addValues(values);
        pusher_.enablePushing();
    }
    void setTryPopPtrInvocations(size_t n)
    {
        try_pop_ptr_.enableTryPopPtr(n);
    }
    void setTryPopRefInvocations(size_t n)
    {
        try_pop_ref_.enableTryPopRef(n);
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
            errorInfo(e);
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