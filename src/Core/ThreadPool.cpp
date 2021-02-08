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

