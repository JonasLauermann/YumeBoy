#include "ppu/LCD.hpp"

#include <cassert>
#include <vector>
#include <iostream>


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
    SDL_RenderCopy(renderer.get(), pixel_matrix_texture.get(), nullptr, nullptr);
    SDL_RenderPresent(renderer.get());

    buffer_it = pixel_buffer.begin();
}

#ifndef NDEBUG
bool LCD::screenshot(const char* fileName) const {
    int width, height;
    SDL_QueryTexture(pixel_matrix_texture.get(), nullptr, nullptr, &width, &height);

    // Create a target texture that allows rendering
    SDL_Texture* renderableTexture = SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, width, height);
    if (!renderableTexture) {
        std::cerr << "Failed to create renderable texture: " << SDL_GetError() << std::endl;
        return false;
    }

    // Set the target texture as the rendering target
    if (SDL_SetRenderTarget(renderer.get(), renderableTexture) != 0) {
        std::cerr << "Failed to set render target: " << SDL_GetError() << std::endl;
        SDL_DestroyTexture(renderableTexture);
        return false;
    }

    // Copy the streaming texture to the renderable target texture
    if (SDL_RenderCopy(renderer.get(), pixel_matrix_texture.get(), nullptr, nullptr) != 0) {
        std::cerr << "Failed to copy texture: " << SDL_GetError() << std::endl;
        SDL_DestroyTexture(renderableTexture);
        return false;
    }

    // Create a surface to hold the pixel data
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888);
    if (!surface) {
        std::cerr << "Failed to create surface: " << SDL_GetError() << std::endl;
        SDL_DestroyTexture(renderableTexture);
        return false;
    }

    // Read pixels from the renderable target texture
    if (SDL_RenderReadPixels(renderer.get(), nullptr, surface->format->format, surface->pixels, surface->pitch) != 0) {
        std::cerr << "Failed to read pixels: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(renderableTexture);
        return false;
    }

    // Save the surface as a BMP file
    if (SDL_SaveBMP(surface, fileName) != 0) {
        std::cerr << "Failed to save BMP: " << SDL_GetError() << std::endl;
    }

    // Clean up
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(renderableTexture);

    // Reset the rendering target back to the default (usually the window)
    SDL_SetRenderTarget(renderer.get(), nullptr);

    return true;
}
#endif