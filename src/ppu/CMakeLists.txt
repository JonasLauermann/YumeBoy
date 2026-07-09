add_library(
    ppu
    OBJECT
    PPU.cpp
    LCD.cpp
    PixelFetcher.cpp
)

target_link_libraries(ppu PRIVATE
    project_warnings
    project_include_dirs
)
