#pragma once

#include <array>
#include <bitset>
#include <cstdint>
#include <map>
#include <raylib.h>
#include <vector>

namespace chipate {

class Chip8 {
public:
    Chip8();
    void init(std::vector<uint8_t> const &program);
    void tick();
    void setKey(int key, bool pressed);

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
    std::array<uint8_t, 4096>        memory;
    std::array<std::bitset<64>, 128> FB; // LowRes Frame buffer
    std::array<uint16_t, 16>         S;  // Stack
    std::array<uint8_t, 16>          V;  // V0 to VF

    uint16_t PC; // Program counter
    uint16_t I;  // Index register
    uint8_t  SP; // Stack pointer

    std::map<uint8_t, bool> K;

    // Timers
    uint8_t delayTimer;
    uint8_t soundTimer;

    bool    waitForKey;
    uint8_t waitForKeyReg;

    bool hiResMode;

    bool exec(uint16_t instruction);

    bool exec_clrs(uint16_t instruction);
    bool exec_retn(uint16_t instruction);
    bool exec_jump(uint16_t instruction);
    bool exec_call(uint16_t instruction);
    bool exec_skeq(uint16_t instruction);
    bool exec_skne(uint16_t instruction);
    bool exec_sreq(uint16_t instruction);
    bool exec_ldim(uint16_t instruction);
    bool exec_addi(uint16_t instruction);
    bool exec_ldrg(uint16_t instruction);
    bool exec_orrg(uint16_t instruction);
    bool exec_andr(uint16_t instruction);
    bool exec_xorr(uint16_t instruction);
    bool exec_addc(uint16_t instruction);
    bool exec_subr(uint16_t instruction);
    bool exec_shrr(uint16_t instruction);
    bool exec_subn(uint16_t instruction);
    bool exec_shlr(uint16_t instruction);
    bool exec_sknr(uint16_t instruction);
    bool exec_ldix(uint16_t instruction);
    bool exec_jmpv(uint16_t instruction);
    bool exec_rand(uint16_t instruction);
    bool exec_draw(uint16_t instruction);
    bool exec_skip(uint16_t instruction);
    bool exec_sknp(uint16_t instruction);
    bool exec_lddt(uint16_t instruction);
    bool exec_ldky(uint16_t instruction);
    bool exec_stdt(uint16_t instruction);
    bool exec_stst(uint16_t instruction);
    bool exec_adin(uint16_t instruction);
    bool exec_ldsp(uint16_t instruction);
    bool exec_lbcd(uint16_t instruction);
    bool exec_strg(uint16_t instruction);
    bool exec_ldrm(uint16_t instruction);

    // Super Chip-48
    bool exec_hirs(uint16_t instruction);
    bool exec_lors(uint16_t instruction);
    bool exec_scrd(uint16_t instruction);
    bool exec_scrl(uint16_t instruction);
    bool exec_scrr(uint16_t instruction);

    bool push(uint16_t data);
    bool pop(uint16_t &data);

    bool step();
};

} // namespace chipate
