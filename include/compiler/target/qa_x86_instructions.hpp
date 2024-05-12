#pragma once

#include "codegenCtx.hpp"
#include "qa_x86_locations.hpp"
#include "qa_x86_registers.hpp"

namespace target {

template <typename T>
concept HasToAsmMethod = requires(T t, CodegenContext& ctx) {
    { t.to_asm(ctx) };
};

auto to_asm_constant(int value) -> std::string;

struct x86Instruction {
    virtual auto to_asm(CodegenContext& ctx) const -> void = 0;
    virtual ~x86Instruction() = default;

    virtual auto debug_str() const -> std::string { return "x86Instruction"; }
};

template <typename T>
struct ImmediateLoad : x86Instruction {
    Register dst;
    T value;

    ImmediateLoad(Register p_dst, T p_value) : dst(p_dst), value(p_value) {}
    auto to_asm(CodegenContext& ctx) const -> void {
        const auto hardcoded_dest = std::get<HardcodedRegister>(dst);
        if (is_float_register(hardcoded_dest.reg)) {
            const auto float_label = ctx.DefineFloatConstant(value);
            const auto ins = "movss " + register_to_asm(dst) + ", " + "[rel " + float_label + "]";
            ctx.AddInstruction(ins);
        } else {
            const auto ins = "mov " + register_to_asm(dst) + ", " + to_asm_constant(value);
            ctx.AddInstruction(ins);
        }
    }

    auto debug_str() const -> std::string override {
        return "ImmediateLoad<" + std::to_string(value) + ">";
    }
};

struct Mov : public x86Instruction {
    Register dst;
    Register src;

    Mov(Register p_dst, Register p_src) : dst(p_dst), src(p_src) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "Mov<" + register_to_asm(dst) + ", " + register_to_asm(src) + ">";
    }
};

struct Load : public x86Instruction {
    Register dst;
    StackLocation src;

    Load(Register p_dst, StackLocation p_src) : dst(p_dst), src(p_src) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "Load<" + register_to_asm(dst) + ", " + stack_location_at_asm(src) + ">";
    }
};

struct ZeroExtend : public x86Instruction {
    Register dst;
    Register src;

    ZeroExtend(Register p_dst, Register p_src) : dst(p_dst), src(p_src) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "ZeroExtend<" + register_to_asm(dst) + ", " + register_to_asm(src) + ">";
    }
};

struct StoreI : public x86Instruction {
    StackLocation dst;
    int value;
    StoreI(StackLocation p_dst, int p_value) : dst(p_dst), value(p_value) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "StoreI<" + stack_location_at_asm(dst) + ", " + std::to_string(value) + ">";
    }
};

struct StoreF : public x86Instruction {
    StackLocation dst;
    Register src;
    float value;
    StoreF(StackLocation p_dst, Register p_src, float p_value)
        : dst(p_dst), src(p_src), value(p_value) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "StoreF<" + stack_location_at_asm(dst) + ", " + register_to_asm(src) + ", " +
               std::to_string(value) + ">";
    }
};

struct Store : public x86Instruction {
    StackLocation dst;
    Register src;

    Store(StackLocation p_dst, Register p_src) : dst(p_dst), src(p_src) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "Store<" + stack_location_at_asm(dst) + ", " + register_to_asm(src) + ">";
    }
};

struct JumpGreater : public x86Instruction {
    std::string label;

    JumpGreater(std::string p_label) : label(p_label) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override { return "JumpGreater<" + label + ">"; }
};

struct JumpLess : public x86Instruction {
    std::string label;
    JumpLess(std::string p_label) : label(p_label) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override { return "JumpLess<" + label + ">"; }
};

struct Jump : public x86Instruction {
    std::string label;

    Jump(std::string p_label) : label(p_label) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override { return "Jump<" + label + ">"; }
};

struct JumpEq : public x86Instruction {
    std::string label;

    JumpEq(std::string p_label) : label(p_label) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override { return "JumpEq<" + label + ">"; }
};

struct AddI : public x86Instruction {
    Register dst;
    int value;

    AddI(Register p_dst, int p_value) : dst(p_dst), value(p_value) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "AddI<" + register_to_asm(dst) + ", " + std::to_string(value) + ">";
    }
};

struct AddMI : public x86Instruction {
    StackLocation dst;
    int value;

    AddMI(StackLocation p_dst, int p_value) : dst(p_dst), value(p_value) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "AddMI<" + stack_location_at_asm(dst) + ", " + std::to_string(value) + ">";
    }
};

struct SubI : public x86Instruction {
    Register dst;
    int value;

    SubI(Register p_dst, int p_value) : dst(p_dst), value(p_value) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "SubI<" + register_to_asm(dst) + ", " + std::to_string(value) + ">";
    }
};

