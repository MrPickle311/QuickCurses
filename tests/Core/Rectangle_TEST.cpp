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
    void initEngine(Size terminal_size)
    {
        engine_.start(terminal_size);
    }
    std::shared_ptr<CompositeObject> getParent()
    {
        return engine_.getRootHook();
    }
    Rectangle& getRectangle(size_t index)
    {
        return rectangles[index];
    }
    Engine& getEngine()
    {
        return engine_;
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
    RectangleSizePointParameters const& data_set = GetParam(); 
    this->initEngine(data_set.terminal_size_});

    if(data_set.should_except_)
        EXPECT_THROW(this->addRectangle(data_set.rec_size_,data_set.rec_point_,this->getEngine(),OutOfTerminal);
    else EXPECT_NO_THROW(this->addRectangle(data_set.rec_size_,data_set.rec_point_,this->getEngine()));
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
    RectangleSizePointParameters{ { {200,200} , false } , { Size{1,1},Point{199,199} } }
    //size argument must be >= 1
    RectangleSizePointParameters{ { {200,200} , true } , { Size{0,1},Point{199,199} } },
    RectangleSizePointParameters{ { {200,200} , true } , { Size{1,0},Point{199,199} } },
    RectangleSizePointParameters{ { {200,200} , true } , { Size{0,0},Point{199,199} } },
    //point argument must be >= 0
    RectangleSizePointParameters{ { {200,200} , true } , { Size{1,1},Point{-1,199} } },
    RectangleSizePointParameters{ { {200,200} , true } , { Size{1,1},Point{199,-2} } },
    RectangleSizePointParameters{ { {200,200} , true } , { Size{1,1},Point{0,-1} } },
    RectangleSizePointParameters{ { {200,200} , true } , { Size{1,1},Point{-2,-4} } }

));

TEST_P(RectanglePointPointTests,BorderTest)
{
    RectanglePoinPiontParameters const& data_set = GetParam(); 
    this->initEngine(data_set.terminal_size_});

    if(data_set.should_except_)
        EXPECT_THROW(this->addRectangle(data_set.rec_point_1,data_set.rec_point_2,this->getEngine()),BadCords);
    else EXPECT_NO_THROW(this->addRectangle(data_set.rec_point_1,data_set.rec_point_2,this->getEngine()));

};

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

class RectangleNoThrowTests:
    public TestBase,
    public testing::TestWithParam<RectanglePoinPiontParameters>
{};

void testCompositeObjConnections(CompositeObject& obj,size_t rec_count)
{
    ASSERT_TRUE(obj.hasChildren());
    ASSERT_EQ(rec_count,obj.getChildrenCount());
}

void addSomeRectangles(CompositeObject& target,size_t rec_count,
                       Point first_point = {50,50},Point second_point = {50,50})
{
    for(size_t i{0}; i < rec_count; ++i)
        this->addRectangle(first_point,second_point,target);
}

TEST_F(RectangleNoThrowTests,ConnectionsTest)
{
    this->initEngine(Size{200,200});
    size_t rec_count = 10;
    addSomeRectangles(this->getEngine(),rec_count);
    
    testCompositeObjConnections(this->getEngine(),rec_count);
    for(auto& rec: this->rectangles)
    {
        //here base space isn't space of terminal(200x200) but it's 50x50
        //inside parent rectangle
        addSomeRectangles(rec,rec_count,Point{0,0},Point{25,25});
        testCompositeObjConnections(rec,rec_count);
    }
};

//very simple testFunction
void testFunction(Rectangle& rec)
{
    //to move rectangle can use Vector type
    Vector v{1,1}; // 1 down, 1 right
    rec.move(v);
}

TEST_F(RectangleNoThrowTests,BrowsingTest)
{
    this->initEngine(Size{200,200});

    size_t rec_count = 10;
    addSomeRectangles(this->getEngine(),rec_count);
    
    // library provides special container for Rectangles which is safe and easy in use
    // (impl: pointers to rectangles)
    // also provides a bunch of special algorithms for  RectanglesList
    // availibity of specific algorithms depends on RectanglesList type 
    // for example, ColorfulRectanglesList provides group algorithms to manipulate 
    // colors on group or one by one
    
    //the base for lists is RectanglesList which for Rectangle type
    RectanglesList list {this->getEngine().getChildrenList()};

    ASSERT_FALSE(list.empty());
    
    //browse is basic algorithm for invoking packed operations on each rectangle
    //variadic template
    ASSERT_NO_THROW(browse(list,testFunction));
    //argument function/functor must be void(Rectangle&) 

    for(auto& rec: this->rectangles)
        for(size_t i{0}; i < rec_count; ++i)
        {
            this->addSomeRectangles(rec,rec_count,Point{15,15},Point{25,25});
            RectanglesList tempList {rec.getChildrenList(())};
            ASSERT_FALSE(tempList.empty());
            EXPECT_NO_THROW(browse(tempList,testFunction);//expect bcs its leaf of the dependency tree
        }
};

/*TODO
    MAKE THE FOLLOWING TESTS ;
    DEFINE OPERATION PACKAGE AND WRITE TESTS FOR OPERATIONS ON LISTS
    PARENTS IMPLEMENTS BROWSE OPERATION DIRECTLY : rec.browseChildren(...)
    BUT FIRST WRITE 
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