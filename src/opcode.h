// SPDX-License-Identifier: WTFPL

#pragma once

#include <array>
#include <cstdint>

namespace chipate {

// nnn or addr - A 12-bit value, the lowest 12 bits of the instruction
// n or nibble - A 4-bit value, the lowest 4 bits of the instruction
// x - A 4-bit value, the lower 4 bits of the high byte of the instruction
// y - A 4-bit value, the upper 4 bits of the low byte of the instruction
// kk or byte - An 8-bit value, the lowest 8 bits of the instruction
enum Opcode : uint16_t
{
    CLS = 0x00E0,  // [....] Clear the display
    RET = 0x00EE,  // [....] Return from subroutine
    JP = 0x1000,   // [1nnn] Jump to address: PC = nnn
    CALL = 0x2000, // [2nnn] Call subroutine: Push current PC to stack, PC = nnn
    SE = 0x3000,   // [3xkk] Skip if equal: SE Vx, kk
    SNE = 0x4000,  // [4xkk] Skip if not equal: SNE Vx, kk
    SER = 0x5000,  // [5xy0] Skip if registers equal: SE Vx, Vy
    LD = 0x6000,   // [6xkk] Load immediate: LD Vx, kk
    ADD = 0x7000,  // [7xkk] Add immediate: ADD Vx, kk
    LDR = 0x8000,  // [8xy0] Load register: LD Vx, Vy
    OR = 0x8001,   // [8xy1] Bitwise OR: OR Vx, Vy
    AND = 0x8002,  // [8xy2] Bitwise AND: AND Vx, Vy
    XOR = 0x8003,  // [8xy3] Bitwise XOR: XOR Vx, Vy
    ADDC = 0x8004, // [8xy4] Add with carry: ADD Vx, Vy
    SUB = 0x8005,  // [8xy5] Subtract with borrow: SUB Vx, Vy
    SHR = 0x8006,  // [8xy6] Shift right: SHR Vx
    SUBN = 0x8007, // [8xy7] Subtract negated: SUBN Vx, Vy
    SHL = 0x800E,  // [8xyE] Shift left: SHL Vx
    SNER = 0x9000, // [9xy0] Skip next instruction if registers not equal: SNE Vx, Vy
    LDI = 0xA000,  // [Annn] Load index register: LD I, nnn
    JPO = 0xB000,  // [Bnnn] Jump with offset: JP V0, nnn
    RND = 0xC000,  // [Ckkk] Random number: RND Vx, kk
    DRW = 0xD000,  // [DxyN] Draw sprite: DRW Vx, Vy, N
    SKP = 0xE09E,  // [Ex9E] Skip if key pressed: SKP Vx
    SKNP = 0xE0A1, // [ExA1] Skip if key not pressed: SKNP Vx
    LDRD = 0xF007, // [Fx07] Load delay timer: LD Vx, DT
    LDK = 0xF00A,  // [Fx0A] Wait for key press: LD Vx, K
    LDDR = 0xF015, // [Fx15] Set delay timer: LD DT, Vx
    LDSR = 0xF018, // [Fx18] Set sound timer: LD ST, Vx
    ADDI = 0xF01E, // [Fx1E] Add to index register: ADD I, Vx
    LDS = 0xF029,  // [Fx29] Load sprite location: LD F, Vx
    LBCD = 0xF033, // [Fx33] Load BCD representation: LD B, Vx
    LDMR = 0xF055, // [Fx55] Store registers: LD [I], Vx
    LDRM = 0xF065, // [Fx65] Load registers: LD Vx, [I]

    // Super Chip-48 opcodes
    HIRS = 0x00FF, // [00FF] Enable high-resolution mode
    LORS = 0x00FE, // [00FE] Enable low-resolution mode
    SCRD = 0x00C0, // [00Cn] Scroll down
    SCRL = 0x00FC, // [00FC] Scroll left
    SCRR = 0x00FB  // [00FB] Scroll right

};

struct OpcodeMatch {
    Opcode opcode;
    uint16_t mask;
};

static std::array<OpcodeMatch, 40> opcodeMatches{
    OpcodeMatch{CLS,  0xFFFF},
    {RET,  0xFFFF},
    {JP,   0xF000},
    {CALL, 0xF000},
    {SE,   0xF000},
    {SNE,  0xF000},
    {SER,  0xF000},
    {LD,   0xF000},
    {ADD,  0xF000},
    {LDR,  0xF00F},
    {OR,   0xF00F},
    {AND,  0xF00F},
    {XOR,  0xF00F},
    {ADDC, 0xF00F},
    {SUB,  0xF00F},
    {SHR,  0xF00F},
    {SUBN, 0xF00F},
    {SHL,  0xF00F},
    {SNER, 0xF000},
    {LDI,  0xF000},
    {JPO,  0xF000},
    {RND,  0xF000},
    {DRW,  0xF000},
    {SKP,  0xF0FF},
    {SKNP, 0xF0FF},
    {LDRD, 0xF00F},
    {LDK,  0xF00F},
    {LDDR, 0xF0FF},
    {LDSR, 0xF0FF},
    {ADDI, 0xF0FF},
    {LDS,  0xF0FF},
    {LBCD, 0xF0FF},
    {LDMR, 0xF0FF},
    {LDRM, 0xF0FF},
    {HIRS, 0xFFFF},
    {LORS, 0xFFFF},
    {SCRD, 0xFFF0},
    {SCRL, 0xFFFF},
    {SCRR, 0xFFFF}
};
} // namespace chipate
