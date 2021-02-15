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

#it has possibly to use a custom libraries
macro(package_add_test_lib TESTNAME FILES )
    add_executable(${TESTNAME} ${FILES})
    target_link_libraries(${TESTNAME} ${GTEST_LIBRARIES} pthread)
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

macro(checkTestFilesAreDefined)
    if(NOT (DEFINED TestFiles)) 
        message(FATAL_ERROR "CMake testing syntax error: TestFiles not defined!")
    endif()
endmacro()

macro(addSubdirectory subdirName)
    add_subdirectory(${subdirName})
    unset(TestFiles)
endmacro()

macro(addAllSubdirectories)
    addSubdirectory(Core)
    addSubdirectory(asyncTestEngine)
endmacro()

function(buildTests testNames)
    foreach(file ${testNames})
        message("The following file will be tested : " ${file})#change file will be tested -->
        message("Creating test rules ...")                            #submodule will be tested
        package_add_test_lib(${file}_TEST ${file}_TEST.cpp )
    endforeach()
endfunction()

if(${TestModule} STREQUAL "All")
    addAllSubdirectories()
elseif(EXISTS TestModule)
    add_subdirectory(${TestModule})
else()
    message(FATAL_ERROR "Bad TestModule option, aborting...")
endif()