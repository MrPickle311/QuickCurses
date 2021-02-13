#pragma once
#include <vector>
#include <future>
#include <thread>
#include <mutex>

//zastosować uogólnionego buildera do tworzenia wstępnie skonfigurowanego obiektu
//szczególnie jeśli brak domyślnego konstruktora

template<typename StoredObject>
class ObjectBuilder
{
public:
    template<typename ... Args>
    static StoredObject build(Args... args)//place here constructor arguements
    {
        return StoredObject{args...};
    }
};

template<typename StoredObject>
class ObjectWrapper
{
private:
    StoredObject obj_;
public:
    template<typename... Args>
    ObjectWrapper(Args... args):
        obj_{ObjectBuilder<StoredObject>::build(args...)}
    {}
    ~ObjectWrapper(){}
    StoredObject& getObject()
    {
        return obj_;
    }
};

template<typename TestedObject>
class CommonSystemObjects
{
private:
    std::shared_future<void> system_ready_indicator_;
    ObjectWrapper<TestedObject> tested_object_wrapper_;
public:
    template<typename... Args>
    CommonSystemObjects(std::promise<void>& go_indicator,Args... args):
    system_ready_indicator_{go_indicator.get_future()},
        tested_object_wrapper_{args...}//argument types must be deduced
    {}
    std::shared_future<void>& getSystemReadyIndicator()
    {
        return system_ready_indicator_;
    }
    void waitForSystemReady()
    {
        system_ready_indicator_.wait();
    }
    ObjectWrapper<TestedObject>& getWrapper()
    {
        return tested_object_wrapper_;
    }
};

template<typename TestedObject>
using SystemChannel = CommonSystemObjects<TestedObject>&;

//all raw-data, direct access
template<typename TestedObject,
         typename OperationReturnType>
class TestedOperationBase
{
private:
    //Main flags,comman data
    CommonSystemObjects<TestedObject>& system_objects_;

    //Action specified lists
    std::vector<std::promise<void>> operations_ready_;
    std::vector<std::future<OperationReturnType>> operations_done_;
private:
    void checkInvariants() const
    {
        if(operations_done_.size() != operations_done_.size())
            throw std::runtime_error("action_done_list_.size() != action_done_list_.size()");
    }
public:
    template<typename... Args>
    TestedOperationBase(CommonSystemObjects<TestedObject>& system_objects):
        system_objects_{system_objects},
        operations_ready_{},
        operations_done_{}
    {}
    virtual ~TestedOperationBase() {}
    virtual void setOperationsNumber(size_t number)
    {
        operations_ready_.resize(number);
        operations_done_.resize(number);
    }
    std::promise<void>& operationReady(size_t number)
    {
        return operations_ready_[number];
    }
    std::future<OperationReturnType>& operationDone(size_t number)
    {
        return operations_done_[number];
    }
    size_t getOperationsCount() const
    {
        checkInvariants();
        return operations_done_.size();
    }
    CommonSystemObjects<TestedObject>& getSystemCommononObjects()
    {
        return system_objects_;
    }
};

//i should add some configuration flags in by structure

template<typename TestedObject,typename OperationReturnType>
class Base
{

};

template<typename TestedObject,typename OperationReturnType>
class EngineComponent
{
private:
    TestedOperationBase<TestedObject,OperationReturnType>& base_;
public:
    EngineComponent(){}
    virtual ~EngineComponent() {}
};

//template-method
//independ from TestingBase
template<typename TestedObject,typename OperationReturnType>
class AsyncEnabler
{
private:
    TestedOperationBase<TestedObject,OperationReturnType>& base_;
protected:
    virtual void printData() const //hook
    {}
    virtual void prepareData() //hook
    {}
    virtual void workload() //hook
    {}
    virtual OperationReturnType operation(size_t i) = 0; //required to implementation by the others
protected:
    AsyncEnabler(TestedOperationBase<TestedObject,OperationReturnType>& base):
        base_{base}
    {}
    virtual ~AsyncEnabler() {}
    void prepareOperations(size_t iterations_nmbr)
    {
        base_.setOperationsNumber(iterations_nmbr);
        printData(); 
        for(size_t i{0}; i < iterations_nmbr; ++i)
        {
            base_.operationDone(i) = std::async(std::launch::async,
                                    [this,i]
                                    {
                                        prepareData();
                                        base_.operationReady(i).set_value(); 
                                        base_.getSystemCommononObjects().//przebudować te funkcje
                                              waitForSystemReady();//na jedną funkcję c
                                        workload();                         //czekającą
                                        return operation(i);
                                    }
                                );
        }
    }
};

template<typename TestedObject,typename OperationReturnType>
class PendingObject
{
private:
    TestedOperationBase<TestedObject,OperationReturnType>& base_;
public:
    PendingObject(TestedOperationBase<TestedObject,OperationReturnType>& base):
        base_{base}
    {}
    virtual ~PendingObject() {}
    void waitForOperationsReadyToStart()
    {
        for(size_t i{0} ; i < base_.getOperationsCount() ; ++i)
            base_.operationReady(i).get_future().wait();
    }
    void waitForOperationsResults()
    {
        for(size_t i{0} ; i < base_.getOperationsCount() ; ++i)
            base_.operationDone(i).get();
    }
};

//base to inherit or specialise
template<typename TestedObject,typename OperationReturnType>
class TestedOperation:
    public PendingObject<TestedObject,OperationReturnType>,
    public AsyncEnabler<TestedObject,OperationReturnType>
{
    using PObject = PendingObject<TestedObject,OperationReturnType>;
    using Enabler = AsyncEnabler<TestedObject,OperationReturnType>;
private:
    TestedOperationBase<TestedObject,OperationReturnType> base_;
    CommonSystemObjects<TestedObject>& system_objects_;
public:
    TestedOperation(CommonSystemObjects<TestedObject>& system_objects):
        PObject{base_},
        Enabler{base_},
        system_objects_{system_objects},
        base_{system_objects_}
    {}
    void setInvocationsCount(size_t count)
    {
        Enabler::prepareOperations(count);
    }
    TestedObject& testedObject()
    {
        return system_objects_.getWrapper().getObject();
    }
    virtual ~TestedOperation() {}
};

template<typename TestedObject>
class AsyncTest
{
private:
    std::promise<void> start_system_indicator_;
    std::atomic_bool run_flag_;
    CommonSystemObjects<TestedObject> system_objects_;
private:
    void unlockThreads()
    {
        start_system_indicator_.set_value();
    }
    void setFlags()
    {
        if(!isTestExecutued())
            unlockThreads();
        run_flag_ = true;
    }
protected:
    virtual void clearOperations(){} //hook
    virtual void otherOperations(){} //hook
public:
    template<typename... Args>
    AsyncTest(Args... args):
        start_system_indicator_{},
        run_flag_{false},
        system_objects_{start_system_indicator_,
                        args...}//TestedObject constructor arguments, like emplace
    {}
    virtual ~AsyncTest() {}
    //template-method
    void runTest()
    {
        try
        {
            if(isTestExecutued())
                throw std::runtime_error("Test is already running!\n");
            wait();
            setFlags();
            getFutures();
            otherOperations();
        }
        catch(...)
        {
            setFlags();
            clearOperations();
            throw;//re-throw to gtest EXPECT_THROW/NO_THROW
        }
        
    }
    bool isTestExecutued() const
    {
        return run_flag_;
    }
    CommonSystemObjects<TestedObject>& joinToSystem()
    {
        return system_objects_;
    }
    virtual void wait() = 0;
    virtual void getFutures() = 0;//zmiana nazwy
};