#include "lower_ir.hpp"
#include <map>
#include <stdexcept>
#include <variant>

namespace target {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

[[nodiscard]] std::vector<Instruction> _Value_To_Location(Register r_dst,
        int v_src, Ctx *ctx) {
    return {LoadI{.dst = r_dst, .value = v_src}};
}

[[nodiscard]] std::vector<Instruction> _Value_To_Location(StackLocation l_dest,
        int v_src, Ctx *ctx) {
    return {StoreI{.dst = l_dest, .value = v_src}};
}

[[nodiscard]] std::vector<Instruction>
_Value_To_Location(Register l, std::monostate m, Ctx *ctx) {
    throw std::runtime_error("Cannot convert monostate to location");
}

[[nodiscard]] std::vector<Instruction>
_Value_To_Location(StackLocation l, std::monostate m, Ctx *ctx) {
    throw std::runtime_error("Cannot convert monostate to location");
}

[[nodiscard]] std::vector<Instruction>
_Value_To_Location(StackLocation l_dest, qa_ir::Temp t_src, Ctx *ctx) {
    const auto reg = ctx->AllocateNewForTemp(t_src);
    return {Store{.dst = l_dest, .src = reg}};
}

[[nodiscard]] std::vector<Instruction>
_Value_To_Location(Register r, qa_ir::Temp t, Ctx *ctx) {
    const auto reg = ctx->AllocateNewForTemp(t);
    return {Mov{.dst = r, .src = reg}};
}

[[nodiscard]] std::vector<Instruction>
_Value_To_Location(Register r, qa_ir::HardcodedRegister t, Ctx *ctx) {
    throw std::runtime_error(
        "Cannot convert hardcoded register to register"); // just for now
}

[[nodiscard]] std::vector<Instruction>
_Value_To_Location(Register r_dst, qa_ir::Variable v_src, Ctx *ctx) {
    const auto variableOffset = ctx->variable_offset.at(v_src.name);
    return {Load{.dst = r_dst, .src = StackLocation{.offset = variableOffset}}};
}

[[nodiscard]] std::vector<Instruction>
_Value_To_Location(StackLocation s_dst, qa_ir::HardcodedRegister r_src,
                   Ctx *ctx) {
    const auto reg = HardcodedRegister{.reg = r_src.reg, .size = r_src.size};
    return {Store{.dst = s_dst, .src = reg}};
}

[[nodiscard]] std::vector<Instruction>
_Value_To_Location(StackLocation s_dst, qa_ir::Variable v_src, Ctx *ctx) {
    const auto reg = ctx->NewRegister(v_src.size);
    auto result = std::vector<Instruction> {};
    // move variable to register
    const auto variableOffset = ctx->variable_offset.at(v_src.name);
    result.push_back(
              Load{.dst = reg, .src = StackLocation{.offset = variableOffset}});
    // store register to stack
    result.push_back(Store{.dst = s_dst, .src = reg});
    return result;
}

[[nodiscard]] std::vector<Instruction>
Register_To_Location(Location l, target::Register reg, Ctx *ctx) {
    if (auto stackLocation = std::get_if<StackLocation>(&l)) {
        return {Store{.dst = *stackLocation, .src = reg}};
    }
    if (auto dest_register = std::get_if<Register>(&l)) {
        return {Mov{.dst = *dest_register, .src = reg}};
    }
    throw std::runtime_error("Cannot convert register to location");
}

#pragma clang diagnostic pop

Location Ctx::AllocateNew(qa_ir::Value v) {
    if (auto tmp = std::get_if<qa_ir::Temp>(&v)) {
        return AllocateNewForTemp(*tmp);
    }
    if (auto variable = std::get_if<qa_ir::Variable>(&v)) {
        const auto variableName = variable->name;
        if (auto it = variable_offset.find(variableName);
                it == variable_offset.end()) {
            stackOffset += variable->size;
            variable_offset[variableName] = stackOffset;
        }
        return StackLocation{.offset = variable_offset.at(variableName)};
    }
    if (auto hardcoded = std::get_if<qa_ir::HardcodedRegister>(&v)) {
        return HardcodedRegister{.reg = hardcoded->reg, .size = hardcoded->size};
    }
    throw std::runtime_error("Cannot allocate new location for value");
}

Register Ctx::AllocateNewForTemp(qa_ir::Temp t) {
    if (auto it = temp_register_mapping.find(t.id);
            it != temp_register_mapping.end()) {
        return it->second;
    }
    const auto reg = VirtualRegister{.id = tempCounter++, .size = t.size};
    temp_register_mapping[t.id] = reg;
    return reg;
}

VirtualRegister Ctx::NewRegister(int size) {
    return VirtualRegister{.id = tempCounter++, .size = size};
}

int Ctx::get_stack_offset() const {
    return stackOffset;
}

std::vector<Instruction> Ctx::toLocation(Location l, qa_ir::Value v) {
    return std::visit(
    [this](auto &&arg1, auto &&arg2) {
        return _Value_To_Location(arg1, arg2, this);
    },
    l, v);
}

[[nodiscard]] std::vector<Instruction> LowerMoveInstruction(qa_ir::Mov move,
        Ctx &ctx) {
    auto dest = move.dst;
    auto src = move.src;
    auto destLocation = ctx.AllocateNew(dest);
    return ctx.toLocation(destLocation, src);
}

[[nodiscard]] std::vector<Instruction> LowerReturnInstruction(qa_ir::Ret ret,
        Ctx &ctx) {
    const auto returnValue = ret.value;
    const auto returnValueSize = qa_ir::SizeOf(returnValue);
    const auto returnRegister = HardcodedRegister{.reg = target::BaseRegister::AX,
                                                  .size = returnValueSize};
    auto result = ctx.toLocation(returnRegister, returnValue);
    auto jumpInstruction = Jump{.label = "end"};
    result.push_back(jumpInstruction);
    return result;
}

[[nodiscard]] Instruction __Create_Arth_Instruction(ast::BinOpKind kind,
        VirtualRegister left_reg,
        int value) {
    if (kind == ast::BinOpKind::Add) {
        return {AddI{.dst = left_reg, .value = value}};
    }
    if (kind == ast::BinOpKind::Sub) {
        return {SubI{.dst = left_reg, .value = value}};
    }
    if (kind == ast::BinOpKind::Eq) {
        return {CmpI{.dst = left_reg, .value = value}};
    }
    if (kind == ast::BinOpKind::Gt) {
        return {CmpI{.dst = left_reg, .value = value}};
    }
    throw std::runtime_error("CreateArthInstruction not implemented");
}

[[nodiscard]] Instruction __Create_Arth_Instruction(ast::BinOpKind kind,
        VirtualRegister left_reg,
        VirtualRegister right_reg) {
    if (kind == ast::BinOpKind::Add) {
        return {Add{.dst = left_reg, .src = right_reg}};
    }
    if (kind == ast::BinOpKind::Sub) {
        return {Sub{.dst = left_reg, .src = right_reg}};
    }
    if (kind == ast::BinOpKind::Eq) {
        return {Cmp{.dst = left_reg, .src = right_reg}};
    }
    if (kind == ast::BinOpKind::Gt) {
        return {Cmp{.dst = left_reg, .src = right_reg}};
    }
    throw std::runtime_error("CreateArthInstruction not implemented");
}

std::vector<Instruction> appendInstructionsForMoveToDest(target::Location dst, std::vector<Instruction> &result,
        target::VirtualRegister value_reg,     Ctx &ctx) {
    const auto move_dest_instructions = Register_To_Location(dst, value_reg, &ctx);
    result.insert(result.end(), move_dest_instructions.begin(),
                  move_dest_instructions.end());
    return result;
}

std::vector<Instruction> finishEqInstructions(std::optional<target::Location> dst, std::vector<Instruction> &result,
        Ctx &ctx) {
    auto newReg = ctx.NewRegister(4);
    result.push_back(SetEAl{.dst = newReg});
    if (!dst.has_value()) {
        return result;
    }
    return appendInstructionsForMoveToDest(dst.value(), result, newReg, ctx);
}

std::vector<Instruction> finishGtInstructions(std::optional<target::Location> dst, std::vector<Instruction> &result,
        Ctx &ctx) {
    auto newReg = ctx.NewRegister(4);
    result.push_back(SetGAl{.dst = newReg});
    if (!dst.has_value()) {
        return result;
    }
    return appendInstructionsForMoveToDest(dst.value(), result, newReg, ctx);
}

[[nodiscard]] std::vector<Instruction>
InstructionForArth(ast::BinOpKind kind, std::optional<target::Location> dst,
                   qa_ir::Value left, int value, Ctx &ctx) {
    const auto leftreg = ctx.NewRegister(SizeOf(left));
    std::vector<Instruction> result = ctx.toLocation(leftreg, left);
    auto arth_instruction = __Create_Arth_Instruction(kind, leftreg, value);
    result.push_back(arth_instruction);
    if (kind == ast::BinOpKind::Eq) {
        return finishEqInstructions(dst, result, ctx);
    }
    if (kind == ast::BinOpKind::Gt) {
        return finishGtInstructions(dst, result, ctx);
    }
    return appendInstructionsForMoveToDest(dst.value(), result, leftreg, ctx);
}

[[nodiscard]] std::vector<Instruction>
InstructionForArth(ast::BinOpKind kind, std::optional<target::Location> dst,
                   qa_ir::Value left, qa_ir::Temp right, Ctx &ctx) {
    const auto leftreg = ctx.NewRegister(SizeOf(left));
    std::vector<Instruction> result = ctx.toLocation(leftreg, left);
    const auto rightreg = ctx.NewRegister(SizeOf(right));
    const auto rightRegister = ctx.toLocation(rightreg, right);
    result.insert(result.end(), rightRegister.begin(), rightRegister.end());
    const auto arth_instruction =
        __Create_Arth_Instruction(kind, leftreg, rightreg);
    result.push_back(arth_instruction);
    if (kind == ast::BinOpKind::Eq) {
        return finishEqInstructions(dst, result, ctx);
    }
    if (kind == ast::BinOpKind::Gt) {
        return finishGtInstructions(dst, result, ctx);
    }
    return appendInstructionsForMoveToDest(dst.value(), result, leftreg, ctx);
}

[[nodiscard]] std::vector<Instruction>
InstructionForArth(ast::BinOpKind kind, std::optional<target::Location> dst,
                   qa_ir::Value left, qa_ir::Variable right, Ctx &ctx) {
    const auto leftreg = ctx.NewRegister(SizeOf(left));
    std::vector<Instruction> result = ctx.toLocation(leftreg, left);
    const auto rightreg = ctx.NewRegister(SizeOf(right));
    const auto rightRegister = ctx.toLocation(rightreg, right);
    result.insert(result.end(), rightRegister.begin(), rightRegister.end());
    const auto arth_instruction =
        __Create_Arth_Instruction(kind, leftreg, rightreg);
    result.push_back(arth_instruction);
    if (kind == ast::BinOpKind::Eq) {
        return finishEqInstructions(dst, result, ctx);
    }
    if (kind == ast::BinOpKind::Gt) {
        return finishGtInstructions(dst, result, ctx);
    }
    return appendInstructionsForMoveToDest(dst.value(), result, leftreg, ctx);
}
/**
 *
 * This functions 'switches' on the type (const/temp/var) of the
 * right hand side of the binary operation
 * If you think about emitting better code, you switch on both the left and right hand side.
 * Instead of nesting, this calls overloaded functions that just pull
 * the lhs to a register and then emit the instructions to perform the operation
 * and move to the destination
 *
*/
[[nodiscard]] std::vector<Instruction>
LowerArth(ast::BinOpKind kind, std::optional<qa_ir::Value> dst,
          qa_ir::Value left, qa_ir::Value right, Ctx &ctx) {
    std::optional<target::Location> dest_location = std::nullopt;
    if (dst.has_value()) {
        dest_location = ctx.AllocateNew(dst.value());
    }
    if (std::holds_alternative<int>(right)) {
        const int value = std::get<int>(right);
        return InstructionForArth(kind, dest_location, left, value, ctx);
    }
    if (std::holds_alternative<qa_ir::Temp>(right)) {
        const auto r = std::get<qa_ir::Temp>(right);
        return InstructionForArth(kind, dest_location, left, r, ctx);
    }
    if (std::holds_alternative<qa_ir::Variable>(right)) {
        const auto variable = std::get<qa_ir::Variable>(right);
        return InstructionForArth(kind, dest_location, left, variable, ctx);
    }
    throw std::runtime_error("LowerAddInstruction not implemented");
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

[[nodiscard]] std::vector<Instruction>
LowerLabelDefInstruction(qa_ir::LabelDef label, Ctx &ctx) {
    return {Label{.name = label.label.name}};
}

[[nodiscard]] std::vector<Instruction>
LowerConditionalJumpInstruction(qa_ir::ConditionalJumpEqual cj, Ctx &ctx) {
    std::vector<Instruction> result;
    result.push_back(JumpEq{.label = cj.trueLabel.name});
    result.push_back(Jump{.label = cj.falseLabel.name});
    return result;
}

[[nodiscard]] std::vector<Instruction>
LowerConditionalJumpGreaterInstruction(qa_ir::ConditionalJumpGreater cj, Ctx &ctx) {
    std::vector<Instruction> result;
    result.push_back(JumpGreater{.label = cj.trueLabel.name});
    result.push_back(Jump{.label = cj.falseLabel.name});
    return result;
}

#pragma clang diagnostic pop

[[nodiscard]] std::vector<Instruction> LowerCallInstruction(qa_ir::Call call,
        Ctx &ctx) {
    auto dest = ctx.AllocateNew(call.dst);
    std::vector<Instruction> result;
    for (int i = call.args.size() - 1; i >= 0 ; i--) {
        if (i >= 6) {
            const qa_ir::Value arg = call.args[i];
            if (std::holds_alternative<int>(arg)) {
                PushI push = PushI{.src = std::get<int>(arg)};
                result.push_back(push);
                continue;
            }
            if (std::holds_alternative<qa_ir::Variable>(arg)) {
                const auto reg = ctx.NewRegister(SizeOf(arg));
                const auto variable = std::get<qa_ir::Variable>(arg);
                const auto variableOffset = ctx.variable_offset.at(variable.name);
                result.push_back(
                          Load{.dst = reg, .src = StackLocation{.offset = variableOffset}});
                result.push_back(Push{.src = reg});
                continue;
            }
            throw std::runtime_error("can't handle non hardcoded int for >= 6");
        }
        const auto arg = call.args[i];
        const auto argbase = target::param_regs[i];
        const auto argsize = SizeOf(arg);
        const auto argreg =
            target::HardcodedRegister{.reg = argbase, .size = argsize};
        auto argToParamRegInstructions = ctx.toLocation(argreg, arg);
        result.insert(result.end(), argToParamRegInstructions.begin(),
                      argToParamRegInstructions.end());
    }
    const auto returnValueSize = SizeOf(call.dst);
    const auto returnRegister = HardcodedRegister{.reg = target::BaseRegister::AX,
                                                  .size = returnValueSize};
    result.push_back(Call{.name = call.name, .dst = returnRegister});
    const auto move_dest_instructions =
        Register_To_Location(dest, returnRegister, &ctx);
    result.insert(result.end(), move_dest_instructions.begin(),
                  move_dest_instructions.end());
    return result;
}

[[nodiscard]] std::vector<Instruction> LowerMovRInstruction(qa_ir::MovR move,
        Ctx &ctx) {
    const auto dst = ctx.AllocateNew(move.dst);
    return ctx.toLocation(dst, move.src);
}

[[nodiscard]] std::vector<Instruction> LowerAddrInstruction(qa_ir::Addr addr,
        Ctx &ctx) {
    std::vector<Instruction> result;
    const auto temp = std::get<qa_ir::Temp>(addr.dst);
    const auto variable = std::get<qa_ir::Variable>(addr.src);
    const auto variableOffset = ctx.variable_offset.at(variable.name);
    const auto reg = ctx.AllocateNewForTemp(temp);
    result.push_back(
              Lea{.dst = reg, .src = StackLocation{.offset = variableOffset}});
    return result;
}

[[nodiscard]] std::vector<Instruction> LowerDerefInstruction(qa_ir::Deref deref,
        Ctx &ctx) {
    std::vector<Instruction> result;
    const auto temp = std::get<qa_ir::Temp>(deref.dst);
    const auto variable = std::get<qa_ir::Variable>(deref.src);
    const auto variableOffset = ctx.variable_offset.at(variable.name);
    const auto depth = deref.depth;
    auto reg = ctx.NewRegister(8);
    result.push_back(
              Load{.dst = reg, .src = StackLocation{.offset = variableOffset}});
    for (int i = 1; i < depth; i++) {
        const auto tempreg = ctx.NewRegister(8);
        result.push_back(IndirectLoad{.dst = tempreg, .src = reg});
        reg = tempreg;
    }
    // indirect mem access
    const auto finalDest = ctx.AllocateNewForTemp(temp);
    result.push_back(IndirectLoad{.dst = finalDest, .src = reg});
    return result;
}

[[nodiscard]] std::vector<Instruction>
LowerDerefStoreInstruction(qa_ir::DerefStore deref, Ctx &ctx) {
    std::vector<Instruction> result;
    // variable_dest holds the address of the variable
    const auto variable_dest = deref.dst;
    // move the variable to a register
    const auto tempregister = ctx.NewRegister(8);
    auto moveInstructions = ctx.toLocation(tempregister, variable_dest);
    result.insert(result.end(), moveInstructions.begin(), moveInstructions.end());
    // load the value at the address
    const auto src = deref.src;
    const auto srcSize = SizeOf(src);
    const auto srcReg = ctx.NewRegister(srcSize);
    auto srcInstructions = ctx.toLocation(srcReg, src);
    result.insert(result.end(), srcInstructions.begin(), srcInstructions.end());
    // store the value at the address using indirect store instructions
    result.push_back(IndirectStore{.dst = tempregister, .src = srcReg});
    return result;
}

[[nodiscard]] std::vector<Instruction>
LowerInstruction(const qa_ir::Operation &op, Ctx &ctx) {
    return std::visit(
    [&](auto &&arg) -> std::vector<Instruction> {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, qa_ir::Mov>) {
            return LowerMoveInstruction(arg, ctx);
        } else if constexpr (std::is_same_v<T, qa_ir::Ret>) {
            return LowerReturnInstruction(arg, ctx);
        } else if constexpr (std::is_same_v<T, qa_ir::Add>) {
            return LowerArth(ast::BinOpKind::Add, arg.dst, arg.left, arg.right,
                             ctx);
        } else if constexpr (std::is_same_v<T, qa_ir::Sub>) {
            return LowerArth(ast::BinOpKind::Sub, arg.dst, arg.left, arg.right,
                             ctx);
        } else if constexpr (std::is_same_v<T, qa_ir::Equal>) {
            return LowerArth(ast::BinOpKind::Eq, arg.dst, arg.left, arg.right,
                             ctx);
        } else if constexpr (std::is_same_v<T, qa_ir::LabelDef>) {
            return LowerLabelDefInstruction(arg, ctx);
        } else if constexpr (std::is_same_v<T, qa_ir::ConditionalJumpEqual>) {
            return LowerConditionalJumpInstruction(arg, ctx);
        } else if constexpr (std::is_same_v<T, qa_ir::ConditionalJumpGreater>) {
            return LowerConditionalJumpGreaterInstruction(arg, ctx);
        } else if constexpr (std::is_same_v<T, qa_ir::Compare>) {
            return LowerArth(ast::BinOpKind::Eq, std::nullopt, arg.left,
                             arg.right, ctx);
        } else if constexpr (std::is_same_v<T, qa_ir::Call>) {
            return LowerCallInstruction(arg, ctx);
        } else if constexpr (std::is_same_v<T, qa_ir::MovR>) {
            return LowerMovRInstruction(arg, ctx);
        } else if constexpr (std::is_same_v<T, qa_ir::Addr>) {
            return LowerAddrInstruction(arg, ctx);
        } else if constexpr (std::is_same_v<T, qa_ir::Deref>) {
            return LowerDerefInstruction(arg, ctx);
        } else if constexpr (std::is_same_v<T, qa_ir::DerefStore>) {
            return LowerDerefStoreInstruction(arg, ctx);
        } else if constexpr (std::is_same_v<T, qa_ir::GreaterThan>) {
            return LowerArth(ast::BinOpKind::Gt, arg.dst, arg.left, arg.right,
                             ctx);
        } else if constexpr (std::is_same_v<T, qa_ir::DefineStackPushed>) {
            const auto size = arg.size;
            const auto name = arg.name;
            ctx.variable_offset[name] = - ctx.stackPassedParameterOffset;
            // 8 not size because (push) is 8 bytes
            ctx.stackPassedParameterOffset += 8;
            return {};
        }

        else {
            throw std::runtime_error(
                "LowerInstruction not implemented for type: " +
                std::to_string(static_cast<int>(op.index())));
        }
    },
    op);
}

[[nodiscard]] std::vector<Frame>
LowerIR(const std::vector<qa_ir::Frame> &frames) {
    std::vector<Frame> result;
    for (const auto &f : frames) {
        std::vector<Instruction> instructions;
        Ctx ctx = Ctx{};
        for (const auto &op : f.instructions) {
            auto ins = LowerInstruction(op, ctx);
            if (ins.empty()) {
                continue;
            }
            instructions.insert(instructions.end(), ins.begin(), ins.end());
        }
        auto new_frame = Frame{f.name, instructions, ctx.get_stack_offset()};
        result.push_back(new_frame);
    }
    return result;
}
} // namespace target