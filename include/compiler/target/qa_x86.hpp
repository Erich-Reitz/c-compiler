#pragma once

#include <concepts>
#include <iostream>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace target {

int sixteenByteAlign(int size) ; 

const int address_size = 8;

// all registers used.
// x86-64 registers
enum class BaseRegister { AX, BX, CX, DX, SI, DI, R8, R9, R10, R11, R12, R13, R14, R15 };

struct HardcodedRegister {
    BaseRegister reg;
    int size;
};

[[nodiscard]] auto param_register_by_convention(int idx, int size) -> HardcodedRegister;


// six system V calling convention registers
inline const std::vector<BaseRegister> param_regs = {BaseRegister::DI, BaseRegister::SI,
                                                     BaseRegister::DX, BaseRegister::CX,
                                                     BaseRegister::R8, BaseRegister::R9};
// general purpose registers
// these are disjoint from the param_regs, so that calls don't clobber them
inline const std::vector<BaseRegister> general_regs = {
    BaseRegister::AX,  BaseRegister::BX,  BaseRegister::R10, BaseRegister::R11,
    BaseRegister::R12, BaseRegister::R13, BaseRegister::R14, BaseRegister::R15};

bool operator==(const HardcodedRegister& lhs, const HardcodedRegister& rhs);

struct VirtualRegister {
    int id;
    int size;
};


using Register = std::variant<HardcodedRegister, VirtualRegister>;

struct StackLocation {
    int offset = 0; 
    bool is_computed = false;
    Register src = {}; 
    int scale = 0; 
};


[[nodiscard]] std::string register_to_asm(Register reg);
[[nodiscard]] std::string stack_location_at_asm(StackLocation sl) ; 




std::ostream& operator<<(std::ostream& os, const StackLocation& loc);


// comparison operators for registers
bool operator<(const Register& lhs, const Register& rhs);

using Location = std::variant<Register, StackLocation>;

int SizeOf(const Location& loc);

template<typename T>
concept HasToAsmMethod = requires(T t) {
    { t.to_asm() } -> std::convertible_to<std::string>;
};

struct Mov {
    Register dst;
    Register src;

    [[nodiscard]] auto to_asm() const -> std::string {
        const auto d = std::get<HardcodedRegister>(dst); 
        const auto s = std::get<HardcodedRegister>(src); 
        if (d == s) return ""; 

        return "\tmov " + register_to_asm(dst) + ", " + register_to_asm(src); 
    }
};

template <typename T>
struct ImmediateLoad  {
    Register dst;
    T value;
    void* src = nullptr;

    [[nodiscard]] auto to_asm() const -> std::string {
        return "\tmov " + register_to_asm(dst) + ", " + std::to_string(value); 
    }; 
};



struct Load {
    Register dst;
    StackLocation src;

    [[nodiscard]] auto to_asm() const -> std::string {
        const auto register_dst = std::get<target::HardcodedRegister>(dst);
        const auto source_prefix = register_dst.size == 4 ? std::string("dword") : std::string("qword");
        return "\tmov " + register_to_asm(dst) + ", " + source_prefix +  stack_location_at_asm(src);  
    }; 
};

struct ZeroExtend {
    Register dst;
    Register src;

    
   [[nodiscard]] auto to_asm() const -> std::string {
        return "\tmovsx " +  register_to_asm(dst) + ", " + register_to_asm(src); 
    }; 
};

template <typename T>
struct StoreImmediate {
    StackLocation dst;
    T value;
    void* src = nullptr;

    [[nodiscard]] auto to_asm() const -> std::string {
        const std::string source_prefix = "dword "; 
        return "\tmov " + source_prefix +  stack_location_at_asm(dst) + ", " + std::to_string(value); 
    }; 
};





struct Store {
    StackLocation dst;
    Register src;

    [[nodiscard]] auto to_asm() const -> std::string {
        const auto register_source = std::get<target::HardcodedRegister>(src);

        const auto source_prefix = register_source.size == 4 ? std::string("dword") : std::string("qword");
        return "\tmov " + source_prefix +  stack_location_at_asm(dst) + ", " + register_to_asm(register_source); 
    }; 
};

struct JumpGreater {
    std::string label;
    void* src = nullptr;
    void* dst = nullptr;

    [[nodiscard]] auto to_asm() const -> std::string {
        return "\tjg ." + label; 
    }    
};

struct JumpLess {
    std::string label;
    void* src = nullptr;
    void* dst = nullptr;

    [[nodiscard]] auto to_asm() const -> std::string {
        return "\tjl ." + label; 
    }
};

struct Jump {
    std::string label;
    void* src = nullptr;
    void* dst = nullptr;

    [[nodiscard]] auto to_asm() const -> std::string {
        return "\tjmp ." + label; 
    }
};

struct JumpEq {
    std::string label;
    void* src = nullptr;
    void* dst = nullptr;

    [[nodiscard]] auto to_asm() const -> std::string {
        return "\tje ." + label; 
    }
};



struct AddI {
    Register dst;
    int value;
    void* src = nullptr;

    [[nodiscard]] auto to_asm() const -> std::string {
        return "\tadd " + register_to_asm(dst) + ", " + std::to_string(value); 
    }; 
};






struct SubI {
    Register dst;
    int value;
    void* src = nullptr;

