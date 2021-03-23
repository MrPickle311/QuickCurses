#include "../../include/Core/ThreadsafeList.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <future>
#include <functional>
#include <iostream>

#define verbose false

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
        #if verbose == true
        std::cout << *origin_value_ << " " << *value << "\n";
        #endif
        return *origin_value_ == *value;
    } 
}; 

template<>
class Equalizer<std::shared_ptr<int>>
{
private:
    std::shared_ptr<int>  origin_value_;
public:
    Equalizer(std::shared_ptr<int> const& origin_value):
        origin_value_{origin_value}
    {}
    bool operator() (std::shared_ptr<int> const& value) const
    {
        
        return *origin_value_ == *value;
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
    EXPECT_EQ(500,this->list_.size());
}

class IntegerTests:
    public ListTest<int>
{
protected:
    using ListTest<int>::ListTest;
};

TEST_F(IntegerTests,PushingRemovingTest)
{
    EXPECT_NO_THROW(this->fillListByValue(2,30));
    EXPECT_EQ(30,this->list_.size());
    EXPECT_NO_THROW(this->multipleForEach([](std::shared_ptr<int>& var){++*var;},30));
    EXPECT_NO_THROW(this->multipleRemoveByValue(2,30));
    EXPECT_EQ(30,this->list_.size());
    EXPECT_NO_THROW(this->multipleRemoveByValue(32,30));
    EXPECT_EQ(0,this->list_.size());
    EXPECT_NO_THROW(this->multipleRemoveByValue(32,30));
}

class StringTests:
    public ListTest<std::string>
{
protected:
    using ListTest<std::string>::ListTest;
};

TEST_F(StringTests,ForeachTest)
{
    EXPECT_NO_THROW(this->fillListByValue("Hello string!",30));
    EXPECT_NO_THROW(this->multipleForEach([](std::shared_ptr<std::string>& var)
    {
        *(--var->end()) = ' ';
        var->append("and hello List!");
        #if verbose == true
        std::cout << *var << '\n';
        #endif
    },1));
    EXPECT_NO_THROW(this->multipleRemoveByValue("Hello string and hello List!",30));
    EXPECT_EQ(0,this->list_.size());
    EXPECT_NO_THROW(this->multipleForEach(testingFunction<std::string>,1));
    EXPECT_EQ(0,this->list_.size());
}

TEST_F(StringTests,FindFirstIfTest)
{
    EXPECT_NO_THROW(this->fillListByValue("Hello string!",10));
    list_.pushFront(std::make_shared<std::string>("XD"));
    EXPECT_NO_THROW(this->fillListByValue("Hello string!",10));
    std::shared_ptr<std::shared_ptr<std::string>> ptr = 
                                    list_.findFirstIf([](auto&& e)
                                    {
                                        if(*e == "XD")
                                            return true;
                                        return false;
                                    });
    **ptr = "NOT";
    ptr = list_.findFirstIf([](auto&& e)
                            {
                                if(*e == "NOT")
                                    return true;
                                return false;
                            });
    EXPECT_EQ(**ptr,"NOT");
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}