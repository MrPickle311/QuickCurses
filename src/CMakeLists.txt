add_subdirectory(Core)

set(MODULES
    Core
    )

add_library(ALL_LIBRARIES STATIC ${MODULES})
target_link_libraries(ALL_LIBRARIES ${MODULES})
set_target_properties(ALL_LIBRARIES PROPERTIES LINKER_LANGUAGE CXX)
