#ifndef CHIP8
#define CHIP8

#include <array>
#include <cstdint>
#include <random>
#include <string>

class Chip8 {
   private:
    std::array<uint8_t, 4096> memory;  // ram, first 512 bytes reserved
    std::array<uint8_t, 16> V;         // general registers, VF = carry bit
    std::array<uint16_t, 16> stack;    // subroutine return addresses
    std::uint8_t delay_timer;          // decrements at 60Hz when nonzero
    std::uint16_t I;                   // stores memory addresses
    std::uint16_t pc;                  // currently executing address
    std::uint16_t sp;                  // points to top of stack
    std::random_device rd;             // used to obtain seed for generator
    std::mt19937 gen;                  // generates pseudo-random numbers

   public:
    Chip8();                                     // loads fontset and variables
    void load_rom(std::string const& path);      // loads user-provided rom
    void emulate_cycle();                        // go through a single cycle
    void step_timers();                          // decrement all timers by one
    std::array<std::uint8_t, 16> keys;           // stores hexadecimal keypad
    std::array<std::uint8_t, 64 * 32> graphics;  // holds pixel data
    std::uint8_t sound_timer;                    // decrement at 60Hz if nonzero
    bool draw_flag;                              // update gfx when true
};
#endif