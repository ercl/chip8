#ifndef CHIP8
#define CHIP_8

#include <array>
#include <cstdint>
#include <string>

class Chip8 {
    private:
        std::array<uint8_t, 4096> memory; // ram, first 512 bytes reserved
        std::array<uint8_t, 16> V; // general registers, VF is carry flag
        std::array<uint16_t, 16> stack; // stores subroutine return addresses
        std::array<uint8_t, 64 * 32> graphics; // holds pixel data
        std::array<uint8_t, 16> keys; // stores hexadecimal keypad
        uint8_t delay_timer; // decrements at 60Hz when nonzero
        uint8_t sound_timer; // decrements at 60Hz when nonzero
        uint16_t I; // stores memory addresses
        uint16_t pc; // stores the currently executing address
        uint16_t sp; // points to top of stack
        uint16_t opcode; // current instruction
        bool draw_flag; // true when graphics need to be drawn
    public:
        void load_rom(std::string path);
        void emulate_cycle();
};
#endif