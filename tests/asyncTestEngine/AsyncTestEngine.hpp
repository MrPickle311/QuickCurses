#include <boost/signals2.hpp>
#include "ThreadsafeHashTable.hpp"

#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include "Operation.hpp"
//każde dołączenie jakiejś operacji generuje nowy slot w hash_table 
//każda operacja będzie mieć swój identyfikator oraz swój sygnał

//lub bardzo możliwe ,że wystarczy jeden sygnał na cały system
//tylko z odpowienim argumentem

using Table = ThreadsafeHashTable<size_t,size_t>;
using Signal = boost::signals2::signal<void ()>;


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

class SystemMonitor
{
private:
    std::shared_future<void> system_ready_indicator_;
    std::atomic_uint64_t     operations_count_;
public:
    SystemMonitor(std::promise<void>& system_ready_gate):
        system_ready_indicator_{system_ready_gate.get_future()},
        operations_count_{0}
    {}
    size_t getOperationsCount() const
    {
        return operations_count_;
    }
    void incrementOperationsCount()
    {
        ++operations_count_;
    }
    void waitForSystemReady()
    {
        system_ready_indicator_.wait();
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

template<typename TestedObject>
class SystemSharedResources
{
private:
    SystemMonitor monitor_;
    ObjectProxy<TestedObject> proxy_;//variadic template constructor
public:
    template<typename... Args>
    SystemSharedResources(std::promise<void>& system_ready_gate,
                          Args... args):
        monitor_{system_ready_gate},
        proxy_{args...}
    {}
    ObjectProxy<TestedObject>& getProxy()
    {
        return proxy_;
    }
};

class PendingObject
{
private:
    
};

class OperationExecuter
{

};

template<typename TestedObject>
class OperationManager // i need to split this class 
{
private:
    SystemSharedResources& resources_;
    OperationExecuter executer_;
    PendingObject pending_object_;
    ActionsInvoker invoker_;
    OperationInfoUpdater updater_;
    std::atomic_uint64_t index_;
    OperationPtr operation_;
public:
    size_t getIndex() const { return index_; }
};

//So, maybe some friend declarations ?

//this class below makes the connections between the system and a TestedOperation
template<typename TestedObject>//builder interface
class ConnectionMaker
{
private:
    SystemSharedResources& resources_;
public:
    ConnectionMaker(SystemSharedResources& resources):
        resources_{resources}
    {}
    void configureOperation(TestedOperation<TestedObject>& operation)
    {
        operation.setConnectionToSystem(resources_.)
    }

    ~ConnectionMaker(SystemSharedResources& resources) {}
};

template<typename TestedObject>
class OperationsProxy
{
private:
    ThreadsafeHashTable<size_t,OperationManager<TestedOperation<TestedObject>>> operations_;
private:
    void updateRecentOperations()
    {
        
    }
public:
    OperationsProxy()
    {}
};

class SystemGateMonitor
{
private:
    std::promise<void> system_ready_gate_;
public:
    SystemGateMonitor()
    {}
    void setSystemReady()
    {
        system_ready_gate_.set_value();
    }
    std::shared_future<void> getConnectionToGate()
    {
        return system_ready_gate_.get_future();
    }
};

template<typename TestedObject>
class System
{
private:
    SystemGateMonitor gate_;
    SystemSharedResources<TestedObject> resources_;
    ConnectionMaker<TestedObject> connection_maker_;
public:
    template<typename... Args>
    System(Args... args):
    //i finished here
    void registerOperation(OperationPtr operation)
    {

    }
    void run()
    {

    }
};

class AsyncTestSupportActions
{
public:
    void actionsBeforeTest() {}
    void actionsAfterTest() {}
};

template<typename TestedObject>
class AsyncTest:
    public AsyncTestSupportActions
{
private:
    std::atomic_bool running_flag_;
    System system_;
public:
    template<typename... Args>
    AsyncTest(Args... args):
        AsyncTestSupportActions{},
        running_flag_{false},
        system_{args...}
    {}
    bool isTestExecutued() const
    {
        return running_flag_;
    }
    void runTest()
    {
        if(!isTestExecutued())
        {
            running_flag_ = true;
            this->actionsBeforeTest();
            system_.run();
            this->actionsAfterTest();
        }
    }
    virtual ~AsyncTest() {}
    void registerOperation(OperationPtr operation)
    {
        if (operation != nullptr)
            system_.registerOperation(operation);
        else throw std::runtime_error("AsyncTest: trying to register nullptr operation!");
    }
};
