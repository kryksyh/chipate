#include "asm.h"

#include <catch2/catch_test_macros.hpp>
#include <vector>

using namespace chipate;

TEST_CASE("Assembly: CLS instruction", "[asm]")
{
    auto bytecode = assemble("cls");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x00);
    REQUIRE(bytecode[1] == 0xE0);
}

TEST_CASE("Assembly: RET instruction", "[asm]")
{
    auto bytecode = assemble("ret");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x00);
    REQUIRE(bytecode[1] == 0xEE);
}

TEST_CASE("Assembly: JP with address", "[asm]")
{
    auto bytecode = assemble("jp 0x200");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x12);
    REQUIRE(bytecode[1] == 0x00);
}

TEST_CASE("Assembly: JP with V0 offset", "[asm]")
{
    auto bytecode = assemble("jp v0 0x300");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0xB3);
    REQUIRE(bytecode[1] == 0x00);
}

TEST_CASE("Assembly: CALL instruction", "[asm]")
{
    auto bytecode = assemble("call 0x400");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x24);
    REQUIRE(bytecode[1] == 0x00);
}

TEST_CASE("Assembly: SE register with byte", "[asm]")
{
    auto bytecode = assemble("se v5 0x42");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x35);
    REQUIRE(bytecode[1] == 0x42);
}

TEST_CASE("Assembly: SE register with register", "[asm]")
{
    auto bytecode = assemble("se v3 v7");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x53);
    REQUIRE(bytecode[1] == 0x70);
}

TEST_CASE("Assembly: SNE register with byte", "[asm]")
{
    auto bytecode = assemble("sne va 0xFF");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x4A);
    REQUIRE(bytecode[1] == 0xFF);
}

TEST_CASE("Assembly: SNE register with register", "[asm]")
{
    auto bytecode = assemble("sne v2 v8");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x92);
    REQUIRE(bytecode[1] == 0x80);
}

TEST_CASE("Assembly: LD register with byte", "[asm]")
{
    auto bytecode = assemble("ld v0 0x05");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x60);
    REQUIRE(bytecode[1] == 0x05);
}

TEST_CASE("Assembly: LD register with register", "[asm]")
{
    auto bytecode = assemble("ld v4 v9");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x84);
    REQUIRE(bytecode[1] == 0x90);
}

TEST_CASE("Assembly: LD I with address", "[asm]")
{
    auto bytecode = assemble("ld i 0x208");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0xA2);
    REQUIRE(bytecode[1] == 0x08);
}

TEST_CASE("Assembly: LD register from DT", "[asm]")
{
    auto bytecode = assemble("ld v3 dt");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0xF3);
    REQUIRE(bytecode[1] == 0x07);
}

TEST_CASE("Assembly: LD DT from register", "[asm]")
{
    auto bytecode = assemble("ld dt v6");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0xF6);
    REQUIRE(bytecode[1] == 0x15);
}

TEST_CASE("Assembly: LD ST from register", "[asm]")
{
    auto bytecode = assemble("ld st v7");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0xF7);
    REQUIRE(bytecode[1] == 0x18);
}

TEST_CASE("Assembly: LD register from K", "[asm]")
{
    auto bytecode = assemble("ld v2 k");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0xF2);
    REQUIRE(bytecode[1] == 0x0A);
}

TEST_CASE("Assembly: LD F from register", "[asm]")
{
    auto bytecode = assemble("ld f v8");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0xF8);
    REQUIRE(bytecode[1] == 0x29);
}

TEST_CASE("Assembly: LD B from register", "[asm]")
{
    auto bytecode = assemble("ld b va");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0xFA);
    REQUIRE(bytecode[1] == 0x33);
}

TEST_CASE("Assembly: LD [I] from register", "[asm]")
{
    auto bytecode = assemble("ld [i] v5");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0xF5);
    REQUIRE(bytecode[1] == 0x55);
}

TEST_CASE("Assembly: LD register from [I]", "[asm]")
{
    auto bytecode = assemble("ld v4 [i]");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0xF4);
    REQUIRE(bytecode[1] == 0x65);
}

