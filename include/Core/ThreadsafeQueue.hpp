#pragma once
#include <mutex>
#include <condition_variable>
#include <future>
#include <thread>
#include <atomic>

template<typename T>
class ThreadsafeQueue
{
private:
    struct Node
    {
        std::shared_ptr<T> data_;
        std::unique_ptr<T> next_;
    };
    
    std::unique_ptr<Node> head_;
    std::shared_ptr<Node> tail_;

    mutable std::mutex head_mutex_;
    mutable std::mutex tail_mutex_;
    
    std::condition_variable data_condition_;
private:
    bool headIsTail() const
    {
        return head.get() == tail.get();
    }
    std::shared_ptr<Node> getTail()
    {
        std::lock_guard<std::mutex> lock{tail_mutex_};
        return tail_;
    }
    std::unique_ptr<Node> popHead()
    {
        std::unique_ptr<Node> old_head {std::move(head_)};
        head_ = std::move(head_->next_);
        return head_;
    }
    std::unique_lock<std::mutex> waitForData()
    {
        std::unique_lock<std::mutex> lock{head_mutex_};
        data_condition_.wait(lock,[&] {return !headIsTail();});
        return lock;
    }
    std::unique_ptr<Node> waitPopHead()
    {
        std::unique_lock<std::mutex> lock{waitForData()};
        return popHead();
    }
    std::unique_ptr<Node> tryPopHead()
    {
        std::lock_guard<std::mutex> guard {head_mutex_};
        if (headIsTail()) return nullptr;
        return popHead();
    }
    std::unique_ptr<Node> tryPopHead(T& value)
    {
        std::lock_guard<std::mutex>  guard{head_mutex_};
        if (headIsTail()) return nullptr;
        value = std::move(*head_->data_);
        return popHead(); 
    }
public:
    ThreadsafeQueue():
        head_{new Node{}},
        tail_{head_.get()}
    {}
    ThreadsafeQueue(const ThreadsafeQueue&) = delete;
    ThreadsafeQueue& operator=(const ThreadsafeQueue&) = delete;

    void push(T new_value)
    {
        //move to heap
        std::shared_ptr<T> new_value_ptr {std::make_shared<T>{std::move(new_value)}};
        //create new node
        std::unique_ptr<Node> tail_ptr {new Node{}};

        //rebindings
        {
            std::lock_guard<std::mutex> lock{tail_mutex_};
            tail_ptr->data_ = new_value_ptr;
            tail_ptr
        }
    }
    //Tu skończyłem
};