add_library(
    cpu
    OBJECT
    CPU.cpp
    instructions/Instruction.cpp
)

target_link_libraries(cpu PRIVATE
    project_warnings
    project_include_dirs
)
