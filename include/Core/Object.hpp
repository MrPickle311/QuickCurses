#pragma once
#include <string>
#include <memory>
#include <ncurses.h>

class Object;

class Object_base
{
friend class Object;
private:
    std::string name_;
public:
    std::string getName() const;
    void setName(const std::string&);
};

class Object
{
private:
    std::unique_ptr<Object_base> base_;
public:
    virtual std::string getString() const = 0;
    virtual void setString(const std::string&) = 0; 
};