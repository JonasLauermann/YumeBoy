add_subdirectory(apu)
add_subdirectory(cartridge)
add_subdirectory(cpu)
add_subdirectory(joypad)
add_subdirectory(ppu)
add_subdirectory(timer)
add_subdirectory(mmu)


set(
    YUMEBOY_SOURCES
    main.cpp
    $<TARGET_OBJECTS:apu>
    $<TARGET_OBJECTS:cartridge>
    $<TARGET_OBJECTS:cpu>
    $<TARGET_OBJECTS:joypad>
    $<TARGET_OBJECTS:ppu>
    $<TARGET_OBJECTS:timer>
    $<TARGET_OBJECTS:mmu>
)

add_executable(${PROJECT_NAME} ${YUMEBOY_SOURCES})
target_link_libraries(${PROJECT_NAME} SDL3::SDL3 ${Boost_LIBRARIES})
