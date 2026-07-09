add_library(
    mmu
    OBJECT
    DMA.cpp
)

target_link_libraries(mmu PRIVATE
    project_warnings
    project_include_dirs
)
