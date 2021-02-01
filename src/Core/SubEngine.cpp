#include "../../include/Core/SubEngine.hpp"

std::shared_ptr<SubEngine> AbstractEngine::setNext(std::shared_ptr<SubEngine> subEngine)
{
    this->nextSubEngine_ = subEngine;
    return subEngine;
}