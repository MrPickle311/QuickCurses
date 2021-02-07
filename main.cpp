#include <iostream>
#include <vector>
#include <ncurses.h>
#include "include/Core/ThreadPool.hpp"
#include "include/Core/ThreadsafeQueue.hpp"

int main()
{
    ThreadsafeQueue<int> queue;
    queue.push(6);
    queue.push(7);
    int a;
    queue.waitAndPop(a);
    std::cout << a << std::endl;
    queue.waitAndPop(a);
    std::cout << a << std::endl;
    if(queue.tryPop(a))
        std::cout << a << std::endl;
    else std::cout << "Not found!" << std::endl;
    return 0;
}
