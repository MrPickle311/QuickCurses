#include "../../include/Core/ThreadsafeList.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <future>
#include <functional>

//Parallel execution tested by ThreadSanitizer
//Below are only base-logic tests

using TestedTypes = testing::Types<int,
                                   std::string,
                                   std::vector<std::pair<int,std::string>>>;

template<typename T>
class TestingFunctions
{
protected://by default its empty function
    void testingFunction(T& value)
    {}
};

template<>
class TestingFunctions<int>
{
protected://by default its empty function
    void testingFunction(int& value)
    {
        ++value;
    }
};

template<>
class TestingFunctions<std::string>
{
protected://by default its empty function
    void testingFunction(std::string& value)
    {
        value.append("xd");
    }
};

template<>
class TestingFunctions<std::vector<std::pair<int,std::string>>>
{
protected://by default its empty function
    void testingFunction(std::vector<std::pair<int,std::string>>& value)
    {
        value.push_back(std::make_pair(56,"xd"));
    }
};

template<typename T>
class ListTest:
    public testing::Test,
    public TestingFunctions<T>
{
protected:
    ThreadsafeList<std::shared_ptr<T>> list_;
    void fillListByValue(T value,size_t count)
    {
        for(size_t i{0}; i < count; ++i)
            list_.pushFront({});
    }

    void multipleRemoveByValue(T value,size_t count)
    {
        auto is_equal = [&](T var)->bool
        {
            return var == value;
        };

        for(size_t i{0}; i < count; ++i)
            list_.removeIf(is_equal);
    }

    void multipleFindFirstIfByValue(T value,size_t count)
    {
        auto is_equal = [&](T var)->bool
        {
            return var == value;
        };

        for(size_t i{0}; i < count; ++i)
            list_.findFirstIf(is_equal);
    }

    void multipleForEach(std::function<void(T)> function,size_t count)
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
                                               
    //EXPECT_NO_THROW(this->multipleForEach(this->testingFunction,op_count));
    EXPECT_NO_THROW(this->fillListByValue(TypeParam {},op_count));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}