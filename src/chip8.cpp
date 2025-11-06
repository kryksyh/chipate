// SPDX-License-Identifier: WTFPL

#include "chip8.h"

#include "log.h"
#include "opcode.h"

#include <cstdint>
#include <random>
#include <stdlib.h>

using namespace chipate;

uint8_t const ROM_DATA[]{
    0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70, 0xF0, 0x10, 0xF0, 0x80, 0xF0, 0xF0,
    0x10, 0xF0, 0x10, 0xF0, 0x90, 0x90, 0xF0, 0x10, 0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0, 0xF0, 0x80,
    0xF0, 0x90, 0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40, 0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0,
    0x10, 0xF0, 0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0, 0xF0, 0x80, 0x80, 0x80,
    0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0, 0xF0, 0x80, 0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80,
};

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
    , hiResMode(false)
{}

void Chip8::init(std::vector<uint8_t> const& program, Quirks const& quirks)
{
    this->quirks = quirks;
    srand(static_cast<unsigned int>(time(nullptr)));
    memory.fill(0);
    FB.fill(0);
    S.fill(0);
    V.fill(0);
    PC = 0x200;
    I = 0;
    SP = 0;
    delayTimer = 0;
    soundTimer = 0;

    std::copy(ROM_DATA, ROM_DATA + 0x200, memory.begin());

    // Load program into memory starting at address 0x200
    std::copy(program.begin(), program.end(), memory.begin() + 0x200);

    logi("Program loaded, size: %zu bytes", program.size());
}

void Chip8::tock()
{
    if (delayTimer)
        delayTimer--;
    if (soundTimer)
        soundTimer--;
    waitForVBlank = false;
}

void Chip8::tick()
{
    if (waitForKey) {
        logt("Waiting for key press...");
        return;
    }

    if (waitForVBlank) {
        logt("Waiting for VBlank...");
        return;
    }

    uint16_t currentInstruction = static_cast<uint16_t>(memory[PC + 1]) | (memory[PC] << 8);

    logd("Fetch @%x: %x", PC, currentInstruction);

    if (!exec(currentInstruction))
        loge("Execution failed at PC: %x", PC);
}

void Chip8::setKey(int key, bool pressed)
{
    uint8_t k = static_cast<uint8_t>(key);
    K[k] = pressed;
    if (waitForKey && pressed) {
        V[waitForKeyReg] = k;
        waitForKey = false;
        logt("Key received: %d -> V%d", key, waitForKeyReg);
    }
}

bool Chip8::exec(uint16_t data)
{
    auto match =
        std::find_if(opcodeMatches.begin(), opcodeMatches.end(),
                     [data](auto const& match) { return (data & match.mask) == match.opcode; });

    step();

    if (match == opcodeMatches.end()) {
        loge("Unknown instruction: @%x: %x", PC, data);
        return false;
    }

    Instruction instruction(data, V);

    switch (match->opcode) {
    case CLS:
        return exec_clrs(instruction);
    case RET:
        return exec_retn(instruction);
    case JP:
        return exec_jump(instruction);
    case CALL:
        return exec_call(instruction);
    case SE:
        return exec_skeq(instruction);
    case SNE:
        return exec_skne(instruction);
    case SER:
        return exec_sreq(instruction);
    case LD:
        return exec_ldim(instruction);
    case ADD:
        return exec_addi(instruction);
    case LDR:
        return exec_ldrg(instruction);
    case OR:
        return exec_orrg(instruction);
    case AND:
        return exec_andr(instruction);
    case XOR:
        return exec_xorr(instruction);
    case ADDC:
        return exec_addc(instruction);
    case SUB:
        return exec_subr(instruction);
    case SHR:
        return exec_shrr(instruction);
    case SUBN:
        return exec_subn(instruction);
    case SHL:
        return exec_shlr(instruction);
    case SNER:
        return exec_sknr(instruction);
    case LDI:
        return exec_ldix(instruction);
    case JPO:
        return exec_jmpv(instruction);
    case RND:
        return exec_rand(instruction);
    case DRW:
        return exec_draw(instruction);
    case SKP:
        return exec_skip(instruction);
    case SKNP:
        return exec_sknp(instruction);
    case LDRD:
        return exec_lddt(instruction);
    case LDK:
        return exec_ldky(instruction);
    case LDDR:
        return exec_stdt(instruction);
    case LDSR:
        return exec_stst(instruction);
    case ADDI:
        return exec_adin(instruction);
    case LDS:
        return exec_ldsp(instruction);
    case LBCD:
        return exec_lbcd(instruction);
    case LDMR:
        return exec_strg(instruction);
    case LDRM:
        return exec_ldrm(instruction);
    case HIRS:
        return exec_hirs(instruction);
    case LORS:
        return exec_lors(instruction);
    case SCRD:
        return exec_scrd(instruction);
    case SCRL:
        return exec_scrl(instruction);
    case SCRR:
        return exec_scrr(instruction);
    }

    loge("Unknown opcode: %x", match->opcode);
    return false;
}

