cmake_minimum_required(VERSION 3.10)

project(QuickCurses)

set(CMAKE_CXX_STANDARD 17)
#-fsanitize=thread niekompatybliny z pointerowymi sanitizerami ,więc trzeba ich używać zamiennie
if(SAN STREQUAL P)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_LINKER_FLAGS  "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
elseif(SAN STREQUAL T)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread ")
    set(CMAKE_LINKER_FLAGS  "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=thread ")
endif()

#faster compiling
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

message("Looking for libraries...")
find_package(Curses REQUIRED)

set(EXTERNAL_PACKAGES_LIST  "ncurses" "curses" )
set(FIND_COMMANDS  "NCURSES_FOUND" "CURSES_FOUND")

list(LENGTH EXTERNAL_PACKAGES_LIST PACKAGES_NMBR)
list(LENGTH FIND_COMMANDS COMMANDS_NMBR)

message("External packages number : ${PACKAGES_NMBR}")

if(NOT(${PACKAGES_NMBR} EQUAL ${COMMANDS_NMBR}))
    message("External packages error!")
endif()

math(EXPR PACKAGES_NMBR "${PACKAGES_NMBR} - 1" OUTPUT_FORMAT DECIMAL)
foreach(x RANGE ${PACKAGES_NMBR})
    list(GET EXTERNAL_PACKAGES_LIST ${x} PKG)
    list(GET FIND_COMMANDS ${x} CMD)
    if(${CMD})
        message("${PKG} found!")
    else()
        message("${PKG} not found!")
    endif()
endforeach()

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

target_include_directories(QuickCurses PUBLIC
                        ${INCLUDE_DIRS}
                        )

if(NOT DEFINED TESTS)
    set(TESTS "OFF")
endif()

if(${TESTS} STREQUAL "ON")
    add_subdirectory(tests)
endif()
