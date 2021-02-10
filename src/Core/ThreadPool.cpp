#include "../../include/Core/ThreadPool.hpp"

//ThreadJoiner

ThreadJoiner::ThreadJoiner(std::vector<std::thread>& threads):
    threads_{threads_}
{}

ThreadJoiner::~ThreadJoiner()
{
    for(auto& thread : threads_)
        if(thread.joinable())
            thread.join();
}

//FunctionWrapper

FunctionWrapper::FunctionWrapper(FunctionWrapper&& wrapper):
    implementation_{std::move(wrapper.implementation_)}
{}

void FunctionWrapper::operator() ()
{
    implementation_->call();
}

FunctionWrapper& FunctionWrapper::operator=(FunctionWrapper&& wrapper)
{
    this->implementation_ = std::move(wrapper.implementation_);
    return *this;
}

//WorkStealingQueue

StealingQueue::StealingQueue()
{}

void StealingQueue::push(DataType data)
{
    std::lock_guard<std::mutex> lock{mutex_};
    tasks_.push_front(std::move(data));
}

bool StealingQueue::empty() const
{
    std::lock_guard<std::mutex> lock{mutex_};
    return tasks_.empty();
}

bool StealingQueue::tryPop(DataType& data)
{
    std::lock_guard<std::mutex> lock{mutex_};
    if(tasks_.empty())
        return false;
    
    data = std::move(tasks_.front());
    tasks_.pop_front();
    return true;
}

bool StealingQueue::trySteal(DataType& data)
{
    std::lock_guard<std::mutex> lock{mutex_};
    if(tasks_.empty())
        return false;
    
    data = std::move(tasks_.back());
    tasks_.pop_back();
    return true;
}

//ThreadPool

//public
ThreadPool::ThreadPool():
        done_{false},
        joiner_{threads_}
    {
        size_t const thread_count {std::thread::hardware_concurrency()};

        try
        {
            for(size_t i = 0; i < thread_count; ++i)
                queues_.push_back(std::unique_ptr<StealingQueue> {new StealingQueue});
            for(size_t i = 0; i < thread_count; ++i)
                threads_.push_back(std::thread{&ThreadPool::workerThread,this,i});
        }   
        catch(...)
        {
            done_ = true;
			throw;
        }
    }

ThreadPool::~ThreadPool()
    {
        done_ = true;
    }

void ThreadPool::runPendingTask()
{
    TaskType task;
    if(popTaskFromLocalQueue(task) ||
        popTaskFromGlobalQueue(task) ||
        popTaskFromOtherQueue(task))
        task();
    else std::this_thread::yield();
}

//private

void ThreadPool::workerThread(size_t my_index)
{
        my_index_ = my_index;
        local_queue_ = queues_[my_index].get();
        while (!done_)//while !done , search new tasks to do
            runPendingTask();
}

bool ThreadPool::popTaskFromLocalQueue(TaskType& task)
{
    return local_queue_ && local_queue_->tryPop(task);
}

bool ThreadPool::popTaskFromGlobalQueue(TaskType& task)
{
    return global_queue_.tryPop(task);
}

bool ThreadPool::popTaskFromOtherQueue(TaskType& task)
{
    for (size_t i {0}; i < queues_.size(); ++i)
    {
        size_t const index = (my_index_ + 1) % queues_.size(); //just shuffle
        if(queues_[index]->trySteal(task))
        return true;
    }   
}