// 00E0     Clear display (CLS)
bool Chip8::exec_clrs(Instruction i)
{
    (void)i;

    FB.fill(0);

    logt("CLS executed, frame buffer cleared");

    return true;
}

// 00EE     Return from subroutine (RET)
bool Chip8::exec_retn(Instruction i)
{
    (void)i;

    bool status = pop(PC);
    if (status)
        logt("RET PC: %x", PC);
    else
        logt("RET failed, PC: %x", PC);

    return status;
}

// 1nnn     Jump to address nnn (JP nnn)
bool Chip8::exec_jump(Instruction i)
{
    PC = i.nnn();
    logt("JP PC: %x", PC);

    return true;
}

// 2nnn     Call subroutine at nnn (CALL nnn)
bool Chip8::exec_call(Instruction i)
{
    push(PC);
    PC = i.nnn();
    logt("CALL PC: %x", PC);

    return true;
}

// 3xkk     Skip next instruction if Vx == kk (SE Vx, kk)
bool Chip8::exec_skeq(Instruction i)
{
    if (i.vx() == i.kk()) {
        step();
        logt("SE +PC: %x", PC);
    }
    else {
        logt("SE -PC: %x", PC);
    }

    return true;
}

// 4xkk     Skip next instruction if Vx != kk (SNE Vx, kk)
bool Chip8::exec_skne(Instruction i)
{
    if (i.vx() != i.kk()) {
        step();
        logt("SNE +PC: %x", PC);
    }
    else {
        logt("SNE -PC: %x", PC);
    }

    return true;
}

// 5xy0     Skip next instruction if Vx == Vy (SE Vx, Vy)
bool Chip8::exec_sreq(Instruction i)
{
    if (i.vx() == i.vy()) {
        step();
        logt("SRE +PC: %x", PC);
    }
    else {
        logt("SRE -PC: %x", PC);
    }

    return true;
}

// 6xkk     Load immediate kk into Vx (LD Vx, kk)
bool Chip8::exec_ldim(Instruction i)
{
    i.vx() = i.kk();
    logt("LD V%d = %x", i.x(), i.kk());

    return true;
}

// 7xkk     Add immediate kk to Vx (ADD Vx, kk)
bool Chip8::exec_addi(Instruction i)
{
    i.vx() += i.kk();
    logt("ADD V%d = %x", i.x(), i.vx());

    return true;
}

// 8xy0     Load Vy into Vx (LD Vx, Vy)
bool Chip8::exec_ldrg(Instruction i)
{
    i.vx() = i.vy();
    logt("LDR V%d = V%d = %x", i.x(), i.y(), i.vx());

    return true;
}

// 8xy1     Bitwise OR Vx with Vy (OR Vx, Vy)
bool Chip8::exec_orrg(Instruction i)
{
    i.vx() |= i.vy();

    Vf = 0;

    logt("OR V%d | V%d = %x", i.x(), i.y(), i.vx());

    return true;
}

// 8xy2     Bitwise AND Vx with Vy (AND Vx, Vy)
bool Chip8::exec_andr(Instruction i)
{
    i.vx() &= i.vy();

    Vf = 0;

    logt("AND V%d & V%d = %x", i.x(), i.y(), i.vx());

    return true;
}

// 8xy3     Bitwise XOR Vx with Vy (XOR Vx, Vy)
bool Chip8::exec_xorr(Instruction i)
{
    i.vx() ^= i.vy();

    Vf = 0;

    logt("XOR V%d ^ V%d = %x", i.x(), i.y(), i.vx());

    return true;
}

// 8xy4     Add Vy to Vx, set VF on carry (ADDC Vx, Vy)
bool Chip8::exec_addc(Instruction i)
{
    uint8_t carry = static_cast<uint16_t>(i.vx()) + i.vy() > 0xFF;

    i.vx() += i.vy();

    Vf = carry;
    logt("ADDC V%d + V%d = %x, V[f]: %x", i.x(), i.y(), i.vx(), Vf);

    return true;
}

// 8xy5     Subtract Vy from Vx, set VF on borrow (SUB Vx, Vy)
bool Chip8::exec_subr(Instruction i)
{
    uint8_t carry = i.vx() >= i.vy();

    i.vx() -= i.vy();
    Vf = carry;
    logt("SUB V%d - V%d = %x, V[f]: %x", i.x(), i.y(), i.vx(), Vf);

    return true;
}

