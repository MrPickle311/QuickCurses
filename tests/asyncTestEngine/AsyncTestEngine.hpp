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
    void incrementIteration()
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

class OptionalActionsInvoker//one for one operation
{
private:
    OptionalActions& optional_actions_;
public:
    OptionalActionsInvoker(ExpectedActions& expected_actions,
                   OptionalActions& optional_actions):
        optional_actions_{optional_actions}
    {}
    void invokePreActions()
    {
        optional_actions_.prepareData();
        optional_actions_.workload();
    }
    void invokePostActions()
    {
        optional_actions_.delay();
    }
};

class SystemMonitor
{
private:
    std::shared_future<void> system_ready_indicator_;
public:
    SystemMonitor(std::shared_future<void> system_ready_gate):
        system_ready_indicator_{system_ready_gate}
    {}
    void waitForSystemReady()
    {
        system_ready_indicator_.wait();
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
    SystemSharedResources(std::shared_future<void> system_ready_gate,
                          Args... args):
        monitor_{system_ready_gate},
        proxy_{args...}
    {}
    ObjectProxy<TestedObject>& getProxy()
    {
        return proxy_;
    }
};

template<typename TestedObject>
class OperationLoop
{
private:
    SystemSharedResources<TestedObject>& resources_;
    OperationInfoUpdater updater_;
    OptionalActionsInvoker invoker_;
    ExpectedActions& expected_actions_;
private:
    void loopUnit()
    {
        invoker_.invokePreActions();
        /*TODO
            Make signal notifications system
            Maybe observer pattern ?
        */

        resources_.monitor_.waitForSystemReady();
        for(std::atomic_int64_t i{0}; i < updater_.getTotalIterationsCount(); ++i)
        {
            expected_actions_.operation();
            updater_.incrementIteration();
        }
        invoker_.invokePostActions();
    }
public:
    OperationLoop(ExpectedActions& expected_actions,
                  OptionalActions& optional_actions,
                  OperationInfo&  info):
        updater_{info},
        invoker_{expected_actions,
                 optional_actions},
        expected_actions_{expected_actions}
    {}
    void loop()
    {
        for(std::atomic_int64_t i{0}; i < updater_.getTotalRepetitionsCount(); ++i)
        {
            loopUnit();
            updater_.resetIterations();
            updater_.incrementRepetition();
        }
    }
};

template<typename TestedObject>
class OperationProxy // i need to split this class 
{
private:
    SystemSharedResources& resources_;
    OperationLoop loop_;

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
    void configureOperation(OperationPtr operation)
    {
        operation->setConnectionToSystem(resources_.getProxy());
    }

    ~ConnectionMaker(SystemSharedResources& resources) {}
};

template<typename TestedObject>
class OperationsProxy
{
private:
    SystemSharedResources& resources_;
    std::atomic_uint64_t current_index_;
    ThreadsafeHashTable<size_t,OperationProxy<TestedOperation<TestedObject>>> operations_;
private:
    void updateRecentOperations(OperationPtr operation)
    {
        
    }
public:
    OperationsProxy(SystemSharedResources& resources):
        resources_{resources},
        current_index_{0},
        operations_{}
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
        gate_{},
        resources_{gate_.getConnectionToGate(),args...}
    {}
    void registerOperation(OperationPtr operation)
    {
        connection_maker_.configureOperation(operation);
    }
    void run()
    {
        gate_.setSystemReady();
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
