#include <boost/signals2.hpp>
#include "ThreadsafeHashTable.hpp"

#include <future>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>

#define interface class 

//każde dołączenie jakiejś operacji generuje nowy slot w hash_table 
//każda operacja będzie mieć swój identyfikator oraz swój sygnał

//lub bardzo możliwe ,że wystarczy jeden sygnał na cały system
//tylko z odpowienim argumentem

using Table = ThreadsafeHashTable<size_t,size_t>;
using Signal = boost::signals2::signal<void ()>;
using Connection = boost::signals2::connection;

template<typename TestedObject>
class ObjectBuilder
{
public:
    template<typename ... Args>
    static TestedObject build(Args... args)//place here constructor arguements
    {
        return TestedObject{args...};
    }
};

template<typename TestedObject>
class ObjectProxy 
{
private:
    TestedObject obj_;
public:
    template<typename... Args>
    ObjectProxy(Args... args):
        obj_{ObjectBuilder<TestedObject>::build(args...)}
    {}
    virtual ~ObjectProxy() {}
    TestedObject& getObject()
    {
        return obj_;
    }
};

class OptionalActions
{
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
    size_t total_repetitions_count_;
    size_t current_repetition_;
    std::mutex repetition_mutex_;
public:
    RepetitionInfo():
        total_repetitions_count_{0},
        current_repetition_{0},
        repetition_mutex_{}
    {}
    size_t getRepetitionsCount() const
    {
        return total_repetitions_count_;
    }
    void incrementCurrentRepetition()
    {
        std::lock_guard<std::mutex> lock{repetition_mutex_};
        if(++current_repetition_ < getRepetitionsCount())
            ++current_repetition_;
        else throw std::out_of_range{"current_repetition_ >= total_repetitions_count_!"};
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
    std::mutex iteration_mutex_;
    size_t current_iteration_;
public:
    IterationInfo():
        iteration_mutex_{}
    {}
    size_t getCurrentIteration() const
    {
        return current_iteration_;
    }
    void incrementCurrentIteration()
    {
        std::lock_guard<std::mutex> lock{iteration_mutex_};
        ++current_iteration_;
    } 
    void resetCurrentIterations()
    {
        std::lock_guard<std::mutex> lock{iteration_mutex_};
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
    template<typename TestedObject>
    friend class ConnectionMaker;
private:
    size_t operation_id_;
    OperationInfo info_;
    ObjectProxy<TestedObject>& proxy_;
public:
    TestedObject& getTestedObject()
    {
        return proxy_.getObject();
    }
};

////////////////////////////////

//I need to make connection in class which is owner of the method 

class OperationInfoUpdater
{
private:
    OperationInfo& info_;
public:
    OperationInfoUpdater(OperationInfo& info):
        info_{info}
    {}
public:
    void resetIterations()
    {
        info_.resetCurrentIterations();
    }
    void invcrementIteration()
    {
        info_.incrementCurrentIteration();
    }
    void incrementRepetition()
    {
        info_.incrementCurrentRepetition();
    }
};


class ActionsInvoker
{
private:
    ExpectedActions& expected_actions_;
    OptionalActions& optional_actions_;
public:
    ActionsInvoker(ExpectedActions& expected_actions,
                   OptionalActions& optional_actions):
        expected_actions_{expected_actions},
        optional_actions_{optional_actions}
    {}
    void invokeOperation()
    {
        expected_actions_.operation();
    }
};

class OperationLoop
{
private:
    OperationInfoUpdater updater_;
    ActionsInvoker invoker_;
public:
    OperationLoop(OptionalActions& expected_actions_,
                  ExpectedActions& required_actions_,
                  OperationInfo&  info):
        updater_{info},

    {}
    void loop()
    {
        
    }
};

class OperationConnections
{
private:
     ActionsInvoker invoker_;
     OperationInfoUpdater updater_;
};

class SystemSharedResources
{
private:
    std::condition_variable system_guardian_;
};

class PendingObject
{
private:
    
};

class OperationExecuter
{

};

class SystemMonitor
{

};

class OperationManager
{
private:
    SystemSharedResources& resources_;
    OperationExecuter executer_;
    PendingObject pending_object_;
    OperationConnections connections;
public:

};

class System
{
public:
    void registerOperation()
    {

    }
};

//So, maybe some friend declarations ?

//this class below makes the connections between the system and a TestedOperation
template<typename TestedObject>//builder interface
class ConnectionMaker
{
public:
    ~ConnectionMaker() {}
    OperationConnections makeConnection(TestedOperation<TestedObject>& operation)
    {

    }
};

template<typename TestedObject>
class AsyncTest
{
private:
    System system_;
    ObjectProxy<TestedObject> proxy_;
    ConnectionMaker connection_maker_;
public:
    virtual bool isTestExecutued() const = 0 ;
    virtual void runTest() = 0;
    virtual ~AsyncTest() {}
    void registerOperation()
};
