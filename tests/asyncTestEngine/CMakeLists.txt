message("Loading tests/asyncTestEngine module...")
 
set(TestLibs
    ThreadsafeHashTable
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
include_directories(${GTEST_INCLUDE_DIRS})
link_libraries(${GTEST_LIBRARIES} pthread)

buildTests(${TestFiles})
