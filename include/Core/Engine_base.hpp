#pragma once
#include <ncurses.h>

//najpierw brudny kod
//potem go upiększ

//classes with *_base are implementations
//files with *_base contains only implementation

//Forward declaration
class EngineCreator_base;

//Engine implementation
class Engine_base
{
friend class EngineCreator_base;
private:
    Engine_base();
public:
    
    ~Engine_base();
};

//This class is the builder for Engine_base class
class EngineCreator_base
{

};

