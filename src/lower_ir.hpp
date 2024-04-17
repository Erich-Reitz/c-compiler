#pragma once

#include <map>
#include <string>

#include "assem.hpp"
#include "operation.hpp"
#include "qa_x86.hpp"

namespace target {
struct Ctx {
  public:
    std::map<std::string, int> variable_offset;
    std::map<int, VirtualRegister> temp_register_mapping;
    Location AllocateNew(qa_ir::Value v);
    Register AllocateNewForTemp(qa_ir::Temp t);
    VirtualRegister NewRegister(int size);
    std::vector<Instruction> toLocation(Location l, qa_ir::Value v);
    [[nodiscard]] int get_stack_offset() const;

    int stackPassedParameterOffset = 16;
  private:
    int tempCounter = 0;
    int stackOffset = 0;

};

[[nodiscard]] std::vector<Frame> LowerIR(const std::vector<qa_ir::Frame> &ops);
} // namespace target