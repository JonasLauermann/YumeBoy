#include "YumeBoy.hpp"

#include <SDL.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    YumeBoy yume_boy("../Tetris (World) (Rev 1).gb");
    // YumeBoy yume_boy("../gb-test-roms/cpu_instrs/cpu_instrs.gb");
    // YumeBoy yume_boy("../gb-test-roms/cpu_instrs/individual/01-special.gb");
    // YumeBoy yume_boy("../gb-test-roms/cpu_instrs/individual/02-interrupts.gb");
    // YumeBoy yume_boy("../gb-test-roms/cpu_instrs/individual/03-op sp,hl.gb");
    // YumeBoy yume_boy("../gb-test-roms/cpu_instrs/individual/04-op r,imm.gb");
    // YumeBoy yume_boy("../gb-test-roms/cpu_instrs/individual/05-op rp.gb");
    // YumeBoy yume_boy("../gb-test-roms/cpu_instrs/individual/06-ld r,r.gb");
    // YumeBoy yume_boy("../gb-test-roms/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb");
    // YumeBoy yume_boy("../gb-test-roms/cpu_instrs/individual/08-misc instrs.gb");
    // YumeBoy yume_boy("../gb-test-roms/cpu_instrs/individual/09-op r,r.gb");
    // YumeBoy yume_boy("../gb-test-roms/cpu_instrs/individual/10-bit ops.gb");
    // YumeBoy yume_boy("../gb-test-roms/cpu_instrs/individual/11-op a,(hl).gb");
    // YumeBoy yume_boy("../gb-test-roms/instr_timing/instr_timing.gb");
    while (true)
        yume_boy.tick();
    return 0;
}
