#pragma once

#include <mutex>
#include <memory>
#include <algorithm>

template<typename T>
class ThreadsafeList
{
private:
    struct Node
    {
        mutable std::mutex mutex_;
        std::shared_ptr<T> data_;
        std::unique_ptr<Node> next_;

        Node():
            next_{nullptr}
        {}

        Node(T const& data):
            data_{std::make_shared<T>(data)}
        {}
    };

    Node head_;

public:
    ThreadsafeList()
    {}

    ~ThreadsafeList()
    {
        remove_if([](Node const& )
        {
            return true;
        });
    }

    ThreadsafeList(ThreadsafeList const& other) = delete;
    ThreadsafeList& operator= (ThreadsafeList const&) = delete;

    //pushing locks only one mutex, so this operation will not cause deadlock
    void pushFront(T const& value)
    {
        std::unique_ptr<Node> new_node {new Node{value}};//allocation
        std::lock_guard<std::mutex>  lock{head_.mutex_};//lock head
        new_node->next_ = std::move(head_.next_);
        //put new_node to the top of the list
        head_.next = std::move(new_node);//head.next points to new_node
    }


    template<typename Function>// void(T)
    void forEach(Function function)
    {
        Node* current = &head_;
        std::unique_lock<std::mutex> lock{head_.mutex_};//lock head
        while(Node* const next {current->next_.get()})//don't take ownership,const pointer
        {
            std::unique_lock<std::mutex> next_lock{next->mutex_};//lock node mutex
            lock.unlock();// unlock previous node
            function(*next->data_);//do
            current = next;// update position to current position
            lock = std::move(next_lock);//hold lock ,but move ownership to lock variable 
        }
    }

    template<typename Predicate>// bool(T)
    std::shared_ptr<T> findFirstIf(Predicate predicate)
    {   
        Node* current = &head_;
        std::unique_lock<std::mutex> lock{head.mutex_};
        while(Node* const next {current->next_.get()})
        {
            std::unique_lock<std::mutex> next_lock{next->mutex_};
            lock.unlock();
            if(predicate(*next->data_))//impl same as forEach
                return next->data_;
            current = next;
            lock = std::move(next_lock);
        }
        return std::shared_ptr<T>{};
    }

    template<typename Predicate>
    void removeIf(Predicate predicate)
    {
        Node* current = &head_;
        std::unique_lock<std::mutex> lock{head.mutex_};
        while(Node* const next {current->next_.get()})
        {
            std::unique_lock<std::mutex> next_lock{next->mutex_};
            if(predicate(*next->data_))//found
            {
                //current->next points to same address like next
                std::unique_ptr<Node> old_next = std::move(current->next_);
                current->next = std::move(next->next_); //rebinding
                next_lock.unlock();
            }//at the end of the range old_next data will be deleted
            else //carry on browsing
            {
                lock.unlock();
                current = next;
                lock = std::move(next_lock);
            }
        }
    }

    //TODO:
    //make double linked list
    //moving nodes left <-> right
    //
    
};