// 8xy6     Shift Vx right by 1, set VF to least significant bit prior to shift (SHR Vx)
bool Chip8::exec_shrr(Instruction i)
{
    uint8_t v = i.vx();
    if (!quirks.shiftVxOnly)
        v = i.vy();

    uint8_t carry = v & 0x01;

    i.vx() = v >> 1;

    Vf = carry;
    logt("SHR V%d = %x, V[f]: %x", i.x(), i.vx(), Vf);

    return true;
}

// 8xy7     Set Vx = Vy - Vx, set VF on borrow (SUBN Vx, Vy)
bool Chip8::exec_subn(Instruction i)
{
    uint8_t carry = i.vy() >= i.vx();

    i.vx() = i.vy() - i.vx();
    Vf = carry;
    logt("SUBN V%d - V%d = %x, V[f]: %x", i.x(), i.y(), i.vx(), Vf);

    return true;
}

// 8xyE     Shift Vx left by 1, set VF to most significant bit prior to shift (SHL Vx)
bool Chip8::exec_shlr(Instruction i)
{
    uint8_t v = i.vx();
    if (!quirks.shiftVxOnly)
        v = i.vy();

    uint8_t carry = (v >> 7) & 0x01;

    i.vx() = v << 1;
    Vf = carry;
    logt("SHL V%d = %x, V[f]: %x", i.x(), i.vx(), Vf);

    return true;
}

// 9xy0     Skip next instruction if Vx != Vy (SNE Vx, Vy)
bool Chip8::exec_sknr(Instruction i)
{
    if (i.vx() != i.vy()) {
        step();
        logt("SNER +PC: %x", PC);
    }
    else {
        logt("SNER -PC: %x", PC);
    }

    return true;
}

// Annn     Load nnn into index register I (LD I, nnn)
bool Chip8::exec_ldix(Instruction i)
{
    I = i.nnn();

    logt("LDI I: %x", I);

    return true;
}

// Bnnn     Jump to address nnn + V0 (JP V0, nnn)
bool Chip8::exec_jmpv(Instruction i)
{
    PC = i.nnn() + V[0];

    logt("JPO PC: %x [V0:%x + %x]", PC, V[0], i.nnn());

    return true;
}

// Cxkk     Set Vx = random byte AND kk (RND Vx, kk)
bool Chip8::exec_rand(Instruction i)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 255);

    i.vx() = dis(gen) & i.kk();

    logt("RND V%d: %x", i.x(), i.vx());

    return true;
}

// Dxyn     Draw sprite at (Vx, Vy) with height n (DRW Vx, Vy, n)
bool Chip8::exec_draw(Instruction i)
{
    Vf = 0;

    size_t screenWidth = hiResMode ? 128 : 64;
    size_t screenHeight = hiResMode ? 64 : 32;

    uint8_t x0 = i.vx() % screenWidth;
    uint8_t y0 = i.vy() % screenHeight;

    for (int col = 0; col < 8; col++) {
        for (int row = 0; row < i.n(); row++) {
            auto x = (col + x0);
            auto y = (row + y0);

            if (x >= screenWidth || y >= screenHeight)
                continue;

            auto curPixel = FB[x][y];
            auto newPixel = memory[I + row] >> (7 - col) & 0x01;

            V[0x0F] |= (curPixel ? newPixel : 0);

            curPixel = curPixel ^ newPixel;
        }
    }

    logt("DRW V%d[%d], V%d[%d], %x", i.x(), i.vx(), i.y(), i.vy(), i.n());
    waitForVBlank = true;
    return true;
}

// Ex9E     Skip next instruction if key with the value of Vx is pressed (SKP Vx)
bool Chip8::exec_skip(Instruction i)
{
    if (K[i.vx()]) {
        step();
        logt("SKP +PC: %x, Key: %d", PC, i.vx());
    }
    else {
        logt("SKP -PC: %x, Key: %d", PC, i.vx());
    }

    return true;
}

// ExA1     Skip next instruction if key with the value of Vx is not pressed (SKNP Vx)
bool Chip8::exec_sknp(Instruction i)
{
    if (!K[i.vx()]) {
        step();
        logt("SKNP +PC: %x, Key: %d", PC, i.vx());
    }
    else {
        logt("SKNP -PC: %x, Key: %d", PC, i.vx());
    }

    return true;
}

// Fx07     Load delay timer value into Vx (LDRD Vx)
bool Chip8::exec_lddt(Instruction i)
{
    i.vx() = delayTimer;
    logt("LDRD V%d: %x", i.x(), i.vx());

    return true;
}

// Fx0A     Wait for a key press, store the value of the key in Vx (LDK Vx)
bool Chip8::exec_ldky(Instruction i)
{
    waitForKey = true;
    waitForKeyReg = i.x();

    logt("LDK V%d, waiting for key...", i.x());
    return true;
}

