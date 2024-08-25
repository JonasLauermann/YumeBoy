#include "YumeBoy.hpp"

#include <SDL.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    // YumeBoy yume_boy("../Tetris (World) (Rev 1).gb");
    YumeBoy yume_boy("../gb-test-roms/cpu_instrs/cpu_instrs.gb");
    while (true)
        yume_boy.tick();
    return 0;
}
