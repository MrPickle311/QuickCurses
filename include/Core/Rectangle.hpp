#pragma once
#include "CompositeObject.hpp"

class Rectangle;

//holds and takes care about data in memory
class RectangleBase
{
protected:
    std::unique_ptr<WINDOW> window_body_;
protected:
    RectangleBase(int nlines, int ncols, int begy, int begx):
        window_body_{newwin(nlines,ncols,begy,begx)}
    {}
    ~RectangleBase()
    {
        delwin(window_body_.get());
    }
};

class Rectangle:
    public CompositeObject,
    public RectangleBase
{
private:
    
};

void f()
{
    
}