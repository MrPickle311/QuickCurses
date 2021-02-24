#include <boost/signals2.hpp>
#include "ThreadsafeHashTable.hpp"

#include <future>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>

//każde dołączenie jakiejś operacji generuje nowy slot w hash_table 
//każda operacja będzie mieć swój identyfikator oraz swój sygnał

//lub bardzo możliwe ,że wystarczy jeden sygnał na cały system
//tylko z odpowienim argumentem

using Table = ThreadsafeHashTable<size_t,size_t>;
using Signal = boost::signals2::signal<void ()>;

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
    std::atomic_uint64_t operation_id_;
    OperationInfo info_;
    std::shared_ptr<ObjectProxy<TestedObject>> proxy_;
public:
    TestedOperation():
        operation_id{},
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
    size_t getTotalIterationsCount() const
    {
        return info_.getTotalIterationsCount();
    }
    size_t getTotalRepetitionsCount() const
    {
        return info_.getTotalRepetitionsCount();
    }
};

using Indicator = std::shared_future<void>;

class SystemMonitor
{
private:
    Indicator system_ready_indicator_;
    std::atomic_uint64_t operations_count_;
public:
    size_t getOperationsCount()
    {
        return operations_count_;
    }
    void incrementOperationsCount()
    {
        ++operations_count_;
    }
};

class ActionsInvoker//one for one operation
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
        optional_actions_.prepareData();
        optional_actions_.workload();
        //here must be sth pending obj
        optional_actions_.delay();
        expected_actions_.operation();
    }
};

class OperationLoop
{
private:
    OperationInfoUpdater updater_;
    ActionsInvoker invoker_;
public:
    OperationLoop(ExpectedActions& expected_actions,
                  OptionalActions& optional_actions,
                  OperationInfo&  info):
        updater_{info},
        invoker_{expected_actions,
                 optional_actions}
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

};

class PendingObject
{
private:
    
};

class OperationExecuter
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

//So, maybe some friend declarations ?

//this class below makes the connections between the system and a TestedOperation
template<typename TestedObject>//builder interface
class ConnectionMaker
{
private:
    SystemSharedResources& resources_;
private:
    void configureOperation(TestedOperation<TestedObject>& operation)
    {
        operation.
    }
public:
    ~ConnectionMaker(SystemSharedResources& resources) {}
};

template<typename TestedObject>
class OperationsProxy
{
private:
    ThreadsafeHashTable<size_t,TestedObject>
};

template<typename TestedObject>
class System
{
private:
    ConnectionMaker<TestedObject> connection_maker_;
private:
    void updateRecentOperations()
    {

    }
public:
    void registerOperation(TestedOperation<TestedObject>& operation)
    {

    }
    void run()
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
    virtual bool isTestExecutued() const
    {

    }
    virtual void runTest()
    {
        system_.run();
    }
    virtual ~AsyncTest() {}
    void registerOperation(ObjectProxy<TestedObject>& operation)
    {

    }
};
