#pragma once
#include <exception>
#include <system_error>

class NoNegativeDimension
{
protected:
    void checkDimensions(int x_dimension,int y_dimension)
    {
        if(x_dimension < 0 || y_dimension < 0)
            throw std::logic_error("Diemnsions cannot be < 0");
    }
};

class Point
{
private:
    int x_;
    int y_;
private:
    
public:
    Point(int x,int y):
        x_{x},
        y_{y}
    {

    }
};

class Size
{
private:
    int width_;
    int heigh_;
private:
public:
};