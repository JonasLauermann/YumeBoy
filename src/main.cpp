#include <YumeBoy.hpp>
#include <cstdlib>
#include <span>

struct Options {
    std::string rom_path;
    bool skip_bootrom = false;
    bool show_help = false;
};

Options parse_args(std::span<char*> args) {
    Options opts;
    auto has_rom = false;

    for (size_t i = 1; i < args.size(); ++i) {
        std::string_view arg = args[i];

        if (arg == "--skip_bootrom") {
            opts.skip_bootrom = true;
        } else if (arg == "-h" or arg == "--help") {
            opts.show_help = true;
        } else {
            opts.rom_path = arg;
            has_rom = true;
        }
    }

    if (not has_rom) {
        std::cerr << "Error: no ROM path provided\n";
        opts.show_help = true;
    }

    return opts;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {

    auto opts = parse_args(std::span(argv, argc));
    if (opts.show_help) {
        std::cout << "Usage: yumeboy [options] <rom_path>\n"
                  << "  --skip_bootrom      Skip startup boot ROM\n";
        return EXIT_SUCCESS;
    }
    
    YumeBoy yume_boy(opts.rom_path, opts.skip_bootrom);
    while (true)
        yume_boy.tick();
    return EXIT_SUCCESS;
}
