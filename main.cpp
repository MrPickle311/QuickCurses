#include <iostream>
#include <vector>
#include <ncurses.h>
#include "include/Core/ThreadPool.hpp"
#include "include/Core/ThreadsafeQueue.hpp"
#include <ratio>

int main()
{
    printf("\e[8;50;100t");
    std::cout << "\x1b]50;" << "Consolas" << "\a" << std::flush;
    //std::cout << ;
    //std::ios::sync_with_stdio();
    initscr();
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
    if(is_term_resized(400,400))
        std::cout << "Can be!" << std::endl;
    
    refresh();
    resize_term(400,400);
    printw(std::to_string(stdscr->_maxx).c_str());
    getch();
    getch();
    getch();
    endwin();
    return 0;
}
