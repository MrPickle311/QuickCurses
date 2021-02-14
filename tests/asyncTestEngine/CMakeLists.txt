message("Engine")
find_package(GTest REQUIRED)
find_package(TBB REQUIRED tbb)
if(${GTEST_FOUND})
    message("FOUND GTEST INT ENGINE")
    enable_testing()
else()
    message(FATAL_ERROR "GTest not found!!!")
endif()

#link_libraries(GTest::GTest GTest::Main)
#add_library(AsyncEngine STATIC)
#target_sources(AsyncEngine PUBLIC ThreadsafeHashTable.hpp)
#set_target_properties(AsyncEngine PROPERTIES LINKER_LANGUAGE CXX)
#target_link_libraries(AsyncEngine GTest::GTest GTest::Main)

link_libraries(${GTEST_LIBRARIES} ${TBB_IMPORTED_TARGETS} pthread)
include_directories(../../include/asyncTestEngine 
                    ${GTEST_INCLUDE_DIRS}
                   )

add_executable(exec exec.cpp)
target_link_libraries(exec ${GTEST_LIBRARIES} pthread)
target_include_directories(exec PUBLIC ../../include/asyncTestEngine 
${GTEST_INCLUDE_DIRS})