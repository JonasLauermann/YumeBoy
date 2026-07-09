add_library(
    joypad
    OBJECT
    Joypad.cpp
)

target_link_libraries(joypad PRIVATE
    project_warnings
    project_include_dirs
)
