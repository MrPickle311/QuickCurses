#include <gtest/gtest.h>
#include <future>
#include "AsyncTestEngine.hpp"
#include "../../include/Core/ThreadsafeQueue.hpp"


// template<typename T>
// class DataStorage
// {
// private:
//     std::vector<T> data_;
// public:
//     T getDataAtIndex(size_t index) const
//     {
//         if(index >= data_.size())
//             return data_[index];
//         else throw std::out_of_range{"Vector index out of range"};
//     }
//     std::vector<T> getRangeOfData() const
//     {
//         return data_;
//     }
//     void addData(std::vector<T> const & data) //casual setter
//     {
//         data_ = data;
//     }
// };

// template<typename T>
// class QueueLittlePushTest:
//     public TestedOperation<ThreadsafeQueue<T>>,
//     public DataStorage<T>
// {
// protected:
//     virtual void operation() // now operation_number it's even handy
//     {
//         size_t iteration {System.getCurrentIteration()};
//         T data_to_push = getDataAtIndex(iteration);
//         System.getTestedObject().push(data_to_push);
//     }
// public:
//     QueueLittlePushTest(System& system_ref):
//         TestedOperation<ThreadsafeQueue<T>>{system_ref},
//         DataStorage<T>{}
//     {}
// };

// template<typename T>
// class QueueBigPushTest:
//     public TestedOperation<ThreadsafeQueue<T>>,
//     public DataStorage<T>
// {
// protected:
//     virtual void operation() //but not here, im pushing all data at once
//     {
//         for(auto& element : getRangeOfData())
//             System::getTestedObject().push(element);
//     }
//     virtual void delay() const
//     {
//         std::this_thread::sleep_for(std::chrono::seconds(2));
//     }
// public:
//     QueueBigPushTest(System& system_ref):
//         TestedOperation<ThreadsafeQueue<T>>{system_ref}
//     {}
// };

// template<typename T>
// class QueueTryPopRefTest:
//     public TestedOperation<ThreadsafeQueue<T>>
// {
// private:
//     std::vector<T> data_from_queue_;
// protected:
//     virtual void operation() 
//     {
//         T temp;
//         System::getTestedObject().tryPop(temp);
//         data_from_queue_.push_back(temp);
//     }
//     virtual void workload()//before wait(), some initialization actions
//     {
//         size_t count {TestedOperation::getThisOperationIterationsCount()};
//         data_from_queue_.resize(count);
//     }
// public:
//     QueueTryPopRefTest(System& system_ref):
//         TestedOperation<ThreadsafeQueue<T>>{system_ref}
//     {}
// };

// template<typename T>
// class QueueClearWithWaitAndPopTest:
//     public TestedOperation<ThreadsafeQueue<T>>
// {
// protected:
//     virtual void operation() 
//     {
//         System::getTestedObject().clear();
//         std::cout << "Current repetition: " << TestedOperation::getCurrentRepetition() << std::endl;
//         System::getTestedObject().waitAndPop();
//     }
// public:
//     QueueClearWithWaitAndPopTest(System& system_ref):
//         TestedOperation<ThreadsafeQueue<T>>{system_ref}
//     {}
// };

// template<typename T>
// class QueueEmptyTest:
//     public TestedOperation<ThreadsafeQueue<T>>
// {
// protected:
//     virtual void operation() 
//     {
//         //or TestedOperation , but only one of them 
//         System::getTestedObject().empty();
//     }
// public:
//     QueueEmptyTest(System& system_ref):
//         TestedOperation<ThreadsafeQueue<T>>{system_ref}
//     {}
// };

