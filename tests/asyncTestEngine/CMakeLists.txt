message("Loading tests/asyncTestEngine module...")

find_package(Boost 1.75.0)

if(Boost_FOUND)
    message("Boost found!")
endif()

set(TestLibs
    ThreadsafeHashTable
    AsyncTestEngine
    )

if(${TestModule} STREQUAL "All")
    set(TestFiles ${TestLibs})
else()
    checkTestFilesAreDefined()
endif()

#TODO : CREATE LIB ASYNCENGINE !!!
#link_libraries(GTest::GTest GTest::Main)
#add_library(AsyncEngine STATIC)
#target_sources(AsyncEngine PUBLIC ThreadsafeHashTable.hpp)
#set_target_properties(AsyncEngine PROPERTIES LINKER_LANGUAGE CXX)
#target_link_libraries(AsyncEngine GTest::GTest GTest::Main)

set(EXTRA_LIBRARIES
    
    )
include_directories(${GTEST_INCLUDE_DIRS} ${Boost_INCLUDE_DIR} )
link_libraries(${GTEST_LIBRARIES} ${Boost_LIBRARIES} pthread)

buildTests("${TestFiles}")
