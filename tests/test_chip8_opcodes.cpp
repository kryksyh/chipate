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
