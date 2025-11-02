#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace chipate {
std::vector<uint8_t> assemble(std::string const& source);
}