TEST_CASE("Assembly: ADD register with byte", "[asm]")
{
    auto bytecode = assemble("add v1 0x10");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x71);
    REQUIRE(bytecode[1] == 0x10);
}

TEST_CASE("Assembly: ADD register with register", "[asm]")
{
    auto bytecode = assemble("add v2 v3");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x82);
    REQUIRE(bytecode[1] == 0x34);
}

TEST_CASE("Assembly: ADD I with register", "[asm]")
{
    auto bytecode = assemble("add i vc");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0xFC);
    REQUIRE(bytecode[1] == 0x1E);
}

TEST_CASE("Assembly: OR instruction", "[asm]")
{
    auto bytecode = assemble("or v5 v6");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x85);
    REQUIRE(bytecode[1] == 0x61);
}

TEST_CASE("Assembly: AND instruction", "[asm]")
{
    auto bytecode = assemble("and v7 v8");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x87);
    REQUIRE(bytecode[1] == 0x82);
}

TEST_CASE("Assembly: XOR instruction", "[asm]")
{
    auto bytecode = assemble("xor v9 va");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x89);
    REQUIRE(bytecode[1] == 0xA3);
}

TEST_CASE("Assembly: SUB instruction", "[asm]")
{
    auto bytecode = assemble("sub vb vc");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x8B);
    REQUIRE(bytecode[1] == 0xC5);
}

TEST_CASE("Assembly: SUBN instruction", "[asm]")
{
    auto bytecode = assemble("subn vd ve");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x8D);
    REQUIRE(bytecode[1] == 0xE7);
}

TEST_CASE("Assembly: SHR with one register", "[asm]")
{
    auto bytecode = assemble("shr v3");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x83);
    REQUIRE(bytecode[1] == 0x06);
}

TEST_CASE("Assembly: SHR with two registers", "[asm]")
{
    auto bytecode = assemble("shr v4 v5");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x84);
    REQUIRE(bytecode[1] == 0x56);
}

TEST_CASE("Assembly: SHL with one register", "[asm]")
{
    auto bytecode = assemble("shl v6");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x86);
    REQUIRE(bytecode[1] == 0x0E);
}

TEST_CASE("Assembly: SHL with two registers", "[asm]")
{
    auto bytecode = assemble("shl v7 v8");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x87);
    REQUIRE(bytecode[1] == 0x8E);
}

TEST_CASE("Assembly: RND instruction", "[asm]")
{
    auto bytecode = assemble("rnd va 0xFF");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0xCA);
    REQUIRE(bytecode[1] == 0xFF);
}

TEST_CASE("Assembly: DRW instruction", "[asm]")
{
    auto bytecode = assemble("drw v0 v1 0x5");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0xD0);
    REQUIRE(bytecode[1] == 0x15);
}

TEST_CASE("Assembly: SKP instruction", "[asm]")
{
    auto bytecode = assemble("skp vb");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0xEB);
    REQUIRE(bytecode[1] == 0x9E);
}

TEST_CASE("Assembly: SKNP instruction", "[asm]")
{
    auto bytecode = assemble("sknp vc");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0xEC);
    REQUIRE(bytecode[1] == 0xA1);
}

TEST_CASE("Assembly: Multiple instructions", "[asm]")
{
    auto bytecode = assemble("ld v0 0x05\nld v1 0x02\nadd v0 v1");
    REQUIRE(bytecode.size() == 6);
    // LD V0, 0x05
    REQUIRE(bytecode[0] == 0x60);
    REQUIRE(bytecode[1] == 0x05);
    // LD V1, 0x02
    REQUIRE(bytecode[2] == 0x61);
    REQUIRE(bytecode[3] == 0x02);
    // ADD V0, V1
    REQUIRE(bytecode[4] == 0x80);
    REQUIRE(bytecode[5] == 0x14);
}

TEST_CASE("Assembly: Instructions with comments", "[asm]")
{
    auto bytecode = assemble("ld v0 0x05 ; Load 5 into V0\nld v1 0x02 ; Load 2 into V1");
    REQUIRE(bytecode.size() == 4);
    REQUIRE(bytecode[0] == 0x60);
    REQUIRE(bytecode[1] == 0x05);
    REQUIRE(bytecode[2] == 0x61);
    REQUIRE(bytecode[3] == 0x02);
}

