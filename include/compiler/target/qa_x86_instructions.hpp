#pragma once

#include "codegenCtx.hpp"

#include "qa_x86_locations.hpp"
#include "qa_x86_registers.hpp"

namespace target {

template<typename T>
concept HasToAsmMethod = requires(T t, CodegenContext& ctx) {
    { t.to_asm(ctx) } ;
};


auto to_asm_constant(int value) -> std::string;



template <typename T>
struct ImmediateLoad  {
    Register dst;
    T value;
    void* src = nullptr;

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
} ; 

struct Mov {
    Register dst;
    Register src;

     auto to_asm(CodegenContext& ctx) const -> void ; 
};


struct Load {
    Register dst;
    StackLocation src;

     auto to_asm(CodegenContext& ctx) const -> void ; 
};

struct ZeroExtend {
    Register dst;
    Register src;


    auto to_asm(CodegenContext& ctx) const -> void ; 
};

struct StoreI {
    StackLocation dst;
    void *src;
    int value;

     auto to_asm(CodegenContext& ctx) const -> void ; 
};

struct StoreF {
    StackLocation dst;
    Register src;
    float value;

     auto to_asm(CodegenContext& ctx) const -> void ; 
};




struct Store {
    StackLocation dst;
    Register src;

     auto to_asm(CodegenContext& ctx) const -> void ; 
};

struct JumpGreater {
    std::string label;
    void* src = nullptr;
    void* dst = nullptr;

     auto to_asm(CodegenContext& ctx) const -> void ; 
};

struct JumpLess {
    std::string label;
    void* src = nullptr;
    void* dst = nullptr;

     auto to_asm(CodegenContext& ctx) const -> void ; 
};

struct Jump {
    std::string label;
    void* src = nullptr;
    void* dst = nullptr;

     auto to_asm(CodegenContext& ctx) const -> void; 
};

struct JumpEq {
    std::string label;
    void* src = nullptr;
    void* dst = nullptr;

     auto to_asm(CodegenContext& ctx) const -> void; 
};



struct AddI {
    Register dst;
    int value;
    void* src = nullptr;

     auto to_asm(CodegenContext& ctx) const -> void ; 
};






struct SubI {
    Register dst;
    int value;
    void* src = nullptr;

     auto to_asm(CodegenContext& ctx) const -> void ; 
};

struct Add {
    Register dst;
    Register src;

     auto to_asm(CodegenContext& ctx) const -> void; 
};

struct Sub {
    Register dst;
    Register src;

     auto to_asm(CodegenContext& ctx) const -> void; 
};

struct Cmp {
    // TODO: not really dest / src
    Register dst;
    Register src;

     auto to_asm(CodegenContext& ctx) const -> void ; 
};



struct CmpI {
    Register dst;
    void* src = nullptr;
    int value;
    

     auto to_asm(CodegenContext& ctx) const -> void; 

}; 

struct CmpF {
    Register dst;
    Register src; 


     auto to_asm(CodegenContext& ctx) const -> void ; 
}; 

struct SetA {
    Register dst;
    void* src = nullptr;

     auto to_asm(CodegenContext& ctx) const -> void; 
};

struct SetEAl {
    Register dst;
    void* src = nullptr;

     auto to_asm(CodegenContext& ctx) const -> void; 
};

struct SetGAl {
    Register dst;
    void* src = nullptr;

     auto to_asm(CodegenContext& ctx) const -> void ; 
};

struct SetNeAl {
    Register dst;
    void* src = nullptr;

     auto to_asm(CodegenContext& ctx) const -> void ; 
};

struct SetLAl {
    Register dst;
    void* src = nullptr;


     auto to_asm(CodegenContext& ctx) const -> void; 
};

struct Label {
    std::string name;
    void* src = nullptr;
    void* dst = nullptr;

     auto to_asm(CodegenContext& ctx) const -> void ; 
};

struct Call {
    std::string name;
    Register dst;
    void* src = nullptr;

     auto to_asm(CodegenContext& ctx) const -> void ; 
};

struct Lea {
    Register dst;
    StackLocation src;

     auto to_asm(CodegenContext& ctx) const -> void ; 
};

struct IndirectLoad {
    Register dst;
    Register src;

     auto to_asm(CodegenContext& ctx) const -> void ; 

};

struct IndirectStore {
    Register dst;
    Register src;

     auto to_asm(CodegenContext& ctx) const -> void; 
};

struct PushI {
    int src;
    void* dst = nullptr;

     auto to_asm(CodegenContext& ctx) const -> void ; 
};

struct Push {
    Register src;
    void* dst = nullptr;

     auto to_asm(CodegenContext& ctx) const -> void; 
}; 


using Instruction = std::variant<Mov, ImmediateLoad<int>, StoreI, Store, Load, Jump, AddI, Add, SubI, Sub,  Cmp,
                 CmpI,  CmpF, SetEAl, SetGAl, Label, JumpEq, Call, Lea, IndirectLoad, JumpGreater,
                 IndirectStore, PushI, Push, JumpLess, SetNeAl, SetLAl, ZeroExtend, ImmediateLoad<float>, StoreF, SetA>;

}