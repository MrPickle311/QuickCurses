message("Loading tests/Core module...")

set(TestLibs
     ThreadsafeQueue
     ThreadPool
     ncurses
    )

set(EXTRA_LIBRARIES
    
   )

include_directories(
                    ../../include/Core 
                    #${CURSES_INCLUDE_DIRS}
                   )
include_directories(${GTEST_INCLUDE_DIRS} )
#link_libraries(${EXTRA_LIBRARIES} pthread )

buildTests("${TestLibs}")