// template<typename T>
// class ThreadsafeQueueTest:
//     public AsyncTest<ThreadsafeQueue<T>>,
//     public testing::Test
// {
//     using AsyncTest = AsyncTest<ThreadsafeQueue<T>>;
// private:
//     QueueLittlePushTest<T> little_push_test_;
//     QueueBigPushTest<T> big_push_test_;
//     QueueTryPopRefTest<T> try_pop_ref_test_;
//     QueueClearWithWaitAndPopTest<T> clear_with_wait_and_pop_test_;
//     QueueEmptyTest<T> empty_test_; // default in-loop iterations count is 1 
// private:
//     void setPushIterations(size_t iterations)
//     {
//         little_push_test_.setIterations(iterations);
//         big_push_test_.setIterations(iterations);
//     }
// protected:
//     ThreadsafeQueueTest():
//         little_push_test_{AsyncTest::getAccessToSystem()},
//         big_push_test_{AsyncTest::getAccessToSystem()},
//         try_pop_ref_test_{AsyncTest::getAccessToSystem()},
//         clear_with_wait_and_pop_test_{AsyncTest::getAccessToSystem()},
//         empty_test_{AsyncTest::getAccessToSystem()}
//     {}
//     void setPushRepetitionsCount(size_t count)
//     {
//         little_push_test_.setRepetitionsCount(count);
//         big_push_test_.setRepetitionsCount(count);
//     }
//     void setEmptyRepetitionsCount(size_t count)
//     {
//         empty_test_.setRepetitionsCount(count);
//     }
//     void setClearWithWaitAndPopRepetitionsCount(size_t count)
//     {
//         clear_with_wait_and_pop_test_.setRepetitionsCount(count);
//     }
//     void setTryPopRepetitionsCount(size_t count)
//     {
//         try_pop_ref_test_.setRepetitionsCount(count);
//     }
//     void setTryPopIterationsCount(size_t count)
//     {
//         try_pop_ref_test_.setIterationsCount(count);
//     }
//     void addDataset(std::vector<T> new_data)
//     {
//         little_push_test_.addData(new_data);
//         big_push_test_.addData(new_data);
//         setPushIterations(new_data.size());
//     }
//     virtual void runTest()//but by default it won't be pure-virtual
//     {
//         System::run();
//     };
// };

// using MyTypes = testing::Types<int,std::string>;
// TYPED_TEST_SUITE(ThreadsafeQueueTest,MyTypes);

// TYPED_TEST(ThreadsafeQueueTest,FirstTest)
// {
//     std::vector<T> data_;
//     for (size_t i = 0; i < 1000; ++i)
//         data_.push_back(T{});
    
//     EXPECT_NO_THROW(this->addDataset(data_));
//     EXPECT_NO_THROW(this->setPushRepetitionsCount(2));
//     EXPECT_NO_THROW(this->setEmptyRepetitionsCount(3));
//     EXPECT_NO_THROW(this->setClearWithWaitAndPopRepetitionsCount(2));
//     EXPECT_NO_THROW(this->setTryPopRepetitionsCount(3));
//     EXPECT_NO_THROW(this->setTryPopIterationsCount(300));

//     EXPECT_NO_THROW(this->runTest());
// }

class Document
{
public:
    typedef boost::signals2::signal<void ()>  signal_t;

public:
    Document()
    {}
    std::mutex mutex;

    /* Connect a slot to the signal which will be emitted whenever
      text is appended to the document. */
    boost::signals2::connection connect(const signal_t::slot_type &subscriber)
    {
        return m_sig.connect(subscriber);
    }

    void append(const char* s)
    {
        std::lock_guard<std::mutex> lock(mutex);
        m_text += s;
        m_sig();
    }

    const std::string& getText() const
    {
        return m_text;
    }

private:
    signal_t    m_sig;
    std::string m_text;
};

class TextView
{
protected:
    Document&               m_document;
    boost::signals2::connection  m_connection;
public:
    TextView(Document& doc): 
        m_document(doc)
    {
        //unsafe,but its initialization
        m_connection = m_document.connect(boost::bind(&TextView::refresh, this));
    }


    //Important!!!
    ~TextView()
    {
        m_connection.disconnect();
    }

    virtual void refresh() const = 0;
};

class SuperTextView:
    public TextView
{
public:
    SuperTextView(Document& document):
        TextView(document)
    {}
    virtual void refresh() const 
    {
        std::cout << "TextView: " << m_document.getText() << std::endl;
    }
};

void push(std::shared_future<void>& future,
          Document& doc)
{
    future.wait();
    doc.append("Hello world!  ");
}

void testDataRacePresence(size_t iterations_nmbr)
{
    Document    doc;
    SuperTextView    v1(doc);

    std::vector<std::thread> threads1;

    threads1.reserve(iterations_nmbr);

    std::promise<void> promise;
    std::shared_future<void> future{promise.get_future()};

    for(size_t i{0};i < iterations_nmbr;++i)
    {
        threads1.emplace_back(push,std::ref(future),std::ref(doc));
    }
    promise.set_value();
    
    for(auto& t : threads1)
        t.join();
}

TEST(T,t1)
{
    EXPECT_NO_THROW(testDataRacePresence(30));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
