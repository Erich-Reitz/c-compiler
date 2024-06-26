
#include "../../../include/compiler/target/qa_x86_instructions.hpp"

#include "../../../include/compiler/target/qa_x86_locations.hpp"

namespace target {

[[nodiscard]] auto to_asm_constant(int value) -> std::string { return std::to_string(value); }

auto to_asm_constant(int value) -> std::string;

auto to_asm_constant(float value) -> std::string;

auto Mov::to_asm(CodegenContext& ctx) const -> void {
    const auto d = std::get<HardcodedRegister>(dst);
    const auto s = std::get<HardcodedRegister>(src);
    if (d != s) {
        const auto ins = "mov " + register_to_asm(dst) + ", " + register_to_asm(src);
        ctx.AddInstruction(ins);
    }
}

auto Load::to_asm(CodegenContext& ctx) const -> void {
    const auto register_dst = std::get<target::HardcodedRegister>(dst);
    const auto source_prefix = register_dst.size == 4 ? std::string("dword") : std::string("qword");
    const auto hardcoded_dest = std::get<HardcodedRegister>(dst);
    if (is_float_register(hardcoded_dest.reg)) {
        const auto ins =
            "movss " + register_to_asm(dst) + ", " + source_prefix + stack_location_at_asm(src);
        ctx.AddInstruction(ins);
    } else {
        const auto ins =
            "mov " + register_to_asm(dst) + ", " + source_prefix + stack_location_at_asm(src);
        ctx.AddInstruction(ins);
    }
}

auto ZeroExtend::to_asm(CodegenContext& ctx) const -> void {
    const auto ins = "movsx " + register_to_asm(dst) + ", " + register_to_asm(src);
    ctx.AddInstruction(ins);
}

auto LoadI::to_asm(CodegenContext& ctx) const -> void {
    const auto ins = "mov " + register_to_asm(dst) + ", " + to_asm_constant(value);
    ctx.AddInstruction(ins);
}

auto StoreI::to_asm(CodegenContext& ctx) const -> void {
    const auto source_prefix = std::string("dword");

    const auto ins =
        "mov " + source_prefix + stack_location_at_asm(dst) + ", " + to_asm_constant(value);
    ctx.AddInstruction(ins);
}

auto StoreF::to_asm(CodegenContext& ctx) const -> void {
    const auto source_prefix = std::string("dword");
    const auto f_value_label = ctx.DefineFloatConstant(value);
    const auto intermediate_move =
        "movss " + register_to_asm(src) + ", " + "[rel " + f_value_label + "]";
    ctx.AddInstruction(intermediate_move);
    const auto move_to_stack =
        "movss " + source_prefix + stack_location_at_asm(dst) + ", " + register_to_asm(src);
    ctx.AddInstruction(move_to_stack);
}

auto Store::to_asm(CodegenContext& ctx) const -> void {
    const auto register_source = std::get<target::HardcodedRegister>(src);
    const auto source_prefix =
        register_source.size == 4 ? std::string("dword") : std::string("qword");
    if (is_float_register(register_source.reg)) {
        const auto ins =
            "movss " + source_prefix + stack_location_at_asm(dst) + ", " + register_to_asm(src);
        ctx.AddInstruction(ins);
    } else {
        const auto ins =
            "mov " + source_prefix + stack_location_at_asm(dst) + ", " + register_to_asm(src);
        ctx.AddInstruction(ins);
    }
}

auto JumpGreater::to_asm(CodegenContext& ctx) const -> void {
    const auto ins = "jg ." + label;
    ctx.AddInstruction(ins);
}

auto JumpLess::to_asm(CodegenContext& ctx) const -> void {
    const auto ins = "jl ." + label;
    ctx.AddInstruction(ins);
}

auto Jump::to_asm(CodegenContext& ctx) const -> void {
    const auto ins = "jmp ." + label;
    ctx.AddInstruction(ins);
}

auto JumpEq::to_asm(CodegenContext& ctx) const -> void {
    const auto ins = "je ." + label;
    ctx.AddInstruction(ins);
}

auto JumpLessThanEqual::to_asm(CodegenContext& ctx) const -> void {
    const auto ins = "jle ." + label;
    ctx.AddInstruction(ins);
}

auto AddI::to_asm(CodegenContext& ctx) const -> void {
    const auto ins = "add " + register_to_asm(dst) + ", " + std::to_string(value);
    ctx.AddInstruction(ins);
}

auto AddM::to_asm(CodegenContext& ctx) const -> void {
    const auto ins = "add " + stack_location_at_asm(dst) + ", " + register_to_asm(src);
    ctx.AddInstruction(ins);
}

auto MultM::to_asm(CodegenContext& ctx) const -> void {
    const auto ins = "imul " + stack_location_at_asm(dst) + ", " + register_to_asm(src);
    ctx.AddInstruction(ins);
}

auto MultI::to_asm(CodegenContext& ctx) const -> void {
    const auto ins = "imul " + register_to_asm(dst) + ", " + std::to_string(value);
    ctx.AddInstruction(ins);
}

auto AddMI::to_asm(CodegenContext& ctx) const -> void {
    std::string source_prefix = "dword";
    const auto ins =
        "add " + source_prefix + stack_location_at_asm(dst) + ", " + to_asm_constant(value);
    ctx.AddInstruction(ins);
}

auto SubI::to_asm(CodegenContext& ctx) const -> void {
    const auto ins = "sub " + register_to_asm(dst) + ", " + std::to_string(value);
    ctx.AddInstruction(ins);
}

auto Add::to_asm(CodegenContext& ctx) const -> void {
    const auto hardcoded_dst = std::get<HardcodedRegister>(dst);
    if (is_float_register(hardcoded_dst.reg)) {
        const auto ins = "addss " + register_to_asm(dst) + ", " + register_to_asm(src);
        ctx.AddInstruction(ins);
    } else {
        const auto ins = "add " + register_to_asm(dst) + ", " + register_to_asm(src);
        ctx.AddInstruction(ins);
    }
}

auto Sub::to_asm(CodegenContext& ctx) const -> void {
    const auto hardcoded_dst = std::get<HardcodedRegister>(dst);
    if (is_float_register(hardcoded_dst.reg)) {
        const auto ins = "subss " + register_to_asm(dst) + ", " + register_to_asm(src);
        ctx.AddInstruction(ins);
    } else {
        const auto ins = "sub " + register_to_asm(dst) + ", " + register_to_asm(src);
        ctx.AddInstruction(ins);
    }
}

auto Mul::to_asm(CodegenContext& ctx) const -> void {
    const auto hardcoded_dst = std::get<HardcodedRegister>(dst);
    if (is_float_register(hardcoded_dst.reg)) {
        const auto ins = "mulss " + register_to_asm(dst) + ", " + register_to_asm(src);
        ctx.AddInstruction(ins);
    } else {
        const auto ins = "imul " + register_to_asm(dst) + ", " + register_to_asm(src);
        ctx.AddInstruction(ins);
    }
}

auto MulRegRegInt::to_asm(CodegenContext& ctx) const -> void {
    const auto hardcoded_dst = std::get<HardcodedRegister>(dst);
    if (is_float_register(hardcoded_dst.reg)) {
        const auto ins = "mulss " + register_to_asm(dst) + ", " + register_to_asm(src);
        ctx.AddInstruction(ins);
    } else {
        const auto ins = "imul " + register_to_asm(dst) + ", " + register_to_asm(src) + ", " +
                         std::to_string(value);
        ctx.AddInstruction(ins);
    }
}

auto IDiv::to_asm(CodegenContext& ctx) const -> void {
    // const auto hardcoded_dst = std::get<HardcodedRegister>(dst);
    const auto ins = "idiv " + register_to_asm(src);
    ctx.AddInstruction(ins);
}

auto CmpI::to_asm(CodegenContext& ctx) const -> void {
    const auto ins = "cmp " + register_to_asm(dst) + ", " + to_asm_constant(value);
    ctx.AddInstruction(ins);
}

auto CmpMI::to_asm(CodegenContext& ctx) const -> void {
    const auto source_prefix = std::string("dword");
    const auto ins =
        "cmp " + source_prefix + stack_location_at_asm(dst) + ", " + to_asm_constant(value);
    ctx.AddInstruction(ins);
}

auto CmpF::to_asm(CodegenContext& ctx) const -> void {
    const auto ins = "comiss " + register_to_asm(dst) + ", " + register_to_asm(src);
    ctx.AddInstruction(ins);
}

auto Cmp::to_asm(CodegenContext& ctx) const -> void {
    const auto ins = "cmp " + register_to_asm(dst) + ", " + register_to_asm(src);
    ctx.AddInstruction(ins);
}

auto SetNA::to_asm(CodegenContext& ctx) const -> void {
    std::string result = "\tsetna al\n\t";

    result += "movzx ";
    result += register_to_asm(dst);
    result += ", al";
    ctx.AddInstruction(result);
}

auto SetA::to_asm(CodegenContext& ctx) const -> void {
    std::string result = "\tseta al\n\t";

    result += "movzx ";
    result += register_to_asm(dst);
    result += ", al";
    ctx.AddInstruction(result);
}

auto SetNB::to_asm(CodegenContext& ctx) const -> void {
    std::string result = "\tsetnb al\n\t";

    result += "movzx ";
    result += register_to_asm(dst);
    result += ", al";
    ctx.AddInstruction(result);
}

auto SetB::to_asm(CodegenContext& ctx) const -> void {
    std::string result = "\tsetb al\n\t";

    result += "movzx ";
    result += register_to_asm(dst);
    result += ", al";
    ctx.AddInstruction(result);
}

auto SetEAl::to_asm(CodegenContext& ctx) const -> void {
    std::string result = "\tsete al\n\t";

    result += "movzx ";
    result += register_to_asm(dst);
    result += ", al";
    ctx.AddInstruction(result);
}

auto SetGAl::to_asm(CodegenContext& ctx) const -> void {
    std::string result = "\tsetg al\n\t";

    result += "movzx ";
    result += register_to_asm(dst);
    result += ", al";
    ctx.AddInstruction(result);
}

auto SetNeAl::to_asm(CodegenContext& ctx) const -> void {
    std::string result = "\tsetne al\n\t";

    result += "movzx ";
    result += register_to_asm(dst);
    result += ", al";
    ctx.AddInstruction(result);
}

auto SetLAl::to_asm(CodegenContext& ctx) const -> void {
    std::string result = "\tsetl al\n\t";

    result += "movzx ";
    result += register_to_asm(dst);
    result += ", al";
    ctx.AddInstruction(result);
}

auto SetLeAl::to_asm(CodegenContext& ctx) const -> void {
    std::string result = "\tsetle al\n\t";

    result += "movzx ";
    result += register_to_asm(dst);
    result += ", al";
    ctx.AddInstruction(result);
}

auto SetGeAl::to_asm(CodegenContext& ctx) const -> void {
    std::string result = "\tsetge al\n\t";

    result += "movzx ";
    result += register_to_asm(dst);
    result += ", al";
    ctx.AddInstruction(result);
}

auto Label::to_asm(CodegenContext& ctx) const -> void {
    const auto label = "." + name + ":";
    ctx.AddInstructionNoIndent(label);
}

auto Call::to_asm(CodegenContext& ctx) const -> void {
    const auto ins = "call " + name;
    ctx.AddInstruction(ins);
}

auto Lea::to_asm(CodegenContext& ctx) const -> void {
    const auto ins = "lea " + register_to_asm(dst) + ", " + stack_location_at_asm(src);
    ctx.AddInstruction(ins);
}

auto IndirectLoad::to_asm(CodegenContext& ctx) const -> void {
    const auto hardcoded_dest = std::get<HardcodedRegister>(dst);
    if (is_float_register(hardcoded_dest)) {
        const auto ins = "movss " + register_to_asm(dst) + ", [" + register_to_asm(src) + "]";
        ctx.AddInstruction(ins);
    } else {
        const auto ins = "mov " + register_to_asm(dst) + ", [" + register_to_asm(src) + "]";
        ctx.AddInstruction(ins);
    }
}

auto IndirectStore::to_asm(CodegenContext& ctx) const -> void {
    const auto ins = "mov [" + register_to_asm(dst) + "], " + register_to_asm(src);
    ctx.AddInstruction(ins);
}

auto PushI::to_asm(CodegenContext& ctx) const -> void {
    const auto ins = "push " + to_asm_constant(src);
    ctx.AddInstruction(ins);
}

auto Push::to_asm(CodegenContext& ctx) const -> void {
    auto source_reg = std::get<HardcodedRegister>(src);
    // can't push registers that aren't 8 bytes
    source_reg.size = 8;
    const auto ins = "push " + register_to_asm(source_reg);
    ctx.AddInstruction(ins);
}

auto CDQ::to_asm(CodegenContext& ctx) const -> void {
    const auto ins = "cdq";
    ctx.AddInstruction(ins);
}

}  // namespace target
