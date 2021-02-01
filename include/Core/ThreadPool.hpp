#pragma once
#include <thread>
#include <queue>
#include <mutex>
#include <future>
#include <condition_variable>
#include <atomic>

class ThreadJoiner
{
private:
    std::vector<std::thread>& threads_;
public:
    explicit ThreadJoiner(std::vector<std::thread>& threads);
    ~ThreadJoiner();
};

class FunctionWrapper
{
private:
    struct ImplementationBase
    {
        virtual void call() = 0;
        virtual ~ImplementationBase() {}
    };

    std::unique_ptr<ImplementationBase> implementation_;

    template<typename PackagedTaskType>
    struct ImplementationType:
        public ImplementationBase
    {
        PackagedTaskType func_;
        ImplementationType(PackagedTaskType&& func_):
            func_{std::move(func_)}
        {}
        void call()
        {
            func_();
        }
    };
public:
    FunctionWrapper() = default;

    template<typename F>
    FunctionWrapper(F&& f):
        implementation_{new ImplementationType{std::move(f)}}
    {}

    FunctionWrapper(FunctionWrapper&& wrapper);

    void operator() ();

    FunctionWrapper& operator=(FunctionWrapper&& wrapper);

    FunctionWrapper(const FunctionWrapper&) = delete;
    FunctionWrapper(FunctionWrapper&) = delete;
    FunctionWrapper& operator=(const FunctionWrapper&) = delete;
};

class StealingQueue
{
    using DataType = FunctionWrapper;
private:
    std::deque<DataType> tasks_;
    mutable std::mutex mutex_;
public:
    StealingQueue();
    StealingQueue(const StealingQueue&) = delete;
    StealingQueue& operator=(const StealingQueue&) = delete;

    void push(DataType data);
    bool empty() const;
    bool tryPop(DataType& data);
    bool trySteal(DataType& data);
};

class ThreadPool
{
    using TaskType = FunctionWrapper;
    using LocalQueueType = std::queue<TaskType>;
private:
    std::atomic_bool done_;
    
};

