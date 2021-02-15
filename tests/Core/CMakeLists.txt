message("Loading tests/Core module...")

set(TestLibs
     ThreadsafeQueue
     ThreadPool
    )

if(${TestModule} STREQUAL "All")
    set(TestFiles "${TestLibs}")
else()
    checkTestFilesAreDefined()
endif()

set(EXTRA_LIBRARIES
    
   )

include_directories(
                    ../../include/Core 
                   )
include_directories(${GTEST_INCLUDE_DIRS})
link_libraries(${EXTRA_LIBRARIES} pthread)

buildTests("${TestFiles}")