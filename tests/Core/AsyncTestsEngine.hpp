#pragma once
#include <vector>
#include <future>
#include <thread>
#include <mutex>

//zastosować uogólnionego buildera do tworzenia wstępnie skonfigurowanego obiektu
//szczególnie jeśli brak domyślnego konstruktora

struct Single{};
struct Collection{};

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

//all raw-data, direct access
template<typename StoredObject,
         typename OperationReturnType>
class AsyncTestingBase
{
private:
    //Main flags,comman data
    std::shared_future<void>& ready_;

    //Action specified lists
    std::vector<std::promise<void>> action_ready_list_;
    std::vector<std::future<OperationReturnType>> action_done_list_;
public:
    template<typename... Args>
    AsyncTestingBase(std::shared_future<void>& ready):
        ready_{ready},
        action_ready_list_{},
        action_done_list_{}
    {}
    virtual ~AsyncTestingBase() {}
    virtual void setFuturesNumber(size_t number)
    {
        action_ready_list_.resize(number);
        action_done_list_.resize(number);
    }
    std::promise<void>& readyPromise(size_t number)
    {
        return action_ready_list_[number];
    }
    std::future<OperationReturnType>& doneFuture(size_t number)
    {
        return action_done_list_[number];
    }
    std::shared_future<void>& readyIndicator()
    {
        return ready_;
    }
    size_t iterationsCount() const
    {
        if(action_done_list_.size() != action_done_list_.size())
            throw std::runtime_error("action_done_list_.size() != action_done_list_.size()");
        return action_done_list_.size();
    }
};

//i should add some configuration flags in by structure

//template-method
//independ from TestingBase
template<typename StoredObject,typename OperationReturnType>
class AsyncEnabler
{
private:
    AsyncTestingBase<StoredObject,OperationReturnType>& base_ref_;
protected:
    virtual void printData() const //hook
    {}
    virtual void prepareData() //hook
    {}
    virtual void workload() //hook
    {}
    virtual OperationReturnType operation(size_t i) = 0; //required to implementation by the others
protected:
    AsyncEnabler(AsyncTestingBase<StoredObject,OperationReturnType>& base_ref):
        base_ref_{base_ref}
    {}
    virtual ~AsyncEnabler() {}
    void enable(size_t iterations_nmbr)
    {
        try
        {
            base_ref_.setFuturesNumber(iterations_nmbr);
            printData(); 
            for(size_t i{0}; i < iterations_nmbr; ++i)
            {
                base_ref_.doneFuture(i) = std::async(std::launch::async,
                                        [this,i]
                                        {
                                            prepareData();
                                            base_ref_.readyPromise(i).set_value(); 
                                            base_ref_.readyIndicator().wait();
                                            workload(); 
                                            return operation(i);
                                        }
                                    );
            }
        }
        catch(...)
        {
            throw;
        }
    }
};

template<typename StoredObject,typename OperationReturnType>
class PendingObject
{
private:
    AsyncTestingBase<StoredObject,OperationReturnType>& base_ref_;
public:
    PendingObject(AsyncTestingBase<StoredObject,OperationReturnType>& base_ref):
        base_ref_{base_ref}
    {}
    virtual ~PendingObject() {}
    void wait()
    {
        try
        {
            for(size_t i{0} ; i < base_ref_.iterationsCount() ; ++i)
                base_ref_.readyPromise(i).get_future().wait();
        }
        catch(...)
        {
            throw;
        }
    }
    void getFuteres()
    {
        try
        {
            for(size_t i{0} ; i < base_ref_.iterationsCount() ; ++i)
                base_ref_.doneFuture(i).get();
        }
        catch(...)
        {
            throw;
        }
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
    AsyncTestingBase<TestedObject,OperationReturnType> base_;
    ObjectWrapper<TestedObject>& obj_;
public:
    TestedOperation(ObjectWrapper<TestedObject>& obj,
                    std::shared_future<void>& ready):
        PObject{base_},
        Enabler{base_},
        base_{ready},
        obj_{obj}
    {}
    void setInvocationsCount(size_t count)
    {
        Enabler::enable(count);
    }
    TestedObject& object()
    {
        return obj_.getObject();
    }
    virtual ~TestedOperation() {}
};

template<typename TestedObject>
class AsyncTest
{
private:
    std::promise<void> go_;
    std::shared_future<void> ready_;
    ObjectWrapper<TestedObject> wrapper_;
protected:
    virtual void clearOperations(){} //hook
    virtual void otherOperations(){} //hook
public:
    template<typename... Args>
    AsyncTest(Args... args):
        wrapper_{args...},//argument types must be deduced
        go_{},
        ready_{go_.get_future()}
    {}
    virtual ~AsyncTest() {}
    //template-method
    void runTest()
    {
        try
        {
            wait();
            go_.set_value();
            getFutures();
            otherOperations();
        }
        catch(...)
        {
            go_.set_value();
            clearOperations();
            throw;
        }
        
    }
    std::shared_future<void>& getReadyIndicator()
    {
        return ready_;
    }
    ObjectWrapper<TestedObject>& getWrapper()
    {
        return wrapper_;
    }
    virtual void wait() = 0;
    virtual void getFutures() = 0;
};