    [[nodiscard]] auto to_asm() const -> std::string {
        return "\tsub " + register_to_asm(dst) + ", " + std::to_string(value); 
    }; 
};

struct Add {
    Register dst;
    Register src;

    [[nodiscard]] auto to_asm() const -> std::string {
        return "\tadd " + register_to_asm(dst) + ", " + register_to_asm(src);  
    }; 
};

struct Sub {
    Register dst;
    Register src;

    [[nodiscard]] auto to_asm() const -> std::string {
        return "\tsub " + register_to_asm(dst) + ", " + register_to_asm(src);  
    }; 
};

struct Cmp {
    // TODO: not really dest / src
    Register dst;
    Register src;

    [[nodiscard]] auto to_asm() const -> std::string {
        return "\tcmp " + register_to_asm(dst) + ", " + register_to_asm(src);  
    }; 
};


template <typename T>
struct CmpImmediate {
    Register dst;
    T value;
    void* src = nullptr;

    [[nodiscard]] auto to_asm() const -> std::string {
        return "\tcmp " + register_to_asm(dst) + ", " + std::to_string(value); 
    }; 
};


struct SetEAl {
    Register dst;
    void* src = nullptr;

    [[nodiscard]] auto to_asm() const -> std::string {
        std::string result = "\tsete al\n\t"; 

        result += "movzx "; 
        result += register_to_asm(dst); 
        result += ", al"; 
        return result;
    }; 
};

struct SetGAl {
    Register dst;
    void* src = nullptr;

    [[nodiscard]] auto to_asm() const -> std::string {
        std::string result = "\tsetg al\n\t"; 

        result += "movzx "; 
        result += register_to_asm(dst); 
        result += ", al"; 
        return result;
    }; 
};

struct SetNeAl {
    Register dst;
    void* src = nullptr;

    [[nodiscard]] auto to_asm() const -> std::string {
        std::string result = "\tsetne al\n\t"; 

        result += "movzx "; 
        result += register_to_asm(dst); 
        result += ", al"; 
        return result;
    }; 
};

struct SetLAl {
    Register dst;
    void* src = nullptr;


    [[nodiscard]] auto to_asm() const -> std::string {
        std::string result = "\tsetl al\n\t"; 

        result += "movzx "; 
        result += register_to_asm(dst); 
        result += ", al"; 
        return result;
    }; 
};

struct Label {
    std::string name;
    void* src = nullptr;
    void* dst = nullptr;

    [[nodiscard]] auto to_asm() const -> std::string {
        return "." + name + ":"; 
    }
};

struct Call {
    std::string name;
    Register dst;
    void* src = nullptr;

    [[nodiscard]] auto to_asm() const -> std::string {
        return "\tcall " + name;
    }
};

struct Lea {
    Register dst;
    StackLocation src;

    [[nodiscard]] auto to_asm() const -> std::string {
        return "\tlea " + register_to_asm(dst) + ", " + stack_location_at_asm(src); 
    }
};

struct IndirectLoad {
    Register dst;
    Register src;
    
    [[nodiscard]] auto to_asm() const -> std::string {
        return "\tmov " + register_to_asm(dst) + ", [" + register_to_asm(src) + "]"; 
    }

};

struct IndirectStore {
    Register dst;
    Register src;

    [[nodiscard]] auto to_asm() const -> std::string {
        return "\tmov [" + register_to_asm(dst) + "], " + register_to_asm(src); 
    }
};

struct PushI {
    int src;
    void* dst = nullptr;
    
    [[nodiscard]] auto to_asm() const -> std::string {
        return "\tpush " + std::to_string(src); 
    }
};

struct Push {
    Register src;
    void* dst = nullptr;

    [[nodiscard]] auto to_asm() const -> std::string {
        auto source_reg = std::get<HardcodedRegister>(src); 
        // can't push registers that aren't 8 bytes
        source_reg.size = 8; 
        return "\tpush " + register_to_asm(source_reg); 
    }    
};

using Instruction =
    std::variant<Mov, ImmediateLoad<int>, StoreImmediate<int>, Store, Load, Jump, AddI, Add, SubI, Sub,  Cmp,
                 CmpImmediate<int>,  CmpImmediate<float>, SetEAl, SetGAl, Label, JumpEq, Call, Lea, IndirectLoad, JumpGreater,
                 IndirectStore, PushI, Push, JumpLess, SetNeAl, SetLAl, ZeroExtend, ImmediateLoad<float>, StoreImmediate<float>>;

std::optional<int> get_src_virtual_id_if_present(const Instruction& ins);
std::optional<int> get_dest_virtual_id_if_present(const Instruction& ins);
std::optional<VirtualRegister> get_src_register(const Instruction& ins);
std::optional<VirtualRegister> get_dest_register(const Instruction& ins);
void set_src_register(Instruction& ins, Register reg);
void set_dest_register(Instruction& ins, Register reg);

template <typename T>
concept HasRegisterSrc = requires(T t) {
    { t.src } -> std::convertible_to<Register>;
};

template <typename T>
concept HasRegisterDest = requires(T t) {
    { t.dst } -> std::convertible_to<Register>;
};

struct Frame {
    std::string name;
    std::vector<Instruction> instructions;
    int size = 0;
};

}  // namespace target