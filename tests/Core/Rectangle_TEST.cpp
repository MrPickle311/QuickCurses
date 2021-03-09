#include <gtest/gtest.h>
#include "../../include/Core/Rectangle.hpp"

//Monochromatic rectangle
//only mechanics

class TestBase
{
protected:
    Engine engine_;
    std::vector<Rectangle> rectangles;
    void addRectangle(Size rectangle_size,Point left_corner_pos,std::shared_ptr<CompositeObject> parent)
    {
        terminals_.emplace_back(parent,rectangle_size,left_corner_pos);
    }
    void addRectangle(Point left_corner_pos,Point right_corner_pos,std::shared_ptr<CompositeObject> parent)
    {
        terminals_.emplace_back(parent,left_corner_pos,right_corner_pos);
    }
    void deleteRectangle(size_t index)
    {
        std::vector<Rectangle> itr{rectangles.begin()};
        std::advance(itr,index);
        rectangles.erase(itr);
    }
    void initEngine(Size terminal_size)
    {
        engine_.start(terminal_size);
    }
    std::shared_ptr<CompositeObject> getParent()
    {
        return engine_.getRootHook();
    }
    size_t getConnectedWindowsCount()
    {
    size_t count {0};
        for(auto& e: rectangles)
            if(e.isConnected())
                ++count;
        return count;
    }
};

struct RectangleParametersBase
{
    Size terminal_size_;
    bool should_except_;
};

struct RectangleSizePointParameters:
    public RectangleParametersBase
{
    Size rec_size_;
    Point rec_point_;  
};

struct RectanglePoinPiontParameters:
    public RectangleParametersBase
{
    Point rec_point_1;
    Point rec_point_2;
};

class RectangleSizePointTests:
    public TestBase,
    public testing::TestWithParam<RectangleSizePointParameters>
{};

class RectanglePointPointTests:
    public TestBase,
    public testing::TestWithParam<RectanglePoinPiontParameters>
{};


TEST_P(RectangleSizePointTests,ConstructorAndBorderTest)
{
    RectangleSizePointParameters const& param = GetParam(); 
    this->initEngine(param.terminal_size_});

    if(param.should_except_)
        EXPECT_THROW(this->addRectangle(param.rec_size_,param.rec_point_,this->getParent()),OutOfTerminal);
    else EXPECT_NO_THROW(this->addRectangle(param.rec_size_,param.rec_point_,this->getParent()));
}

INSTANTIATE_TYPED_TEST_CASE_P(BasicTests,RectangleSizePointTests,::testing::Values(
    //constructor requires size_t ,so rectangle cannot fly through upper and left border
    RectangleSizePointParameters{ { {200,200} , false } , { {Size{XSize{40},YSize{20}}} , {Point{XCord{0},YCord{0}}} } },
    RectangleSizePointParameters{ { {200,200} , false } , { {Size{30,30}} , {Point{50,40}} } },
    //so if right border of a rectangle will cross the border of the screen 
    //system will throw an exception with following type : OutOfTerminal exception
    RectangleSizePointParameters{ { {200,200} , true } , { Size{30,30} , Point{200,200} } },
    RectangleSizePointParameters{ { {200,200} , true } , { Size{30,30} , Point{20,180} } },
    RectangleSizePointParameters{ { {200,200} , true } , { Size{30,30} , Point{180,20} } },
    //single-cell rectangle
    RectangleSizePointParameters{ { {200,200} , false } , { Size{1,1},Point{199,199} } },
    //size argument must be >= 1
    RectangleSizePointParameters{ { {200,200} , true } , { Size{0,1},Point{199,199} } },
    RectangleSizePointParameters{ { {200,200} , true } , { Size{1,0},Point{199,199} } },
    RectangleSizePointParameters{ { {200,200} , true } , { Size{0,0},Point{199,199} } },
    //point argument must be >= 0
    RectangleSizePointParameters{ { {200,200} , true } , { Size{1,1},Point{-1,199} } },
    RectangleSizePointParameters{ { {200,200} , true } , { Size{1,1},Point{199,-2} } },
    RectangleSizePointParameters{ { {200,200} , false } , { Size{1,1},Point{199,199} } },
    RectangleSizePointParameters{ { {200,200} , false } , { Size{1,1},Point{199,199} } }

));


TEST_P(RectanglePointPointTests,BorderTest)
{
    RectanglePoinPiontParameters const& param = GetParam(); 
    this->initEngine(param.terminal_size_});

    if(param.should_except_)
        EXPECT_THROW(this->addRectangle(param.rec_point_1,param.rec_point_2,this->getParent()),BadCords);
    else EXPECT_NO_THROW(this->addRectangle(param.rec_point_1,param.rec_point_2,this->getParent()));

}

INSTANTIATE_TYPED_TEST_CASE_P(BasicTests,RectanglePointPointTests,::testing::Values(
    //to fix some bugs Rectangle provides constructor which takes two Points 
    RectanglePoinPiontParameters{ { {200,200} , false } , { Point{150,150},Point{199,199} } },
    RectanglePoinPiontParameters{ { {200,200} , false } , { Point{0,0},Point{199,199} } },
    RectanglePoinPiontParameters{ { {200,200} , false } , { Point{150,150},Point{150,150} } },
     //following statements should throw bcs right-bottom corner has cords closer to the origin
    RectanglePoinPiontParameters{ { {200,200} , true } , { Point{150,180},Point{20,150} } },
    RectanglePoinPiontParameters{ { {200,200} , true } , { Point{180,150},Point{20,150} } },
    RectanglePoinPiontParameters{ { {200,200} , true } , { Point{180,180},Point{150,150} } }
));

/*TODO
    MAKE THE FOLLOWING TESTS 
*/
//Make the other tests
class MovingTest:
    public TestBase
{
protected:
    void moveRectangle(size_t index,Point where)
    {
        rectangles[i].move(where);
    }
    void hideRectangle(size_t index)
    {
        rectangles[i].hide();
    }
    bool isHidden(size_t index) const
    {
        return rectangles[i].isHidden();
    }
    bool isShown(size_t index) const
    {
        return rectangles[i].isShown();
    }
    void moveDown(size_t index)
    {
        rectangles[i].moveDown();
    }
    bool canMoveDown(size_t index)
    {
        return rectangles[i].canMoveDown();
    }
};

//Testing binding and relative positioning 
class BindingTest:
    public MovingTest
{
protected:
};

//after....
class CompositeTest:
    public MovingTest
{
protected:
    
};

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}