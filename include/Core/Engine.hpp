#pragma once
#include <thread>

//najpierw brudny kod
//potem go upiększ

//classes with *_base are implementations
//files with *_base contains only implementation

//Forward declarations
class EngineCreator_base;

class Engine;

class RootRectangleWrapper
{

};

class ThreadRedistributor
{

};

//Engine implementation
class Engine_base
{
friend class Engine;
friend class EngineCreator_base;
private:
    Engine_base();
public:
    void processSignals();//SignalsProcessor
    ~Engine_base();
};

//user interface
class Engine
{
private:
    std::unique_ptr<Engine_base> base_;
public:
     void run();
};

//This class is the builder for Engine_base class
class EngineCreator_base
{
public:
     
};

class EngineCreator
{
public:
};
