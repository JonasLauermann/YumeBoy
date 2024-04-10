#include "YumeBoy.hpp"

#include <SDL.h>

int main(int argc, char *argv[]) {
    YumeBoy yume_boy("../Tetris (World) (Rev 1).gb");
    while (true)
        yume_boy.tick();
    return 0;
}
