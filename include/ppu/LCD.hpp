#pragma once

#include <array>
#include <memory>
#include <SDL3/SDL.h>


struct LCDSaveState;

class LCD {
    public:
    static const uint8_t DISPLAY_WIDTH = 160;
    static const uint8_t DISPLAY_HEIGHT = 144;
    static constexpr uint64_t FRAME_NS = 16740000;  // number of nanoseconds between frames

    using pixel_buffer_t = std::array<uint8_t, DISPLAY_WIDTH * DISPLAY_HEIGHT * 4>;

    private:
    struct sdl_deleter
    {
        void operator()(SDL_Window *p) const { SDL_DestroyWindow(p); }
        void operator()(SDL_Renderer *p) const { SDL_DestroyRenderer(p); }
        void operator()(SDL_Texture *p) const { SDL_DestroyTexture(p); }
    };

    std::unique_ptr<SDL_Window, sdl_deleter> window;
    std::unique_ptr<SDL_Renderer, sdl_deleter> renderer;
    std::unique_ptr<SDL_Texture, sdl_deleter> pixel_matrix_texture;
    pixel_buffer_t pixel_buffer;
    pixel_buffer_t::iterator buffer_it;

    bool power_ = false;

    uint64_t next_frame = FRAME_NS;   // time until the next frame should be rendered, time given in nanoseconds

    public:
    enum class Color : uint8_t {
        WHITE = 0,
        LIGHT_GRAY = 1,
        DARK_GRAY = 2,
        BLACK = 3
    };

    LCD([[maybe_unused]] const char *title, [[maybe_unused]] int width, [[maybe_unused]] int height) : buffer_it(pixel_buffer.begin()) {
        SDL_Init(SDL_INIT_VIDEO);

        window = std::unique_ptr<SDL_Window, sdl_deleter>(SDL_CreateWindow("YumeBoy", DISPLAY_WIDTH * 4, DISPLAY_HEIGHT * 4, SDL_WINDOW_BORDERLESS), sdl_deleter());
        renderer = std::unique_ptr<SDL_Renderer, sdl_deleter>(SDL_CreateRenderer(window.get(), nullptr), sdl_deleter());
        pixel_matrix_texture =  std::unique_ptr<SDL_Texture, sdl_deleter>(SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, DISPLAY_WIDTH, DISPLAY_HEIGHT), sdl_deleter());
        /* use nearest pixel scaling mode for a pixel perfect image */
        SDL_SetTextureScaleMode(pixel_matrix_texture.get(), SDL_SCALEMODE_NEAREST);
    }

    void power(bool on) { power_ = on; }

    void push_pixel(Color c);

    void update_screen();

    LCDSaveState save_state();

    void load_state(LCDSaveState state);
    
#ifndef NDEBUG
    bool screenshot(const char* fileName) const;
#endif
};