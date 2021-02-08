#pragma once
#include <mutex>
#include <condition_variable>
#include <future>
#include <thread>
#include <atomic>
#include <iostream>
template<typename T>
class ThreadsafeQueue
{
private:
    struct Node
    {
        std::shared_ptr<T> data_;
        std::unique_ptr<Node> next_;
    };
    
    std::unique_ptr<Node> head_;
    Node* tail_;//virtual node,so its fully safe

    mutable std::mutex head_mutex_;
    mutable std::mutex tail_mutex_;
    
    std::condition_variable data_condition_;
private:
    bool headIsTail() const
    {
        return head_.get() == tail_;
    }
    Node* getTail()
    {
        std::lock_guard<std::mutex> lock{tail_mutex_};
        return tail_;
    }
    std::unique_ptr<Node> popHead()
    {
        std::unique_ptr<Node> old_head {std::move(head_)};
        head_ = std::move(old_head->next_);
        return old_head;
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
    std::unique_ptr<Node> waitPopHead(T& value)
    {
        std::unique_lock<std::mutex> lock{waitForData()};
        value = std::move(*head_->data_);
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
        //move to the heap
        std::shared_ptr<T> new_value_ptr {std::make_shared<T>(std::move(new_value))};
        //create a new virtual node
        std::unique_ptr<Node> tail_ptr {new Node};

        //rebindings
        {
            //dangerous!
            Node* const temp_node = tail_ptr.get();
            //lock
            std::lock_guard<std::mutex> lock{tail_mutex_};
            //rebindings operations
            tail_->data_ = new_value_ptr;
            tail_->next_ = std::move(tail_ptr);
            tail_ = temp_node;
        }
        data_condition_.notify_one();
    }
    
    std::shared_ptr<T> waitAndPop()
    {
        std::unique_ptr<Node> const old_head {waitPopHead()};
        return old_head->data_;
    }
    void waitAndPop(T& value)
    {
        std::unique_ptr<Node> const old_head {waitPopHead(value)};
    }
    std::shared_ptr<T> tryPop()
    {
        std::unique_ptr<Node> old_head {tryPopHead()};
        return old_head ? old_head->data_ : nullptr;
    }
    bool tryPop(T& value)
    {
        std::unique_ptr<Node> const old_head {tryPopHead(value)};
        return old_head != nullptr;
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lock{head_mutex_};
        return headIsTail();
    }
    void clear()
    {
        std::lock_guard<std::mutex> lock{tail_mutex_};
        while(!headIsTail())
        {
            std::cout << "Deleting ...\n";
            tryPop();
        }
            
    }
};