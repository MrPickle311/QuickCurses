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
};

class MovingTest:
    public testing::Test,
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

TEST_F(MovingTest,BorderTest)
{
    this->initEngine(Size{XSize{200},YSize{200}});
    this->addRectangle(Size{XSize{40},YSize{20}},Point{XCord{0},YCord{0}},this->getParent());
    this->addRectangle(Size{30,30},Point{50,40},this->getParent());
    //constructor requires size_t ,so rectangle cannot fly through upper and left border
    
    //so if right border of a rectangle will cross the border of the screen 
    //system will throw an exception with following type : OutOfTerminal exception
    EXPECT_THROW(this->addRectangle(Size{30,30},Point(200,200),this->getParent(),OutOfTerminal));
    EXPECT_THROW(this->addRectangle(Size{30,30},Point(20,180),this->getParent(),OutOfTerminal));
    EXPECT_THROW(this->addRectangle(Size{30,30},Point(180,20),this->getParent(),OutOfTerminal));

    //to fix  it Rectangle provides constructor which takes two Points 
    EXPECT_NO_THROW(this->addRectangle(Point{150,150},Point{199,199}));
    EXPECT_NO_THROW(this->addRectangle(Point{0,0},Point{199,199}));
    EXPECT_NO_THROW(this->addRectangle(Point{150,150},Point{150,150})); //single-cell rectangle

    //following statements should throw bcs right-bottom corner has cords closer to the origin
    EXPECT_THROW(this->addRectangle(Point{150,180},Point(20,150),this->getParent(),BadCords));
    EXPECT_THROW(this->addRectangle(Point{180,150},Point(20,150),this->getParent(),BadCords));
    EXPECT_THROW(this->addRectangle(Point{180,180},Point(150,150),this->getParent(),BadCords));
}

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