#include "chip8.h"

#include "log.h"
#include "opcode.h"

#include <stdlib.h>

using namespace chipate;

Chip8::Chip8()
    : memory{}
    , FB{}
    , S{}
    , V{}
    , PC(0x200)
    , I(0)
    , SP(0)
    , delayTimer(0)
    , soundTimer(0)
    , waitForKey(false)
    , waitForKeyReg(0)
{}

void Chip8::init(std::vector<uint8_t> const& program)
{
    srand(static_cast<unsigned int>(time(nullptr)));
    memory.fill(0);
    FB.fill(0);
    S.fill(0);
    V.fill(0);
    PC         = 0x200;
    I          = 0;
    SP         = 0;
    delayTimer = 0;
    soundTimer = 0;

    // Load program into memory starting at address 0x200
    std::copy(program.begin(), program.end(), memory.begin() + 0x200);

    logi("Program loaded, size: %zu bytes", program.size());
}

void Chip8::tick()
{
    if (waitForKey) {
        logt("Waiting for key press...");
        return;
    }

    uint16_t currentInstruction = static_cast<uint16_t>(memory[PC]) << 8 | memory[PC + 1];

    logd("Fetch @%x: %x", PC, currentInstruction);

    if (!exec(currentInstruction))
        loge("Execution failed at PC: %x", PC);
}

void Chip8::setKey(int key, bool pressed)
{
    uint8_t k = static_cast<uint8_t>(key);
    K[k]      = pressed;
    if (waitForKey && pressed) {
        V[waitForKeyReg] = k;
        waitForKey       = false;
        logt("Key received: %d -> V%d", key, waitForKeyReg);
    }
}

bool Chip8::exec(uint16_t instruction)
{
    auto match =
        std::find_if(opcodeMatches.begin(), opcodeMatches.end(), [instruction](auto const& match) {
            return (instruction & match.mask) == match.opcode;
        });

    step();

    if (match == opcodeMatches.end()) {
        loge("Unknown instruction: @%x: %x", PC, instruction);
        return false;
    }

    switch (match->opcode) {
    case CLRS:
        return exec_clrs(instruction);
    case RETN:
        return exec_retn(instruction);
    case JUMP:
        return exec_jump(instruction);
    case CALL:
        return exec_call(instruction);
    case SKEQ:
        return exec_skeq(instruction);
    case SKNE:
        return exec_skne(instruction);
    case SREQ:
        return exec_sreq(instruction);
    case LDIM:
        return exec_ldim(instruction);
    case ADDI:
        return exec_addi(instruction);
    case LDRG:
        return exec_ldrg(instruction);
    case ORRG:
        return exec_orrg(instruction);
    case ANDR:
        return exec_andr(instruction);
    case XORR:
        return exec_xorr(instruction);
    case ADDC:
        return exec_addc(instruction);
    case SUBR:
        return exec_subr(instruction);
    case SHRR:
        return exec_shrr(instruction);
    case SUBN:
        return exec_subn(instruction);
    case SHLR:
        return exec_shlr(instruction);
    case SKNR:
        return exec_sknr(instruction);
    case LDIX:
        return exec_ldix(instruction);
    case JMPV:
        return exec_jmpv(instruction);
    case RAND:
        return exec_rand(instruction);
    case DRAW:
        return exec_draw(instruction);
    case SKIP:
        return exec_skip(instruction);
    case SKNP:
        return exec_sknp(instruction);
    case LDDT:
        return exec_lddt(instruction);
    case LDKY:
        return exec_ldky(instruction);
    case STDT:
        return exec_stdt(instruction);
    case STST:
        return exec_stst(instruction);
    case ADIN:
        return exec_adin(instruction);
    case LDSP:
        return exec_ldsp(instruction);
    case LBCD:
        return exec_lbcd(instruction);
    case STRG:
        return exec_strg(instruction);
    case LDRM:
        return exec_ldrm(instruction);
    }

    loge("Unknown opcode: %x", match->opcode);
    return false;
}

bool Chip8::exec_clrs(uint16_t instruction)
{
    (void)instruction;

    FB.fill(0);

    logt("CLRS executed, frame buffer cleared");

    return true;
}

bool Chip8::exec_retn(uint16_t instruction)
{
    bool status = pop(PC);
    if (status)
        logt("RETN PC: %x", PC);
    else
        logt("RETN failed, PC: %x", PC);

    return status;
}

