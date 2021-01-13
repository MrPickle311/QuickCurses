cmake_minimum_required(VERSION 3.17)

project(QuickCurses)

set(CMAKE_CXX_STANDARD 17)
find_package(Curses REQUIRED)

set(SOURCES
    main.cpp
    )

set(INCLUDE_DIRS
    include/Core
    ${CURSES_INCLUDE_DIRS}
    )

set(LIBRARIES
    ${CURSES_LIBRARIES}
    ALL_LIBRARIES
    )

add_executable(QuickCurses ${SOURCES})

add_subdirectory(src)

target_link_libraries(QuickCurses
                      ${LIBRARIES}
                      pthread
                      )

target_link_directories(QuickCurses PUBLIC
                        ${INCLUDE_DIRS}
                        )