TEST_CASE("Assembly: Complete program example", "[asm]")
{
    std::string program = R"(
        ld v0 0x05
        ld v1 0x02
        ld i 0x208
        drw v0 v1 0x1
    )";

    auto bytecode = assemble(program);
    REQUIRE(bytecode.size() == 8);

    // LD V0, 0x05
    REQUIRE(bytecode[0] == 0x60);
    REQUIRE(bytecode[1] == 0x05);

    // LD V1, 0x02
    REQUIRE(bytecode[2] == 0x61);
    REQUIRE(bytecode[3] == 0x02);

    // LD I, 0x208
    REQUIRE(bytecode[4] == 0xA2);
    REQUIRE(bytecode[5] == 0x08);

    // DRW V0, V1, 1
    REQUIRE(bytecode[6] == 0xD0);
    REQUIRE(bytecode[7] == 0x11);
}

TEST_CASE("Assembly: Hexadecimal register references", "[asm]")
{
    auto bytecode = assemble("ld vf 0xFF");
    REQUIRE(bytecode.size() == 2);
    REQUIRE(bytecode[0] == 0x6F);
    REQUIRE(bytecode[1] == 0xFF);
}

TEST_CASE("Assembly: Various number formats", "[asm]")
{
    SECTION("Decimal")
    {
        auto bytecode = assemble("ld v0 255");
        REQUIRE(bytecode.size() == 2);
        REQUIRE(bytecode[0] == 0x60);
        REQUIRE(bytecode[1] == 0xFF);
    }

    SECTION("Hexadecimal with 0x prefix")
    {
        auto bytecode = assemble("ld v0 0xFF");
        REQUIRE(bytecode.size() == 2);
        REQUIRE(bytecode[0] == 0x60);
        REQUIRE(bytecode[1] == 0xFF);
    }

    SECTION("Octal")
    {
        auto bytecode = assemble("ld v0 0377");
        REQUIRE(bytecode.size() == 2);
        REQUIRE(bytecode[0] == 0x60);
        REQUIRE(bytecode[1] == 0xFF);
    }
}

TEST_CASE("Assembly: Edge cases for address range", "[asm]")
{
    SECTION("Minimum address")
    {
        auto bytecode = assemble("jp 0x000");
        REQUIRE(bytecode.size() == 2);
        REQUIRE(bytecode[0] == 0x10);
        REQUIRE(bytecode[1] == 0x00);
    }

    SECTION("Maximum address")
    {
        auto bytecode = assemble("jp 0xFFF");
        REQUIRE(bytecode.size() == 2);
        REQUIRE(bytecode[0] == 0x1F);
        REQUIRE(bytecode[1] == 0xFF);
    }
}

TEST_CASE("Assembly: Edge cases for byte values", "[asm]")
{
    SECTION("Minimum byte")
    {
        auto bytecode = assemble("ld v0 0x00");
        REQUIRE(bytecode.size() == 2);
        REQUIRE(bytecode[0] == 0x60);
        REQUIRE(bytecode[1] == 0x00);
    }

    SECTION("Maximum byte")
    {
        auto bytecode = assemble("ld v0 0xFF");
        REQUIRE(bytecode.size() == 2);
        REQUIRE(bytecode[0] == 0x60);
        REQUIRE(bytecode[1] == 0xFF);
    }
}

TEST_CASE("Assembly: Edge cases for nibble values", "[asm]")
{
    SECTION("Minimum nibble")
    {
        auto bytecode = assemble("drw v0 v1 0x0");
        REQUIRE(bytecode.size() == 2);
        REQUIRE(bytecode[0] == 0xD0);
        REQUIRE(bytecode[1] == 0x10);
    }

    SECTION("Maximum nibble")
    {
        auto bytecode = assemble("drw v0 v1 0xF");
        REQUIRE(bytecode.size() == 2);
        REQUIRE(bytecode[0] == 0xD0);
        REQUIRE(bytecode[1] == 0x1F);
    }
}
