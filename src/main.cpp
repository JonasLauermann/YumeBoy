#include "YumeBoy.hpp"


int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    std::string rom_path = "../Tetris (World) (Rev 1).gb";
    // std::string rom_path = "../gb-test-roms/cpu_instrs/cpu_instrs.gb", true;
    // std::string rom_path = "../gb-test-roms/cpu_instrs/individual/01-special.gb";
    // std::string rom_path = "../gb-test-roms/cpu_instrs/individual/02-interrupts.gb";
    // std::string rom_path = "../gb-test-roms/cpu_instrs/individual/03-op sp,hl.gb";
    // std::string rom_path = "../gb-test-roms/cpu_instrs/individual/04-op r,imm.gb";
    // std::string rom_path = "../gb-test-roms/cpu_instrs/individual/05-op rp.gb";
    // std::string rom_path = "../gb-test-roms/cpu_instrs/individual/06-ld r,r.gb";
    // std::string rom_path = "../gb-test-roms/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb";
    // std::string rom_path = "../gb-test-roms/cpu_instrs/individual/08-misc instrs.gb";
    // std::string rom_path = "../gb-test-roms/cpu_instrs/individual/09-op r,r.gb";
    // std::string rom_path = "../gb-test-roms/cpu_instrs/individual/10-bit ops.gb";
    // std::string rom_path = "../gb-test-roms/cpu_instrs/individual/11-op a,(hl).gb";
    // std::string rom_path = "../gb-test-roms/instr_timing/instr_timing.gb";
    YumeBoy yume_boy(rom_path, false);
    while (true)
        yume_boy.tick();
    return 0;
}
