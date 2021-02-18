#include <boost/signals2.hpp>
#include "ThreadsafeHashTable.hpp"

#include <future>
#include <mutex>
#include <shared_mutex>

//każde dołączenie jakiejś operacji generuje nowy slot w hash_table 
//każda operacja będzie mieć swój identyfikator oraz swój sygnał

//lub bardzo możliwe ,że wystarczy jeden sygnał na cały system
//tylko z odpowienim argumentem

using Signal = boost::signals2::signal<void()>;
using Table = ThreadsafeHashTable<size_t,size_t>;

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

//ok its safe
class SystemReadyIndicator
{
private:
    std::shared_future<void> indicator_;
public:
    SystemReadyIndicator(std::promise<void>& system_ready_flag):
        indicator_{system_ready_flag.get_future()}
    {}
    void waitForSystemReady()
    {
        indicator_.wait();
    }
};

class SystemOperationCounter
{
private:
    bool tablesAreEqual(Table const& left,Table const& right) const
    {
        return left == right;
    }
public:
    bool isSystemReady(Table const& ) const
    {
        return true;
    }
};

class SystemOperationsCountGuardian
{
private: 
    std::mutex operations_count_mutex_;
    size_t operations_count_;//mutex!
    Table operations_ready_table_;
    Table all_operations_table_;
public:

};

class OperationsBlocker
{
private:
    std::promise<void> start_system_indicator_;
public:

};

template<typename TestedObject>
class SystemSharedObjects//rename to System
{
private://operations cannot have direct acces the following objects -> unsafe
    SystemReadyIndicator system_ready_indicator_;
    ObjectProxy<TestedObject> object_proxy_;
    OperationsBlocker blocker_;

};

//mediator
template<typename TestedObject>
class System
{
private:
    SystemSharedObjects<TestedObject> shared_objects_;
};

template<typename TestedObject,typename OperationReturnType>
class InvocationsPusher //defined by me
{
public:
    void addInvocations(size_t new_invocations_count) = 0;
};

template<typename TestedObject,typename OperationReturnType>
class OperationBase
{
private:
    size_t total_operation_invocations_count_;
    size_t const operation_id_;
public:
    size_t getInvocationsCount() const
    {
        return total_operation_invocations_count_;
    }
};

class OperationsExecuterHooks
{
protected:
    virtual void printData() const {}
    virtual void prepareData() {}
    virtual void workload() {}
};

template<typename TestedObject,typename OperationReturnType>
class OperationsExecuter:
    public OperationsExecuterHooks
{
protected:
    void executeOperations(size_t invocations_number) = 0;
};

template<typename TestedObject,typename OperationReturnType>
class TestedOperation:
    public InvocationsPusher<TestedObject,OperationReturnType>

{
public:
    virtual OperationReturnType operation() = 0; //defined by user
};

template<typename TestedObject>
class AsyncTest
{
public:
    bool isTestExecutued() const = 0 ;
    void runTest() = 0;
    virtual ~AsyncTestEngine() {}
};
