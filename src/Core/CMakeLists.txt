add_library(Core STATIC
            Engine_base.cpp
            Object_base.cpp
            EventLoop_base.cpp
            Window_base.cpp
            )
set_target_properties(Core PROPERTIES LINKER_LANGUAGE CXX)
