// SPDX-License-Identifier: WTFPL

#pragma once

#include <array>
#include <bitset>
#include <cstdint>
#include <map>
#include <raylib.h>
#include <vector>

namespace chipate {

using Registers = std::array<uint8_t, 16>;

struct Quirks {
    // Shift operations only use Vx
    bool shiftVxOnly = false;
    // Load/Store increment I by X + 1
    bool loadStoreIAdd = false;
    // Jump with Vx offset, where x is the most significant nibble of nnn instead of V0
    bool jumpWithVx = false;
    // Bitwise logic do not set VF
    bool logicNoVF = false;
    // Wrap sprites around screen edges
    bool spriteWrap = false;
    // Legacy SCHIP scroll n/2
    bool legacySchipScroll = false;
};

struct Instruction {
    Instruction(uint16_t d, Registers& regs)
        : data(d)
        , registers(regs)
    {}
    uint16_t data;
    Registers& registers;

    uint8_t& vx()
    {
        return registers[x()];
    }
    uint8_t& vy()
    {
        return registers[y()];
    }

    uint16_t nnn() const
    {
        return data & 0x0FFF;
    }
    uint8_t n() const
    {
        return data & 0x000F;
    }
    uint8_t x() const
    {
        return (data & 0x0F00) >> 8;
    }
    uint8_t y() const
    {
        return (data & 0x00F0) >> 4;
    }
    uint8_t kk() const
    {
        return data & 0x00FF;
    }
};

class Chip8 {
public:
    Chip8();
    void init(std::vector<uint8_t> const& program, Quirks const& quirks = {});
    void tick();
    void tock();
    void setKey(int key, bool pressed);
    void setQuirks(Quirks const& quirks)
    {
        this->quirks = quirks;
    }

    bool hiRes() const
    {
        return hiResMode;
    }

    // Grant tests access to internals without adding public accessors
    friend class Chip8TestAccess;

    std::array<std::bitset<64>, 128> fb() const
    {
        return FB;
    }

private:
    std::array<uint8_t, 4096> memory;
    std::array<std::bitset<64>, 128> FB; // LowRes Frame buffer
    std::array<uint16_t, 16> S;          // Stack
    std::array<uint8_t, 16> V;           // V0 to VF
    uint8_t& Vf = V[0x0F];

    uint16_t PC; // Program counter
    uint16_t I;  // Index register
    uint8_t SP;  // Stack pointer

    std::map<uint8_t, bool> K;

    Quirks quirks;

    // Timers
    uint8_t delayTimer;
    uint8_t soundTimer;

    bool waitForKey;
    uint8_t waitForKeyReg;

    bool hiResMode;
    bool waitForVBlank;

    bool exec(uint16_t instruction);

    bool exec_clrs(Instruction i);
    bool exec_retn(Instruction i);
    bool exec_jump(Instruction i);
    bool exec_call(Instruction i);
    bool exec_skeq(Instruction i);
    bool exec_skne(Instruction i);
    bool exec_sreq(Instruction i);
    bool exec_ldim(Instruction i);
    bool exec_addi(Instruction i);
    bool exec_ldrg(Instruction i);
    bool exec_orrg(Instruction i);
    bool exec_andr(Instruction i);
    bool exec_xorr(Instruction i);
    bool exec_addc(Instruction i);
    bool exec_subr(Instruction i);
    bool exec_shrr(Instruction i);
    bool exec_subn(Instruction i);
    bool exec_shlr(Instruction i);
    bool exec_sknr(Instruction i);
    bool exec_ldix(Instruction i);
    bool exec_jmpv(Instruction i);
    bool exec_rand(Instruction i);
    bool exec_draw(Instruction i);
    bool exec_skip(Instruction i);
    bool exec_sknp(Instruction i);
    bool exec_lddt(Instruction i);
    bool exec_ldky(Instruction i);
    bool exec_stdt(Instruction i);
    bool exec_stst(Instruction i);
    bool exec_adin(Instruction i);
    bool exec_ldsp(Instruction i);
    bool exec_lbcd(Instruction i);
    bool exec_strg(Instruction i);
    bool exec_ldrm(Instruction i);

    // Super Chip-48
    bool exec_hirs(Instruction i);
    bool exec_lors(Instruction i);
    bool exec_scrd(Instruction i);
    bool exec_scrl(Instruction i);
    bool exec_scrr(Instruction i);

    bool push(uint16_t data);
    bool pop(uint16_t& data);

    bool step();
};

} // namespace chipate
