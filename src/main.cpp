#include <SDL2/SDL.h>
#include <array>
#include <iostream>

constexpr int SCREEN_WIDTH = 640;
constexpr int SCREEN_HEIGHT = 320;

int main() {
    std::cout << "Hello World\n";

    SDL_Window* window = nullptr;
    SDL_Texture* texture = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Event event;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not be initialized! SDL_Error: ";
        std::cerr << SDL_GetError() << "\n";
        SDL_Quit();
        exit(1);
    }

    window = SDL_CreateWindow("chip8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: ";
        std::cerr << SDL_GetError() << "\n";
        SDL_Quit();
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL_Error: ";
        std::cerr << SDL_GetError() << "\n";
        SDL_Quit();
        exit(1);
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    if (texture == nullptr) {
        std::cerr << "Texture could not be created! SDL_Error: ";
        std::cerr << SDL_GetError() << "\n";
        SDL_Quit();
        exit(1);
    }
    
    std::array<uint32_t, 64 * 32> spixels;
    spixels.fill(0xFFFFFFFF);
    uint32_t* pixels = nullptr;
    int pitch;
    SDL_LockTexture(texture, nullptr, reinterpret_cast<void**>(&pixels), &pitch);

    for (int i = 0; i < 64 * 32; i++) {
        pixels[i] = spixels[i];
    }

    SDL_UnlockTexture(texture);

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);

    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
        SDL_Delay(10);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}