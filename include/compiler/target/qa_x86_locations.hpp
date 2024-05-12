#pragma once

#include <string>

#include "qa_x86_registers.hpp"

namespace target {

struct StackLocation {
    int offset = 0;
    bool is_computed = false;
    Register src = {};
    int scale = 0;
    int offest_from_base = 0;

    bool operator==(const StackLocation& other) const { return offset == other.offset; }
};

using Location = std::variant<Register, StackLocation>;

[[nodiscard]] std::string stack_location_at_asm(StackLocation sl);

}  // namespace target