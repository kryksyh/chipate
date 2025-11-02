#include "asm.h"
#include "chip8.h"

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

using namespace chipate;

TEST_CASE("Chip8: DRW opcode draws sprite to framebuffer", "[chip8][draw]")
{
    Chip8 cpu;

    std::string program = R"(
        ld v0 0x05
        ld v1 0x02
        ld i 0x208
        drw v0 v1 0x1
        db 0x80
    )";

    cpu.init(assemble(program));

    cpu.tick();
    cpu.tick();
    cpu.tick();
    cpu.tick();

    auto fb = cpu.fb();

    // V0 == 5, V1 == 2, sprite 0x80 has highest bit set -> sets pixel at (5,2)
    REQUIRE(fb[5].test(2) == true);
}
