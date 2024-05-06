#pragma once

#include <map>
#include <string>

#include "../qa_ir/assem.hpp"
#include "../qa_ir/qa_ir.hpp"
#include "qa_x86_frame.hpp"
#include "qa_x86_instructions.hpp"
#include "qa_x86_locations.hpp"
#include "qa_x86_registers.hpp"

namespace target {
struct Ctx {
   public:
    std::map<std::string, StackLocation> variable_offset = {};
    std::map<int, VirtualRegister> temp_register_mapping = {};

    [[nodiscard]] Location AllocateNew(qa_ir::Value v, std::vector<Instruction>& instructions);
    [[nodiscard]] StackLocation get_stack_location(const qa_ir::Variable& v,
                                                   std::vector<Instruction>& instructions);
    [[nodiscard]] Register AllocateNewForTemp(qa_ir::Temp t);
    [[nodiscard]] VirtualRegister NewIntegerRegister(int size);
    [[nodiscard]] VirtualRegister NewFloatRegister(int size);
    [[nodiscard]] std::vector<Instruction> toLocation(Location l, qa_ir::Value v);
    [[nodiscard]] int get_stack_offset() const;

    void define_stack_pushed_variable(const std::string& name);

   private:
    int tempCounter = 0;
    int stackOffset = 0;
    int stackPassedParameterOffset = 16;
};

[[nodiscard]] auto LowerInstruction(qa_ir::Add add, Ctx& ctx) -> std::vector<Instruction>;
[[nodiscard]] auto LowerInstruction(qa_ir::Sub sub, Ctx& ctx) -> std::vector<Instruction>;
[[nodiscard]] auto LowerInstruction(qa_ir::Call call, Ctx& ctx) -> std::vector<Instruction>;
[[nodiscard]] auto LowerInstruction(qa_ir::Ret ret, Ctx& ctx) -> std::vector<Instruction>;
[[nodiscard]] auto LowerInstruction(qa_ir::ConditionalJumpEqual cj, Ctx& ctx)
    -> std::vector<Instruction>;
[[nodiscard]] auto LowerInstruction(qa_ir::ConditionalJumpNotEqual cj, Ctx& ctx)
    -> std::vector<Instruction>;
[[nodiscard]] auto LowerInstruction(qa_ir::ConditionalJumpGreater cj, Ctx& ctx)
    -> std::vector<Instruction>;
[[nodiscard]] auto LowerInstruction(qa_ir::ConditionalJumpGreater cj, Ctx& ctx)
    -> std::vector<Instruction>;
[[nodiscard]] auto LowerInstruction(qa_ir::ConditionalJumpLess cj, Ctx& ctx)
    -> std::vector<Instruction>;
[[nodiscard]] auto LowerInstruction(qa_ir::LabelDef label, Ctx& ctx) -> std::vector<Instruction>;

[[nodiscard]] std::vector<Frame> LowerIR(const std::vector<qa_ir::Frame>& ops);
}  // namespace target