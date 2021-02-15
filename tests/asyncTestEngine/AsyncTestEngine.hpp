#include "../../extern/sigslot/signal.hpp"
#include "ThreadsafeHashTable.hpp"

#include <future>
#include <mutex>
#include <shared_mutex>

//każde dołączenie jakiejś operacji generuje nowy slot w hash_table 
//każda operacja będzie mieć swój identyfikator oraz swój sygnał

//lub bardzo możliwe ,że wystarczy jeden sygnał na cały system
//tylko z odpowienim argumentem

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
    void wait()
    {
        indicator_.wait();
    }
};

template<typename TestedObject>
class SystemSharedObjects
{
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
    size_t total_invocations_count_;
public:
    size_t getInvocationsCount() const
    {
        return total_invocations_count_;
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
class AsyncTestEngine
{
public:
    bool isTestExecutued() const = 0 ;
    void runTest() = 0;
    virtual ~AsyncTestEngine() {}
};


template<typename TestedObject,typename OperationReturnType>
class TestedMethodImpl
{
private:
    size_t total_invocations_count_;
public:
    void addInvocations(size_t new_invocations_count)
    {
        total_invocations_count_ += new_invocations_count;
    }
};