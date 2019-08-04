#include <SDL2/SDL.h>
#include <array>
#include <cstdint>
#include <iostream>
#include "chip8.h"

constexpr int WIDTH = 64;
constexpr int HEIGHT = 32;
constexpr int SCALE = 10;

void sdl_error() {
    std::cerr << "SDL has encountered an error: ";
    std::cerr << SDL_GetError() << "\n";
    SDL_Quit();
    exit(1);
}

int main() {
    SDL_Window* window = nullptr;
    SDL_Texture* texture = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Event event;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        sdl_error();
    }

    window = SDL_CreateWindow("chip8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH * SCALE, HEIGHT * SCALE, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        sdl_error();
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == nullptr) {
        sdl_error();
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    if (texture == nullptr) {
        sdl_error();
    }

    std::array<std::uint32_t, 64 * 32> spixels;
    spixels.fill(0xFFFFFFFF);

    Chip8 chip8;
    chip8.load_rom("../roms/PONG");

    bool quit = false;
    while (!quit) {
        chip8.emulate_cycle();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
        if (chip8.get_draw_flag()) {
            chip8.set_draw_flag(false);
            std::uint32_t* pixels = nullptr;
            int pitch;

            SDL_LockTexture(texture, nullptr, reinterpret_cast<void**>(&pixels), &pitch);
            for (int i = 0; i < WIDTH * HEIGHT; i++) {
                pixels[i] = (chip8.get_graphics_value(i) == 0) ? 0x000000FF : 0xFFFFFFFF;
            }
            SDL_UnlockTexture(texture);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }
        SDL_Delay(3);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}