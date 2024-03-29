#include "chip8.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

Chip8::Chip8() : memory(),  // everything but pc must be cleared initially
                 V(),
                 stack(),
                 keys(),
                 graphics(),
                 delay_timer(0),
                 sound_timer(0),
                 I(0),
                 pc(0x200),  // program counter must start at 0x200 in memory
                 sp(0),
                 draw_flag(false) {
    // initialize random number generator
    gen.seed(rd());

    // load the fontset into memory
    std::array<std::uint8_t, 80> fontset = {{
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
    std::copy(fontset.begin(), fontset.end(), memory.begin());
}

void Chip8::load_rom(std::string const& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Rom Load Error: failed to open file\n";
        exit(1);
    }

    std::ifstream::pos_type file_size = file.tellg();
    if (file_size == std::ifstream::pos_type(-1)) {  // failed to get fize size
        std::cerr << "Rom Load Error: failed to get file size\n";
    }
    if (file_size > memory.size() - 512) {
        std::cerr << "Rom Load Error: rom cannot fit in memory\n";
    }

    std::vector<std::uint8_t> buffer(file_size);
    file.seekg(0, std::ios::beg);
    if (!file) {
        std::cerr << "Rom Load Error: failed to seek data\n";
    }

    file.read(reinterpret_cast<char*>(buffer.data()), file_size);
    if (!file) {
        std::cerr << "Rom Load Error: failed to read rom\n";
    }
    std::copy(buffer.begin(), buffer.end(), memory.begin() + 512);
}

void Chip8::emulate_cycle() {
    std::uint32_t opcode = memory[pc] << 8 | memory[pc + 1];  // get instruction

    std::uint16_t x = (opcode & 0x0F00) >> 8;  // second 4 bits e.g. 0xA(B)CD
    std::uint16_t y = (opcode & 0x00F0) >> 4;  // third 4 bits e.g. 0xAB(C)D
    std::uint16_t kk = opcode & 0x00FF;        // lower byte e.g. 0xAB(CD)
    std::uint16_t n = opcode & 0x000F;         // last 4 bits e.g. 0xABC(D)

    pc += 2;

    switch (opcode & 0xF000) {  // first 4 bits decide the instruction
        case 0x0000:            // possible instructions are 0x00E0 or 0x00EE
            switch (opcode) {
                case 0x00E0:  // clear display
                    graphics.fill(0);
                    draw_flag = true;

                    break;
                case 0x00EE:  // return from a subroutine
                    pc = stack[--sp];
                    pc += 2;
                    break;
                default:  // invalid opcode found
                    std::cerr << "Undefined 0x0000 opcode: " << opcode << "\n";
            }
            break;
        case 0x1000:  // 0x1nnn, jump to address nnn
            pc = opcode & 0x0FFF;
            break;
        case 0x2000:  // 0x2nnn, call address nnn
            pc -= 2;
            stack[sp++] = pc;  // store current address on stack first
            pc = opcode & 0x0FFF;
            break;
        case 0x3000:  // 0x3xkk, skip next instruction if Vx = kk
            if (V[x] == kk) {
                pc += 2;
            }
            break;
        case 0x4000:  // 0x4xk, skip next instruction if Vx != kk
            if (V[x] != kk) {
                pc += 2;
            }
            break;
        case 0x5000:  // 0x5xy0, skip next instruction if Vx == Vy
            if (V[x] == V[y]) {
                pc += 2;
            }
            break;
        case 0x6000:  // 0x6xkk, puts value kk into Vx
            V[x] = kk;
            break;
        case 0x7000:  // 0x7xkk, set Vx = Vx + kk
            V[x] += kk;
            break;
        case 0x8000:          // possible instructions are 0x8xy(0-7, E)
            switch (n) {      // check last 4 bits, 0xABC(D)
                case 0x0000:  // 0x8xy0, set Vx = Vy
                    V[x] = V[y];
                    break;
                case 0x0001:  // 0x8xy1, set Vx = Vx OR Vy
                    V[x] |= V[y];
                    break;
                case 0x0002:  // 0x8xy2, set Vx = Vx AND Vy
                    V[x] &= V[y];
                    break;
                case 0x0003:  // 0x8xy3, set Vx = Vx XOR Vy
                    V[x] ^= V[y];
                    break;
                case 0x0004:                        // 0x8xy4, set Vx = Vx + Vy, VF = carry
                    V[0xF] = (V[x] + V[y]) > 0xFF;  // VF = 1 if carry occurs
                    V[x] += V[y];
                    break;
                case 0x0005:  // 0x8xy5, set Vx = Vx - Vy, VF = NOT borrow
                    V[0xF] = V[x] > V[y];
                    V[x] -= V[y];
                    break;
                case 0x0006:            // 0x8xy6, Vx = Vx SHR 1, VF = LSB prior to shift
                    V[0xF] = V[x] & 1;  // set as V[x]'s least significant bit
                    V[x] >>= 1;
                    // V[x] = V[y] >> 1;  // breaks Zophar ROMS
                    break;
                case 0x0007:  // 0x8xy7, set Vx = Vy - Vx, VF = NOT borrow
                    V[0xF] = V[y] > V[x];
                    V[x] = V[y] - V[x];
                    break;
                case 0x000E:             // 0x8xyE, Vx = Vx SHL 1, VF = MSB prior to shift
                    V[0xF] = V[x] >> 7;  // MSB = 8th bit since VF is an uint8_t
                    V[x] <<= 1;
                    // V[x] = V[y] << 1;  // breaks Zophar ROMS
                    break;
                default:  // invalid opcode found
                    std::cerr << "Undefined 0x8000 opcode: " << opcode << "\n";
            }
            break;
        case 0x9000:  // 0x9xy0, skip next instruction if Vx != Vy
            if (V[x] != V[y]) {
                pc += 2;
            }
            break;
        case 0xA000:  // 0xAnnn, set I = nnn
            I = opcode & 0xFFF;
            break;
        case 0xB000:  // 0xBnnn, jump to location nnn + V0
            pc = (opcode & 0xFFF) + V[0];
            break;
        case 0xC000:  // 0xCxkk, set Vx = random byte and kk
        {             // to prevent initialization error
            V[x] = std::uniform_int_distribution<>(0, 255)(gen) & kk;
        } break;
        case 0xD000:  // 0xDxyn, draws sprite
            // sprite is 8 x n pixels and located at (Vx, Vy)
            draw_flag = true;
            V[0xF] = 0;
            std::uint8_t pixel_row;  // each pixel in a row is 1 bit
            for (int y_line = 0; y_line < n; ++y_line) {
                pixel_row = memory[I + y_line];  // sprite starts at I
                for (int x_line = 0; x_line < 8; ++x_line) {
                    // go through the row 1 bit at a time
                    // true if pixel needs to be drawn
                    if (pixel_row & (0b10000000 >> x_line)) {
                        // the coordinate in row-major form
                        // must be modded with 2048 for proper wrapping
                        std::uint16_t coord = (V[x] + x_line + ((V[y] + y_line) * 64)) % 2048;
                        bool collision = (graphics[coord] == 1);
                        // OR with collision because VF is 1 when there is at
                        // least one collision
                        V[0xF] |= collision;
                        graphics[coord] ^= 1;
                    }
                }
            }
            break;
        case 0xE000:  // possible instructions are 0xEx9E, 0xExA1
            switch (kk) {
                case 0x009E:  // 0xEx9E, skip next instruction if keypress = Vx
                    if (keys[V[x]]) {
                        pc += 2;
                    }
                    break;
                case 0x00A1:  // 0xExA1, skip next instruction if keypress != Vx
                    if (!keys[V[x]]) {
                        pc += 2;
                    }
                    break;
                default:  // invalid opcode found
                    std::cerr << "Undefined 0xEx00 opcode: " << opcode << "\n";
            }
            break;
        case 0xF000:  // possible instructions: 0xFx(07,0A,15,18,1E,29,33,55,65)
            switch (kk) {
                case 0x0007:  // 0xFx07, set Vx = delay timer value
                    V[x] = delay_timer;
                    break;
                case 0x000A:  // 0xFx0A, wait for keypress, store value in Vx
                {
                    pc -= 2;
                    bool waiting = true;
                    for (int i = 0; i < keys.size(); ++i) {
                        if (keys[i] != 0) {
                            V[x] = i;
                            waiting = false;
                            break;
                        }
                    }
                    if (waiting) {
                        return;
                    }
                    pc += 2;
                    break;
                }
                case 0x0015:  // 0xFx15, set delay timer = Vx

                    delay_timer = V[x];
                    break;
                case 0x0018:  // 0xFx18, set sound timer = Vx
                    sound_timer = V[x];
                    break;
                case 0x001E:                      // 0xFx1E, set I = I + Vx
                    V[0xF] = (I + V[x]) > 0xFFF;  // check for carry
                    I += V[x];
                    break;
                case 0x0029:       // 0xFx29, set I = location of sprite for digit Vx
                    I = V[x] * 5;  // sprites are 4x5
                    break;
                case 0x0033:
                    // 0xFx33, store BCD representation of Vx at I, I+1, I+2
                    memory[I] = V[x] / 100;
                    memory[I + 1] = (V[x] / 10) % 10;
                    memory[I + 2] = V[x] % 10;
                    break;
                case 0x0055:  // 0xFx55, stores V0 - Vx in memory starting at I
                    for (int i = 0; i <= x; i++) {
                        memory[I + i] = V[i];
                    }
                    break;
                case 0x0065:  // 0xFx65, read V0 - Vx from memory starting at I
                    for (int i = 0; i <= x; i++) {
                        V[i] = memory[I + i];
                    }
                    break;
                default:  // invalid opcode found
                    std::cerr << "Undefined 0xF000 opcode: " << opcode << "\n";
            }
            break;
        default:  // invalid opcode found
            std::cerr << "Undefined opcode: " << opcode << "\n";
    }
}

void Chip8::step_timers() {
    if (delay_timer > 0) {
        delay_timer--;
    }
    if (sound_timer > 0) {
        sound_timer--;
    }
}