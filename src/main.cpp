#include <SDL2/SDL.h>
#include <array>
#include <cstdint>
#include <iostream>
#include "chip8.h"

constexpr int WIDTH = 64;
constexpr int HEIGHT = 32;
constexpr int SCALE = 10;
constexpr int FPS = 60;
constexpr int TICKS_PER_FRAME = 1000 / FPS;
constexpr int INSTRUCTIONS_PER_STEP = 10;

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

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        sdl_error();
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    if (texture == nullptr) {
        sdl_error();
    }

    Chip8 chip8;
    chip8.load_rom("../roms/PONG2");

    std::uint32_t start;
    std::uint32_t delta_time;
    bool quit = false;
    while (!quit) {
        start = SDL_GetTicks();
        for (int i = 0; i < INSTRUCTIONS_PER_STEP; i++) {
            chip8.emulate_cycle();
        }
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
        delta_time = SDL_GetTicks() - start;
        if (TICKS_PER_FRAME > delta_time) {
            chip8.step_timers();
            SDL_Delay(TICKS_PER_FRAME - delta_time);
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}