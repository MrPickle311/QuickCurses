message("Loading Tests module...")

macro(package_add_test TESTNAME FILES)
    add_executable(${TESTNAME} ${FILES})
    target_link_libraries(${TESTNAME}  ${LIBRARIES} pthread)
    gtest_discover_tests(${TESTNAME}
                        WORKING_DIRECTORY ${PROJECT_DIR} #dont change , very pretty file structure
                        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
                        )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER ${PROJECT_DIR}/build)
endmacro()

find_package(Curses REQUIRED)

#it has possibly to use a custom libraries
macro(package_add_test_lib TESTNAME FILES )
    add_executable(${TESTNAME} ${FILES})
    target_link_libraries(${TESTNAME} ${GTEST_LIBRARIES} ${CURSES_LIBRARIES} pthread)
    gtest_discover_tests(${TESTNAME}
                        WORKING_DIRECTORY ${PROJECT_DIR}
                        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}/tests/Core"
                        )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER ${PROJECT_DIR}/build)
endmacro()

#### GTEST ######################################
find_package(GTest REQUIRED)

if(${GTEST_FOUND})
    enable_testing()
else()
    message(FATAL_ERROR "GTest not found ,abort testing!")
endif()

###################################################

macro(addSubdirectories)
    add_subdirectory(Core)
endmacro()

function(buildTests testNames)
    foreach(file ${testNames})
        message("The following file will be tested : " ${file})#change file will be tested -->
        message("Creating test rules ...")                            #submodule will be tested
        package_add_test_lib(${file}_TEST ${file}_TEST.cpp )
    endforeach()
endfunction()

addSubdirectories()