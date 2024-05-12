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

using ins_list = std::vector<Instruction>;

struct Ctx {
   public:
    std::map<std::string, StackLocation> variable_offset = {};
    std::map<int, VirtualRegister> temp_register_mapping = {};

    [[nodiscard]] Location AllocateNew(qa_ir::Value v, ins_list& instructions);
    [[nodiscard]] StackLocation get_stack_location(const qa_ir::Variable& v,
                                                   ins_list& instructions);
    [[nodiscard]] Register AllocateNewForTemp(qa_ir::Temp t);
    [[nodiscard]] VirtualRegister NewIntegerRegister(int size);
    [[nodiscard]] VirtualRegister NewFloatRegister(int size);
    [[nodiscard]] ins_list toLocation(Location l, qa_ir::Value v);
    [[nodiscard]] ins_list LocationToLocation(Location l, qa_ir::Value v);
    [[nodiscard]] int get_stack_offset() const;

    void define_stack_pushed_variable(const std::string& name);

   private:
    int tempCounter = 0;
    int stackOffset = 0;
    int stackPassedParameterOffset = 16;
};

template <ast::BaseType T, ast::BaseType U>
[[nodiscard]] auto LowerInstruction(qa_ir::Add<T, U> add, Ctx& ctx) -> ins_list;
[[nodiscard]] auto LowerInstruction(qa_ir::Sub sub, Ctx& ctx) -> ins_list;
[[nodiscard]] auto LowerInstruction(qa_ir::Call call, Ctx& ctx) -> ins_list;
[[nodiscard]] auto LowerInstruction(qa_ir::Ret ret, Ctx& ctx) -> ins_list;
[[nodiscard]] auto LowerInstruction(qa_ir::ConditionalJumpEqual cj, Ctx& ctx) -> ins_list;
[[nodiscard]] auto LowerInstruction(qa_ir::ConditionalJumpNotEqual cj, Ctx& ctx) -> ins_list;
[[nodiscard]] auto LowerInstruction(qa_ir::ConditionalJumpGreater cj, Ctx& ctx) -> ins_list;
[[nodiscard]] auto LowerInstruction(qa_ir::ConditionalJumpGreater cj, Ctx& ctx) -> ins_list;
[[nodiscard]] auto LowerInstruction(qa_ir::ConditionalJumpLess cj, Ctx& ctx) -> ins_list;
[[nodiscard]] auto LowerInstruction(qa_ir::LabelDef label, Ctx& ctx) -> ins_list;

[[nodiscard]] std::vector<Frame> LowerIR(const std::vector<qa_ir::Frame>& ops);
}  // namespace target