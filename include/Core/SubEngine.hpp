#pragma once
#include "../Core/Object.hpp"

class SubEngine
{
public:
    virtual ~SubEngine() {}
    virtual void process() = 0;
    virtual std::shared_ptr<SubEngine> setNext(std::shared_ptr<SubEngine> subEngine) = 0;
};

class AbstractEngine:
    public SubEngine,
    public Object
{
private:
    std::shared_ptr<SubEngine> nextSubEngine_;
public:
    ~AbstractEngine() {}
    virtual std::shared_ptr<SubEngine> setNext(std::shared_ptr<SubEngine> subEngine);
    virtual void process();
};