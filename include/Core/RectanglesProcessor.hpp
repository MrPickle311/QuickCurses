#pragma once
#include "SubEngine.hpp"

class RectanglesProcessor;

class RectanglesProcessor_base
{
private:

public:
};

class RectanglesProcessor
{
private:
    std::shared_ptr<RectanglesProcessor_base> base_;
};