#include "../../include/Core/Dimension.hpp"
#include <gtest/gtest.h>
#include <type_traits>

template<typename T>
void print(T value)
{
    std::cout << value << '\n';
}

template<>
void print(bool value)
{
    std::cout << std::boolalpha << value << '\n';
}

//Basic dimensions have common operators ,so i no need test all
TEST(DimensionTests,MethodTests)
{
    Width w1{5};
    EXPECT_EQ(w1.getValue(),5);
    w1.replace(6);
    EXPECT_EQ(w1.getValue(),6);
    Width w2{w1};
    EXPECT_EQ(w2.getValue(),w1.getValue());
    XCoordinate x1{6};
    EXPECT_EQ(x1.getValue(),6);
    x1.replace(23);
    EXPECT_EQ(x1.getValue(),23);
    XCoordinate x2{x1};
    EXPECT_EQ(x1.getValue(),x2.getValue());
}

template<typename From,typename TO>
void checkUnconvertibility()
{
    bool pred {std::is_convertible<From,TO>::value};
    EXPECT_FALSE(pred);
}

TEST(DimensionTests,ConversionTests)
{
    checkUnconvertibility<Height,Width>();
    checkUnconvertibility<Width,Height>();
    
    checkUnconvertibility<Width,XCoordinate>();
    checkUnconvertibility<XCoordinate,Height>();

    checkUnconvertibility<XCoordinate,YCoordinate>();
    checkUnconvertibility<YCoordinate,XCoordinate>();
}

TEST(DimensionTests,OperatorsTests)
{
    Width w1{5};
    Width w2{w1};
    w1 += Width{5};//10
    EXPECT_FALSE(w1 == w2);
    EXPECT_TRUE(w1 != w2);
    EXPECT_FALSE(w1 <= w2);
    EXPECT_TRUE(w1 >= w2);
    EXPECT_FALSE(w1 < w2);
    EXPECT_TRUE(w1 > w2);

    EXPECT_EQ((w1 + w2).getValue(),15);
    EXPECT_EQ((w1 - w2).getValue(),5);
}

TEST(DimensionTests,ConstructorThrowingTest)
{
    EXPECT_NO_THROW(Width w1{5});
    EXPECT_THROW(Width w1{-5},std::logic_error);
    EXPECT_THROW(Width w1{0},std::logic_error);
    EXPECT_THROW(XCoordinate w1{-5},std::logic_error);
    EXPECT_NO_THROW(XCoordinate w1{0});
}

TEST(DimensionTests,MethodThrowingTest)
{
    Width w1{5};
    XCoordinate x1{5};

    EXPECT_THROW(x1.replace(-3),std::logic_error);
    EXPECT_THROW(w1.replace(-3),std::logic_error);
    EXPECT_THROW(w1.replace(0),std::logic_error);

    EXPECT_NO_THROW(x1.replace(0));
    EXPECT_NO_THROW(x1.replace(1));
    EXPECT_NO_THROW(w1.replace(4));
}

class DimensionGeneratorTest:
    public ::testing::Test
{
protected:
    XCoordinateGenerator x_gen_;
    WidthGenerator w_gen_;
protected:
    DimensionGeneratorTest():
        x_gen_{XCoordinate{0},XCoordinate{10},XCoordinate{2}},
        w_gen_{1,2,9}
    {}
};

TEST_F(DimensionGeneratorTest,ConstructorssTests)
{   
    EXPECT_EQ(x_gen_.getStartValue(),XCoordinate{0});
    EXPECT_EQ(x_gen_.getEndValue(),XCoordinate{10});
    EXPECT_EQ(x_gen_.getStepValue(),XCoordinate{2});

    EXPECT_EQ(w_gen_.getStartValue(),Width{1});
    EXPECT_EQ(w_gen_.getEndValue(),Width{2});
    EXPECT_EQ(w_gen_.getStepValue(),Width{9});    
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}