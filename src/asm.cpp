#include "asm.h"

#include <cctype>

using namespace chipate;

char const* skip_space(char const* it)
{
    while (*it && std::isspace(*it))
        it++;
    if (*it == ';')
        while (*it && *it != '\n')
            it++;
    while (*it && std::isspace(*it))
        it++;
    return it;
}

char const* advance(char const* it)
{
    while (*it && std::isalpha(*it))
        it++;
    return it;
}

std::vector<uint8_t> assemble(std::string const& source)
{
    char const* it = source.c_str();

    while (*it) {
        it = skip_space(it);

        char const* start = it;
        it                = advance(it);
        std::string mnemonic(start, it);

        it    = skip_space(it);
        start = it;
        it    = advance(it);
        std::string arg(start, it);

        it++;
    }
}
