#include <concepts>
#include <functional>
#include <map>
#include <optional>
#include <stdexcept>

#include <variant>

#include "../../../include/ast/ast.hpp"
#include "../../../include/compiler/target/lower_ir.hpp"

namespace target {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

bool l_value_ctx = false;
[[nodiscard]] std::vector<Instruction> _Value_To_Location(Register r_dst, qa_ir::ConstInt v_src,
                                                          Ctx* ctx) {
    return {LoadI{.dst = r_dst, .value = v_src.numerical_value}};
}

[[nodiscard]] std::vector<Instruction> _Value_To_Location(StackLocation l_dest,
                                                          qa_ir::ConstInt v_src, Ctx* ctx) {
    return {StoreI{.dst = l_dest, .value = v_src.numerical_value}};
}

[[nodiscard]] std::vector<Instruction> _Value_To_Location(StackLocation l_dest, qa_ir::Temp t_src,
                                                          Ctx* ctx) {
    const auto reg = ctx->AllocateNewForTemp(t_src);
    return {Store{.dst = l_dest, .src = reg}};
}

[[nodiscard]] std::vector<Instruction> _Value_To_Location(Register r, qa_ir::Temp t, Ctx* ctx) {
    const auto reg = ctx->AllocateNewForTemp(t);
    return {Mov{.dst = r, .src = reg}};
}

[[nodiscard]] std::vector<Instruction> _Value_To_Location(Register r, target::HardcodedRegister t,
                                                          Ctx* ctx) {
    throw std::runtime_error("Cannot convert hardcoded register to register");  // just for now
}

[[nodiscard]] std::vector<Instruction> _Value_To_Location(Register r_dst, qa_ir::Variable v_src,
                                                          Ctx* ctx) {
    std::vector<Instruction> result;
    if (v_src.type.is_array && !v_src.offset ) {
        const auto lea = Lea{.dst = r_dst, .src = ctx->get_stack_location(v_src, result)};
        result.push_back(lea);
        return result;
    }
    
    if (v_src.type.is_pointer && v_src.offset ) {
        const auto lea = Load{.dst = r_dst, .src = ctx->get_stack_location(v_src, result)};
        result.push_back(lea);
        return result;
    }

    if (v_src.type.is_pointer && !v_src.offset && !l_value_ctx ) {
        const auto lea = Lea{.dst = r_dst, .src = ctx->get_stack_location(v_src, result)};
        result.push_back(lea);
        return result;
    }
    const auto variableOffset = ctx->get_stack_location(v_src, result);
    result.emplace_back(Load{.dst = r_dst, .src = variableOffset});
    return result;
}

[[nodiscard]] std::vector<Instruction> _Value_To_Location(StackLocation s_dst,
                                                          target::HardcodedRegister r_src,
                                                          Ctx* ctx) {
    return {Store{.dst = s_dst, .src = r_src}};
}

[[nodiscard]] std::vector<Instruction> _Value_To_Location(StackLocation s_dst,
                                                          qa_ir::Variable v_src, Ctx* ctx) {
    const auto sizeOfSource = SizeOf(v_src);
    const auto reg = ctx->NewRegister(sizeOfSource);
    auto result = std::vector<Instruction>{};
    // move variable to register
    const auto base_variable = ctx->get_stack_location(v_src, result);
    result.push_back(Load{.dst = reg, .src = base_variable});
    // store register to stack
    result.push_back(Store{.dst = s_dst, .src = reg});
    return result;
}

[[nodiscard]] auto Register_To_Location(Location l, target::Register reg, Ctx* ctx) -> Instruction {
    if (auto stackLocation = std::get_if<StackLocation>(&l)) {
        return Store{.dst = *stackLocation, .src = reg};
    }
    if (auto dest_register = std::get_if<Register>(&l)) {
        return Mov{.dst = *dest_register, .src = reg};
    }
    throw std::runtime_error("Cannot convert register to location");
}

[[nodiscard]] auto Register_To_Location(Location l, target::Register reg, Ctx& ctx) -> Instruction {
    if (auto stackLocation = std::get_if<StackLocation>(&l)) {
        return Store{.dst = *stackLocation, .src = reg};
    }
    if (auto dest_register = std::get_if<Register>(&l)) {
        return Mov{.dst = *dest_register, .src = reg};
    }
    throw std::runtime_error("Cannot convert register to location");
}

Location Ctx::AllocateNew(qa_ir::Value v, std::vector<Instruction> &instructions) {
    if (auto tmp = std::get_if<qa_ir::Temp>(&v)) {
        return AllocateNewForTemp(*tmp);
    }
    if (auto variable = std::get_if<qa_ir::Variable>(&v)) {
        const auto variableName = variable->name;
        if (auto it = variable_offset.find(variableName); it == variable_offset.end()) {
            const auto variableSize = variable->type.size;
            const auto stackOffsetAfterAdd = stackOffset + variableSize;
            /** based off looking at what GCC emits */
            if (stackOffsetAfterAdd > 16) {
                stackOffset = sixteenByteAlign(stackOffsetAfterAdd);
            } else {
                stackOffset = stackOffsetAfterAdd;
            }
            variable_offset[variableName] = StackLocation{.offset = stackOffset, .is_computed = false, .src = {}, .scale = 0};
        }
        return get_stack_location(*variable, instructions);
    }
    if (auto hardcoded = std::get_if<target::HardcodedRegister>(&v)) {
        return *hardcoded;
    }
    throw std::runtime_error("Cannot allocate new location for value");
}

StackLocation Ctx::get_stack_location(const qa_ir::Variable& v, std::vector<Instruction>& instructions) {
    if (v.type.is_array && v.offset) {
            auto base = variable_offset.at(v.name);
            if (std::holds_alternative<qa_ir::ConstInt>(*v.offset)) {
                const auto offset = std::get<qa_ir::ConstInt>(*v.offset).numerical_value  * v.type.points_to_size;  
                return StackLocation{.offset = base.offset - offset, .is_computed = false, .src = {}, .scale = 0};
            }
            if (std::holds_alternative<qa_ir::Variable>(*v.offset)) {
                const auto offsetVariable = std::get<qa_ir::Variable>(*v.offset);
                const auto offsetVariableOffset = get_stack_location(offsetVariable, instructions);
                const auto reg = NewRegister(SizeOf(offsetVariable));
                    
                instructions.push_back(Load{.dst = reg, .src = offsetVariableOffset});
                const auto upperreg = NewRegister(8);
                instructions.push_back(ZeroExtend{.dst = upperreg, .src = reg});
                return StackLocation{.offset = base.offset, .is_computed = true, .src = upperreg, .scale = v.type.points_to_size};
            }

            throw std::runtime_error("Unsupported offset type");
    }
    
    if (v.type.is_pointer && v.offset) {
        auto base = variable_offset.at(v.name);
        if (std::holds_alternative<qa_ir::ConstInt>(*v.offset)) {
            const auto offset = std::get<qa_ir::ConstInt>(*v.offset).numerical_value  * v.type.points_to_size;  
            return StackLocation{.offset = base.offset - offset, .is_computed = false, .src = {}, .scale = 0};
        }
        if (std::holds_alternative<qa_ir::Variable>(*v.offset)) {
            const auto offsetVariable = std::get<qa_ir::Variable>(*v.offset);
            const auto offsetVariableOffset = get_stack_location(offsetVariable, instructions);
            const auto reg = NewRegister(SizeOf(offsetVariable));
                
            instructions.push_back(Load{.dst = reg, .src = offsetVariableOffset});
            const auto upperreg = NewRegister(8);
            instructions.push_back(ZeroExtend{.dst = upperreg, .src = reg});

            // issue a lea
            const auto base_offset = 0; // why: ?
            const auto offset_reg = NewRegister(8);
            const auto offset_of_element = StackLocation{.offset = base_offset, .is_computed = true, .src = upperreg, .scale = v.type.points_to_size};
            instructions.push_back(Lea{.dst = offset_reg, .src = offset_of_element});

            // move the address pointed to
            const auto final_reg = NewRegister(8);
            instructions.push_back(Load{.dst = final_reg, .src = base});
            instructions.push_back(Add{.dst = final_reg, .src = offset_reg});

            return StackLocation{.offset = 0, .is_computed = true, .src = final_reg, .scale = 1};
        }

        if (std::holds_alternative<qa_ir::Temp>(*v.offset)) {
            const auto offsetVariable = std::get<qa_ir::Temp>(*v.offset);
            const auto offsetVariableReg = AllocateNewForTemp(offsetVariable);
            const auto upperreg = NewRegister(8);
            instructions.push_back(ZeroExtend{.dst = upperreg, .src = offsetVariableReg});

            // issue a lea
            const auto base_offset = 0; // why: ?
            const auto offset_reg = NewRegister(8);
            const auto offset_of_element = StackLocation{.offset = base_offset, .is_computed = true, .src = upperreg, .scale = v.type.points_to_size};
            instructions.push_back(Lea{.dst = offset_reg, .src = offset_of_element});

            // move the address pointed to
            const auto final_reg = NewRegister(8);
            instructions.push_back(Load{.dst = final_reg, .src = base});
            instructions.push_back(Add{.dst = final_reg, .src = offset_reg});

            return StackLocation{.offset = 0, .is_computed = true, .src = final_reg, .scale = 1};
        }


        throw std::runtime_error("Unsupported offset type");
    }

    return variable_offset.at(v.name);
}

Register Ctx::AllocateNewForTemp(qa_ir::Temp t) {
    if (auto it = temp_register_mapping.find(t.id); it != temp_register_mapping.end()) {
        return it->second;
    }
    const auto reg = VirtualRegister{.id = tempCounter++, .size = t.size};
    temp_register_mapping[t.id] = reg;
    return reg;
}

VirtualRegister Ctx::NewRegister(int size) {
    if (size == 8) {
        return VirtualRegister{.id = tempCounter++, .size = size};
    }
    if (size == 4) {
        return VirtualRegister{.id = tempCounter++, .size = size};
    }

    throw std::runtime_error("Unsupported register size");
}

int Ctx::get_stack_offset() const { return stackOffset; }

void Ctx::define_stack_pushed_variable(const std::string& name) {
    variable_offset[name] = StackLocation{.offset = -stackPassedParameterOffset, .is_computed = false, .src = {}, .scale = 0};
    stackPassedParameterOffset += 8;
}

std::vector<Instruction> Ctx::toLocation(Location l, qa_ir::Value v) {
    return std::visit(
        [this](auto&& arg1, auto&& arg2) { return _Value_To_Location(arg1, arg2, this); }, l, v);
}

[[nodiscard]] std::vector<Instruction> LowerInstruction(qa_ir::Mov move, Ctx& ctx) {
    std::vector<Instruction> result;
    auto dest = move.dst;
    auto src = move.src;
    auto destLocation = ctx.AllocateNew(dest, result);
    const auto ins = ctx.toLocation(destLocation, src);
    std::ranges::copy(ins, std::back_inserter(result));
    return result;
}

[[nodiscard]] std::vector<Instruction> LowerInstruction(qa_ir::Ret ret, Ctx& ctx) {
    const auto returnValue = ret.value;
    const auto returnValueSize = qa_ir::SizeOf(returnValue);
    const auto returnRegister =
        HardcodedRegister{.reg = target::BaseRegister::AX, .size = returnValueSize};
    auto result = ctx.toLocation(returnRegister, returnValue);
    auto jumpInstruction = Jump{.label = "end"};
    result.push_back(jumpInstruction);
    return result;
}

std::vector<Instruction> Create_ArthBin_Instruction_Sequence(ast::BinOpKind kind,
                                                             std::optional<target::Location> dst,
                                                             Register reg, qa_ir::ConstInt value,
                                                             Ctx& ctx) {
    static const std::map<ast::BinOpKind, std::function<Instruction(Register, int)>> ops = {
        {ast::BinOpKind::Add,
         [](Register v_reg, int v_value) -> Instruction { return AddI{.dst = v_reg, .value = v_value}; }},
        {ast::BinOpKind::Sub,
         [](Register v_reg, int v_value) -> Instruction { return SubI{.dst = v_reg, .value = v_value}; }}};
    auto op_it = ops.find(kind);
    if (op_it == ops.end()) {
        throw std::runtime_error("Unsupported operation kind");
    }
    std::vector<Instruction> result = {op_it->second(reg, value.numerical_value)};
    if (dst.has_value()) {
        result.push_back(Register_To_Location(dst.value(), reg, ctx));
    }
    return result;
}

std::vector<Instruction> Create_ArthBin_Instruction_Sequence(ast::BinOpKind kind,
                                                             std::optional<target::Location> dst,
                                                             Register result_reg, Register src_reg,
                                                             Ctx& ctx) {
    static const std::map<ast::BinOpKind, std::function<Instruction(Register, Register)>> ops = {
        {ast::BinOpKind::Add,
         [](Register v_dst, Register v_src) -> Instruction { return Add{.dst = v_dst, .src = v_src}; }},
        {ast::BinOpKind::Sub,
         [](Register v_dst, Register v_src) -> Instruction { return Sub{.dst = v_dst, .src = v_src}; }}};
    auto op_it = ops.find(kind);
    if (op_it == ops.end()) {
        throw std::runtime_error("Unsupported operation kind");
    }
    std::vector<Instruction> result = {op_it->second(result_reg, src_reg)};
    if (dst.has_value()) {
        result.push_back(Register_To_Location(dst.value(), result_reg, ctx));
    }
    return result;
}

static const std::map<ast::BinOpKind, std::function<void(std::vector<Instruction>&, Register)>>
    comparision_ops = {
        {ast::BinOpKind::Eq, [](std::vector<Instruction>& result, Register newReg) { result.push_back(SetEAl{.dst = newReg}); }},
        {ast::BinOpKind::Gt, [](std::vector<Instruction>& result, Register newReg) { result.push_back(SetGAl{.dst = newReg}); }},
        {ast::BinOpKind::Neq, [](std::vector<Instruction>& result, Register newReg) { result.push_back(SetNeAl{.dst = newReg});}},
        {ast::BinOpKind::Lt, [](std::vector<Instruction>& result, Register newReg) { result.push_back(SetLAl{.dst = newReg}); }},
    };

std::vector<Instruction> Create_Comparison_Instruction_Sequence(ast::BinOpKind kind,
                                                                std::optional<target::Location> dst,
                                                                Register reg, qa_ir::ConstInt value,
                                                                Ctx& ctx) {
    std::vector<Instruction> result = {CmpI{.dst = reg, .value = value.numerical_value}};
    Register newReg = ctx.NewRegister(4);
    auto op_it = comparision_ops.find(kind);
    if (op_it != comparision_ops.end()) {
        op_it->second(result, newReg);
    } else {
        throw std::runtime_error("Unsupported comparison kind");
    }
    if (dst.has_value()) {
        result.push_back(Register_To_Location(dst.value(), newReg, ctx));
    }
    return result;
}

std::vector<Instruction> Create_Comparison_Instruction_Sequence(ast::BinOpKind kind,
                                                                std::optional<target::Location> dst,
                                                                Register reg1, Register reg2,
                                                                Ctx& ctx) {
    std::vector<Instruction> result = {Cmp{.dst = reg1, .src = reg2}};
    Register newReg = ctx.NewRegister(4);
    auto op_it = comparision_ops.find(kind);
    if (op_it != comparision_ops.end()) {
        op_it->second(result, newReg);
    } else {
        throw std::runtime_error("Unsupported comparison kind");
    }
    if (dst.has_value()) {
        result.push_back(Register_To_Location(dst.value(), newReg, ctx));
    }
    return result;
}

template <typename T>
[[nodiscard]] auto Create_Arth_Instruction(ast::BinOpKind kind, std::optional<target::Location> dst,
                                           Register result_reg, T rhs, Ctx& ctx)
    -> std::vector<Instruction> {
    std::vector<Instruction> result;
    if (ast::is_arithmetic(kind)) {
        return Create_ArthBin_Instruction_Sequence(kind, dst, result_reg, rhs, ctx);
    }
    if (ast::is_comparison(kind)) {
        return Create_Comparison_Instruction_Sequence(kind, dst, result_reg, rhs, ctx);
    }
    throw std::runtime_error("Unsupported operation kind");
}

template <typename T>
std::pair<Register, std::vector<Instruction>> ensureRegister(T operand, Ctx& ctx) {
    if constexpr (qa_ir::IsIRLocation<T>) {
        Register reg = ctx.NewRegister(SizeOf(operand));
        std::vector<Instruction> instructions = ctx.toLocation(reg, operand);
        return {reg, instructions};
    } else {
        static_assert(qa_ir::IsRegister<T>, "Operand must be a Register or IRLocation");
        return {operand, {}};
    }
}

template <typename T, typename U>
    requires(qa_ir::IsIRLocation<T> || qa_ir::IsRegister<T>) &&
            (qa_ir::IsIRLocation<U> || qa_ir::IsRegister<U>)
std::vector<Instruction> InstructionForArth(ast::BinOpKind kind,
                                            std::optional<target::Location> dst, T left, U right,
                                            Ctx& ctx) {
    std::vector<Instruction> result;
    auto [result_reg, left_instructions] = ensureRegister(left, ctx);
    std::ranges::copy(left_instructions, std::back_inserter(result));
    auto [right_reg, right_instructions] = ensureRegister(right, ctx);
    std::ranges::copy(right_instructions, std::back_inserter(result));
    const auto arth_instructions = Create_Arth_Instruction(kind, dst, result_reg, right_reg, ctx);
    std::ranges::copy(arth_instructions, std::back_inserter(result));
    return result;
}

template <typename T>
    requires qa_ir::IsRegister<T>
std::vector<Instruction> InstructionForArth(ast::BinOpKind kind,
                                            std::optional<target::Location> dst, T result_reg,
                                            qa_ir::ConstInt value, Ctx& ctx) {
    return Create_Arth_Instruction(kind, dst, result_reg, value, ctx);
}

template <typename LeftType>
    requires qa_ir::IsIRLocation<LeftType>
std::vector<Instruction> InstructionForArth(ast::BinOpKind kind,
                                            std::optional<target::Location> dst, LeftType left,
                                            qa_ir::ConstInt value, Ctx& ctx) {
    auto [result_reg, result] = ensureRegister(left, ctx);
    const auto rest_instructions = Create_Arth_Instruction(kind, dst, result_reg, value, ctx);
    std::ranges::copy(rest_instructions, std::back_inserter(result));
    return result;
}

template <typename RightType>
    requires(qa_ir::IsIRLocation<RightType> || qa_ir::IsRegister<RightType>)
std::vector<Instruction> InstructionForArth(ast::BinOpKind kind,
                                            std::optional<target::Location> dst,
                                            qa_ir::ConstInt value, RightType right, Ctx& ctx) {
    const target::Register result_reg = ctx.NewRegister(4);
    std::vector<Instruction> result = {LoadI{.dst = result_reg, .value = value.numerical_value}};
    auto [rhs_reg, move_to_rhs_instructions] = ensureRegister(right, ctx);
    std::ranges::copy(move_to_rhs_instructions, std::back_inserter(result));
    const auto rest_instructions = Create_Arth_Instruction(kind, dst, result_reg, rhs_reg, ctx);
    std::ranges::copy(rest_instructions, std::back_inserter(result));
    return result;
}

std::vector<Instruction> InstructionForArth(ast::BinOpKind kind,
                                            std::optional<target::Location> dst,
                                            qa_ir::ConstInt left, qa_ir::ConstInt right, Ctx& ctx) {
    auto result_reg = ctx.NewRegister(4);
    auto src_reg = ctx.NewRegister(4);
    std::vector<Instruction> result = {LoadI{.dst = result_reg, .value = left.numerical_value},
                                       LoadI{.dst = src_reg, .value = right.numerical_value}};
    const auto rest_instructions = InstructionForArth(kind, dst, result_reg, src_reg, ctx);
    std::ranges::copy(rest_instructions, std::back_inserter(result));
    return result;
}

[[nodiscard]] std::vector<Instruction> LowerArth(ast::BinOpKind kind, qa_ir::Value dst,
                                                 qa_ir::Value left, qa_ir::Value right, Ctx& ctx) {
    // allocate new will return a location of a variable if it already exists
    std::vector<Instruction> result;
    auto dest_location = ctx.AllocateNew(dst, result);
    auto visitor = [&](auto&& left, auto&& right) -> std::vector<Instruction> {
        return InstructionForArth(kind, dest_location, left, right, ctx);
    };
    const auto op =  std::visit(visitor, left, right);
    std::ranges::copy(op, std::back_inserter(result));
    return result;
}

auto LowerInstruction(qa_ir::LabelDef label, Ctx& ctx) -> std::vector<Instruction> {
    return {Label{.name = label.label.name}};
}

[[nodiscard]] std::vector<Instruction> LowerInstruction(qa_ir::ConditionalJumpEqual cj, Ctx& ctx) {
    std::vector<Instruction> result;
    result.push_back(JumpEq{.label = cj.trueLabel.name});
    result.push_back(Jump{.label = cj.falseLabel.name});
    return result;
}

[[nodiscard]] std::vector<Instruction> LowerInstruction(qa_ir::ConditionalJumpNotEqual cj,
                                                        Ctx& ctx) {
    std::vector<Instruction> result;
    result.push_back(JumpEq{.label = cj.falseLabel.name});
    result.push_back(Jump{.label = cj.trueLabel.name});
    return result;
}

[[nodiscard]] std::vector<Instruction> LowerInstruction(qa_ir::ConditionalJumpGreater cj,
                                                        Ctx& ctx) {
    std::vector<Instruction> result;
    result.push_back(JumpGreater{.label = cj.trueLabel.name});
    result.push_back(Jump{.label = cj.falseLabel.name});
    return result;
}

auto LowerInstruction(qa_ir::ConditionalJumpLess cj, Ctx& ctx) -> std::vector<Instruction> {
    std::vector<Instruction> result;
    result.push_back(JumpLess{.label = cj.trueLabel.name});
    result.push_back(Jump{.label = cj.falseLabel.name});
    return result;
}

[[nodiscard]] std::vector<Instruction> LowerInstruction(qa_ir::Call call, Ctx& ctx) {
    std::vector<Instruction> result;
    auto dest = ctx.AllocateNew(call.dst, result);
    for (auto it = call.args.begin(); it != call.args.end(); ++it) {
        auto dist = std::distance(call.args.begin(), it);
        std::size_t index = static_cast<std::size_t>(dist);
        if (index >= 6) {
            if (std::holds_alternative<qa_ir::ConstInt>(*it)) {
                result.push_back(PushI{.src = std::get<qa_ir::ConstInt>(*it).numerical_value});
                continue;
            }
            if (std::holds_alternative<qa_ir::Variable>(*it)) {
                const auto size = SizeOf(*it);
                const auto reg = ctx.NewRegister(size);
                const auto variable = std::get<qa_ir::Variable>(*it);
                const auto variableOffset = ctx.variable_offset.at(variable.name);
                result.push_back(Load{.dst = reg, .src = variableOffset});
                result.push_back(Push{.src = reg});
                continue;
            }
            throw std::runtime_error("can't handle non-hardcoded int for >= 6");
        }
        const auto argbase = target::param_regs.at(index);
        const auto argsize = SizeOf(*it);
        const auto argreg = target::HardcodedRegister{.reg = argbase, .size = argsize};
        auto argToParamRegInstructions = ctx.toLocation(argreg, *it);
        result.insert(result.end(), argToParamRegInstructions.begin(),
                      argToParamRegInstructions.end());
    }
    const auto returnValueSize = SizeOf(call.dst);
    const auto returnRegister =
        HardcodedRegister{.reg = target::BaseRegister::AX, .size = returnValueSize};
    result.push_back(Call{.name = call.name, .dst = returnRegister});
    const auto move_dest_instructions = Register_To_Location(dest, returnRegister, &ctx);
    result.emplace_back(move_dest_instructions);
    return result;
}

[[nodiscard]] std::vector<Instruction> LowerInstruction(qa_ir::MovR move, Ctx& ctx) {
    std::vector<Instruction> result;
    const auto dst = ctx.AllocateNew(move.dst, result);
    const auto ins =  ctx.toLocation(dst, move.src);
    std::ranges::copy(ins, std::back_inserter(result));
    return result;
}

[[nodiscard]] std::vector<Instruction> LowerInstruction(qa_ir::Addr addr, Ctx& ctx) {
    std::vector<Instruction> result;
    const auto temp = std::get<qa_ir::Temp>(addr.dst);
    const auto variable = std::get<qa_ir::Variable>(addr.src);
    const auto variableOffset = ctx.variable_offset.at(variable.name);
    const auto reg = ctx.AllocateNewForTemp(temp);
    result.push_back(Lea{.dst = reg, .src = variableOffset});
    return result;
}

[[nodiscard]] std::vector<Instruction> LowerInstruction(qa_ir::Deref deref, Ctx& ctx) {
    std::vector<Instruction> result;
    const auto temp = std::get<qa_ir::Temp>(deref.dst);
    const auto variable = std::get<qa_ir::Variable>(deref.src);
    const auto variableOffset = ctx.variable_offset.at(variable.name);
    const auto depth = deref.depth;
    auto reg = ctx.NewRegister(8);
    result.push_back(Load{.dst = reg, .src = variableOffset});
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

[[nodiscard]] auto LowerInstruction(qa_ir::DerefStore deref, Ctx& ctx) {
    std::vector<Instruction> result;
    // variable_dest holds the address of the variable
     auto variable_dest = deref.dst;
    // move the variable to a register
    const auto tempregister = ctx.NewRegister(8);
    l_value_ctx = true;
    auto moveInstructions = ctx.toLocation(tempregister, variable_dest);
    l_value_ctx = false;
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

auto LowerInstruction(qa_ir::Add arg, Ctx& ctx) -> std::vector<Instruction> {
    return LowerArth(ast::BinOpKind::Add, arg.dst, arg.left, arg.right, ctx);
}

auto LowerInstruction(qa_ir::Sub arg, Ctx& ctx) -> std::vector<Instruction> {
    return LowerArth(ast::BinOpKind::Sub, arg.dst, arg.left, arg.right, ctx);
}

auto LowerInstruction(qa_ir::Equal arg, Ctx& ctx) -> std::vector<Instruction> {
    return LowerArth(ast::BinOpKind::Eq, arg.dst, arg.left, arg.right, ctx);
}

auto LowerInstruction(qa_ir::NotEqual arg, Ctx& ctx) -> std::vector<Instruction> {
    return LowerArth(ast::BinOpKind::Neq, arg.dst, arg.left, arg.right, ctx);
}

auto LowerInstruction(qa_ir::LessThan arg, Ctx& ctx) -> std::vector<Instruction> {
    return LowerArth(ast::BinOpKind::Lt, arg.dst, arg.left, arg.right, ctx);
}

auto LowerInstruction(qa_ir::GreaterThan arg, Ctx& ctx) -> std::vector<Instruction> {
    return LowerArth(ast::BinOpKind::Gt, arg.dst, arg.left, arg.right, ctx);
}

auto LowerInstruction(qa_ir::Compare arg, Ctx& ctx) -> std::vector<Instruction> {
    auto visitor = [&](auto&& left, auto&& right) -> std::vector<Instruction> {
        return InstructionForArth(ast::BinOpKind::Eq, std::nullopt, left, right, ctx);
    };
    return std::visit(visitor, arg.left, arg.right);
}

auto LowerInstruction(qa_ir::DefineStackPushed arg, Ctx& ctx) -> std::vector<Instruction> {
    auto name = arg.name;
    ctx.define_stack_pushed_variable(name);
    return {};
}

auto LowerInstruction(qa_ir::Jump arg, Ctx& ctx) -> std::vector<Instruction> {
    return {Jump{.label = arg.label.name}};
}

[[nodiscard]] std::vector<Instruction> GenerateInstructionsForOperation(const qa_ir::Operation& op,
                                                                        Ctx& ctx) {
    return std::visit([&ctx](auto&& arg) { return LowerInstruction(arg, ctx); }, op);
}
#pragma GCC diagnostic pop

[[nodiscard]] std::vector<Frame> LowerIR(const std::vector<qa_ir::Frame>& frames) {
    std::vector<Frame> result;
    for (const auto& f : frames) {
        std::vector<Instruction> instructions;
        Ctx ctx = Ctx{};
        for (const auto& op : f.instructions) {
            auto ins = GenerateInstructionsForOperation(op, ctx);
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
}  // namespace target