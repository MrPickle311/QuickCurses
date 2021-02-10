#pragma once
#include <queue>
#include "ThreadsafeQueue.hpp"

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
    ThreadsafeQueue<TaskType> global_queue_;
    std::vector<std::unique_ptr<StealingQueue>> queues_;
    std::vector<std::thread> threads_;
    ThreadJoiner joiner_;

    inline static thread_local StealingQueue* local_queue_;
    inline static thread_local size_t my_index_;
private:
    void workerThread(size_t my_index);
    bool popTaskFromLocalQueue(TaskType& task);
    bool popTaskFromGlobalQueue(TaskType& task);
    bool popTaskFromOtherQueue(TaskType& task);
public:
    //DODAĆ RĘCZNY WYBÓR ILOŚCI WĄTKÓW
    ThreadPool();
    virtual ~ThreadPool();
    void runPendingTask();
    template<typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type> submit(FunctionType new_task)
    {
        typedef typename std::result_of<FunctionType()>::type ResultType;

        std::packaged_task<ResultType()> task{std::move(new_task)};

        std::future<ResultType> res{task.get_future()};

        if(local_queue_) local_queue_->push(std::move(task));
        else global_queue_->push(std::move(task));
    }
};

