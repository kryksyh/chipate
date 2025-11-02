#include "chip8.h"

#include <catch2/catch_test_macros.hpp>
#include <vector>

using namespace chipate;

TEST_CASE("Chip8: DRAW opcode draws sprite to framebuffer", "[chip8][draw]")
{
    Chip8 cpu;

    // Program layout (loaded at 0x200):
    // 0x200: 60 05    LD V0, 0x05
    // 0x202: 61 02    LD V1, 0x02
    // 0x204: A2 08    LD I, 0x208   (sprite placed immediately after instructions)
    // 0x206: D0 11    DRW V0, V1, 1
    // 0x208: 80       sprite byte: 1000 0000

    std::vector<uint8_t> program = {
        0x60, 0x05, // LD V0, 0x05
        0x61, 0x02, // LD V1, 0x02
        0xA2, 0x08, // LD I, 0x208
        0xD0, 0x11, // DRW V0, V1, 1
        0x80        // sprite data (1 row)
    };

    cpu.init(program);

    // Execute four instructions
    cpu.tick(); // LD V0
    cpu.tick(); // LD V1
    cpu.tick(); // LD I
    cpu.tick(); // DRW

    auto fb = cpu.fb();

    // V0 == 5, V1 == 2, sprite 0x80 has highest bit set -> sets pixel at (5,2)
    REQUIRE(fb[5].test(2) == true);
}
