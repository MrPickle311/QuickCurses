#include "../../include/Core/ThreadsafeQueue.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <list>
#include <vector>

class ThreadsafeQueueTest:
    public testing::Test
{
protected:
    
};


//concurrent test-template , thanks to Anthony Williams
template<typename T>
void test_concurrent_queue_tryPop(size_t tasks_nmbr,
                                              std::vector<T> values_to_insert)
{
    std::cout << "Start!\n";
    
    std::cout << "Elements : \n";
    for(auto e: values_to_insert)
        std::cout << e << " ";

    if(tasks_nmbr != values_to_insert.size())
    {
        std::cout << "Tasks number not equal to number of values to insert!,throwing...";
        throw std::exception{};
    }
    else std::cout << "\nSize OK";

    ThreadsafeQueue<T> queue;

    std::promise<void> go;
    std::shared_future<void> ready{go.get_future()};

    std::vector<std::promise<void>> push_ready_list;
    std::vector<std::promise<void>> pop_ready_list;

    std::vector<std::future<void>> push_done_list;
    std::vector<std::future<std::shared_ptr<T>>> pop_done_list;
    push_done_list.resize(tasks_nmbr);
    pop_done_list.resize(tasks_nmbr);
    try
    {
        for(size_t i{0}; i < tasks_nmbr; ++i)
        {
            push_ready_list.push_back(std::promise<void>{});
            pop_ready_list.push_back(std::promise<void>{});

            push_done_list[i] = std::async(std::launch::async,
                                [&queue,&ready,&push_ready_list,&values_to_insert,i]
                                {
                                    push_ready_list[i].set_value();
                                    ready.wait();
                                    queue.push(values_to_insert[i]);
                                }
                            );
            pop_done_list[i] = std::async(std::launch::async,
                                [&queue,&ready,&pop_ready_list,i]
                                {
                                    pop_ready_list[i].set_value();
                                    ready.wait();
                                    return queue.tryPop();
                                }
                            );
        }
        for(auto& p: push_ready_list)
            p.get_future().wait();
        for(auto& p: pop_ready_list)
            p.get_future().wait();
        go.set_value();

        for(size_t i{0}; i < tasks_nmbr; ++i)
            push_done_list[i].get();
        std::cout << std::endl;  
        for(size_t i{0}; i < tasks_nmbr; ++i)
        {
            std::shared_ptr<T> ptr = pop_done_list[i].get();
            if(ptr != nullptr)
                std::cout << *ptr << std::endl;            
        }
        std::cout << std::endl << std::endl;  
        while(!queue.empty())
        {
            T a;
            queue.tryPop(a);
            std::cout << a << std::endl;
        }
        if(!queue.empty())
            throw std::runtime_error("Queue is not empty");
    }
    catch(...)
    {
        go.set_value();
        throw;
    }
}

template<typename T>
void test_concurrent_queue_waitAndPop(size_t tasks_nmbr,
                                        std::vector<T> values_to_insert)
{
    std::cout << "Start!\n";
    
    std::cout << "Elements : \n";
    for(auto e: values_to_insert)
        std::cout << e << " ";

    if(tasks_nmbr != values_to_insert.size())
    {
        std::cout << "Tasks number not equal to number of values to insert!,throwing...";
        throw std::exception{};
    }
    else std::cout << "\nSize OK";

    ThreadsafeQueue<T> queue;

    std::promise<void> go;
    std::shared_future<void> ready{go.get_future()};

    std::vector<std::promise<void>> push_ready_list;
    std::vector<std::promise<void>> pop_ready_list;

    std::vector<std::future<void>> push_done_list;
    std::vector<std::future<std::shared_ptr<T>>> pop_done_list;
    push_done_list.resize(tasks_nmbr);
    pop_done_list.resize(tasks_nmbr);
    try
    {
        for(size_t i{0}; i < tasks_nmbr; ++i)
        {
            push_ready_list.push_back(std::promise<void>{});
            pop_ready_list.push_back(std::promise<void>{});

            push_done_list[i] = std::async(std::launch::async,
                                [&queue,&ready,&push_ready_list,&values_to_insert,i]
                                {
                                    push_ready_list[i].set_value();
                                    ready.wait();
                                    queue.push(values_to_insert[i]);
                                }
                            );
            pop_done_list[i] = std::async(std::launch::async,
                                [&queue,&ready,&pop_ready_list,i]
                                {
                                    pop_ready_list[i].set_value();
                                    ready.wait();
                                    return queue.waitAndPop();
                                }
                            );
        }
        for(auto& p: push_ready_list)
            p.get_future().wait();
        for(auto& p: pop_ready_list)
            p.get_future().wait();
        go.set_value();

        for(size_t i{0}; i < tasks_nmbr; ++i)
            push_done_list[i].get();
        std::cout << std::endl;  

        for(size_t i{0}; i < tasks_nmbr; ++i)
        {
            std::shared_ptr<T> ptr = pop_done_list[i].get();
            if(ptr != nullptr)
                std::cout << *ptr << std::endl;
            else throw std::runtime_error("ptr is nullptr!");            
        }
        std::cout << std::endl << std::endl;  
        if(!queue.empty())
            throw std::runtime_error("Queue is not empty");
    }
    catch(...)
    {
        go.set_value();
        throw;
    }

}

TEST(TEST,FirstTest)
{
    ThreadsafeQueue<int> queue;
    queue.push(5);
    queue.push(6);
    int a;
    queue.tryPop(a);
    EXPECT_EQ(a,5);
}

TEST(TEST,SecondTest)
{
    size_t COUNT = 200;
    std::vector<int> v;
    for (int i = 0; i < COUNT;++i)
        v.push_back(i);
    EXPECT_NO_THROW(test_concurrent_queue_tryPop(COUNT,v));
    EXPECT_NO_THROW(test_concurrent_queue_waitAndPop<int>(COUNT,v));


}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}