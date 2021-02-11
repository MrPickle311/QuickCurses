#pragma once
#include <vector>
#include <future>
#include <thread>
#include <mutex>

//zastosować uogólnionego buildera do tworzenia wstępnie skonfigurowanego obiektu
//szczególnie jeśli brak domyślnego konstruktora

struct Single{};
struct Collection{};

template<typename StoredObject,typename ... Args>
class ObjectBuilder
{
public:
    StoredObject build(Args... args)//place here constructor arguements
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
    SingleObjectWrapper(Args... args):
        obj_{args...}
    {}
    ~SingleObjectWrapper(){}
    StoredObject& getObject()
    {
        return obj_;
    }
};

//all raw-data, direct access
//ROZDZIELIĆ I UOGÓLNIĆ
template<typename StoredType,typename OperationReturnType>
class TestingBase
{
private:
    //Main flags,comman data
    ThreadsafeQueue<StoredType>& queue_;
    std::shared_future<void>& ready_;

    //Action specified lists
    std::vector<std::promise<void>> action_ready_list_;
    std::vector<std::future<OperationReturnType>> action_done_list_;
public:
    TestingBase(ThreadsafeQueue<StoredType>& queue,std::shared_future<void>& ready):
        queue_{queue},
        ready_{ready},
        action_ready_list_{},
        action_done_list_{}
    {}
    virtual ~TestingBase() {}
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
    ThreadsafeQueue<StoredType>& queue()
    {
        return queue_;
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
template<typename StoredType,typename OperationReturnType>
class AsyncEnabler
{
private:
    TestingBase<StoredType,OperationReturnType>& base_ref_;
protected:
    virtual void printData() const //hook
    {}
    virtual void prepareData() //hook
    {}
    virtual void workload() //hook
    {}
    virtual OperationReturnType operation(size_t i) = 0; //required to implementation by the others
public:
    AsyncEnabler(TestingBase<StoredType,OperationReturnType>& base_ref):
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

template<typename StoredType,typename OperationReturnType>
class PendingObject
{
private:
    TestingBase<StoredType,OperationReturnType>& base_ref_;
public:
    PendingObject(TestingBase<StoredType,OperationReturnType>& base_ref):
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
