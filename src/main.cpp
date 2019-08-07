#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
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

constexpr std::array<SDL_Keycode, 16> keymap{
    SDLK_x, SDLK_1, SDLK_2, SDLK_3,   // 0 1 2 3
    SDLK_q, SDLK_w, SDLK_e, SDLK_a,   // 4 5 6 7
    SDLK_s, SDLK_d, SDLK_z, SDLK_c,   // 8 9 A B
    SDLK_4, SDLK_r, SDLK_f, SDLK_v};  // C D E F

void sdl_error() {
    std::cerr << "SDL has encountered an error: ";
    std::cerr << SDL_GetError() << "\n";
    SDL_Quit();
    exit(1);
}

void init_sdl(SDL_Window*& window, SDL_Texture*& texture, SDL_Renderer*& renderer) {
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
}

void init_audio(Mix_Chunk*& chunk) {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer has encountered an error: ";
        std::cerr << Mix_GetError() << "\n";
        SDL_Quit();
        Mix_Quit();
        exit(1);
    }
    chunk = Mix_LoadWAV("../resources/beep.wav");
    if (chunk == nullptr) {
        std::cerr << "SDL_mixer has encountered an error: ";
        std::cerr << Mix_GetError() << "\n";
        SDL_Quit();
        Mix_Quit();
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    SDL_Window* window = nullptr;
    SDL_Texture* texture = nullptr;
    SDL_Renderer* renderer = nullptr;
    Mix_Chunk* chunk = nullptr;
    SDL_Event event;

    init_sdl(window, texture, renderer);
    init_audio(chunk);

    Chip8 chip8;
    if (argc > 1) {
        chip8.load_rom(argv[1]);
    } else {
        chip8.load_rom("../roms/PONG2");
    }

    std::uint32_t start_time;
    std::uint32_t delta_time;
    bool quit = false;
    while (!quit) {
        start_time = SDL_GetTicks();
        for (int i = 0; i < INSTRUCTIONS_PER_STEP; i++) {
            chip8.emulate_cycle();
        }
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    for (int i = 0; i < keymap.size(); i++) {
                        if (event.key.keysym.sym == keymap[i]) {
                            chip8.press_key(i);
                        }
                    }
                    break;
                case SDL_KEYUP:
                    for (int i = 0; i < keymap.size(); i++) {
                        if (event.key.keysym.sym == keymap[i]) {
                            chip8.release_key(i);
                        }
                    }
                    break;
            }
        }

        if (chip8.get_sound_timer() > 0) {
            Mix_PlayChannel(-1, chunk, 0);
        }

        if (chip8.get_draw_flag()) {
            chip8.reset_draw_flag();
            std::uint32_t* pixels = nullptr;
            int pitch;
            SDL_LockTexture(texture, nullptr, reinterpret_cast<void**>(&pixels), &pitch);
            for (int i = 0; i < WIDTH * HEIGHT; i++) {
                pixels[i] = (chip8.get_pixel_data(i) == 0) ? 0x000000FF : 0xFFFFFFFF;
            }
            SDL_UnlockTexture(texture);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }
        delta_time = SDL_GetTicks() - start_time;
        if (TICKS_PER_FRAME > delta_time) {
            chip8.step_timers();
            SDL_Delay(TICKS_PER_FRAME - delta_time);
        }
    }
    Mix_FreeChunk(chunk);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_Quit();
    SDL_Quit();
    return 0;
}