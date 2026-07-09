add_library(
    timer
    OBJECT
    Timer.cpp
)

target_link_libraries(timer PRIVATE
    project_warnings
    project_include_dirs
)
