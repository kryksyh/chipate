// SPDX-License-Identifier: WTFPL

#include "asm.h"

#include "log.h"
#include "opcode.h"

#include <cctype>
#include <cstdint>
#include <functional>
#include <map>

namespace chipate {

namespace {
char const* skip_space(char const* it)
{
    while (*it && std::isspace(*it) && *it != '\n')
        it++;
    if (*it == ';')
        while (*it && *it != '\n')
            it++;
    return it;
}

char const* nextLine(char const* it)
{
    while (*it && *it != '\n')
        it++;
    if (*it == '\n')
        it++;
    return it;
}

std::string nextToken(char const*& it)
{
    it = skip_space(it);

    char const* begin = it;
    while (*it && (std::isalpha(*it) || std::isdigit(*it)) || *it == '[' || *it == ']')
        it++;
    if (it && it != begin)
        return std::string(begin, it);
    return {};
}

using process = std::function<uint16_t(std::vector<std::string> const&)>;

bool is_reg(std::string const& arg)
{
    return arg.size() == 2 && arg[0] == 'v';
}

bool is_index(std::string const& arg)
{
    return arg.size() == 1 && arg[0] == 'i';
}

bool is_dt(std::string const& arg)
{
    return arg.size() == 2 && arg == "dt";
}

bool is_st(std::string const& arg)
{
    return arg.size() == 2 && arg == "st";
}

bool is_f(std::string const& arg)
{
    return arg.size() == 1 && arg[0] == 'f';
}

bool is_k(std::string const& arg)
{
    return arg.size() == 1 && arg[0] == 'k';
}

bool is_b(std::string const& arg)
{
    return arg.size() == 1 && arg[0] == 'b';
}

bool is_indirect(std::string const& arg)
{
    return arg.size() == 3 && arg == "[i]";
}

bool is_nibble(std::string const& arg)
{
    try {
        auto val = std::stoul(arg, nullptr, 0);
        return val <= 0x0F;
    }
    catch (...) {
        return false;
    }
}

bool is_byte(std::string const& arg, bool forceHex = false)
{
    try {
        auto val = std::stoul(arg, nullptr, forceHex ? 16 : 0);
        return val <= 0xFF;
    }
    catch (...) {
        return false;
    }
}

bool is_address(std::string const& arg)
{
    try {
        auto val = std::stoul(arg, nullptr, 0);
        return val <= 0x0FFF;
    }
    catch (...) {
        return false;
    }
}

uint16_t reg(std::string const& arg)
{
    return std::stoul(arg.substr(1), nullptr, 16);
}

uint8_t nibble(std::string const& arg)
{
    return static_cast<uint8_t>(std::stoul(arg, nullptr, 0) & 0x0F);
}

uint8_t byte(std::string const& arg, bool forceHex = false)
{
    return static_cast<uint8_t>(std::stoul(arg, nullptr, forceHex ? 16 : 0) & 0xFF);
}

uint16_t word(std::string const& arg)
{
    return static_cast<uint16_t>(std::stoul(arg, nullptr, 0) & 0xFFFF);
}

uint16_t address(std::string const& arg)
{
    return word(arg) & 0x0FFF;
}

#define INVALID_ARGS(cmd)                                                                          \
    loge("Invalid arguments for %s: ", cmd);                                                       \
    if (args.size() > 0)                                                                           \
        loge("\t\t%s", args[0].c_str());                                                           \
    if (args.size() > 1)                                                                           \
        loge("\t\t%s", args[1].c_str());                                                           \
    if (args.size() > 2)                                                                           \
        loge("\t\t%s", args[2].c_str());                                                           \
    if (args.size() > 3)                                                                           \
        loge("\t\t%s", args[3].c_str());                                                           \
    return 0xFFFF

std::map<std::string, process> processors{
    {"cls",
     [](std::vector<std::string> const& args) -> uint16_t {
         return 0x00E0;
     }          },
    {"ret",
     [](std::vector<std::string> const& args) -> uint16_t {
         return 0x00EE;
     }          },
    {"jp",
     [](std::vector<std::string> const& args) -> uint16_t {
         if (args.size() == 1 && is_address(args[0])) { // simple jump
             uint16_t addr = address(args[0]);
             return 0x1000 | addr;
         }
         else if (args.size() == 2 && args[0] == "v0") { // jump with offset
             uint16_t addr = address(args[1]);
             return 0xB000 | addr;
         }
         INVALID_ARGS("jp");
     }          },
    {"call",
     [](std::vector<std::string> const& args) -> uint16_t {
         if (args.size() == 1 && is_address(args[0])) {
             uint16_t addr = std::stoul(args[0], nullptr, 0) & 0x0FFF;
             return 0x2000 | addr;
         }

         INVALID_ARGS("call");
     }          },
    {"se",
     [](std::vector<std::string> const& args) -> uint16_t {
         if (args.size() != 2) {
             INVALID_ARGS("se");
         }

         if (is_reg(args[0]) && is_reg(args[1])) {
             auto x = reg(args[0]);
             auto y = reg(args[1]);

             return 0x5000 | (x << 8) | (y << 4);
         }
         else if (is_reg(args[0]) && is_byte(args[1])) {
             auto x = reg(args[0]);
             auto b = byte(args[1]);

             return 0x3000 | (x << 8) | b;
         }
         INVALID_ARGS("se");
     }          },
    {"sne",
     [](std::vector<std::string> const& args) -> uint16_t {
         if (args.size() != 2) {
             INVALID_ARGS("sne");
         }

         if (is_reg(args[0]) && is_reg(args[1])) {
             auto x = reg(args[0]);
             auto y = reg(args[1]);

             return 0x9000 | (x << 8) | (y << 4);
         }
         else if (is_reg(args[0])) {
             auto x = reg(args[0]);
             auto b = byte(args[1]);

             return 0x4000 | (x << 8) | b;
         }
         INVALID_ARGS("sne");
     }          },
    {"ld",
     [](std::vector<std::string> const& args) -> uint16_t {
         if (args.size() != 2) {
             INVALID_ARGS("ld");
         }

         if (is_reg(args[0]) && is_reg(args[1])) {
             auto x = reg(args[0]);
             auto y = reg(args[1]);

             return 0x8000 | (x << 8) | (y << 4);
         }
         else if (is_reg(args[0]) && is_dt(args[1])) {
             auto x = reg(args[0]);
             return 0xF007 | (x << 8);
         }
         else if (is_reg(args[0]) && is_k(args[1])) {
             auto x = reg(args[0]);
             return 0xF00A | (x << 8);
         }
         else if (is_dt(args[0]) && is_reg(args[1])) {
             auto x = reg(args[1]);
             return 0xF015 | (x << 8);
         }
         else if (is_st(args[0]) && is_reg(args[1])) {
             auto x = reg(args[1]);
             return 0xF018 | (x << 8);
         }
         else if (is_f(args[0]) && is_reg(args[1])) {
             auto x = reg(args[1]);
             return 0xF029 | (x << 8);
         }
         else if (is_b(args[0]) && is_reg(args[1])) {
             auto x = reg(args[1]);
             return 0xF033 | (x << 8);
         }
         else if (is_indirect(args[0]) && is_reg(args[1])) {
             auto x = reg(args[1]);
             return 0xF055 | (x << 8);
         }
         else if (is_reg(args[0]) && is_indirect(args[1])) {
             auto x = reg(args[0]);
             return 0xF065 | (x << 8);
         }
         else if (is_index(args[0]) && is_address(args[1])) {
             auto addr = address(args[1]);
             return 0xA000 | addr;
         }
         else if (is_reg(args[0]) && is_byte(args[1])) {
             auto x = reg(args[0]);
             auto b = byte(args[1]);
             return 0x6000 | (x << 8) | b;
         }
         INVALID_ARGS("ld");
     }          },
    {"add",
     [](std::vector<std::string> const& args) -> uint16_t {
         if (args.size() != 2) {
             INVALID_ARGS("add");
         }

         if (is_reg(args[0]) && is_reg(args[1])) {
             auto x = reg(args[0]);
             auto y = reg(args[1]);

             return 0x8004 | (x << 8) | (y << 4);
         }
         else if (is_index(args[0]) && is_reg(args[1])) {
             auto x = reg(args[1]);

             return 0xF01E | (x << 8);
         }
         else if (is_reg(args[0]) && is_byte(args[1])) {
             auto x = reg(args[0]);
             auto b = byte(args[1]);

             return 0x7000 | (x << 8) | b;
         }
         INVALID_ARGS("add");
     }          },
    {"or",
     [](std::vector<std::string> const& args) -> uint16_t {
         if (args.size() != 2) {
             INVALID_ARGS("or");
         }

         if (is_reg(args[0]) && is_reg(args[1])) {
             auto x = reg(args[0]);
             auto y = reg(args[1]);

             return 0x8001 | (x << 8) | (y << 4);
         }
         INVALID_ARGS("or");
     }          },
    {"and",
     [](std::vector<std::string> const& args) -> uint16_t {
         if (args.size() != 2) {
             INVALID_ARGS("and");
         }

         if (is_reg(args[0]) && is_reg(args[1])) {
             auto x = reg(args[0]);
             auto y = reg(args[1]);

             return 0x8002 | (x << 8) | (y << 4);
         }
         INVALID_ARGS("and");
     }          },
    {"xor",
     [](std::vector<std::string> const& args) -> uint16_t {
         if (args.size() != 2) {
             INVALID_ARGS("xor");
         }

         if (is_reg(args[0]) && is_reg(args[1])) {
             auto x = reg(args[0]);
             auto y = reg(args[1]);

             return 0x8003 | (x << 8) | (y << 4);
         }
         INVALID_ARGS("xor");
     }          },
    {"sub",
     [](std::vector<std::string> const& args) -> uint16_t {
         if (args.size() != 2) {
             INVALID_ARGS("sub");
         }

         if (is_reg(args[0]) && is_reg(args[1])) {
             auto x = reg(args[0]);
             auto y = reg(args[1]);

             return 0x8005 | (x << 8) | (y << 4);
         }
         INVALID_ARGS("sub");
     }          },
    {"subn",
     [](std::vector<std::string> const& args) -> uint16_t {
         if (args.size() != 2) {
             INVALID_ARGS("subn");
         }

         if (is_reg(args[0]) && is_reg(args[1])) {
             auto x = reg(args[0]);
             auto y = reg(args[1]);

             return 0x8007 | (x << 8) | (y << 4);
         }
         INVALID_ARGS("subn");
     }          },
    {"shr",
     [](std::vector<std::string> const& args) -> uint16_t {
         if (args.empty() || args.size() > 2) {
             INVALID_ARGS("shr");
         }

         if (args.size() == 2 && is_reg(args[0]) && is_reg(args[1])) {
             auto x = reg(args[0]);
             auto y = reg(args[1]);

             return 0x8006 | (x << 8) | (y << 4);
         }
         else if (args.size() == 1 && is_reg(args[0])) {
             auto x = reg(args[0]);
             return 0x8006 | (x << 8);
         }
         INVALID_ARGS("shr");
     }          },

    {"shl",
     [](std::vector<std::string> const& args) -> uint16_t {
         if (args.empty() || args.size() > 2) {
             INVALID_ARGS("shl");
         }

         if (args.size() == 2 && is_reg(args[0]) && is_reg(args[1])) {
             auto x = reg(args[0]);
             auto y = reg(args[1]);

             return 0x800E | (x << 8) | (y << 4);
         }
         else if (args.size() == 1 && is_reg(args[0])) {
             auto x = reg(args[0]);
             return 0x800E | (x << 8);
         }
         INVALID_ARGS("shl");
     }          },
    {"rnd",
     [](std::vector<std::string> const& args) -> uint16_t {
         if (args.size() != 2) {
             INVALID_ARGS("rnd");
         }
         if (is_reg(args[0]) && is_byte(args[1])) {
             auto x = reg(args[0]);
             auto b = byte(args[1]);
             return 0xC000 | (x << 8) | b;
         }
         INVALID_ARGS("rnd");
     }          },
    {"drw",
     [](std::vector<std::string> const& args) -> uint16_t {
         if (args.size() != 3) {
             loge("Invalid argument count for drw: %d", args.size());
             return 0xFFFF;
         }
         if (is_reg(args[0]) && is_reg(args[1]) && is_nibble(args[2])) {
             auto x = reg(args[0]);
             auto y = reg(args[1]);
             auto b = nibble(args[2]);
             return 0xD000 | (x << 8) | (y << 4) | b;
         }
         loge("Invalid arguments for drw");
         return 0xFFFF;
     }          },
    {"skp",
     [](std::vector<std::string> const& args) -> uint16_t {
         if (args.size() != 1) {
             loge("Invalid argument count for skp: %d", args.size());
             return 0xFFFF;
         }
         if (is_reg(args[0])) {
             auto x = reg(args[0]);
             return 0xE09E | (x << 8);
         }
         loge("Invalid arguments for skp");
         return 0xFFFF;
     }          },
    {"sknp",
     [](std::vector<std::string> const& args) -> uint16_t {
         if (args.size() != 1) {
             loge("Invalid argument count for sknp: %d", args.size());
             return 0xFFFF;
         }
         if (is_reg(args[0])) {
             auto x = reg(args[0]);
             return 0xE0A1 | (x << 8);
         }
         loge("Invalid arguments for sknp");
         return 0xFFFF;
     }          },
    {"db",   [](std::vector<std::string> const& args) -> uint16_t {
         if (args.size() != 1) {
             INVALID_ARGS("db");
         }
         if (is_byte(args[0])) {
             auto b = byte(args[0]);
             return b;
         }
         INVALID_ARGS("db");
     }}
};

} // namespace

std::vector<uint8_t> assemble(std::string const& source)
{
    char const*          it = source.c_str();
    std::vector<uint8_t> bytecode;

    size_t lineNumber = 1;

    while (*it) {

        auto mnemonic = nextToken(it);

        if (mnemonic.empty()) {
            it = nextLine(it);
            ++lineNumber;
            continue;
        }

        std::vector<std::string> args;

        std::string arg;

        do {
            arg = nextToken(it);
            if (!arg.empty())
                args.push_back(arg);
        }
        while (!arg.empty());

        if (mnemonic == "db") {
            for (auto const& a: args) {
                if (!is_byte(a, true)) {
                    loge("Invalid byte value for db at line %d: %s", lineNumber, a.c_str());
                    return {};
                }
                bytecode.push_back(byte(a, true));
            }
            it = nextLine(it);
            ++lineNumber;
            continue;
        }

        if (processors.find(mnemonic) == processors.end()) {
            loge("Unknown instruction at line %d: %s", lineNumber, mnemonic.c_str());
            return {};
        }

        uint16_t instruction = processors[mnemonic](args);

        if (instruction == 0xFFFF) {
            loge("Failed to assemble instruction at line: %d", lineNumber);
            return {};
        }

        bytecode.push_back((instruction >> 8) & 0xFF);
        bytecode.push_back(instruction & 0xFF);

        it = nextLine(it);
        ++lineNumber;
    }

    return bytecode;
}
} // namespace chipate
