#pragma once
#include "CompositeObject.hpp"

class Rectangle;

class Rectangle_base
{

};

class Rectangle:
    public CompositeObject
{
private:
    std::unique_ptr<Rectangle_base> base_;
};

