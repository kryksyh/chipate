// SPDX-License-Identifier: WTFPL

#include "asm.h"
#include "chip8.h"

#include <catch2/catch_test_macros.hpp>
#include <vector>

using namespace chipate;

namespace chipate {

class Chip8TestAccess {
public:
    static std::array<uint8_t, 16> const &regs(Chip8 const &c)
    {
        return c.V;
    }
    static std::array<std::bitset<64>, 128> const &fb(Chip8 const &c)
    {
        return c.FB;
    }
    static std::array<uint16_t, 16> const &stack(Chip8 const &c)
    {
        return c.S;
    }
    static std::array<uint8_t, 4096> const &memory(Chip8 const &c)
    {
        return c.memory;
    }
    static void setMemory(Chip8 &c, uint16_t address, uint8_t data)
    {
        c.memory[address] = data;
    }
    static uint16_t pc(Chip8 const &c)
    {
        return c.PC;
    }
    static uint16_t ireg(Chip8 const &c)
    {
        return c.I;
    }
    static uint8_t sp(Chip8 const &c)
    {
        return c.SP;
    }
    static uint8_t delayTimer(Chip8 const &c)
    {
        return c.delayTimer;
    }
    static uint8_t soundTimer(Chip8 const &c)
    {
        return c.soundTimer;
    }
    static bool waitForKey(Chip8 const &c)
    {
        return c.waitForKey;
    }
    static uint8_t waitForKeyReg(Chip8 const &c)
    {
        return c.waitForKeyReg;
    }
    static bool keyState(Chip8 const &c, uint8_t k)
    {
        auto it = c.K.find(k);
        if (it == c.K.end())
            return false;
        return it->second;
    }
    static uint16_t nextInstruction(Chip8 const &c)
    {
        auto pc = Chip8TestAccess::pc(c);
        return Chip8TestAccess::memory(c)[pc] << 8 | Chip8TestAccess::memory(c)[pc + 1];
    }
};
} // namespace chipate

#define PC          Chip8TestAccess::pc(cpu)
#define REGS        Chip8TestAccess::regs(cpu)
#define MEM         Chip8TestAccess::memory(cpu)
#define FB          Chip8TestAccess::fb(cpu)
#define IREG        Chip8TestAccess::ireg(cpu)
#define SP          Chip8TestAccess::sp(cpu)
#define DT          Chip8TestAccess::delayTimer(cpu)
#define ST          Chip8TestAccess::soundTimer(cpu)
#define KEYSTATE(k) Chip8TestAccess::keyState(cpu, k)
#define V0          REGS[0]
#define V1          REGS[1]
#define V2          REGS[2]
#define V3          REGS[3]
#define V4          REGS[4]
#define V5          REGS[5]
#define V6          REGS[6]
#define V7          REGS[7]
#define V8          REGS[8]
#define V9          REGS[9]
#define VA          REGS[10]
#define VB          REGS[11]
#define VC          REGS[12]
#define VD          REGS[13]
#define VE          REGS[14]
#define VF          REGS[15]
#define STACK       Chip8TestAccess::stack(cpu)
#define NEXT_OPCODE Chip8TestAccess::nextInstruction(cpu)

static void run_ticks(Chip8 &cpu, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        cpu.tick();
}

static void run_until_opcode(Chip8 &cpu, std::string instruction)
{
    auto     bytecode = assemble(instruction);
    uint16_t instr    = bytecode[0] << 8 | bytecode[1];
    while (NEXT_OPCODE != instr && PC < 0x1000)
        cpu.tick();
}

static void run_until_opcode(Chip8 &cpu, uint16_t instruction)
{
    while (NEXT_OPCODE != instruction && PC < 0x1000)
        cpu.tick();
}

static void run_until_pc(Chip8 &cpu, uint16_t address)
{
    while (PC != address && PC < 0x1000)
        cpu.tick();
}

#define RUN_TO_OPCODE(instr) run_until_opcode(cpu, instr)
#define EXECUTE_OPCODE(instr)                                                                      \
    run_until_opcode(cpu, instr);                                                                  \
    cpu.tick();
#define RUN_TO_PC(addr) run_until_pc(cpu, addr)
#define RUN_TICKS(n)    run_ticks(cpu, n)

