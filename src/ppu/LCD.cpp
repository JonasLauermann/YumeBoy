#include "ppu/LCD.hpp"

#include <cassert>
#include <vector>
#include <iostream>

#include <savestate/LCDSaveState.hpp>


void LCD::push_pixel(Color c)
{
    assert(buffer_it != pixel_buffer.end());

    switch (c)
    {
    case Color::WHITE:
        *buffer_it++ = 233;
        *buffer_it++ = 239;
        *buffer_it++ = 236;
        *buffer_it++ = 255;
        break;

    case Color::LIGHT_GRAY:
        *buffer_it++ = 160;
        *buffer_it++ = 160;
        *buffer_it++ = 139;
        *buffer_it++ = 255;
        break;

    case Color::DARK_GRAY:
        *buffer_it++ = 85;
        *buffer_it++ = 85;
        *buffer_it++ = 104;
        *buffer_it++ = 255;
        break;

    case Color::BLACK:
        *buffer_it++ = 33;
        *buffer_it++ = 30;
        *buffer_it++ = 32;
        *buffer_it++ = 255;
        break;

    default:
        std::unreachable();
    }
}

void LCD::update_screen()
{
    assert(buffer_it == pixel_buffer.end());

    if (power_) [[likely]]
    {
        SDL_UpdateTexture(pixel_matrix_texture.get(), nullptr, pixel_buffer.data(), DISPLAY_WIDTH * sizeof(uint8_t) * 4);
    }
    else
    {
        SDL_UpdateTexture(pixel_matrix_texture.get(), nullptr, std::vector<uint8_t>(255).data(), DISPLAY_WIDTH * sizeof(uint8_t) * 4);
    }
    SDL_RenderTexture(renderer.get(), pixel_matrix_texture.get(), nullptr, nullptr);
    SDL_RenderPresent(renderer.get());

    buffer_it = pixel_buffer.begin();

    // check if the next frame should be rendered or if the thread should sleep
    if (SDL_GetTicksNS() < next_frame) {
        SDL_DelayNS(next_frame - SDL_GetTicksNS());
    }
    next_frame = SDL_GetTicksNS() + FRAME_NS;
}

LCDSaveState LCD::save_state()
{
    LCDSaveState s = {
        pixel_buffer,
        buffer_it,

        power_,

        next_frame,
    };
    return s;
}

void LCD::load_state(LCDSaveState state)
{
    pixel_buffer = state.pixel_buffer;
    buffer_it = state.buffer_it;

    power_ = state.power_;

    next_frame = state.next_frame;
}

#ifndef NDEBUG
bool LCD::screenshot(const char *fileName) const
{
    float width_f, height_f;
    SDL_GetTextureSize(pixel_matrix_texture.get(), &width_f, &height_f);
    auto width = (int)width_f;
    auto height = (int)height_f;

    // Create a target texture that allows rendering
    SDL_Texture* renderableTexture = SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, width, height);
    if (!renderableTexture) {
        std::cerr << "Failed to create renderable texture: " << SDL_GetError() << std::endl;
        return false;
    }

    // Set the target texture as the rendering target
    if (not SDL_SetRenderTarget(renderer.get(), renderableTexture)) {
        std::cerr << "Failed to set render target: " << SDL_GetError() << std::endl;
        SDL_DestroyTexture(renderableTexture);
        return false;
    }

    // Copy the streaming texture to the renderable target texture
    if (not SDL_RenderTexture(renderer.get(), pixel_matrix_texture.get(), nullptr, nullptr)) {
        std::cerr << "Failed to copy texture: " << SDL_GetError() << std::endl;
        SDL_DestroyTexture(renderableTexture);
        return false;
    }

    // Read pixels from the renderable target texture
    SDL_Surface* surface = SDL_RenderReadPixels(renderer.get(), nullptr);
    if (not surface) {
        std::cerr << "Failed to read pixels: " << SDL_GetError() << std::endl;
        SDL_DestroyTexture(renderableTexture);
        return false;
    }

    // Save the surface as a BMP file
    if (SDL_SaveBMP(surface, fileName) != 0) {
        std::cerr << "Failed to save BMP: " << SDL_GetError() << std::endl;
    }

    // Clean up
    SDL_DestroySurface(surface);
    SDL_DestroyTexture(renderableTexture);

    // Reset the rendering target back to the default (usually the window)
    SDL_SetRenderTarget(renderer.get(), nullptr);

    return true;
}
#endif