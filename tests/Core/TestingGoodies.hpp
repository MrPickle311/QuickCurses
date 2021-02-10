#pragma once

#include <vector>
#include <iostream>
#include <algorithm>
#include <string>

template<typename T>
std::vector<T> mergeVectors(const std::vector<T>& left_vector,const std::vector<T>& right_vector)
{
    std::vector<T> new_values;
    size_t const new_size {left_vector.size() + right_vector.size()};
    new_values.resize(new_size);

    auto ptr = std::copy(left_vector.begin(), left_vector.end(),new_values.begin());
    std::copy(right_vector.begin(), right_vector.end(),ptr);
    new_values.shrink_to_fit();
    return new_values;
}

//make specializations to print various types 
template<typename T>
struct Printer
{
    void operator() (const T& element)
    {
        std::cout << element << " ";
    }
};

template<>
struct Printer<std::vector<std::pair<int,std::string>>>
{
    void operator() (const std::vector<std::pair<int,std::string>>& element)
    {
        for(auto& e: element)
            std::cout << "( " << e.first << " , " << e.second << " ) ";
    }
};

//predefined data generators

template <typename T>
class DataGenerator
{
public:
    std::vector<T> generate(size_t count) const;
};

template<>
class DataGenerator<int>
{
public:
    std::vector<int> generate(size_t count) const
    {
        std::vector<int> v;
        v.resize(count);
        for (size_t i = 0; i < count; ++i)
           v.push_back(i);
        return v;
    }
};

template<>
class DataGenerator<std::string>
{
public:
    std::vector<std::string> generate(size_t count) const
    {
        std::vector<std::string> v;
        v.reserve(count);
        for (size_t i = 0; i < count; ++i)
           v.push_back("string data");
        return v;
    }
};

template<>
class DataGenerator<std::vector<std::pair<int,std::string>>>
{
public:
    std::vector<std::vector<std::pair<int,std::string>>> generate(size_t count) const
    {
        std::vector<std::vector<std::pair<int,std::string>>> v;
        v.reserve(count);
        for (size_t i = 0; i < count; i++)
        {
            std::vector<std::pair<int,std::string>> v1;
            v1.reserve(count / 4);
            for (size_t i = 0; i < count / 4; i++)
                v1.push_back(std::make_pair(4,std::to_string(i)));
            v.push_back(v1);
        }
        return v;
    }
};