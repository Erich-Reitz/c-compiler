#include "../../../include/compiler/target/lower_ir.hpp"

#include <concepts>
#include <functional>
#include <map>
#include <optional>
#include <stdexcept>
#include <variant>

#include "../../../include/ast/ast.hpp"
#include "../../../include/compiler/target/qa_x86.hpp"

namespace target {
using bt = ast::BaseType;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

bool l_value_ctx = false;

template <typename ImmediateType>
[[nodiscard]] ins_list _Value_To_Location(Register r_dst, qa_ir::Immediate<ImmediateType> v_src,
                                          Ctx* ctx) {
    return {ImmediateLoad<ImmediateType>(r_dst, v_src.numerical_value)};
}

[[nodiscard]] ins_list _Value_To_Location(StackLocation l_dest, qa_ir::Immediate<int> v_src,
                                          Ctx* ctx) {
    return {StoreI(l_dest, v_src.numerical_value)};
}

[[nodiscard]] ins_list _Value_To_Location(StackLocation l_dest, qa_ir::Immediate<float> v_src,
                                          Ctx* ctx) {
    const auto intermediate_reg = ctx->NewFloatRegister(4);
    return {StoreF(l_dest, intermediate_reg, v_src.numerical_value)};
}

[[nodiscard]] ins_list _Value_To_Location(StackLocation l_dest, qa_ir::Temp t_src, Ctx* ctx) {
    const auto reg = ctx->AllocateNewForTemp(t_src);
    return {Store(l_dest, reg)};
}

[[nodiscard]] ins_list _Value_To_Location(Register r, qa_ir::Temp t, Ctx* ctx) {
    const auto reg = ctx->AllocateNewForTemp(t);
    return {Mov(r, reg)};
}

[[nodiscard]] ins_list _Value_To_Location(Register r, target::HardcodedRegister t, Ctx* ctx) {
    throw std::runtime_error("Cannot convert hardcoded register to register");  // just for now
}

[[nodiscard]] ins_list _Value_To_Location(Register r_dst, qa_ir::Variable v_src, Ctx* ctx) {
    ins_list result;
    const auto base = ctx->get_stack_location(v_src, result);
    if (v_src.type.base_type == bt::ARRAY) {
        const auto lea = Lea(r_dst, base);
        result.push_back(lea);
        return result;
    }

    result.emplace_back(Load(r_dst, base));
    return result;
}

[[nodiscard]] ins_list _Value_To_Location(StackLocation s_dst, target::HardcodedRegister r_src,
                                          Ctx* ctx) {
    return {Store(s_dst, r_src)};
}

[[nodiscard]] ins_list _Value_To_Location(StackLocation s_dst, qa_ir::Variable v_src, Ctx* ctx) {
    const auto sizeOfSource = SizeOf(v_src);
    const auto reg = ctx->NewIntegerRegister(sizeOfSource);
    auto result = ins_list{};
    // move variable to register
    const auto base_variable = ctx->get_stack_location(v_src, result);
    result.push_back(Load(reg, base_variable));
    // store register to stack
    result.push_back(Store(s_dst, reg));
    return result;
}

[[nodiscard]] auto Register_To_Location(Location l, target::Register reg, Ctx* ctx) -> Instruction {
    if (auto stackLocation = std::get_if<StackLocation>(&l)) {
        return Store(*stackLocation, reg);
    }
    if (auto dest_register = std::get_if<Register>(&l)) {
        return Mov(*dest_register, reg);
    }
    throw std::runtime_error("Cannot convert register to location");
}

[[nodiscard]] auto Register_To_Location(Location l, target::Register reg, Ctx& ctx) -> Instruction {
    if (auto stackLocation = std::get_if<StackLocation>(&l)) {
        return Store(*stackLocation, reg);
    }
    if (auto dest_register = std::get_if<Register>(&l)) {
        return Mov(*dest_register, reg);
    }
    throw std::runtime_error("Cannot convert register to location");
}

Location Ctx::AllocateNew(qa_ir::Value v, ins_list& instructions) {
    if (auto tmp = std::get_if<qa_ir::Temp>(&v)) {
        return AllocateNewForTemp(*tmp);
    }
    if (auto variable = std::get_if<qa_ir::Variable>(&v)) {
        const auto variableName = variable->name;
        if (auto it = variable_offset.find(variableName); it == variable_offset.end()) {
            const auto variableSize = variable->type.GetSize();
            const auto stackOffsetAfterAdd = stackOffset + variableSize;
            /** based off looking at what GCC emits */
            if (stackOffsetAfterAdd > 16) {
                stackOffset = sixteenByteAlign(stackOffsetAfterAdd);
            } else {
                stackOffset = stackOffsetAfterAdd;
            }
            variable_offset[variableName] =
                StackLocation{.offset = stackOffset, .is_computed = false, .src = {}, .scale = 0};
        }
        return get_stack_location(*variable, instructions);
    }
    if (auto hardcoded = std::get_if<target::HardcodedRegister>(&v)) {
        return *hardcoded;
    }
    throw std::runtime_error("Cannot allocate new location for value");
}

StackLocation Ctx::get_stack_location(const qa_ir::Variable& v, ins_list& instructions) {
    return variable_offset.at(v.name);
}

Register newRegisterForVariable(qa_ir::Variable operand, Ctx& ctx) {
    if (operand.type.is_float()) {
        return ctx.NewFloatRegister(4);
    }
    if (operand.type.is_int()) {
        return ctx.NewIntegerRegister(4);
    }
    if (operand.type.is_int_ptr()) {
        return ctx.NewIntegerRegister(8);
    }
    throw std::runtime_error("Unsupported operand type");
}

Register Ctx::AllocateNewForTemp(qa_ir::Temp t) {
    if (auto it = temp_register_mapping.find(t.id); it != temp_register_mapping.end()) {
        return it->second;
    }
    const auto virtual_reg_kind =
        t.type.is_float() ? VirtualRegisterKind::FLOAT : VirtualRegisterKind::INT;
    const auto reg =
        VirtualRegister{.id = tempCounter++, .size = t.type.GetSize(), .kind = virtual_reg_kind};
    temp_register_mapping[t.id] = reg;
    return reg;
}

VirtualRegister Ctx::NewIntegerRegister(int size) {
    if (size == 8) {
        return VirtualRegister{.id = tempCounter++, .size = size};
    }
    if (size == 4) {
        return VirtualRegister{.id = tempCounter++, .size = size};
    }
    throw std::runtime_error("Unsupported register size");
}

VirtualRegister Ctx::NewFloatRegister(int size) {
    if (size == 8) {
        return VirtualRegister{
            .id = tempCounter++, .size = size, .kind = VirtualRegisterKind::FLOAT};
    }
    if (size == 4) {
        return VirtualRegister{
            .id = tempCounter++, .size = size, .kind = VirtualRegisterKind::FLOAT};
    }

    throw std::runtime_error("Unsupported register size");
}

int Ctx::get_stack_offset() const { return stackOffset; }

void Ctx::define_stack_pushed_variable(const std::string& name) {
    variable_offset[name] = StackLocation{
        .offset = -stackPassedParameterOffset, .is_computed = false, .src = {}, .scale = 0};
    stackPassedParameterOffset += 8;
}

ins_list Ctx::toLocation(Location l, qa_ir::Value v) {
    return std::visit(
        [this](auto&& arg1, auto&& arg2) { return _Value_To_Location(arg1, arg2, this); }, l, v);
}

ins_list Ctx::LocationToLocation(Location l, qa_ir::Value v) {
    if (std::holds_alternative<qa_ir::Variable>(v)) {
        std::vector<Instruction> result;
        auto variable = std::get<qa_ir::Variable>(v);
        const auto stackLocation = get_stack_location(variable, result);
        const auto load_address = Load(std::get<Register>(l), stackLocation);
        result.push_back(load_address);
        return result;
    }
    if (std::holds_alternative<qa_ir::Temp>(v)) {
        std::vector<Instruction> result;
        auto variable = std::get<qa_ir::Temp>(v);
        const auto reg = AllocateNewForTemp(variable);
        const auto load_address = Mov(std::get<Register>(l), reg);
        result.push_back(load_address);
        return result;
    }

    throw std::runtime_error("Unsupported qa_ir::Value type.");
}

[[nodiscard]] ins_list LowerInstruction(qa_ir::Mov move, Ctx& ctx) {
    ins_list result;
    auto dest = move.dst;
    auto src = move.src;
    auto destLocation = ctx.AllocateNew(dest, result);
    const auto ins = ctx.toLocation(destLocation, src);
    std::ranges::copy(ins, std::back_inserter(result));
    return result;
}

[[nodiscard]] ins_list LowerInstruction(qa_ir::Ret ret, Ctx& ctx) {
    const auto returnValue = ret.value;
    const auto returnValueSize = qa_ir::SizeOf(returnValue);
    const auto returnRegister =
        HardcodedRegister{.reg = target::BaseRegister::AX, .size = returnValueSize};
    auto result = ctx.toLocation(returnRegister, returnValue);
    auto jumpInstruction = Jump("end");
    result.push_back(jumpInstruction);
    return result;
}

/** -------------------------------------------------------------- */

Register ensureRegister(qa_ir::Temp operand, Ctx& ctx) { return ctx.AllocateNewForTemp(operand); }

Register ensureRegister(target::Register operand, Ctx& ctx) { return operand; }

auto OperationInstructions(qa_ir::Add<bt::INT, bt::INT> kind, target::Location dst,
                           qa_ir::IsEphemeral auto lhs_temp, qa_ir::IsImmediate auto value,
                           Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto lhs_reg = ensureRegister(lhs_temp, ctx);
    result.push_back(AddI(lhs_reg, value.numerical_value));
    result.push_back(Register_To_Location(dst, lhs_reg, ctx));
    return result;
}

auto OperationInstructions(qa_ir::Add<bt::FLOAT, bt::FLOAT> kind, target::Location dst,
                           qa_ir::IsEphemeral auto lhs_temp, qa_ir::IsImmediate auto value,
                           Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto lhs_reg = ensureRegister(lhs_temp, ctx);
    const auto rhs_reg = ctx.NewFloatRegister(4);
    result.push_back(ImmediateLoad<float>(rhs_reg, value.numerical_value));
    result.push_back(Add(lhs_reg, rhs_reg));
    result.push_back(Register_To_Location(dst, lhs_reg, ctx));
    return result;
}


auto OperationInstructions(qa_ir::Sub kind, target::Location dst, qa_ir::IsEphemeral auto lhs_temp,
                           qa_ir::Immediate<int> value, Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto lhs_reg = ensureRegister(lhs_temp, ctx);
    result.push_back(SubI(lhs_reg, value.numerical_value));
    result.push_back(Register_To_Location(dst, lhs_reg, ctx));
    return result;
}

auto OperationInstructions(qa_ir::Sub kind, target::Location dst, Register lhs_reg,
                           qa_ir::Immediate<float> value, Ctx& ctx) -> ins_list {
    throw std::runtime_error("sub, location, register, immediate float");
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::Add<T, U> kind, target::Location dst, Register lhs_reg,
                           qa_ir::Immediate<float> value, Ctx& ctx) -> ins_list {
    throw std::runtime_error("add, location, register, immediate float");
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::Add<T, U> kind, target::Location dst, qa_ir::Variable lhs_var,
                           qa_ir::IsEphemeral auto rhs_temp, Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto lhs_stack_location = ctx.get_stack_location(lhs_var, result);
    const auto lhs_reg = newRegisterForVariable(lhs_var, ctx);
    result.push_back(Load(lhs_reg, lhs_stack_location));
    const auto rhs_reg = ensureRegister(rhs_temp, ctx);
    result.push_back(Add(lhs_reg, rhs_reg));
    result.push_back(Register_To_Location(dst, lhs_reg, ctx));
    return result;
}

auto OperationInstructions(qa_ir::NotEqual kind, target::Location dst, Register result_reg,
                           qa_ir::Immediate<float> value, Ctx& ctx) -> ins_list {
    throw std::runtime_error("ne, location, register, immediate float");
}

auto OperationInstructions(qa_ir::NotEqual kind, target::Location dst, Register result_reg,
                           qa_ir::Immediate<int> value, Ctx& ctx) -> ins_list {
    throw std::runtime_error("ne, location, register, immediate int");
}

auto OperationInstructions(qa_ir::Equal kind, target::Location dst, Register result_reg,
                           qa_ir::Immediate<float> value, Ctx& ctx) -> ins_list {
    throw std::runtime_error("eq, location, register, immediate float");
}

auto OperationInstructions(qa_ir::Equal kind, target::Location dst, Register result_reg,
                           qa_ir::Immediate<int> value, Ctx& ctx) -> ins_list {
    throw std::runtime_error("eq, location, register, immediate int");
}

// nothing? prevents rhs_value from being a int, but we already decided that this is an float
// compare
auto OperationInstructions(qa_ir::LessThan<bt::FLOAT, bt::FLOAT> kind, target::Location dst,
                           qa_ir::IsEphemeral auto lhs_temp, qa_ir::IsImmediate auto value,
                           Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto lhs_reg = ensureRegister(lhs_temp, ctx);
    const auto intermediate_reg_for_value = ctx.NewFloatRegister(4);
    result.push_back(ImmediateLoad<float>(intermediate_reg_for_value, value.numerical_value));
    result.push_back(CmpF(lhs_reg, intermediate_reg_for_value));
    const auto integer_reg_for_result = ctx.NewIntegerRegister(4);
    result.push_back(SetA(integer_reg_for_result));
    result.push_back(Register_To_Location(dst, integer_reg_for_result, ctx));
    return result;
}

auto OperationInstructions(qa_ir::LessThan<bt::INT, bt::INT> kind, target::Location dst,
                           Register result_reg, qa_ir::Immediate<int> value, Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto intermediate_reg_for_value = ctx.NewIntegerRegister(4);
    result.push_back(ImmediateLoad<int>(intermediate_reg_for_value, value.numerical_value));
    result.push_back(Cmp(result_reg, intermediate_reg_for_value));
    const auto integer_reg_for_result = ctx.NewIntegerRegister(4);
    result.push_back(SetLAl(integer_reg_for_result));
    result.push_back(Register_To_Location(dst, integer_reg_for_result, ctx));
    return result;
}

auto OperationInstructions(qa_ir::Sub kind, target::Location dst, qa_ir::Variable lhs_var,
                           qa_ir::IsEphemeral auto rhs_reg, Ctx& ctx) -> ins_list {
    throw std::runtime_error("lt, location, lhs_var, rhs_reg");
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::GreaterThan<T, U> kind, target::Location dst, qa_ir::Temp lhs_var,
                           qa_ir::IsImmediate auto rhs_reg, Ctx& ctx) -> ins_list {
    throw std::runtime_error("gt, location, lhs_var, rhs_reg");
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::GreaterThan<T, U> kind, target::Location dst,
                           qa_ir::Variable lhs_var, qa_ir::IsEphemeral auto rhs_reg, Ctx& ctx)
    -> ins_list {
    throw std::runtime_error("gt, location, lhs_var, rhs_reg");
}

auto OperationInstructions(qa_ir::LessThan<bt::INT, bt::INT> kind, target::Location dst,
                           qa_ir::Variable lhs_var, qa_ir::IsEphemeral auto rhs_reg, Ctx& ctx)
    -> ins_list {
    throw std::runtime_error("lt, location, lhs_var, rhs_reg");
}

auto OperationInstructions(qa_ir::LessThan<bt::FLOAT, bt::FLOAT> kind, target::Location dst,
                           qa_ir::Variable lhs_var, qa_ir::IsEphemeral auto rhs_reg, Ctx& ctx)
    -> ins_list {
    throw std::runtime_error("lt, location, lhs_var, rhs_reg");
}

auto OperationInstructions(qa_ir::Equal kind, target::Location dst, qa_ir::Variable lhs_var,
                           qa_ir::IsEphemeral auto rhs_temp, Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto lhs_stack_location = ctx.get_stack_location(lhs_var, result);
    const auto lhs_reg = newRegisterForVariable(lhs_var, ctx);
    result.push_back(Load(lhs_reg, lhs_stack_location));
    const auto rhs_reg = ensureRegister(rhs_temp, ctx);
    result.push_back(Cmp(lhs_reg, rhs_reg));
    const auto integer_reg_for_result = ctx.NewIntegerRegister(4);
    result.push_back(SetEAl(integer_reg_for_result));
    result.push_back(Register_To_Location(dst, integer_reg_for_result, ctx));
    return result;
}

auto OperationInstructions(qa_ir::NotEqual kind, target::Location dst,
                           qa_ir::IsEphemeral auto lhs_temp, qa_ir::IsIRLocation auto rhs_var,
                           Ctx& ctx) -> ins_list {
    throw std::runtime_error("ne, location, lhs_var, rhs_reg");
}

auto OperationInstructions(qa_ir::Equal kind, target::Location dst,
                           qa_ir::IsEphemeral auto lhs_temp, qa_ir::IsIRLocation auto rhs_var,
                           Ctx& ctx) -> ins_list {
    throw std::runtime_error("ne, location, lhs_var, rhs_reg");
}

auto OperationInstructions(qa_ir::Equal kind, target::Location dst,
                           qa_ir::IsEphemeral auto lhs_temp, qa_ir::IsImmediate auto rhs_value,
                           Ctx& ctx) -> ins_list {
    throw std::runtime_error("ne, location, lhs_var, rhs_reg");
}

auto OperationInstructions(qa_ir::NotEqual kind, target::Location dst,
                           qa_ir::IsEphemeral auto lhs_temp, qa_ir::IsImmediate auto rhs_value,
                           Ctx& ctx) -> ins_list {
    throw std::runtime_error("ne, location, lhs_var, rhs_reg");
}

auto OperationInstructions(qa_ir::NotEqual kind, target::Location dst, qa_ir::Variable lhs_var,
                           qa_ir::IsEphemeral auto rhs_reg, Ctx& ctx) -> ins_list {
    throw std::runtime_error("ne, location, lhs_var, rhs_reg");
}

auto OperationInstructions(qa_ir::GreaterThan<bt::INT, bt::INT> kind, target::Location dst,
                           qa_ir::Variable lhs, qa_ir::IsImmediate auto rhs, Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto lhs_stack_location = ctx.get_stack_location(lhs, result);
    result.push_back(CmpMI(lhs_stack_location, rhs.numerical_value));
    const auto integer_reg_for_result = ctx.NewIntegerRegister(4);
    result.push_back(SetGAl(integer_reg_for_result));
    result.push_back(Register_To_Location(dst, integer_reg_for_result, ctx));
    return result;
}

auto OperationInstructions(qa_ir::GreaterThan<bt::FLOAT, bt::FLOAT> kind, target::Location dst,
                           qa_ir::Variable lhs, qa_ir::IsImmediate auto rhs, Ctx& ctx) -> ins_list {
    throw std::runtime_error(
        "OperationInstructions(qa_ir::GreaterThan<bt::FLOAT, bt::FLOAT> kind, target::Location "
        "dst, "
        "qa_ir::Variable lhs, qa_ir::IsImmediate auto rhs, Ctx& ctx)");
}

auto OperationInstructions(qa_ir::LessThan<bt::FLOAT, bt::FLOAT> kind, target::Location dst,
                           qa_ir::Variable lhs, qa_ir::Variable rhs, Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto intermediate_reg_for_rhs_value = ctx.NewFloatRegister(4);
    result.push_back(Load(intermediate_reg_for_rhs_value, ctx.get_stack_location(rhs, result)));
    const auto lhs_stack_location = ctx.get_stack_location(lhs, result);
    result.push_back(CmpM<bt::FLOAT>(intermediate_reg_for_rhs_value, lhs_stack_location));
    const auto integer_reg_for_result = ctx.NewIntegerRegister(4);
    result.push_back(SetA(integer_reg_for_result));
    result.push_back(Register_To_Location(dst, integer_reg_for_result, ctx));
    return result;
}

// nothing? prevents rhs_value from being a int, but we already decided that this is an float
// compare
auto OperationInstructions(qa_ir::LessThan<bt::FLOAT, bt::FLOAT> kind, target::Location dst,
                           qa_ir::Variable lhs, qa_ir::IsImmediate auto rhs, Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto intermediate_reg_for_rhs_value = ctx.NewFloatRegister(4);
    result.push_back(ImmediateLoad<float>(intermediate_reg_for_rhs_value, rhs.numerical_value));
    const auto lhs_stack_location = ctx.get_stack_location(lhs, result);
    result.push_back(CmpM<bt::FLOAT>(intermediate_reg_for_rhs_value, lhs_stack_location));
    const auto integer_reg_for_result = ctx.NewIntegerRegister(4);
    result.push_back(SetA(integer_reg_for_result));
    result.push_back(Register_To_Location(dst, integer_reg_for_result, ctx));
    return result;
}

// nothing? prevents rhs_value from being a float, but we already decided that this is an int
// compare
auto OperationInstructions(qa_ir::LessThan<bt::INT, bt::INT> kind, target::Location dst,
                           qa_ir::Variable lhs, qa_ir::IsImmediate auto rhs, Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto intermediate_reg_for_rhs_value = ctx.NewIntegerRegister(4);
    result.push_back(ImmediateLoad<int>(intermediate_reg_for_rhs_value, rhs.numerical_value));
    const auto intermediate_reg_for_lhs_value = ctx.NewIntegerRegister(4);
    result.push_back(Load(intermediate_reg_for_lhs_value, ctx.get_stack_location(lhs, result)));
    result.push_back(Cmp(intermediate_reg_for_lhs_value, intermediate_reg_for_rhs_value));
    const auto integer_reg_for_result = ctx.NewIntegerRegister(4);
    result.push_back(SetLAl(integer_reg_for_result));
    result.push_back(Register_To_Location(dst, integer_reg_for_result, ctx));
    return result;
}

// nothing? prevents rhs_value from being a float, but we already decided that this is an int
// compare
auto OperationInstructions(qa_ir::LessThan<bt::INT, bt::INT> kind, target::Location dst,
                           qa_ir::IsEphemeral auto lhs, qa_ir::IsImmediate auto rhs, Ctx& ctx)
    -> ins_list {
    std::vector<Instruction> result;
    const auto lhs_reg = ensureRegister(lhs, ctx);
    result.push_back(CmpI(lhs_reg, rhs.numerical_value));
    const auto integer_reg_for_result = ctx.NewIntegerRegister(4);
    result.push_back(SetLAl(integer_reg_for_result));
    result.push_back(Register_To_Location(dst, integer_reg_for_result, ctx));
    return result;
}

auto OperationInstructions(qa_ir::NotEqual kind, target::Location dst, qa_ir::Variable lhs,
                           qa_ir::Immediate<float> rhs, Ctx& ctx) -> ins_list {
    throw std::runtime_error(
        "OperationInstructions(qa_ir::NotEqual kind, target::Location dst, qa_ir::Variable lhs, "
        "qa_ir::Immediate<float> rhs, Ctx& ctx)");
}

auto OperationInstructions(qa_ir::NotEqual kind, target::Location dst, qa_ir::Variable lhs,
                           qa_ir::Immediate<int> rhs, Ctx& ctx) -> ins_list {
    throw std::runtime_error(
        "OperationInstructions(qa_ir::NotEqual kind, target::Location dst, qa_ir::Variable lhs, "
        "qa_ir::Immediate<float> rhs, Ctx& ctx)");
}

auto OperationInstructions(qa_ir::Equal kind, target::Location dst, qa_ir::Variable lhs,
                           qa_ir::Immediate<float> rhs, Ctx& ctx) -> ins_list {
    throw std::runtime_error(
        "OperationInstructions(qa_ir::Equal kind, target::Location dst, qa_ir::Variable lhs, "
        "qa_ir::Immediate<float> rhs, Ctx& ctx)");
}

auto OperationInstructions(qa_ir::Equal kind, target::Location dst, qa_ir::Variable lhs,
                           qa_ir::Immediate<int> rhs, Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto intermediate_reg_for_rhs_value = ctx.NewIntegerRegister(4);
    result.push_back(ImmediateLoad<int>(intermediate_reg_for_rhs_value, rhs.numerical_value));
    result.push_back(
        CmpM<bt::INT>(intermediate_reg_for_rhs_value, ctx.get_stack_location(lhs, result)));
    const auto integer_reg_for_result = ctx.NewIntegerRegister(4);
    result.push_back(SetEAl(integer_reg_for_result));
    result.push_back(Register_To_Location(dst, integer_reg_for_result, ctx));
    return result;
}

auto OperationInstructions(qa_ir::Sub kind, target::Location dst, qa_ir::IsEphemeral auto lhs,
                           qa_ir::IsIRLocation auto rhs, Ctx& ctx) -> ins_list {
    throw std::runtime_error("sub, location, lhs, rhs");
}

auto OperationInstructions(qa_ir::Sub kind, target::Location dst, qa_ir::IsEphemeral auto lhs,
                           qa_ir::Immediate<float> rhs, Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;

    const auto intermediate_reg_for_lhs_value = ensureRegister(lhs, ctx);
    const auto intermediate_reg_for_value = ctx.NewFloatRegister(4);

    result.push_back(ImmediateLoad<float>(intermediate_reg_for_value, rhs.numerical_value));

    result.push_back(Sub(intermediate_reg_for_lhs_value, intermediate_reg_for_value));

    result.push_back(Register_To_Location(dst, intermediate_reg_for_lhs_value, ctx));
    return result;
}

auto OperationInstructions(qa_ir::Sub kind, target::Location dst, qa_ir::Variable lhs,
                           qa_ir::Immediate<float> rhs, Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto intermediate_reg_for_value = ctx.NewFloatRegister(4);
    result.push_back(ImmediateLoad<float>(intermediate_reg_for_value, rhs.numerical_value));
    const auto lhs_stack_location = ctx.get_stack_location(lhs, result);
    const auto intermediate_reg_for_lhs_value = ctx.NewFloatRegister(4);
    result.push_back(Load(intermediate_reg_for_lhs_value, lhs_stack_location));
    result.push_back(Sub(intermediate_reg_for_lhs_value, intermediate_reg_for_value));
    result.push_back(Register_To_Location(dst, intermediate_reg_for_lhs_value, ctx));
    return result;
}

auto OperationInstructions(qa_ir::Sub kind, target::Location dst, qa_ir::Variable lhs,
                           qa_ir::Immediate<int> rhs, Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto lhs_stack_location = ctx.get_stack_location(lhs, result);
    const auto intermediate_reg_for_lhs_value = ctx.NewIntegerRegister(4);
    result.push_back(Load(intermediate_reg_for_lhs_value, lhs_stack_location));
    result.push_back(SubI(intermediate_reg_for_lhs_value, rhs.numerical_value));
    result.push_back(Register_To_Location(dst, intermediate_reg_for_lhs_value, ctx));
    return result;
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::Add<T, U> kind, StackLocation dst, qa_ir::Variable lhs,
                           qa_ir::Immediate<float> rhs, Ctx& ctx) -> ins_list {
    throw std::runtime_error("add, stack location, variable, immediate float");
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::Add<T, U> kind, Register dst, qa_ir::Variable lhs,
                           qa_ir::Immediate<float> rhs, Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto intermediate_reg_for_value = ctx.NewFloatRegister(4);
    result.push_back(ImmediateLoad<float>(intermediate_reg_for_value, rhs.numerical_value));
    result.push_back(Add(dst, intermediate_reg_for_value));
    return result;
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::Add<T, U> kind, target::Location dst, qa_ir::Variable lhs,
                           qa_ir::Immediate<float> rhs, Ctx& ctx) -> ins_list {
    return std::visit(
        [&](auto&& dest_v_arg) { return OperationInstructions(kind, dest_v_arg, lhs, rhs, ctx); },
        dst);
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::Add<T, U> kind, StackLocation dst, qa_ir::Variable lhs,
                           qa_ir::Immediate<int> rhs, Ctx& ctx) -> ins_list {
    throw std::runtime_error("dst, lhs, rhs, ctx");
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::Add<T, U> kind, Register dst, qa_ir::Variable lhs,
                           qa_ir::Immediate<int> rhs, Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto intermediate_reg_for_value = ctx.NewIntegerRegister(4);
    result.push_back(Load(intermediate_reg_for_value, ctx.get_stack_location(lhs, result)));
    result.push_back(AddI(intermediate_reg_for_value, rhs.numerical_value));
    result.push_back(Mov(dst, intermediate_reg_for_value));
    return result;
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::Add<T, U> kind, target::Location dst, qa_ir::Variable lhs,
                           qa_ir::Immediate<int> rhs, Ctx& ctx) -> ins_list {
    return std::visit(
        [&](auto&& dest_v_arg) { return OperationInstructions(kind, dest_v_arg, lhs, rhs, ctx); },
        dst);
}

auto OperationInstructions(qa_ir::GreaterThan<bt::INT, bt::INT> kind, target::Location dst,
                           qa_ir::Variable value1, qa_ir::Variable value2, Ctx& ctx) -> ins_list {
    const auto intermediate_reg_for_value1 = newRegisterForVariable(value1, ctx);
    std::vector<Instruction> result;
    result.push_back(Load(intermediate_reg_for_value1, ctx.get_stack_location(value1, result)));
    const auto rhs_stack_location = ctx.get_stack_location(value2, result);
    result.push_back(CmpM<bt::INT>(intermediate_reg_for_value1, rhs_stack_location));
    const auto integer_reg_for_result = ctx.NewIntegerRegister(4);
    result.push_back(SetGAl(integer_reg_for_result));
    return result;
}

auto OperationInstructions(qa_ir::GreaterThan<bt::FLOAT, bt::FLOAT> kind, target::Location dst,
                           qa_ir::Variable value1, qa_ir::Variable value2, Ctx& ctx) -> ins_list {
    const auto intermediate_reg_for_value1 = newRegisterForVariable(value1, ctx);
    std::vector<Instruction> result;
    result.push_back(Load(intermediate_reg_for_value1, ctx.get_stack_location(value1, result)));
    const auto rhs_stack_location = ctx.get_stack_location(value2, result);
    result.push_back(CmpM<bt::FLOAT>(intermediate_reg_for_value1, rhs_stack_location));
    const auto integer_reg_for_result = ctx.NewIntegerRegister(4);
    result.push_back(SetA(integer_reg_for_result));
    return result;
}

auto OperationInstructions(qa_ir::LessThan<bt::INT, bt::INT> kind, target::Location dst,
                           qa_ir::Variable value1, qa_ir::Variable value2, Ctx& ctx) -> ins_list {
    const auto intermediate_reg_for_value1 = newRegisterForVariable(value1, ctx);
    std::vector<Instruction> result;
    result.push_back(Load(intermediate_reg_for_value1, ctx.get_stack_location(value1, result)));
    result.push_back(
        CmpM<bt::INT>(intermediate_reg_for_value1, ctx.get_stack_location(value2, result)));
    const auto integer_reg_for_result = ctx.NewIntegerRegister(4);
    result.push_back(SetLAl(integer_reg_for_result));
    result.push_back(Register_To_Location(dst, integer_reg_for_result, ctx));
    return result;
}

auto OperationInstructions(qa_ir::NotEqual kind, target::Location dst, qa_ir::Variable value1,
                           qa_ir::Variable value2, Ctx& ctx) -> ins_list {
    const auto intermediate_reg_for_value1 = newRegisterForVariable(value1, ctx);
    std::vector<Instruction> result;
    result.push_back(Load(intermediate_reg_for_value1, ctx.get_stack_location(value1, result)));
    result.push_back(
        CmpM<bt::INT>(intermediate_reg_for_value1, ctx.get_stack_location(value2, result)));
    const auto integer_reg_for_result = ctx.NewIntegerRegister(4);
    result.push_back(SetNeAl(integer_reg_for_result));
    result.push_back(Register_To_Location(dst, integer_reg_for_result, ctx));
    return result;
}

auto OperationInstructions(qa_ir::Equal kind, target::Location dst, qa_ir::Variable value1,
                           qa_ir::Variable value2, Ctx& ctx) -> ins_list {
    throw std::runtime_error("eq, location, var , var");
}

auto OperationInstructions(qa_ir::Sub kind, target::Location dst, qa_ir::Variable value1,
                           qa_ir::Variable value2, Ctx& ctx) -> ins_list {
    throw std::runtime_error("sub, location, var , var");
}

// std::visit on result to get specialization for result = result + 1
auto OperationInstructions(qa_ir::Add<bt::INT, bt::INT> kind, target::Location dst,
                           qa_ir::Variable value1, qa_ir::Variable value2, Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto intermediate_reg_value1 = ctx.NewIntegerRegister(4);
    result.push_back(Load(intermediate_reg_value1, ctx.get_stack_location(value1, result)));
    const auto intermediate_reg_value2 = ctx.NewIntegerRegister(4);
    result.push_back(Load(intermediate_reg_value2, ctx.get_stack_location(value2, result)));
    result.push_back(Add(intermediate_reg_value1, intermediate_reg_value2));
    result.push_back(Register_To_Location(dst, intermediate_reg_value1, ctx));
    return result;
}

auto OperationInstructions(qa_ir::Add<bt::INT, bt::INT> kind, target::Location dst,
                           qa_ir::IsImmediate auto value1, qa_ir::Variable value2, Ctx& ctx)
    -> ins_list {
    std::vector<Instruction> result;
    const auto intermediate_reg = ctx.NewIntegerRegister(4);
    result.push_back(Load(intermediate_reg, ctx.get_stack_location(value2, result)));
    result.push_back(AddI(intermediate_reg, value1.numerical_value));
    result.push_back(Register_To_Location(dst, intermediate_reg, ctx));
    return result;
}

auto OperationInstructions(qa_ir::Add<bt::INT, bt::INT> kind, target::Location dst,
                           qa_ir::IsEphemeral auto lhs_temp, qa_ir::IsEphemeral auto rhs_value,
                           Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto lhs_reg = ensureRegister(lhs_temp, ctx);
    const auto rhs_reg = ensureRegister(rhs_value, ctx);
    result.push_back(Add(lhs_reg, rhs_reg));
    result.push_back(Register_To_Location(dst, lhs_reg, ctx));
    return result;
}

auto OperationInstructions(qa_ir::Add<bt::INT, bt::INT> kind, target::Location dst,
                           qa_ir::IsEphemeral auto lhs_temp, qa_ir::Variable rhs_value, Ctx& ctx)
    -> ins_list {
    std::vector<Instruction> result;
    const auto lhs_reg = ensureRegister(lhs_temp, ctx);
    const auto rhs_reg = newRegisterForVariable(rhs_value, ctx);
    result.push_back(Load(rhs_reg, ctx.get_stack_location(rhs_value, result)));
    result.push_back(Add(lhs_reg, rhs_reg));
    result.push_back(Register_To_Location(dst, lhs_reg, ctx));
    return result;
}

auto OperationInstructions(qa_ir::Add<bt::FLOAT, bt::FLOAT> kind, target::Location dst,
                           qa_ir::IsEphemeral auto lhs_temp, qa_ir::Variable rhs_value, Ctx& ctx)
    -> ins_list {
    std::vector<Instruction> result;
    const auto lhs_reg = ensureRegister(lhs_temp, ctx);
    const auto rhs_reg = newRegisterForVariable(rhs_value, ctx);
    result.push_back(Load(rhs_reg, ctx.get_stack_location(rhs_value, result)));
    result.push_back(Add(lhs_reg, rhs_reg));
    result.push_back(Register_To_Location(dst, lhs_reg, ctx));
    return result;
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::GreaterThan<T, U> kind, target::Location dst,
                           qa_ir::IsEphemeral auto lhs_temp, qa_ir::IsEphemeral auto rhs_value,
                           Ctx& ctx) -> ins_list {
    throw std::runtime_error("add<float, float>, immediate, var , var");
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::LessThan<T, U> kind, target::Location dst,
                           qa_ir::IsEphemeral auto lhs_temp, qa_ir::IsEphemeral auto rhs_value,
                           Ctx& ctx) -> ins_list {
    throw std::runtime_error("add<float, float>, immediate, var , var");
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::LessThan<T, U> kind, target::Location dst,
                           qa_ir::IsEphemeral auto lhs_temp, qa_ir::Variable rhs_value, Ctx& ctx)
    -> ins_list {
    throw std::runtime_error("add<float, float>, immediate, var , var");
}

auto OperationInstructions(qa_ir::NotEqual kind, target::Location dst,
                           qa_ir::IsEphemeral auto lhs_temp, qa_ir::IsEphemeral auto rhs_value,
                           Ctx& ctx) -> ins_list {
    throw std::runtime_error("add<float, float>, immediate, var , var");
}

auto OperationInstructions(qa_ir::Equal kind, target::Location dst,
                           qa_ir::IsEphemeral auto lhs_temp, qa_ir::IsEphemeral auto rhs_value,
                           Ctx& ctx) -> ins_list {
    throw std::runtime_error("add<float, float>, immediate, var , var");
}

auto OperationInstructions(qa_ir::Sub kind, target::Location dst, qa_ir::IsEphemeral auto lhs_temp,
                           qa_ir::IsEphemeral auto rhs_value, Ctx& ctx) -> ins_list {
    throw std::runtime_error("add<float, float>, immediate, var , var");
}

auto OperationInstructions(qa_ir::Add<bt::FLOAT, bt::FLOAT> kind, target::Location dst,
                           qa_ir::Immediate<float> value1, qa_ir::Variable value2, Ctx& ctx)
    -> ins_list {
    throw std::runtime_error("add<float, float>, immediate, var , var");
}

auto OperationInstructions(qa_ir::Add<bt::FLOAT, bt::FLOAT> kind, target::Location dst,
                           qa_ir::Immediate<int> value1, qa_ir::Variable value2, Ctx& ctx)
    -> ins_list {
    throw std::runtime_error("add<float, float>, immediate, var , var");
}

auto OperationInstructions(qa_ir::Add<bt::FLOAT, bt::FLOAT> kind, target::Location dst,
                           qa_ir::IsEphemeral auto lhs_temp, qa_ir::IsEphemeral auto rhs_value,
                           Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto lhs_reg = ensureRegister(lhs_temp, ctx);
    const auto rhs_reg = ensureRegister(rhs_value, ctx);
    result.push_back(Add(lhs_reg, rhs_reg));
    return result;
}

auto OperationInstructions(qa_ir::Add<bt::FLOAT, bt::FLOAT> kind, target::Location dst,
                           qa_ir::Variable value1, qa_ir::Variable value2, Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto lhs_reg = newRegisterForVariable(value1, ctx);
    result.push_back(Load(lhs_reg, ctx.get_stack_location(value1, result)));
    const auto rhs_reg = newRegisterForVariable(value2, ctx);
    result.push_back(Load(rhs_reg, ctx.get_stack_location(value2, result)));
    result.push_back(Add(lhs_reg, rhs_reg));
    result.push_back(Register_To_Location(dst, lhs_reg, ctx));
    return result;
}

auto OperationInstructions(qa_ir::Sub kind, target::Location dst, qa_ir::Immediate<int> value1,
                           qa_ir::Variable value2, Ctx& ctx) -> ins_list {
    throw std::runtime_error("gt, location, immediate float, immediate float");
}

auto OperationInstructions(qa_ir::Sub kind, target::Location dst, qa_ir::Immediate<float> value1,
                           qa_ir::Variable value2, Ctx& ctx) -> ins_list {
    throw std::runtime_error("gt, location, immediate float, immediate float");
}

auto OperationInstructions(qa_ir::Equal kind, target::Location dst, qa_ir::Immediate<int> value1,
                           qa_ir::Variable value2, Ctx& ctx) -> ins_list {
    throw std::runtime_error("gt, location, immediate float, immediate float");
}

auto OperationInstructions(qa_ir::Equal kind, target::Location dst, qa_ir::Immediate<float> value1,
                           qa_ir::Variable value2, Ctx& ctx) -> ins_list {
    throw std::runtime_error("gt, location, immediate float, immediate float");
}

auto OperationInstructions(qa_ir::NotEqual kind, target::Location dst, qa_ir::Immediate<int> value1,
                           qa_ir::Variable value2, Ctx& ctx) -> ins_list {
    throw std::runtime_error("gt, location, immediate float, immediate float");
}

auto OperationInstructions(qa_ir::NotEqual kind, target::Location dst,
                           qa_ir::Immediate<float> value1, qa_ir::Variable value2, Ctx& ctx)
    -> ins_list {
    throw std::runtime_error("gt, location, immediate float, immediate float");
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::LessThan<T, U> kind, target::Location dst,
                           qa_ir::Immediate<int> value1, qa_ir::Variable value2, Ctx& ctx)
    -> ins_list {
    throw std::runtime_error("gt, location, immediate float, immediate float");
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::LessThan<T, U> kind, target::Location dst,
                           qa_ir::Immediate<float> value1, qa_ir::Variable value2, Ctx& ctx)
    -> ins_list {
    throw std::runtime_error("gt, location, immediate float, immediate float");
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::GreaterThan<T, U> kind, target::Location dst,
                           qa_ir::IsImmediate auto value1, qa_ir::Variable value2, Ctx& ctx)
    -> ins_list {
    throw std::runtime_error("gt, location, immediate float, immediate float");
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::GreaterThan<T, U> kind, target::Location dst,
                           qa_ir::IsEphemeral auto value1, qa_ir::Variable value2, Ctx& ctx)
    -> ins_list {
    throw std::runtime_error("gt, location, immediate float, immediate float");
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::GreaterThan<T, U> kind, target::Location dst,
                           qa_ir::Immediate<float> value1, qa_ir::Immediate<float> value2, Ctx& ctx)
    -> ins_list {
    throw std::runtime_error("gt, location, immediate float, immediate float");
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::GreaterThan<T, U> kind, target::Location dst,
                           qa_ir::Immediate<int> value1, qa_ir::Immediate<int> value2, Ctx& ctx)
    -> ins_list {
    throw std::runtime_error("gt, location, immediate int, immediate int");
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::GreaterThan<T, U> kind, target::Location dst, Register result_reg,
                           qa_ir::Immediate<float> value, Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto intermediate_reg_for_value = ctx.NewFloatRegister(4);
    result.push_back(ImmediateLoad<float>(intermediate_reg_for_value, value.numerical_value));
    result.push_back(CmpF(result_reg, intermediate_reg_for_value));
    const auto integer_reg_for_result = ctx.NewIntegerRegister(4);
    result.push_back(SetA(integer_reg_for_result));
    result.push_back(Register_To_Location(dst, integer_reg_for_result, ctx));
    return result;
}

template <bt T, bt U>
auto OperationInstructions(qa_ir::GreaterThan<T, U> kind, target::Location dst, Register result_reg,
                           qa_ir::Immediate<int> value, Ctx& ctx) -> ins_list {
    std::vector<Instruction> result;
    const auto intermediate_reg_for_value = ctx.NewIntegerRegister(4);
    result.push_back(ImmediateLoad<int>(intermediate_reg_for_value, value.numerical_value));
    result.push_back(Cmp(result_reg, intermediate_reg_for_value));
    const auto integer_reg_for_result = ctx.NewIntegerRegister(4);
    result.push_back(SetGAl(integer_reg_for_result));
    result.push_back(Register_To_Location(dst, integer_reg_for_result, ctx));
    return result;
}

ins_list LowerCompare(qa_ir::Compare<bt::FLOAT, bt::FLOAT> kind, qa_ir::IsIRLocation auto lhs_var,
                      qa_ir::IsIRLocation auto rhs_var, Ctx& ctx) {
    throw std::runtime_error("compare, var, var");
}

ins_list LowerCompare(qa_ir::Compare<bt::INT, bt::INT> kind, qa_ir::IsIRLocation auto lhs_var,
                      qa_ir::IsIRLocation auto rhs_var, Ctx& ctx) {
    throw std::runtime_error("compare, var, var");
}

ins_list LowerCompare(qa_ir::Compare<bt::FLOAT, bt::FLOAT> kind, qa_ir::IsIRLocation auto rhs_var,
                      qa_ir::IsEphemeral auto lhs_temp, Ctx& ctx) {
    throw std::runtime_error("compare, var, emph");
}

ins_list LowerCompare(qa_ir::Compare<bt::INT, bt::INT> kind, qa_ir::IsIRLocation auto rhs_var,
                      qa_ir::IsEphemeral auto lhs_temp, Ctx& ctx) {
    throw std::runtime_error("compare, var, emph");
}

ins_list LowerCompare(qa_ir::Compare<bt::INT, bt::INT> kind, qa_ir::IsIRLocation auto lhs_var,
                      qa_ir::IsImmediate auto value, Ctx& ctx) {
    throw std::runtime_error("compare, var, value");
}

ins_list LowerCompare(qa_ir::Compare<bt::INT, bt::INT> kind, qa_ir::IsEphemeral auto lhs_temp,
                      qa_ir::IsEphemeral auto rhs_temp, Ctx& ctx) {
    throw std::runtime_error("compare, emph, emph");
}

ins_list LowerCompare(qa_ir::Compare<bt::INT, bt::INT> kind, qa_ir::IsEphemeral auto lhs_temp,
                      qa_ir::IsIRLocation auto rhs_value, Ctx& ctx) {
    throw std::runtime_error("compare, emph, var");
}

// nothing? prevents rhs_value from being a float, but we already decided that this is an int
// compare
ins_list LowerCompare(qa_ir::Compare<bt::INT, bt::INT> kind, qa_ir::IsEphemeral auto lhs_temp,
                      qa_ir::IsImmediate auto rhs_value, Ctx& ctx) {
    std::vector<Instruction> result;
    const auto lhs_reg = ensureRegister(lhs_temp, ctx);
    result.push_back(CmpI(lhs_reg, rhs_value.numerical_value));
    return result;
}

ins_list LowerCompare(qa_ir::Compare<bt::INT, bt::INT> kind, qa_ir::IsImmediate auto lhs_temp,
                      qa_ir::IsImmediate auto rhs_value, Ctx& ctx) {
    throw std::runtime_error("compare, value, value");
}

ins_list LowerCompare(qa_ir::Compare<bt::INT, bt::INT> kind, qa_ir::IsImmediate auto lhs_temp,
                      qa_ir::IsEphemeral auto rhs_value, Ctx& ctx) {
    throw std::runtime_error("compare, value, emph");
}

ins_list LowerCompare(qa_ir::Compare<bt::INT, bt::INT> kind, qa_ir::IsImmediate auto lhs_temp,
                      qa_ir::IsIRLocation auto rhs_value, Ctx& ctx) {
    throw std::runtime_error("compare, value, var");
}

template <typename ArthStructTag>
ins_list InstructionForArth(ArthStructTag tag, target::Location dst,
                            qa_ir::IsIRLocation auto lhs_var, qa_ir::IsImmediate auto value,
                            Ctx& ctx) {
    return OperationInstructions(tag, dst, lhs_var, value, ctx);
}

template <typename ArthStructTag>
ins_list InstructionForArth(ArthStructTag tag, target::Location dst,
                            qa_ir::IsIRLocation auto lhs_var, qa_ir::IsEphemeral auto rhs_temp,
                            Ctx& ctx) {
    return OperationInstructions(tag, dst, lhs_var, rhs_temp, ctx);
}

// should template the immediate types
template <typename ArthStructTag>
ins_list InstructionForArth(ArthStructTag tag, target::Location dst, qa_ir::IsImmediate auto value1,
                            qa_ir::IsImmediate auto value2, Ctx& ctx) {
    throw std::runtime_error("immediate1, immediate2");
}

template <typename ArthStructTag>
ins_list InstructionForArth(ArthStructTag tag, target::Location dst, qa_ir::IsImmediate auto value,
                            qa_ir::IsIRLocation auto rhs, Ctx& ctx) {
    return OperationInstructions(tag, dst, value, rhs, ctx);
}

template <typename ArthStructTag>
ins_list InstructionForArth(ArthStructTag tag, target::Location dst, qa_ir::IsIRLocation auto value,
                            qa_ir::IsIRLocation auto rhs, Ctx& ctx) {
    return OperationInstructions(tag, dst, value, rhs, ctx);
}

template <typename ArthStructTag>
ins_list InstructionForArth(ArthStructTag tag, target::Location dst, qa_ir::IsEphemeral auto value,
                            qa_ir::IsEphemeral auto rhs, Ctx& ctx) {
    return OperationInstructions(tag, dst, value, rhs, ctx);
}

template <typename ArthStructTag>
ins_list InstructionForArth(ArthStructTag tag, target::Location dst, qa_ir::IsEphemeral auto value,
                            qa_ir::IsIRLocation auto rhs, Ctx& ctx) {
    return OperationInstructions(tag, dst, value, rhs, ctx);
}

template <typename ArthStructTag>
ins_list InstructionForArth(ArthStructTag tag, target::Location dst, qa_ir::IsImmediate auto value,
                            qa_ir::IsEphemeral auto rhs, Ctx& ctx) {
    throw std::runtime_error("immediate, emph");
}

template <typename ArthStructTag>
ins_list InstructionForArth(ArthStructTag tag, target::Location dst,
                            qa_ir::IsEphemeral auto result_reg, qa_ir::IsImmediate auto value,
                            Ctx& ctx) {
    return OperationInstructions(tag, dst, result_reg, value, ctx);
}

template <typename ArthStructTag>
[[nodiscard]] ins_list LowerArth(ArthStructTag tag, qa_ir::Value dst, qa_ir::Value left,
                                 qa_ir::Value right, Ctx& ctx) {
    // allocate new will return a location of a variable if it already exists
    ins_list result;
    const Location dest_location = ctx.AllocateNew(dst, result);
    auto visitor = [&](auto&& left_p, auto&& right_p) -> ins_list {
        return InstructionForArth(tag, dest_location, left_p, right_p, ctx);
    };
    const auto op = std::visit(visitor, left, right);
    std::ranges::copy(op, std::back_inserter(result));
    return result;
}

auto LowerInstruction(qa_ir::LabelDef label, Ctx& ctx) -> ins_list {
    return {Label(label.label.name)};
}

[[nodiscard]] ins_list LowerInstruction(qa_ir::ConditionalJumpEqual cj, Ctx& ctx) {
    ins_list result;
    result.push_back(JumpEq(cj.trueLabel.name));
    result.push_back(Jump(cj.falseLabel.name));
    return result;
}

[[nodiscard]] ins_list LowerInstruction(qa_ir::ConditionalJumpNotEqual cj, Ctx& ctx) {
    ins_list result;
    result.push_back(JumpEq(cj.falseLabel.name));
    result.push_back(Jump(cj.trueLabel.name));
    return result;
}

[[nodiscard]] ins_list LowerInstruction(qa_ir::ConditionalJumpGreater cj, Ctx& ctx) {
    ins_list result;
    result.push_back(JumpGreater(cj.trueLabel.name));
    result.push_back(Jump(cj.falseLabel.name));
    return result;
}

auto LowerInstruction(qa_ir::ConditionalJumpLess cj, Ctx& ctx) -> ins_list {
    ins_list result;
    result.push_back(JumpLess(cj.trueLabel.name));
    result.push_back(Jump(cj.falseLabel.name));
    return result;
}

[[nodiscard]] ins_list LowerInstruction(qa_ir::Call call, Ctx& ctx) {
    ins_list result;
    auto dest = ctx.AllocateNew(call.dst, result);
    for (auto it = call.args.begin(); it != call.args.end(); ++it) {
        auto dist = std::distance(call.args.begin(), it);
        std::size_t index = static_cast<std::size_t>(dist);
        if (index >= 6) {
            if (std::holds_alternative<qa_ir::Immediate<int>>(*it)) {
                const auto value = std::get<qa_ir::Immediate<int>>(*it).numerical_value;
                result.push_back(PushI(value));
                continue;
            }
            if (std::holds_alternative<qa_ir::Variable>(*it)) {
                const auto size = SizeOf(*it);
                const auto reg = ctx.NewIntegerRegister(size);
                const auto variable = std::get<qa_ir::Variable>(*it);
                const auto variableOffset = ctx.variable_offset.at(variable.name);
                result.push_back(Load(reg, variableOffset));
                result.push_back(Push(reg));
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
    result.push_back(Call(call.name, returnRegister));
    const auto move_dest_instructions = Register_To_Location(dest, returnRegister, &ctx);
    result.emplace_back(move_dest_instructions);
    return result;
}

[[nodiscard]] ins_list LowerInstruction(qa_ir::MovR move, Ctx& ctx) {
    ins_list result;
    const auto dst = ctx.AllocateNew(move.dst, result);
    const auto ins = ctx.toLocation(dst, move.src);
    std::ranges::copy(ins, std::back_inserter(result));
    return result;
}

[[nodiscard]] ins_list LowerInstruction(qa_ir::Addr addr, Ctx& ctx) {
    ins_list result;
    const auto temp = std::get<qa_ir::Temp>(addr.dst);
    const auto variable = std::get<qa_ir::Variable>(addr.src);
    const auto variableOffset = ctx.variable_offset.at(variable.name);
    const auto reg = ctx.AllocateNewForTemp(temp);
    result.push_back(Lea(reg, variableOffset));
    return result;
}

[[nodiscard]] ins_list LowerInstruction(qa_ir::Deref deref, Ctx& ctx) {
    ins_list result;
    const auto final_temp_dest = std::get<qa_ir::Temp>(deref.dst);
    if (std::holds_alternative<qa_ir::Variable>(deref.src)) {
        const auto variable = std::get<qa_ir::Variable>(deref.src);
        const auto depth = deref.depth;
        auto base_reg = ctx.NewIntegerRegister(8);
        const auto stk_location = ctx.get_stack_location(variable, result);
        result.push_back(Load(base_reg, stk_location));
        for (int i = 1; i < depth; i++) {
            const auto tempreg = ctx.NewIntegerRegister(8);
            result.push_back(IndirectLoad(tempreg, base_reg));
            base_reg = tempreg;
        }
        // indirect mem access
        const auto finalDest = ctx.AllocateNewForTemp(final_temp_dest);
        result.push_back(IndirectLoad(finalDest, base_reg));
        return result;
    }
    if (std::holds_alternative<qa_ir::Temp>(deref.src)) {
        const auto temp = std::get<qa_ir::Temp>(deref.src);
        const auto depth = deref.depth;
        auto reg = ctx.AllocateNewForTemp(temp);
        for (int i = 1; i < depth; i++) {
            const auto tempreg = ctx.NewIntegerRegister(8);
            result.push_back(IndirectLoad(tempreg, reg));
            reg = tempreg;
        }
        // indirect mem access
        const auto finalDest = ctx.AllocateNewForTemp(final_temp_dest);
        result.push_back(IndirectLoad(finalDest, reg));
        return result;
    }
    throw std::runtime_error("deref switch fallthrough");
}

[[nodiscard]] auto LowerInstruction(qa_ir::DerefStore deref, Ctx& ctx) {
    ins_list result;
    // variable_dest holds the address of the variable
    auto variable_dest = deref.dst;
    // move the variable to a register
    const auto tempregister = ctx.NewIntegerRegister(8);
    l_value_ctx = true;
    auto moveInstructions = ctx.LocationToLocation(tempregister, variable_dest);
    l_value_ctx = false;
    result.insert(result.end(), moveInstructions.begin(), moveInstructions.end());
    // load the value at the address
    const auto src = deref.src;
    const auto srcSize = SizeOf(src);
    const auto srcReg = ctx.NewIntegerRegister(srcSize);
    auto srcInstructions = ctx.toLocation(srcReg, src);
    result.insert(result.end(), srcInstructions.begin(), srcInstructions.end());
    // store the value at the address using indirect store instructions
    result.push_back(IndirectStore(tempregister, srcReg));
    return result;
}

template <bt T, bt U>
auto LowerInstruction(qa_ir::Add<T, U> arg, Ctx& ctx) -> ins_list {
    return LowerArth(arg, arg.dst, arg.left, arg.right, ctx);
}

auto LowerInstruction(qa_ir::Sub arg, Ctx& ctx) -> ins_list {
    return LowerArth(arg, arg.dst, arg.left, arg.right, ctx);
}

auto LowerInstruction(qa_ir::Equal arg, Ctx& ctx) -> ins_list {
    return LowerArth(arg, arg.dst, arg.left, arg.right, ctx);
}

auto LowerInstruction(qa_ir::NotEqual arg, Ctx& ctx) -> ins_list {
    return LowerArth(arg, arg.dst, arg.left, arg.right, ctx);
}

template <bt T, bt U>
auto LowerInstruction(qa_ir::LessThan<T, U> arg, Ctx& ctx) -> ins_list {
    return LowerArth(arg, arg.dst, arg.left, arg.right, ctx);
}
template <bt T, bt U>
auto LowerInstruction(qa_ir::GreaterThan<T, U> arg, Ctx& ctx) -> ins_list {
    return LowerArth(arg, arg.dst, arg.left, arg.right, ctx);
}

template <bt T, bt U>
auto LowerInstruction(qa_ir::Compare<T, U> arg, Ctx& ctx) -> ins_list {
    ins_list result;
    auto visitor = [&](auto&& left_p, auto&& right_p) -> ins_list {
        return LowerCompare(arg, left_p, right_p, ctx);
    };
    const auto op = std::visit(visitor, arg.left, arg.right);
    std::ranges::copy(op, std::back_inserter(result));
    return result;
}

auto LowerInstruction(qa_ir::DefineStackPushed arg, Ctx& ctx) -> ins_list {
    auto name = arg.name;
    ctx.define_stack_pushed_variable(name);
    return {};
}

auto LowerInstruction(qa_ir::DefineArray arg, Ctx& ctx) -> ins_list {
    const qa_ir::Value arr_variable = qa_ir::Variable{.name = arg.name, .type = arg.type};
    ins_list result;
    std::ignore = ctx.AllocateNew(arr_variable, result);
    return result;
}

auto LowerInstruction(qa_ir::PointerOffset arg, Ctx& ctx) -> ins_list {
    const auto base = arg.base;
    const auto offset = arg.offset;
    const auto base_reg = ctx.NewIntegerRegister(target::address_size);
    const auto base_location = ctx.toLocation(base_reg, base);
    ins_list result;
    std::ranges::copy(base_location, std::back_inserter(result));
    if (std::holds_alternative<qa_ir::Immediate<int>>(offset)) {
        const auto offset_value = std::get<qa_ir::Immediate<int>>(offset).numerical_value;
        const auto computed_stack_location =
            StackLocation{.offset = 0,
                          .is_computed = true,
                          .src = base_reg,
                          .scale = 1,
                          .offest_from_base = offset_value * arg.basisType.GetSize()};
        const auto final_dest = ctx.AllocateNew(arg.dst, result);
        const auto intermediate_result_reg = ctx.NewIntegerRegister(SizeOf(arg.dst));
        result.push_back(Lea(intermediate_result_reg, computed_stack_location));
        result.push_back(Register_To_Location(final_dest, intermediate_result_reg, ctx));
        return result;
    }
    if (std::holds_alternative<qa_ir::Variable>(offset)) {
        const auto offset_variable = std::get<qa_ir::Variable>(offset);
        const auto offset_variable_offset = ctx.get_stack_location(offset_variable, result);
        const auto offset_reg = ctx.NewIntegerRegister(SizeOf(offset_variable));
        result.push_back(Load(offset_reg, offset_variable_offset));
        if (offset_reg.size != target::address_size) {
            const auto resized_offset_reg = ctx.NewIntegerRegister(target::address_size);
            result.push_back(ZeroExtend(resized_offset_reg, offset_reg));
            // use the resized offset to compute the value to add to the address
            const auto computed_offset_reg = ctx.NewIntegerRegister(target::address_size);
            const auto stk_location = StackLocation{.offset = 0,
                                                    .is_computed = true,
                                                    .src = resized_offset_reg,
                                                    .scale = arg.basisType.GetSize()};

            result.push_back(Lea(computed_offset_reg, stk_location));
            result.push_back(Add(base_reg, computed_offset_reg));
            const auto final_dest = ctx.AllocateNew(arg.dst, result);
            result.push_back(Register_To_Location(final_dest, base_reg, ctx));
            return result;
        }
        throw std::runtime_error("Unsupported offset type");
    }
    if (std::holds_alternative<qa_ir::Temp>(offset)) {
        const auto offset_temp = std::get<qa_ir::Temp>(offset);
        const auto offset_reg = ctx.AllocateNewForTemp(offset_temp);
        if (SizeOf(offset_reg) != target::address_size) {
            const auto resized_offset_reg = ctx.NewIntegerRegister(target::address_size);
            result.push_back(ZeroExtend(resized_offset_reg, offset_reg));
            // use the resized offset to compute the value to add to the address
            const auto computed_offset_reg = ctx.NewIntegerRegister(target::address_size);
            const auto stk_location = StackLocation{.offset = 0,
                                                    .is_computed = true,
                                                    .src = resized_offset_reg,
                                                    .scale = arg.basisType.GetSize()};

            result.push_back(Lea(computed_offset_reg, stk_location));
            result.push_back(Add(base_reg, computed_offset_reg));
            const auto final_dest = ctx.AllocateNew(arg.dst, result);
            result.push_back(Register_To_Location(final_dest, base_reg, ctx));
            return result;
        }
        throw std::runtime_error("Unsupported offset type");
    }
    throw std::runtime_error("Unsupported offset type");
}

auto LowerInstruction(qa_ir::Jump arg, Ctx& ctx) -> ins_list { return {Jump(arg.label.name)}; }

[[nodiscard]] ins_list GenerateInstructionsForOperation(const qa_ir::Operation& op, Ctx& ctx) {
    return std::visit([&ctx](auto&& arg) { return LowerInstruction(arg, ctx); }, op);
}
#pragma GCC diagnostic pop

[[nodiscard]] std::vector<Frame> LowerIR(const std::vector<qa_ir::Frame>& frames) {
    std::vector<Frame> result;
    for (const auto& f : frames) {
        ins_list instructions;
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