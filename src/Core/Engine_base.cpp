#include "../../include/Core/Engine_base.hpp"

Engine_base::~Engine_base()
{
    endwin();
}

Engine_base::Engine_base()
{
    initscr();
}