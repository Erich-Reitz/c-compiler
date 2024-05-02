#pragma once

#include "qa_x86_instructions.hpp"

#include <string>
#include <vector>

namespace target {
struct Frame {
    std::string name;
    std::vector<Instruction> instructions;
    int size = 0;
};
}