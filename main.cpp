#include "YumeBoy.hpp"

int main() {
    YumeBoy yume_boy("../Tetris (World) (Rev 1).gb");
    while (true)
        yume_boy.tick();
    return 0;
}