// Fx15     Set delay timer = Vx (LDDR Vx)
bool Chip8::exec_stdt(Instruction i)
{
    delayTimer = i.vx();
    logt("LDDR DT: %x", delayTimer);

    return true;
}

// Fx18     Set sound timer = Vx (LDSR Vx)
bool Chip8::exec_stst(Instruction i)
{
    soundTimer = i.vx();
    logt("LDSR ST: %x", soundTimer);

    return true;
}

// Fx1E     Add Vx to index register I (ADDI Vx)
bool Chip8::exec_adin(Instruction i)
{
    I += i.vx();
    logt("ADDI I: %x", I);

    return true;
}

// Fx29     Load sprite location for digit Vx into index register I (LDS Vx)
bool Chip8::exec_ldsp(Instruction i)
{
    if (i.vx() > 0x0F) {
        loge("LDS error: V%d (%x) out of range", i.x(), i.vx());
        i.vx() &= 0x0F;
    }

    I = i.vx() * 5;
    logt("LDS I: %x", I);

    return true;
}

// Fx33     Store BCD representation of Vx in memory locations I, I+1, and I+2 (LBCD Vx)
bool Chip8::exec_lbcd(Instruction i)
{
    memory[I] = i.vx() / 100;
    memory[I + 1] = (i.vx() / 10) % 10;
    memory[I + 2] = i.vx() % 10;

    logt("LBCD %d @ %x", i.vx(), I);

    return true;
}

// Fx55     Store registers V0 through Vx in memory starting at location I (LDMR Vx)
bool Chip8::exec_strg(Instruction i)
{
    uint8_t x = i.x();

    for (uint8_t j = 0; j <= x; ++j)
        memory[I + j] = V[j];

    I += x + 1;

    logt("LDMR V0-V%d @ %x", x, I);

    return true;
}

// Fx65     Read registers V0 through Vx from memory starting at location I (LDRM Vx)
bool Chip8::exec_ldrm(Instruction i)
{
    for (uint8_t j = 0; j <= i.x(); ++j)
        V[j] = memory[I + j];

    I += i.x() + 1;

    logt("LDRM V0-V%d @ %x", i.x(), I);

    return true;
}

// 00FF     Enable high-resolution mode (HIRS)
bool Chip8::exec_hirs(Instruction i)
{
    (void)i;

    hiResMode = true;

    logt("HIRS, Enabled high-resolution mode");

    return true;
}

// 00FE     Enable low-resolution mode (LORS)
bool Chip8::exec_lors(Instruction i)
{
    (void)i;

    hiResMode = false;

    logt("LORS, Enabled low-resolution mode");

    return true;
}

// 00FD     Scroll down n pixels (SCRD n)
bool Chip8::exec_scrd(Instruction i)
{
    size_t maxCols = hiRes() ? 128 : 64;
    size_t maxRows = hiRes() ? 64 : 32;
    uint8_t n = i.n();

    if (!hiRes() && quirks.legacySchipScroll)
        n /= 2;

    for (size_t r = maxRows - 1; r >= n; --r)
        for (size_t c = 0; c < maxCols; ++c)
            FB[c][r] = FB[c][r - n];
    for (size_t r = 0; r < n; ++r)
        for (size_t c = 0; c < maxCols; ++c)
            FB[c][r] = 0;

    logt("SCRD %d", n);

    return true;
}

// 00FC     Scroll left 4 pixels (SCRL)
bool Chip8::exec_scrl(Instruction i)
{
    (void)i;
    uint8_t n = 4;

    if (!hiRes() && quirks.legacySchipScroll)
        n /= 2;

    size_t maxCols = hiRes() ? 128 : 64;
    size_t maxRows = hiRes() ? 64 : 32;

    for (size_t c = 0; c < maxCols - n; c++)
        for (size_t r = 0; r < maxRows; r++)
            FB[c][r] = FB[c + n][r];
    for (size_t c = maxCols - n; c < maxCols; c++)
        for (size_t r = 0; r < maxRows; r++)
            FB[c][r] = 0;

    logt("SCRL %d", n);

    return true;
}

// 00FB     Scroll right 4 pixels (SCRR)
bool Chip8::exec_scrr(Instruction i)
{
    (void)i;
    uint8_t n = 4;

    if (!hiRes() && quirks.legacySchipScroll)
        n /= 2;

    size_t maxCols = hiRes() ? 128 : 64;
    size_t maxRows = hiRes() ? 64 : 32;

    for (size_t c = maxCols - n; c > 0; c--)
        for (size_t r = 0; r < maxRows; r++)
            FB[c + n - 1][r] = FB[c - 1][r];
    for (size_t c = 0; c < n; c++)
        for (size_t r = 0; r < maxRows; r++)
            FB[c][r] = 0;

    logt("SCRL %d", n);

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
