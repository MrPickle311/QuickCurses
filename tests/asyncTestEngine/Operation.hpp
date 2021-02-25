#pragma once
#include "ObjectProxy.hpp"
#include <atomic>
#include <future>

class OptionalActions
{
public:
    virtual void prepareData() {}
    virtual void workload() {}
    virtual void delay() {}
};

class ExpectedActions
{
public:
    virtual void operation() = 0; //defined by user
};

class RepetitionInfo
{
    friend class OperationInfoUpdater;
private:
    std::atomic_uint64_t total_repetitions_count_;
    std::atomic_uint64_t current_repetition_;
public:
    RepetitionInfo():
        total_repetitions_count_{0},
        current_repetition_{0}
    {}
    size_t getTotalRepetitionsCount() const //do not show implementation details
    {
        return total_repetitions_count_;
    }
    void incrementCurrentRepetition()
    {
        if( ++current_repetition_ < total_repetitions_count_ )
            ++current_repetition_;
        else throw std::out_of_range{"RepetitionInfo : current_repetition_ >= total_repetitions_count_ ! "};
    }
    size_t getCurrentRepetition() const
    {
        return current_repetition_;
    }
    void setRepetitionsCount(size_t count)
    {
        total_repetitions_count_ = count;
    }
};

class IterationInfo
{
    friend class OperationInfoUpdater;
private:
    std::atomic_uint64_t current_iteration_;
    std::atomic_uint64_t total_iterations_nmbr_;
public:
    IterationInfo():
        current_iteration_{0}
    {}
    size_t getTotalIterationsCount() const
    {
        return total_iterations_nmbr_;
    }
    void setTotalIterationsCount(size_t count)
    {
        total_iterations_nmbr_ = count;
    } 
    size_t getCurrentIteration() const
    {
        return current_iteration_;
    }
    void incrementCurrentIteration()
    {
        ++current_iteration_;
    } 
    void resetCurrentIterations()
    {
        current_iteration_ = 0;
    }
};

//each operation will have 3 signals
//first for incrementCurrentRepetition()
//second for incrementCurrentIteration()
//thrid for resetCurrentIteration()

class OperationInfo:
    public IterationInfo,
    public RepetitionInfo
{};

//constructor of below class takes a connection in the constructor
template<typename TestedObject>
class TestedOperation:
    public OptionalActions,
    public ExpectedActions
{
    using Connections = ObjectProxy<TestedObject>&;
    template<typename TestedObject>
    friend class ConnectionMaker;
private:
    OperationInfo info_;
    ObjectProxy<TestedObject>& proxy_;
private:
    void setConnectionToSystem(ObjectProxy<TestedObject>& proxy)
    {
        proxy_ = proxy;
    }
public:
    TestedOperation():
        info_{},
        proxy_{}
    {}
    TestedObject& getTestedObject()
    {
        if (proxy_ != nullptr)
            return proxy_.getObject();
        else throw std::runtime_error("TestedOperation : operation not connected to the system!");
    }
};

template<typename TestedOperation>
using OperationPtr = std::shared_ptr<TestedOperation<TestedObject>>;