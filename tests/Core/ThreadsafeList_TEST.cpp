#include "../../include/Core/ThreadsafeList.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <future>
#include <functional>
#include <iostream>

//Parallel execution tested by ThreadSanitizer
//Below are only base-logic tests

using TestedTypes = testing::Types<int,
                                  std::string,
                                  std::vector<std::pair<int,std::string>>>;

template<typename T>
//by default its empty function
void testingFunction(std::shared_ptr<T>& value)
{}

template<>
//by default its empty function
void testingFunction(std::shared_ptr<int>& value)
{
    ++*value;
}

template<>
//by default its empty function
void testingFunction(std::shared_ptr<std::string>& value)
{
    value->append("xd");
}

template<>
//by default its empty function
void testingFunction(std::shared_ptr<std::vector<std::pair<int,std::string>>>& value)
{
    value->push_back(std::make_pair(56,"xd"));
}

template<typename T>
class Equalizer
{
private:
    T origin_value_;
public:
    Equalizer(T const& origin_value):
        origin_value_{origin_value}
    {}
    bool operator() (T const& value) const
    {
        return origin_value_ == value;
    } 
}; 

template<typename T>
class ListTest:
    public testing::Test
{
protected:
    ThreadsafeList<std::shared_ptr<T>> list_;
    void fillListByValue(T value,size_t count)
    {
        for(size_t i{0}; i < count; ++i)
            list_.pushFront(std::make_shared<T>(value));
    }

    void multipleRemoveByValue(T value,size_t count)
    {
        Equalizer<std::shared_ptr<T>> is_equal{std::make_shared<T>(value)};

        for(size_t i{0}; i < count; ++i)
            list_.removeIf(is_equal);
    }

    void multipleFindFirstIfByValue(T value,size_t count)
    {
        Equalizer<std::shared_ptr<T>> is_equal{std::make_shared<T>(value)};

        for(size_t i{0}; i < count; ++i)
            list_.findFirstIf(is_equal);
    }

    void multipleForEach(std::function<void(std::shared_ptr<T>&)> function,size_t count)
    {
        for(size_t i{0}; i < count; ++i)
            list_.forEach(function);
    }

};

TYPED_TEST_SUITE(ListTest,TestedTypes);

TYPED_TEST(ListTest,ThrowingTest)
{
    size_t const op_count {500};
    EXPECT_NO_THROW(this->fillListByValue(TypeParam {},op_count));
    EXPECT_NO_THROW(this->multipleFindFirstIfByValue(TypeParam {},op_count));
    EXPECT_EQ(op_count,this->list_.size());    
    EXPECT_NO_THROW(this->multipleForEach(testingFunction<TypeParam>,op_count));
    EXPECT_NO_THROW(this->multipleRemoveByValue(TypeParam {},op_count));
    EXPECT_EQ(0,this->list_.size());
}

void g(std::shared_ptr<int>& ptr)
{
    *ptr = 5;
    std::cout << *ptr << "\n";
}

void f(std::function<void(std::shared_ptr<int>&)> function)
{
    std::shared_ptr<int> ptr{new int{6}};
    function(ptr);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}