struct Add : public x86Instruction {
    Register dst;
    Register src;

    Add(Register p_dst, Register p_src) : dst(p_dst), src(p_src) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "Add<" + register_to_asm(dst) + ", " + register_to_asm(src) + ">";
    }
};

struct Sub : public x86Instruction {
    Register dst;
    Register src;

    Sub(Register p_dst, Register p_src) : dst(p_dst), src(p_src) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "Sub<" + register_to_asm(dst) + ", " + register_to_asm(src) + ">";
    }
};

struct CmpM : public x86Instruction {
    // TODO: not really dest / src
    Register dst;
    StackLocation src;

    CmpM(Register p_dst, StackLocation p_src) : dst(p_dst), src(p_src) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "Cmp<" + register_to_asm(dst) + ", " + stack_location_at_asm(src) + ">";
    }
};

struct Cmp : public x86Instruction {
    // TODO: not really dest / src
    Register dst;
    Register src;

    Cmp(Register p_dst, Register p_src) : dst(p_dst), src(p_src) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "Cmp<" + register_to_asm(dst) + ", " + register_to_asm(src) + ">";
    }
};

struct CmpI : public x86Instruction {
    Register dst;
    int value;

    CmpI(Register p_dst, int p_value) : dst(p_dst), value(p_value) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "CmpI<" + register_to_asm(dst) + ", " + std::to_string(value) + ">";
    }
};

struct CmpF : public x86Instruction {
    Register dst;
    Register src;

    CmpF(Register p_dst, Register p_src) : dst(p_dst), src(p_src) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "CmpF<" + register_to_asm(dst) + ", " + register_to_asm(src) + ">";
    }
};

struct SetA : public x86Instruction {
    Register dst;

    SetA(Register p_dst) : dst(p_dst) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override { return "SetA<" + register_to_asm(dst) + ">"; }
};

struct SetEAl : public x86Instruction {
    Register dst;

    SetEAl(Register p_dst) : dst(p_dst) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "SetEAl<" + register_to_asm(dst) + ">";
    }
};

struct SetGAl : public x86Instruction {
    Register dst;

    SetGAl(Register p_dst) : dst(p_dst) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "SetGAl<" + register_to_asm(dst) + ">";
    }
};

struct SetNeAl : public x86Instruction {
    Register dst;

    SetNeAl(Register p_dst) : dst(p_dst) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "SetNeAl<" + register_to_asm(dst) + ">";
    }
};

struct SetLAl : public x86Instruction {
    Register dst;

    SetLAl(Register p_dst) : dst(p_dst) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "SetLAl<" + register_to_asm(dst) + ">";
    }
};

struct Label : public x86Instruction {
    std::string name;

    Label(std::string p_name) : name(p_name) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override { return "Label<" + name + ">"; }
};

struct Call : public x86Instruction {
    std::string name;
    Register dst;

    Call(std::string p_name, Register p_dst) : name(p_name), dst(p_dst) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "Call<" + name + ", " + register_to_asm(dst) + ">";
    }
};

struct Lea : public x86Instruction {
    Register dst;
    StackLocation src;

    Lea(Register p_dst, StackLocation p_src) : dst(p_dst), src(p_src) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "Lea<" + register_to_asm(dst) + ", " + stack_location_at_asm(src) + ">";
    }
};

struct IndirectLoad : public x86Instruction {
    Register dst;
    Register src;

    IndirectLoad(Register p_dst, Register p_src) : dst(p_dst), src(p_src) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "IndirectLoad<" + register_to_asm(dst) + ", " + register_to_asm(src) + ">";
    }
};

struct IndirectStore : public x86Instruction {
    Register dst;
    Register src;

    IndirectStore(Register p_dst, Register p_src) : dst(p_dst), src(p_src) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override {
        return "IndirectStore<" + register_to_asm(dst) + ", " + register_to_asm(src) + ">";
    }
};

struct PushI : public x86Instruction {
    int src;

    PushI(int p_src) : src(p_src) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override { return "PushI<" + std::to_string(src) + ">"; }
};

struct Push : public x86Instruction {
    Register src;

    Push(Register p_src) : src(p_src) {}
    auto to_asm(CodegenContext& ctx) const -> void;
    auto debug_str() const -> std::string override { return "Push<" + register_to_asm(src) + ">"; }
};

using Instruction =
    std::variant<Mov, ImmediateLoad<int>, StoreI, Store, Load, Jump, AddI, Add, SubI, Sub, Cmp,
                 CmpI, CmpF, SetEAl, SetGAl, Label, JumpEq, Call, Lea, IndirectLoad, JumpGreater,
                 IndirectStore, PushI, Push, JumpLess, SetNeAl, SetLAl, ZeroExtend,
                 ImmediateLoad<float>, StoreF, SetA, CmpM>;

}  // namespace target