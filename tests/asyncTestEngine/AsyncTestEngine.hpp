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
class System//rename to System
{
private://operations cannot have direct acces the following objects -> unsafe
    SystemReadyIndicator system_ready_indicator_;
    ObjectProxy<TestedObject> object_proxy_;
    OperationsBlocker blocker_;

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
private:
    size_t total_repetitions_count_;
    size_t current_repetition_;
public:
    RepetitionInfo(size_t const total_repetitions_count):
        total_repetitions_count_{total_repetitions_count},
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

template<typename TestedObject>
struct Connection
{
    Signal& increment_repetition_;
    Signal& increment_iteration_;
    Signal& reset_iterations_;
    System& system_;
};



//constructor of below class takes a connection in the constructor
template<typename TestedObject>
class TestedOperation:
    public OptionalActions,
    public ExpectedActions
{
private:
    size_t const operation_id_;
    System<TestedObject>& system_;
    OperationInfo info_;
    ConnectionMaker maker_; //it makes connetions between OperationsExecuter and 
                            //OperationInfo

};


template<typename TestedObject>
class AsyncTest
{
private:
    System<TestedObject> system_;
public:
    bool isTestExecutued() const = 0 ;
    void runTest() = 0;
    virtual ~AsyncTestEngine() {}
};
