#include "../../include/Core/ThreadsafeQueue.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <algorithm>

//This code is simple, not fast because its test code

std::vector<int> mergeVectors(const std::vector<int>& left_vector,const std::vector<int>& right_vector)
{
    std::vector<int> new_values;
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

class QueueRefs
{
protected:
    ThreadsafeQueue<int>& queue_;
    std::shared_future<void>& ready_;
    QueueRefs(ThreadsafeQueue<int>& queue,std::shared_future<void>& ready):
        queue_{queue},
        ready_{ready}
    {}
};

class QueuePusher:
    public QueueRefs
{
private:
    std::vector<int> values_to_insert;
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
    QueuePusher(ThreadsafeQueue<int>& queue,std::shared_future<void>& ready):
        QueueRefs(queue,ready),
        values_to_insert{},
        push_ready_list_{},
        push_done_list_{},
        pushes_nmbr_{0}
    {}
    void addValues(std::vector<int> values)
    {
        std::vector<int> new_values{mergeVectors(values,values_to_insert)};
        pushes_nmbr_ = new_values.size();
        values_to_insert.clear();
        values_to_insert = std::move(new_values);
        increasePushFutures();
    }
    void printValuesToInsert() const
    {
        std::cout << "Elements : \n";
        for(auto e: values_to_insert)
            std::cout << e << " ";
        std::cout << std::endl;
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
                                            ready_.wait();
                                            queue_.push(values_to_insert[i]);
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

class QueueTryPopPtr:
    public QueueRefs
{
private:
    std::vector<std::promise<void>> try_pop_ptr_ready_list_;
    std::vector<std::future<std::shared_ptr<int>>> try_pop_ptr_done_list_;
private:
void increaseTryPopPtrFutures(size_t n)
{
    try_pop_ptr_ready_list_.resize(n);
    try_pop_ptr_done_list_.resize(n);
}
public:
    QueueTryPopPtr(ThreadsafeQueue<int>& queue,std::shared_future<void>& ready):
        QueueRefs(queue, ready),
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
                                        ready_.wait();
                                        return queue_.tryPop();
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

class QueueTryPopRef:
    public QueueRefs
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
    QueueTryPopRef(ThreadsafeQueue<int>& queue,std::shared_future<void>& ready):
        QueueRefs(queue, ready),
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
                                        int x;
                                        try_pop_ref_ready_list_[i].set_value();
                                        ready_.wait();
                                        pred =  queue_.tryPop(x);
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
class ThreadsafeQueueTest:
    public testing::Test
{
protected:
    std::shared_ptr<ThreadsafeQueue<int>> queue_;
    std::promise<void> go_;
    std::shared_future<void> ready_;

    QueuePusher pusher_;
    QueueTryPopPtr try_pop_ptr_;
    QueueTryPopRef try_pop_ref_;

    ThreadsafeQueueTest():
        queue_{new ThreadsafeQueue<int>},
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
    void setPushValues(std::vector<int> values)
    {
        pusher_.addValues(values);
        pusher_.printValuesToInsert();
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

TEST_F(ThreadsafeQueueTest,TryPopTest)
{
    std::vector<int> v;
    for (size_t i = 0; i < 1000; i++)
        v.push_back(i);
    EXPECT_NO_THROW(setPushValues(v));
    EXPECT_NO_THROW(setTryPopPtrInvocations(300));
    EXPECT_NO_THROW(setTryPopRefInvocations(400));
    EXPECT_NO_THROW(runTest());
    EXPECT_NO_THROW(clearQueue());
    EXPECT_TRUE(queue_->empty());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}