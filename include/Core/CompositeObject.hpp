#pragma once
#include "Object.hpp"

class CompositeObject:
    public Object
{
public:
    virtual void setParent(CompositeObject&) = 0;
};