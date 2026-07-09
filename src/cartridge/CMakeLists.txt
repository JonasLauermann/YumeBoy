add_library(
    cartridge
    OBJECT
    Cartridge.cpp
)

target_link_libraries(cartridge PRIVATE
    project_warnings
    project_include_dirs
)
