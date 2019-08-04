#ifndef CHIP8
#define CHIP8

#include <array>
#include <cstdint>
#include <random>
#include <string>

class Chip8 {
   private:
    std::array<uint8_t, 4096> memory;       // ram, first 512 bytes reserved
    std::array<uint8_t, 16> V;              // general registers, VF = carry bit
    std::array<uint16_t, 16> stack;         // subroutine return addresses
    std::array<uint8_t, 64 * 32> graphics;  // holds pixel data
    std::array<uint8_t, 16> keys;           // stores hexadecimal keypad
    std::uint8_t delay_timer;               // decrements at 60Hz when nonzero
    std::uint8_t sound_timer;               // decrements at 60Hz when nonzero
    std::uint16_t I;                        // stores memory addresses
    std::uint16_t pc;                       // currently executing address
    std::uint16_t sp;                       // points to top of stack
    std::uint16_t opcode;                   // current instruction
    std::random_device rd;                  // used to obtain seed for generator
    std::mt19937 gen;                       // generates pseudo-random numbers
    bool draw_flag;                         // true when gfx needs to be updated

   public:
    Chip8();
    void load_rom(std::string path);
    void emulate_cycle();
    bool get_draw_flag();
    void set_draw_flag(bool value);
};
#endif