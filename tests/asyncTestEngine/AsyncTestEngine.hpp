#include <boost/signals2.hpp>
#include "ThreadsafeHashTable.hpp"

#include <future>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>

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
private:
    Connection prepare_data_connection_;
    Connection workload_connection_;
    Connection delay_connection_;
public:
    virtual void prepareData() {}
    virtual void workload() {}
    virtual void delay() {}
};

class ExpectedActions
{
private:
    Connection operation_connection;
public:
    virtual void operation() = 0; //defined by user
    void setupConnection()  //it will break hermetization
    {

    }
};

class RepetitionInfo
{
private:
    size_t total_repetitions_count_;
    size_t current_repetition_;
public:
    RepetitionInfo():
        total_repetitions_count_{0},
        current_repetition_{0}
    {}
    size_t getRepetitionsCount() const
    {
        return total_repetitions_count_;
    }
    void incrementCurrentRepetition()
    {
        if(++current_repetition_ < total_repetitions_count_)
            ++current_repetition_;
        else throw std::out_of_range{"current_repetition_ >= total_repetitions_count_!"};
    }
    size_t getCurrentRepetition() const
    {
        return current_repetition_;
    }
};

class IterationInfo
{
private:
    size_t current_iteration_;
public:
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
    Signal iterations_reset_;
    Signal inrement_iteration_;
    Signal increment_repetition_;
    OperationInfoUpdater(RepetitionInfo& repetition_info,
                         IterationInfo& iteration_info):
        iterations_reset_{boost::bind()}
    {}
public:
    void resetIterations()
    {
        iterations_reset_();
    }
    void invcrementIteration()
    {
        inrement_iteration_();
    }
    void incrementRepetition()
    {
        increment_repetition_();
    }
};

class ActionsInvoker
{
private:
    Signal invoke_operation_;
    Signal invoke_delay_;
    Signal invoke_workload_;
    Signal invoke_prepare_data_;
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
};
