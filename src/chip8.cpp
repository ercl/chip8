#include "chip8.h"
#include <fstream>
#include <iostream>
#include <vector>

Chip8::Chip8() {
    // program counter must start at memory location 0x200
    pc = 0x200;

    // the stack, display, memory, and key arrays must be cleared
    memory.fill(0);
    V.fill(0);
    keys.fill(0);
    graphics.fill(0);
    stack.fill(0);

    // all registers other than the program counter must also be set to 0
    delay_timer = 0;
    sound_timer = 0;
    I = 0;
    sp = 0;

    // no initial instruction
    opcode = 0;

    // nothing to draw initially
    draw_flag = false;

    // load the fontset into memory
    std::array<uint8_t, 80> fontset = {{
        0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
        0x20, 0x60, 0x20, 0x20, 0x70,  // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
        0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
        0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
        0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
        0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
        0xF0, 0x80, 0xF0, 0x80, 0x80   // F
    }};
    for (int i = 0; i < 80; ++i) {
        memory[i] = fontset[i];
    }
}

void Chip8::load_rom(std::string path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    std::ifstream::pos_type file_size = file.tellg();
    std::vector<uint8_t> buffer(file_size);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(buffer.data()), file_size);
    for (int i = 0; i < file_size; i++) {
        memory[i + 512] = buffer[i];  // first 512 bytes are reserved
    }
}

void Chip8::emulate_cycle() {
    opcode = memory[pc] << 8 | memory[pc + 1];  // get instruction
    uint16_t x = (opcode & 0x0F00) >> 8;        // second 4 bits e.g. 0xA(B)CD
    uint16_t y = (opcode & 0x00F0) >> 4;        // third 4 bits e.g. 0xAB(C)D
    uint16_t kk = opcode & 0x00FF;              // lower byte e.g. 0xAB(CD)

    switch (opcode & 0xF000) {  // first 4 bits decide the instruction
        // first 4 bits are 0, possible instructions are 0x00E0 or 0x00EE
        case 0x0000:
            switch (opcode) {
                case 0x00E0:  // clear display
                    graphics.fill(0);
                    draw_flag = true;
                    pc += 2;
                    break;
                case 0x00EE:  // return from a subroutine
                    pc = stack[--sp];
                    pc += 2;
                    break;
                default: // invalid opcode found
                    std::cerr << "Undefined 0x0000 opcode: " << opcode << "\n";
            }
    }
}