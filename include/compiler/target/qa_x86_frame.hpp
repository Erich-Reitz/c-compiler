#pragma once

#include <string>
#include <vector>

#include "qa_x86_instructions.hpp"

namespace target {
struct Frame {
    std::string name;
    std::vector<Instruction> instructions;
    int size = 0;
};
}  // namespace target