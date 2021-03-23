#pragma once
#include "Object.hpp"

class CompositeObject:
    public Object
{
protected:
    std::shared_ptr<CompositeObject> parent_;
public:
    virtual void setParent(CompositeObject&) = 0;
};