bool Chip8::exec_jump(uint16_t instruction)
{
    uint16_t addr = instruction & 0x0FFF;
    PC            = addr;
    logt("JUMP PC: %x", PC);

    return true;
}

bool Chip8::exec_call(uint16_t instruction)
{
    uint16_t addr = instruction & 0x0FFF;
    push(PC);
    PC = addr;
    logt("CALL PC: %x", PC);

    return true;
}

bool Chip8::exec_skeq(uint16_t instruction)
{
    uint8_t x   = (instruction & 0x0F00) >> 8;
    uint8_t val = (instruction & 0x00FF);

    if (V[x] == val) {
        step();
        logt("SE +PC: %x", PC);
    }
    else {
        logt("SE -PC: %x", PC);
    }

    return true;
}

bool Chip8::exec_skne(uint16_t instruction)
{
    uint8_t x   = (instruction & 0x0F00) >> 8;
    uint8_t val = (instruction & 0x00FF);

    if (V[x] != val) {
        step();
        logt("SNE +PC: %x", PC);
    }
    else {
        logt("SNE -PC: %x", PC);
    }

    return true;
}

bool Chip8::exec_sreq(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;
    uint8_t y = (instruction & 0x00F0) >> 4;

    if (V[x] == V[y]) {
        step();
        logt("SRE +PC: %x", PC);
    }
    else {
        logt("SRE -PC: %x", PC);
    }

    return true;
}

bool Chip8::exec_ldim(uint16_t instruction)
{
    uint8_t x   = (instruction & 0x0F00) >> 8;
    uint8_t val = (instruction & 0x00FF);

    V[x] = val;
    logt("LDIM V%d = %x", x, val);

    return true;
}

bool Chip8::exec_addi(uint16_t instruction)
{
    uint8_t x   = (instruction & 0x0F00) >> 8;
    uint8_t val = (instruction & 0x00FF);

    V[x] += val;
    logt("ADDI V%d = %x", x, val);

    return true;
}

bool Chip8::exec_ldrg(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;
    uint8_t y = (instruction & 0x00F0) >> 4;

    V[x] = V[y];
    logt("LDRG V%d = V%d = %x", x, y, V[x]);

    return true;
}

bool Chip8::exec_orrg(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;
    uint8_t y = (instruction & 0x00F0) >> 4;

    V[x] |= V[y];
    logt("ORRG V%d | V%d = %x", x, y, V[x]);

    return true;
}

bool Chip8::exec_andr(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;
    uint8_t y = (instruction & 0x00F0) >> 4;

    V[x] &= V[y];
    logt("ANDR V%d & V%d = %x", x, y, V[x]);

    return true;
}

bool Chip8::exec_xorr(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;
    uint8_t y = (instruction & 0x00F0) >> 4;

    V[x] ^= V[y];
    logt("XORR V%d ^ V%d = %x", x, y, V[x]);

    return true;
}

bool Chip8::exec_addc(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;
    uint8_t y = (instruction & 0x00F0) >> 4;
    uint8_t f = 0x0F;

    V[f] = static_cast<uint16_t>(V[x]) + V[y] > 0xFF;

    V[x] += V[y];
    logt("ADDC V%d + V%d = %x, V[f]: %x", x, y, V[x], V[f]);

    return true;
}

bool Chip8::exec_subr(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;
    uint8_t y = (instruction & 0x00F0) >> 4;
    uint8_t f = 0x0F;

    V[f] = V[x] > V[y];

    V[x] -= V[y];
    logt("SUBR V%d - V%d = %x, V[f]", x, y, V[x], V[f]);

    return true;
}

bool Chip8::exec_shrr(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;
    uint8_t f = 0x0F;

    V[f] = V[x] & 0x01;

    V[x] >>= 1;
    logt("SHRR V%d = %x, V[f]", x, V[x], V[f]);

    return true;
}

bool Chip8::exec_subn(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;
    uint8_t y = (instruction & 0x00F0) >> 4;
    uint8_t f = 0x0F;

    V[f] = V[y] > V[x];

    V[x] = V[y] - V[x];
    logt("ADDC V%d - V%d = %x, V[f]", x, y, V[x], V[f]);

    return true;
}

bool Chip8::exec_shlr(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;
    uint8_t f = 0x0F;

    V[f] = (V[x] >> 7) & 0x01;

    V[x] <<= 1;
    logt("SHLR V%d = %x, V[f]", x, V[x], V[f]);

    return true;
}