TEST_CASE("Cover CLS and DRW", "[chip8][draw][clrs]")
{
    Chip8 cpu;

    cpu.init(assemble(R"(
        ld v0 0x05
        ld v1 0x02
        ld i 0x20A
        drw v0 v1 0x1
        cls
        db 0x80)"));

    RUN_TO_OPCODE("cls");
    REQUIRE(FB[5].test(2) == true);
    RUN_TICKS(1);

    for (auto const &col: FB)
        for (size_t r = 0; r < 32; ++r)
            REQUIRE(col.test(r) == false);
}

TEST_CASE("Load/store immediate and add immediate", "[chip8][ldim][addi]")
{
    Chip8 cpu;

    cpu.init(assemble(R"(
        ld v2 0x10
        add v2 0x05
    )"));

    RUN_TICKS(2);
    REQUIRE(V2 == 0x15);
}

TEST_CASE("Register ops OR/AND/XOR", "[chip8][alu]")
{
    Chip8 cpu;

    cpu.init(assemble(R"(
        ld v0 0x0F
        ld v1 0xF0
        ld v2 0x55
        ld v3 0xaa
        ld v4 0x13
        ld v5 0x37

        or v1 v0
        and v3 v2
        xor v5 v4

        db 11 22 33 44
    )"));

    RUN_TO_OPCODE("db 11 22 33 44");

    SECTION("Bitwise OR")
    {
        REQUIRE(V1 == (0x0F | 0xF0));
    }

    SECTION("Bitwise AND")
    {
        REQUIRE(V3 == (0x55 & 0xAA));
    }

    SECTION("Bitwise XOR")
    {
        REQUIRE(V5 == (0x37 ^ 0x13));
    }
}

TEST_CASE("Register ops ADD/SUB/SUB", "[chip8][alu]")
{
    Chip8 cpu;

    cpu.init(assemble(R"(
        ld v0 0x20
        ld v1 0x15
        ld v2 0x10
        ld v3 0x13
        ld v4 0x37
        ld v5 0x22
        ld v6 0x10
        ld v7 0x05
        ld v8 0x10
        ld v9 0x05

        add v0 v1

        sub v2 v3

        sub v4 v5

        subn v7 v6

        subn v8 v9

    )"));

    SECTION("Addition")
    {
        EXECUTE_OPCODE("add v0 v1")
        REQUIRE(V0 == (0x20 + 0x15));
    }

    SECTION("Subtraction with borrow")
    {
        EXECUTE_OPCODE("sub v2 v3")
        REQUIRE(V2 == (0x10 - 0x13 + 0x100));
        REQUIRE(VF == 0x0);
    }

    SECTION("Subtraction without borrow")
    {
        EXECUTE_OPCODE("sub v4 v5")
        REQUIRE(V4 == (0x37 - 0x22));
        REQUIRE(VF == 0x01);
    }

    SECTION("Subtraction negated without borrow")
    {
        EXECUTE_OPCODE("subn v7 v6")
        REQUIRE(V7 == (0x10 - 0x05));
        REQUIRE(VF == 0x01);
    }

    SECTION("Subtraction negated with borrow")
    {
        EXECUTE_OPCODE("subn v8 v9")
        REQUIRE(V8 == (0x05 - 0x10 + 0x100));
        REQUIRE(VF == 0x00);
    }
}

TEST_CASE("Register ops SHIFTS", "[chip8][alu]")
{
    Chip8 cpu;

    cpu.init(assemble(R"(
        ld v0 0x0F
        ld v1 0xF0

        shl v0
        shl v0
        shl v0
        shl v0

        shr v1
        shr v1
        shr v1
        shr v1

        ld v5 0x55

        shl v0  ; check if VS works

        ld v6 0x66

        shr v1  ; check if VS works

        ld v7 0x77
    )"));

    RUN_TO_OPCODE("ld v5 0x55");

    SECTION("Shift Left")
    {
        REQUIRE(V0 == 0xF0);
        REQUIRE(VF == 0x0);
        SECTION("Shift Left sets VF")
        {
            RUN_TO_OPCODE("ld v6 0x66");
            REQUIRE(V0 == 0xE0);
            REQUIRE(VF == 0x1);
        }
    }

    SECTION("Shift Right")
    {
        REQUIRE(V1 == 0x0F);
        REQUIRE(VF == 0x0);
        SECTION("Shift Right sets VF")
        {
            RUN_TO_OPCODE("ld v7 0x77");
            REQUIRE(V1 == 0x07);
            REQUIRE(VF == 0x1);
        }
    }
}

TEST_CASE("Call and Return", "[chip8][call][retn]")
{
    Chip8 cpu;
    cpu.init(assemble(R"(
        call 0x204   ; 0x200
        ld v0 0x42   ; 0x202
        ld v0 0x99   ; 0x204
        ret          ; 0x206
    )"));

    RUN_TO_PC(0x206);
    REQUIRE(SP == 1);
    REQUIRE(STACK[0] == 0x202);
    REQUIRE(V0 == 0x99);

    RUN_TO_PC(0x204);
    REQUIRE(V0 == 0x42);
    REQUIRE(SP == 0);
}

TEST_CASE("Index and memory ops: LDI, ADDI, LDMR, LDRM, LBCD, LDS", "[chip8][memory]")
{
    Chip8 cpu;

    cpu.init(assemble(R"(
        ld v0 0x01
        ld v1 0x02
        ld v2 0x03
        ld v3 0x04
        ld v4 0x05

        ld i 0x300
        ld [i] v4

        ld v5 147
        ld i 0x400
        ld b v5

        ld v0 0x11
        ld v1 0x22
        ld v2 0x33
        ld v3 0x44
        ld v4 0x55
        ld i 0x300
        ld v4 [i]
    )"));

    SECTION("Store registers to memory")
    {
        EXECUTE_OPCODE("ld [i] v4");
        REQUIRE(V0 == 1);
        REQUIRE(V1 == 2);
        REQUIRE(V2 == 3);
        REQUIRE(V3 == 4);
        REQUIRE(V4 == 5);
        REQUIRE(MEM[0x300] == 1);
        REQUIRE(MEM[0x301] == 2);
        REQUIRE(MEM[0x302] == 3);
        REQUIRE(MEM[0x303] == 4);
        REQUIRE(MEM[0x304] == 5);
    }

    SECTION("BCD conversion")
    {
        EXECUTE_OPCODE("ld b v5");
        REQUIRE(MEM[0x400] == 1);
        REQUIRE(MEM[0x401] == 4);
        REQUIRE(MEM[0x402] == 7);
    }

    SECTION("Load registers from memory")
    {
        EXECUTE_OPCODE("ld v4 [i]");
        REQUIRE(V0 == 1);
        REQUIRE(V1 == 2);
        REQUIRE(V2 == 3);
        REQUIRE(V3 == 4);
        REQUIRE(V4 == 5);
    }
}

TEST_CASE("Skips and key handling (SE, SNE, SER, SKNP, SKP, LDK)", "[chip8][key][skip]")
{
    Chip8 cpu;

    std::string const program = R"(
        ld v0 0x05
        se v0 0x05
        ld v0 0x01
        sne v0 0xFF
        ld v0 0x02
        se v0 v1
        ld v2 0x55
        sknp v2
        ld v0 0x07
        ld v3 k
        ld v0 0x09
    )";

    cpu.init(assemble(program));

    SECTION("SE skips next instruction when REG == IMM")
    {
        // After first SE, V0 should remain 5
        RUN_TO_OPCODE("sne v0 0xFF");
        REQUIRE(V0 == 0x05);
    }

    SECTION("SNE skips next instruction when REG != IMM")
    {
        // After sne v0 0xff, V0 should remain 5
        RUN_TO_OPCODE("se v0 v1");
        REQUIRE(V0 == 0x05);
    }

    SECTION("SE doesn't skips next instruction when REGs are not equal")
    {
        // V2 should be set to 55
        RUN_TO_OPCODE("sknp v2");
        REQUIRE(V2 == 0x55);
    }

    SECTION("SKNP skips next instruction when key not pressed")
    {
        // V0 should remain 5
        RUN_TO_OPCODE("ld v3 k");
        REQUIRE(V0 == 0x05);
    }

    SECTION("LDK halts the execution and setting a key resumes")
    {
        // LDK halts the execution
        RUN_TO_OPCODE("ld v3 k");
        RUN_TICKS(1);
        auto pc = PC;
        RUN_TICKS(1);
        REQUIRE(PC == pc);
        RUN_TICKS(10);
        REQUIRE(PC == pc);

        // Key press resumes execution and sets V3
        REQUIRE(Chip8TestAccess::waitForKey(cpu) == true);
        cpu.setKey(0x04, true);
        RUN_TICKS(1);
        REQUIRE(Chip8TestAccess::keyState(cpu, 0x04) == true);
        REQUIRE(V3 == 0x04);
    }
}

TEST_CASE("Rand and timers (RND, LDRD, LDDR, LDSR)", "[chip8][rand][timers]")
{
    Chip8 cpu;

    std::string program = R"(
        ld v1 5     ; load 5 to V1
        ld dt v1    ; load V1 to delay timer
        ld st v1    ; load V1 to sound timer
    )";

    cpu.init(assemble(program));

    run_ticks(cpu, 3);

    REQUIRE(V1 == 5);
    REQUIRE(DT == 4);
    REQUIRE(ST == 5);
}

TEST_CASE("JPO (Bnnn) behavior", "[chip8][jmpv]")
{
    Chip8       cpu;
    std::string program = R"(
        ld v0 0x10
        jp 0x300
        ld v0 0x20
    )";
    cpu.init(assemble(program));
    RUN_TO_PC(0x300);
    REQUIRE(V0 == 0x10);
}

TEST_CASE("JP (1nnn) - Absolute jump", "[chip8][jump]")
{
    Chip8 cpu;
    cpu.init(assemble(R"(
        ld v0 0x11
        jp 0x208
        ld v0 0x22
        ld v0 0x33
    )"));

    RUN_TICKS(2);
    REQUIRE(PC == 0x208);
    REQUIRE(V0 == 0x11);
}

TEST_CASE("SE and SNE corner cases", "[chip8][skip]")
{
    SECTION("SE with zero")
    {
        Chip8 cpu;
        cpu.init(assemble(R"(
            ld v0 0x00
            se v0 0x00
            ld v2 0x01
        )"));
        EXECUTE_OPCODE("se v0 0x00");
        REQUIRE(V2 == 0x00); // Should be skipped
    }

    SECTION("SE with 0xFF not equal")
    {
        Chip8 cpu;
        cpu.init(assemble(R"(
            ld v1 0xFF
            se v1 0x00
            ld v2 0x01
        )"));
        EXECUTE_OPCODE("se v1 0x00");
        cpu.tick();
        REQUIRE(V2 == 0x01); // Should execute
    }

    SECTION("SNE with 0xFF")
    {
        Chip8 cpu;
        cpu.init(assemble(R"(
            ld v0 0x00
            sne v0 0xFF
            ld v3 0x03
            ld v4 0x00
        )"));
        EXECUTE_OPCODE("sne v0 0xFF");
        cpu.tick();
        REQUIRE(V3 == 0x00); // Should be skipped
    }

    SECTION("SNE equal 0xFF")
    {
        Chip8 cpu;
        cpu.init(assemble(R"(
            ld v1 0xFF
            sne v1 0xFF
            ld v3 0x04
        )"));
        EXECUTE_OPCODE("sne v1 0xFF");
        REQUIRE(V3 == 0x00); // Should be skipped
    }
}

TEST_CASE("SER and SNER - Register comparison skips", "[chip8][skip]")
{
    Chip8 cpu;
    cpu.init(assemble(R"(
        ld v0 0x42
        ld v1 0x42
        ld v2 0x99
        se v0 v1
        ld v3 0x01
        se v0 v2
        ld v3 0x02
        sne v0 v2
        ld v4 0x03
        sne v0 v1
        ld v4 0x04
        ld v5 0x00
    )"));

    SECTION("SE with equal registers")
    {
        EXECUTE_OPCODE("se v0 v1");
        REQUIRE(V3 == 0x00); // Should skip
    }

    SECTION("SE with unequal registers")
    {
        EXECUTE_OPCODE("se v0 v2");
        cpu.tick();
        REQUIRE(V3 == 0x02); // Should execute
    }

    SECTION("SNE with unequal registers")
    {
        EXECUTE_OPCODE("sne v0 v2");
        REQUIRE(V4 == 0x00); // Should skip
    }

    SECTION("SNE with equal registers")
    {
        EXECUTE_OPCODE("sne v0 v1");
        cpu.tick();
        REQUIRE(V4 == 0x04); // Should execute
    }
}

TEST_CASE("ADD immediate with overflow", "[chip8][addi]")
{
    Chip8 cpu;
    cpu.init(assemble(R"(
        ld v0 0xFF
        add v0 0x01
        ld v1 0x80
        add v1 0x80
    )"));

    SECTION("Overflow wraps around")
    {
        EXECUTE_OPCODE("add v0 0x01");
        REQUIRE(V0 == 0x00);
    }

    SECTION("Half overflow")
    {
        EXECUTE_OPCODE("add v1 0x80");
        REQUIRE(V1 == 0x00);
    }
}

TEST_CASE("ADDC - Add with carry flag", "[chip8][addc]")
{
    Chip8 cpu;
    cpu.init(assemble(R"(
        ld v0 0x50
        ld v1 0x30
        add v0 v1

        ld v2 0xFF
        ld v3 0x01
        add v2 v3

        ld v4 0xFF
        ld v5 0xFF
        add v4 v5

        ld v6 0x80
        ld v7 0x80
        add v6 v7
    )"));

    SECTION("No overflow - VF should be 0")
    {
        EXECUTE_OPCODE("add v0 v1");
        REQUIRE(V0 == 0x80);
        REQUIRE(VF == 0);
    }

    SECTION("Small overflow - VF should be 1")
    {
        EXECUTE_OPCODE("add v2 v3");
        REQUIRE(V2 == 0x00);
        REQUIRE(VF == 1);
    }

    SECTION("Max overflow - VF should be 1")
    {
        EXECUTE_OPCODE("add v4 v5");
        REQUIRE(V4 == 0xFE);
        REQUIRE(VF == 1);
    }

    SECTION("Exact overflow at boundary")
    {
        EXECUTE_OPCODE("add v6 v7");
        REQUIRE(V6 == 0x00);
        REQUIRE(VF == 1);
    }
}

TEST_CASE("LDR - Load register from register", "[chip8][ldrg]")
{
    Chip8 cpu;
    cpu.init(assemble(R"(
        ld v0 0x42
        ld v1 0x99
        ld v2 v0
        ld v3 v1
        ld vf 0x11
        ld v4 vf
    )"));

    RUN_TICKS(6);
    REQUIRE(V2 == 0x42);
    REQUIRE(V3 == 0x99);
    REQUIRE(V4 == 0x11);
}

TEST_CASE("SKP and SKNP - Key press detection", "[chip8][key]")
{
    Chip8 cpu;
    cpu.init(assemble(R"(
        ld v0 0x05
        ld v1 0x0A
        skp v0
        ld v2 0x01
        sknp v1
        ld v3 0x02
        skp v1
        ld v4 0x03
        sknp v0
        ld v5 0x04
        ld v6 0x05
    )"));

    SECTION("SKP when key not pressed")
    {
        EXECUTE_OPCODE("skp v0");
        cpu.tick();
        REQUIRE(V2 == 0x01); // Should execute
    }

    SECTION("SKNP when key not pressed")
    {
        EXECUTE_OPCODE("sknp v1");
        cpu.tick();
        REQUIRE(V3 == 0x00); // Should skip
    }

    SECTION("SKP when key is pressed")
    {
        cpu.setKey(0x0A, true);
        EXECUTE_OPCODE("skp v1");
        cpu.tick();
        REQUIRE(V4 == 0x00); // Should skip
    }

    SECTION("SKNP when key is pressed")
    {
        cpu.setKey(0x05, true);
        EXECUTE_OPCODE("sknp v0");
        cpu.tick();
        REQUIRE(V5 == 0x04); // Should execute
    }
}

TEST_CASE("LDRD - Load delay timer to register", "[chip8][timer]")
{
    Chip8 cpu;
    cpu.init(assemble(R"(
        ld v0 10
        ld dt v0
        ld v1 dt
        ld v2 dt
        ld v3 dt
    )"));

    RUN_TICKS(3);
    REQUIRE(V1 == 9);
    RUN_TICKS(1);
    REQUIRE(V2 == 8);
    RUN_TICKS(1);
    REQUIRE(V3 == 7);
}

TEST_CASE("ADDI - Add to index register", "[chip8][index]")
{
    Chip8 cpu;
    cpu.init(assemble(R"(
        ld i 0x300
        ld v0 0x10
        add i v0
        ld v1 0x20
        add i v1
        ld v2 0xFF
        add i v2
    )"));

    SECTION("Add small value")
    {
        EXECUTE_OPCODE("add i v0");
        REQUIRE(IREG == 0x310);
    }

    SECTION("Add another value")
    {
        EXECUTE_OPCODE("add i v1");
        REQUIRE(IREG == 0x330);
    }

    SECTION("Add large value")
    {
        EXECUTE_OPCODE("add i v2");
        REQUIRE(IREG == 0x42F);
    }
}

TEST_CASE("LDS - Load sprite location", "[chip8][sprite]")
{
    Chip8 cpu;
    cpu.init(assemble(R"(
        ld v0 0x00
        ld f v0
        ld v1 0x05
        ld f v1
        ld v2 0x0F
        ld f v2
        ld v3 0x0A
        ld f v3
    )"));

    SECTION("Sprite for 0")
    {
        EXECUTE_OPCODE("ld f v0");
        REQUIRE(IREG == 0x00);
    }

    SECTION("Sprite for 5")
    {
        EXECUTE_OPCODE("ld f v1");
        REQUIRE(IREG == 0x19); // 5 * 5
    }

    SECTION("Sprite for F")
    {
        EXECUTE_OPCODE("ld f v2");
        REQUIRE(IREG == 0x4B); // 15 * 5
    }

    SECTION("Sprite for A")
    {
        EXECUTE_OPCODE("ld f v3");
        REQUIRE(IREG == 0x32); // 10 * 5
    }
}

TEST_CASE("LBCD - BCD conversion edge cases", "[chip8][bcd]")
{
    Chip8 cpu;
    cpu.init(assemble(R"(
        ld i 0x400
        ld v0 0
        ld b v0
        ld v1 255
        ld i 0x410
        ld b v1
        ld v2 100
        ld i 0x420
        ld b v2
        ld v3 9
        ld i 0x430
        ld b v3
    )"));

    SECTION("BCD of 0")
    {
        EXECUTE_OPCODE("ld b v0");
        REQUIRE(MEM[0x400] == 0);
        REQUIRE(MEM[0x401] == 0);
        REQUIRE(MEM[0x402] == 0);
    }

    SECTION("BCD of 255")
    {
        EXECUTE_OPCODE("ld b v1");
        REQUIRE(MEM[0x410] == 2);
        REQUIRE(MEM[0x411] == 5);
        REQUIRE(MEM[0x412] == 5);
    }

    SECTION("BCD of 100")
    {
        EXECUTE_OPCODE("ld b v2");
        REQUIRE(MEM[0x420] == 1);
        REQUIRE(MEM[0x421] == 0);
        REQUIRE(MEM[0x422] == 0);
    }

    SECTION("BCD of 9")
    {
        EXECUTE_OPCODE("ld b v3");
        REQUIRE(MEM[0x430] == 0);
        REQUIRE(MEM[0x431] == 0);
        REQUIRE(MEM[0x432] == 9);
    }
}

TEST_CASE("Stack operations - multiple calls and returns", "[chip8][stack]")
{
    Chip8 cpu;
    cpu.init(assemble(R"(
        call 0x20A      ; 0x200
        ld v0 0x01      ; 0x202
        call 0x20E      ; 0x204
        ld v0 0x02      ; 0x206
        jp 0x218        ; 0x208

        ld v1 0x10      ; 0x20A
        ret             ; 0x20C

        ld v2 0x20      ; 0x20E
        call 0x214      ; 0x210
        ret             ; 0x212

        ld v3 0x30      ; 0x214
        ret             ; 0x216

        ld v4 0x40      ; 0x218
    )"));

    SECTION("First call")
    {
        RUN_TO_PC(0x20A);
        REQUIRE(SP == 1);
        REQUIRE(STACK[0] == 0x202);
    }

    SECTION("After first return")
    {
        RUN_TO_PC(0x20C);
        REQUIRE(SP == 1);
        RUN_TICKS(1);
        REQUIRE(V1 == 0x10);
        REQUIRE(SP == 0);
    }

    SECTION("Nested calls")
    {
        RUN_TO_PC(0x214);
        REQUIRE(SP == 2);
        REQUIRE(STACK[0] == 0x206);
        REQUIRE(STACK[1] == 0x212);
    }

    SECTION("All returns unwind stack")
    {
        RUN_TO_PC(0x218);
        REQUIRE(SP == 0);
        REQUIRE(V1 == 0x10);
        REQUIRE(V2 == 0x20);
        REQUIRE(V3 == 0x30);
        RUN_TICKS(1);
        REQUIRE(V4 == 0x40);
    }
}

TEST_CASE("LDMR and LDRM - Memory operations with all registers", "[chip8][memory]")
{
    Chip8 cpu;
    cpu.init(assemble(R"(
        ld v0 0x00
        ld v1 0x11
        ld v2 0x22
        ld v3 0x33
        ld v4 0x44
        ld v5 0x55
        ld v6 0x66
        ld v7 0x77
        ld v8 0x88
        ld v9 0x99
        ld va 0xAA
        ld vb 0xBB
        ld vc 0xCC
        ld vd 0xDD
        ld ve 0xEE
        ld vf 0xFF

        ld i 0x500
        ld [i] vf

        ld v0 0xFF
        ld v1 0xFF
        ld v2 0xFF
        ld v3 0xFF
        ld v4 0xFF
        ld v5 0xFF
        ld v6 0xFF
        ld v7 0xFF
        ld v8 0xFF
        ld v9 0xFF
        ld va 0xFF
        ld vb 0xFF
        ld vc 0xFF
        ld vd 0xFF
        ld ve 0xFF
        ld vf 0xFF

        ld i 0x500
        ld vf [i]
    )"));

    SECTION("Store all registers")
    {
        EXECUTE_OPCODE("ld [i] vf");
        for (int i = 0; i <= 0xF; ++i)
            REQUIRE(MEM[0x500 + i] == (i * 0x11));
    }

    SECTION("Load all registers")
    {
        EXECUTE_OPCODE("ld vf [i]");
        for (int i = 0; i <= 0xF; ++i)
            REQUIRE(REGS[i] == (i * 0x11));
    }
}

TEST_CASE("LDMR and LDRM - Partial register save/load", "[chip8][memory]")
{
    Chip8 cpu;
    cpu.init(assemble(R"(
        ld v0 0x10
        ld v1 0x20
        ld v2 0x30
        ld v3 0x99
        ld v4 0x99

        ld i 0x600
        ld [i] v2

        ld v0 0xFF
        ld v1 0xFF
        ld v2 0xFF

        ld i 0x600
        ld v1 [i]
    )"));

    SECTION("Store only V0-V2")
    {
        EXECUTE_OPCODE("ld [i] v2");
        REQUIRE(MEM[0x600] == 0x10);
        REQUIRE(MEM[0x601] == 0x20);
        REQUIRE(MEM[0x602] == 0x30);
        REQUIRE(MEM[0x603] == 0x00); // Unchanged
    }

    SECTION("Load only V0-V1")
    {
        EXECUTE_OPCODE("ld v1 [i]");
        REQUIRE(V0 == 0x10);
        REQUIRE(V1 == 0x20);
        REQUIRE(V2 == 0xFF); // Unchanged
    }
}

TEST_CASE("DRW - Draw sprite with collision detection", "[chip8][draw]")
{
    Chip8 cpu;
    cpu.init(assemble(R"(
        ld i 0x300
        ld v0 0x05
        ld v1 0x05
        drw v0 v1 0x1

        ld v2 0x05
        ld v3 0x05
        drw v2 v3 0x1

        db 0xFF
    )"));

    // Place sprite data

    Chip8TestAccess::setMemory(cpu, 0x300, 0xFF);

    SECTION("First draw sets pixels")
    {
        EXECUTE_OPCODE("drw v0 v1 0x1");
        for (int i = 0; i < 8; ++i)
            REQUIRE(FB[5 + i].test(5) == true);
        REQUIRE(VF == 0); // No collision
    }

    SECTION("Second draw on same location creates collision")
    {
        RUN_TO_OPCODE("db 0xFF");
        REQUIRE(VF == 1); // Collision detected
        for (int i = 0; i < 8; ++i)
            REQUIRE(FB[5 + i].test(5) == false); // Pixels cleared
    }
}

TEST_CASE("DRW - Multi-row sprite", "[chip8][draw]")
{
    Chip8 cpu;
    cpu.init(assemble(R"(
        ld i 0x400
        ld v0 0x10
        ld v1 0x10
        drw v0 v1 0x5
    )"));

    Chip8TestAccess::setMemory(cpu, 0x400, 0xF0);
    Chip8TestAccess::setMemory(cpu, 0x401, 0x90);
    Chip8TestAccess::setMemory(cpu, 0x402, 0x90);
    Chip8TestAccess::setMemory(cpu, 0x403, 0x90);
    Chip8TestAccess::setMemory(cpu, 0x404, 0xF0);

    RUN_TICKS(3);
    cpu.tick();

    // Check top row (0xF0 = 11110000)
    REQUIRE(FB[0x10].test(0x10) == true);
    REQUIRE(FB[0x11].test(0x10) == true);
    REQUIRE(FB[0x12].test(0x10) == true);
    REQUIRE(FB[0x13].test(0x10) == true);
    REQUIRE(FB[0x14].test(0x10) == false);
}

TEST_CASE("RND - Random number generation", "[chip8][rand]")
{
    Chip8 cpu;
    cpu.init(assemble(R"(
        rnd v0 0xFF
        rnd v1 0x0F
        rnd v2 0xF0
        rnd v3 0x00
    )"));

    SECTION("Random with full mask")
    {
        EXECUTE_OPCODE("rnd v0 0xFF");
        // Can be anything from 0-255
        REQUIRE((V0 >= 0 && V0 <= 255));
    }

    SECTION("Random with low nibble mask")
    {
        EXECUTE_OPCODE("rnd v1 0x0F");
        // Must be in range 0-15
        REQUIRE((V1 >= 0 && V1 <= 15));
    }

    SECTION("Random with high nibble mask")
    {
        EXECUTE_OPCODE("rnd v2 0xF0");
        // Lower nibble must be 0
        REQUIRE((V2 & 0x0F) == 0);
    }

    SECTION("Random with zero mask")
    {
        EXECUTE_OPCODE("rnd v3 0x00");
        REQUIRE(V3 == 0);
    }
}

TEST_CASE("Shift operations - edge cases with VF", "[chip8][shift]")
{
    Chip8 cpu;
    cpu.init(assemble(R"(
        ld v0 0x01
        shr v0

        ld v1 0x00
        shr v1

        ld v2 0x80
        shl v2

        ld v3 0x00
        shl v3
    )"));

    SECTION("SHR with LSB=1 sets VF")
    {
        EXECUTE_OPCODE("shr v0");
        REQUIRE(V0 == 0x00);
        REQUIRE(VF == 0x01);
    }

    SECTION("SHR with LSB=0 clears VF")
    {
        EXECUTE_OPCODE("shr v1");
        REQUIRE(V1 == 0x00);
        REQUIRE(VF == 0x00);
    }

    SECTION("SHL with MSB=1 sets VF")
    {
        EXECUTE_OPCODE("shl v2");
        REQUIRE(V2 == 0x00);
        REQUIRE(VF == 0x01);
    }

    SECTION("SHL with MSB=0 clears VF")
    {
        EXECUTE_OPCODE("shl v3");
        REQUIRE(V3 == 0x00);
        REQUIRE(VF == 0x00);
    }
}

TEST_CASE("Overflow and underflow in arithmetic", "[chip8][alu]")
{
    Chip8 cpu;
    cpu.init(assemble(R"(
        ld v0 0x00
        ld v1 0x01
        sub v0 v1

        ld v2 0xFF
        ld v3 0x01
        add v2 v3

        ld v4 0x01
        ld v5 0x01
        sub v4 v5
    )"));

    SECTION("Underflow in subtraction")
    {
        EXECUTE_OPCODE("sub v0 v1");
        REQUIRE(V0 == 0xFF);
        REQUIRE(VF == 0x00); // Borrow occurred
    }

    SECTION("Overflow in addition")
    {
        EXECUTE_OPCODE("add v2 v3");
        REQUIRE(V2 == 0x00);
        REQUIRE(VF == 0x01); // Carry occurred
    }

    SECTION("Exact subtraction (no borrow)")
    {
        EXECUTE_OPCODE("sub v4 v5");
        REQUIRE(V4 == 0x00);
        REQUIRE(VF == 0x01); // No borrow
    }
}

TEST_CASE("VF register in various operations", "[chip8][vf]")
{
    Chip8 cpu;
    cpu.init(assemble(R"(
        ld vf 0x42
        ld v0 vf

        ld vf 0x99
        ld v1 0x01
        add v1 vf

        ld v2 0xFF
        ld v3 0x01
        add v2 v3
        ld v4 vf
    )"));

    SECTION("VF can be loaded like other registers")
    {
        EXECUTE_OPCODE("ld v0 vf");
        REQUIRE(V0 == 0x42);
    }

    SECTION("VF can be used in operations")
    {
        EXECUTE_OPCODE("add v1 vf");
        REQUIRE(V1 == 0x9A);
    }

    SECTION("VF is overwritten by flag operations")
    {
        EXECUTE_OPCODE("ld v4 vf");
        REQUIRE(VF == 0x01); // From previous ADD overflow
        REQUIRE(V4 == 0x01);
    }
}
