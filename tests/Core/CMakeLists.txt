message("Core")
set_target_properties(Core PROPERTIES LINKER_LANGUAGE CXX)
find_package(GTest REQUIRED)

if(${GTEST_FOUND})
    enable_testing()
else()
    message(FATAL_ERROR "GTest not found!")
endif()

if(NOT (DEFINED TestFiles)) 
    message(FATAL_ERROR "CMake testing syntax error: TestFiles not defined!")
endif()

set(TestLibs
    ThreadsafeQueue_TEST.cpp
    ThreadPool_TEST.cpp
    )

set(CMAKE_CXX_STANDARD 17)

include_directories(../../include/Core ${GTEST_INCLUDE_DIRS})

#przyspieszenie kompilacji
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()


macro(package_add_test TESTNAME FILES)
    add_executable(${TESTNAME} ${FILES})
    target_link_libraries(${TESTNAME} ${GTEST_LIBRARIES}  pthread)
    gtest_discover_tests(${TESTNAME}
                        WORKING_DIRECTORY ${PROJECT_DIR} #dont change , very pretty file structure
                        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
                        )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER ${PROJECT_DIR}/build)
endmacro()

#it has possibly to use a custom libraries
macro(package_add_test_lib TESTNAME FILES LIBRARIES)
    add_executable(${TESTNAME} ${FILES})
    target_link_libraries(${TESTNAME} ${GTEST_LIBRARIES} ${LIBRARIES} pthread)
    gtest_discover_tests(${TESTNAME}
                        WORKING_DIRECTORY ${PROJECT_DIR}
                        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}/tests/Core"
                        )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER Core)
endmacro()

foreach(file ${TestFiles})
    message("The following file will be tested : " ${file})
    message("Creating test...")
    package_add_test(${file}_TEST ${file}_TEST.cpp)
endforeach()