bool Chip8::exec_sknr(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;
    uint8_t y = (instruction & 0x00F0) >> 4;

    if (V[x] != V[y]) {
        step();
        logt("SKNR +PC: %x", PC);
    }
    else {
        logt("SKNR -PC: %x", PC);
    }

    return true;
}

bool Chip8::exec_ldix(uint16_t instruction)
{
    uint16_t val = (instruction & 0x0FFF);

    I = val;

    logt("LDIX I: %x", I);

    return true;
}

bool Chip8::exec_jmpv(uint16_t instruction)
{
    uint16_t val = (instruction & 0x0FFF);

    PC = val + V[0];

    logt("JMPV PC: %x [V0:%x + %x]", PC, V[0], val);

    return true;
}

bool Chip8::exec_rand(uint16_t instruction)
{
    uint8_t x   = (instruction & 0x0F00) >> 8;
    uint8_t val = (instruction & 0x00FF);

    V[x] = (rand() % 256) & val;

    logt("RAND V%d: %x", x, V[x]);

    return true;
}

bool Chip8::exec_draw(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;
    uint8_t y = (instruction & 0x00F0) >> 4;
    uint8_t n = (instruction & 0x000F);
    uint8_t f = 0x0F;

    V[f] = 0;

    for (uint8_t row = 0; row < n; row++) {
        uint8_t spr = memory[I + row];
        for (uint8_t col = 0; col < 8; col++) {
            uint8_t pixel = (spr >> (7 - col)) & 0x01;
            if (!pixel)
                continue;

            FB[V[x] + col][V[y] + row] = pixel;
        }
    }

    logt("DRAW V%d, V%d, %x", x, y, n);
    return true;
}

bool Chip8::exec_skip(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;

    if (K[V[x]]) {
        step();
        logt("SKIP +PC: %x, Key: %d", PC, V[x]);
    }
    else {
        logt("SKIP -PC: %x, Key: %d", PC, V[x]);
    }

    return true;
}

bool Chip8::exec_sknp(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;

    if (!K[V[x]]) {
        step();
        logt("SKNP +PC: %x, Key: %d", PC, V[x]);
    }
    else {
        logt("SKNP -PC: %x, Key: %d", PC, V[x]);
    }

    return true;
}

bool Chip8::exec_lddt(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;

    V[x] = delayTimer;
    logt("LDDT V%d: %x", x, V[x]);

    return true;
}

bool Chip8::exec_ldky(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;

    waitForKey    = true;
    waitForKeyReg = x;

    logt("LDKY V%d, waiting for key...", x);

    return true;
}

bool Chip8::exec_stdt(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;

    delayTimer = V[x];
    logt("STDT DT: %x", delayTimer);

    return true;
}

bool Chip8::exec_stst(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;

    soundTimer = V[x];
    logt("STST ST: %x", soundTimer);

    return true;
}

bool Chip8::exec_adin(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;

    I += V[x];
    logt("ADIN I: %x", I);

    return true;
}

bool Chip8::exec_ldsp(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;

    if (V[x] > 0x0F) {
        loge("LDSP error: V%d (%x) out of range", x, V[x]);
        V[x] &= 0x0F;
    }

    I = V[x] * 5;
    logt("LDSP I: %x", I);

    return true;
}

bool Chip8::exec_lbcd(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;

    memory[I]     = V[x] / 100;
    memory[I + 1] = (V[x] / 10) % 10;
    memory[I + 2] = V[x] % 10;

    logt("LBCD %d @ %x", V[x], I);

    return true;
}

bool Chip8::exec_strg(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;

    for (uint8_t i = 0; i <= x; ++i)
        memory[I + i] = V[i];

    logt("STRG V0-V%d @ %x", x, I);

    return true;
}

bool Chip8::exec_ldrm(uint16_t instruction)
{
    uint8_t x = (instruction & 0x0F00) >> 8;

    for (uint8_t i = 0; i <= x; ++i)
        V[i] = memory[I + i];

    logt("LDRM V0-V%d @ %x", x, I);

    return true;
}

bool Chip8::push(uint16_t data)
{
    if (SP == S.size()) {
        loge("Stack overflow!");
        return false;
    }

    S[SP] = data;
    SP++;
    return true;
}

bool Chip8::pop(uint16_t& data)
{
    if (SP == 0) {
        loge("Stack underflow!");
        return false;
    }
    SP--;
    data = S[SP];
    return true;
}

bool Chip8::step()
{
    PC += 2;

    return true;
}
