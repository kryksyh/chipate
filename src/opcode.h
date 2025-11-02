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
    CLRS = 0x00E0, // [....] Clear the display
    RETN = 0x00EE, // [....] Return from subroutine
    JUMP = 0x1000, // [1nnn] Jump to address: PC = nnn
    CALL = 0x2000, // [2nnn] Call subroutine: Push current PC to stack, PC = nnn
    SKEQ = 0x3000, // [3xkk] Skip if equal: SE Vx, kk
    SKNE = 0x4000, // [4xkk] Skip if not equal: SNE Vx, kk
    SREQ = 0x5000, // [5xy0] Skip if registers equal: SE Vx, Vy
    LDIM = 0x6000, // [6xkk] Load immediate: LD Vx, kk
    ADDI = 0x7000, // [7xkk] Add immediate: ADD Vx, kk
    LDRG = 0x8000, // [8xy0] Load register: LD Vx, Vy
    ORRG = 0x8001, // [8xy1] Bitwise OR: OR Vx, Vy
    ANDR = 0x8002, // [8xy2] Bitwise AND: AND Vx, Vy
    XORR = 0x8003, // [8xy3] Bitwise XOR: XOR Vx, Vy
    ADDC = 0x8004, // [8xy4] Add with carry: ADD Vx, Vy
    SUBR = 0x8005, // [8xy5] Subtract with borrow: SUB Vx, Vy
    SHRR = 0x8006, // [8xy6] Shift right: SHR Vx
    SUBN = 0x8007, // [8xy7] Subtract negated: SUBN Vx, Vy
    SHLR = 0x800E, // [8xyE] Shift left: SHL Vx
    SKNR = 0x9000, // [9xy0] Skip next instruction if registers not equal: SNE Vx, Vy
    LDIX = 0xA000, // [Annn] Load index register: LD I, nnn
    JMPV = 0xB000, // [Bnnn] Jump with offset: JP V0, nnn
    RAND = 0xC000, // [Ckkk] Random number: RND Vx, kk
    DRAW = 0xD000, // [DxyN] Draw sprite: DRW Vx, Vy, N
    SKIP = 0xE09E, // [Ex9E] Skip if key pressed: SKP Vx
    SKNP = 0xE0A1, // [ExA1] Skip if key not pressed: SKNP Vx
    LDDT = 0xF007, // [Fx07] Load delay timer: LD Vx, DT
    LDKY = 0xF00A, // [Fx0A] Wait for key press: LD Vx, K
    STDT = 0xF015, // [Fx15] Set delay timer: LD DT, Vx
    STST = 0xF018, // [Fx18] Set sound timer: LD ST, Vx
    ADIN = 0xF01E, // [Fx1E] Add to index register: ADD I, Vx
    LDSP = 0xF029, // [Fx29] Load sprite location: LD F, Vx
    LBCD = 0xF033, // [Fx33] Load BCD representation: LD B, Vx
    STRG = 0xF055, // [Fx55] Store registers: LD [I], Vx
    LDRM = 0xF065  // [Fx65] Load registers: LD Vx, [I]
};

struct OpcodeMatch {
    Opcode   opcode;
    uint16_t mask;
};

std::array<OpcodeMatch, 35> opcodeMatches{
    OpcodeMatch{CLRS, 0xFFFF},
    {RETN, 0xFFFF},
    {JUMP, 0xF000},
    {CALL, 0xF000},
    {SKEQ, 0xF000},
    {SKNE, 0xF000},
    {SREQ, 0xF000},
    {LDIM, 0xF000},
    {ADDI, 0xF000},
    {LDRG, 0xF00F},
    {ORRG, 0xF00F},
    {ANDR, 0xF00F},
    {XORR, 0xF00F},
    {ADDC, 0xF00F},
    {SUBR, 0xF00F},
    {SHRR, 0xF00F},
    {SUBN, 0xF00F},
    {SHLR, 0xF00F},
    {SKNR, 0xF000},
    {LDIX, 0xF000},
    {JMPV, 0xF000},
    {RAND, 0xF000},
    {DRAW, 0xF000},
    {SKIP, 0xF0FF},
    {SKNP, 0xF0FF},
    {LDDT, 0xF00F},
    {LDKY, 0xF00F},
    {STDT, 0xF0FF},
    {STST, 0xF0FF},
    {ADIN, 0xF0FF},
    {LDSP, 0xF0FF},
    {LBCD, 0xF0FF},
    {STRG, 0xF0FF},
    {LDRM, 0xF0FF}
};
} // namespace chipate
