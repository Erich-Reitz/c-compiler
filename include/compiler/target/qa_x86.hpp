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

std::ostream& operator<<(std::ostream& os, BaseRegister reg);

// six system V calling convention registers
inline const std::vector<BaseRegister> param_regs = {BaseRegister::DI, BaseRegister::SI,
                                                     BaseRegister::DX, BaseRegister::CX,
                                                     BaseRegister::R8, BaseRegister::R9};
// general purpose registers
// these are disjoint from the param_regs, so that calls don't clobber them
inline const std::vector<BaseRegister> general_regs = {
    BaseRegister::AX,  BaseRegister::BX,  BaseRegister::R10, BaseRegister::R11,
    BaseRegister::R12, BaseRegister::R13, BaseRegister::R14, BaseRegister::R15};

[[nodiscard]] std::string to_asm(HardcodedRegister reg);

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

std::ostream& operator<<(std::ostream& os, const StackLocation& loc);


// comparison operators for registers
bool operator<(const Register& lhs, const Register& rhs);

using Location = std::variant<Register, StackLocation>;

int SizeOf(const Location& loc);

struct Mov {
    Register dst;
    Register src;
};




template<typename T>
concept HasToAsmMethod = requires(T t) {
    { t.to_asm() } -> std::convertible_to<std::string>;
};

struct LoadI  {
    Register dst;
    int value;
    void* src = nullptr;
};

struct LoadF {
    Register dst;
    float value;
    void* src = nullptr;
};

struct Load {
    Register dst;
    StackLocation src;
};

struct ZeroExtend {
    Register dst;
    Register src;
};

struct StoreI {
    StackLocation dst;
    int value;
    void* src = nullptr;
};

struct StoreF {
    StackLocation dst;
    float value;
    void* src = nullptr;
};


struct Store {
    StackLocation dst;
    Register src;
};

struct JumpGreater {
    std::string label;
    void* src = nullptr;
    void* dst = nullptr;
};

struct JumpLess {
    std::string label;
    void* src = nullptr;
    void* dst = nullptr;
};

struct Jump {
    std::string label;
    void* src = nullptr;
    void* dst = nullptr;
};

struct JumpEq {
    std::string label;
    void* src = nullptr;
    void* dst = nullptr;
};

struct AddI {
    Register dst;
    int value;
    void* src = nullptr;
};

struct AddMI {
    StackLocation dst;
    int value;
    void* src = nullptr;
};

struct SubMI {
    StackLocation dst;
    int value;
    void* src = nullptr;
};

struct SubI {
    Register dst;
    int value;
    void* src = nullptr;
};

struct Add {
    Register dst;
    Register src;
};

struct Sub {
    Register dst;
    Register src;
};

struct Cmp {
    // TODO: not really dest / src
    Register dst;
    Register src;
};

struct CmpI {
    // TODO: not really dest..
    Register dst;
    int value;
    void* src = nullptr;
};

struct CmpF {
    // TODO: not really dest..
    Register dst;
    float value;
    void* src = nullptr;
};

struct SetEAl {
    Register dst;
    void* src = nullptr;
};

struct SetGAl {
    Register dst;
    void* src = nullptr;
};

struct SetNeAl {
    Register dst;
void* src = nullptr;
    };

struct SetLAl {
    Register dst;
    void* src = nullptr;
};

struct Label {
    std::string name;
    void* src = nullptr;
    void* dst = nullptr;
};

struct Call {
    std::string name;
    Register dst;
    void* src = nullptr;
};

struct Lea {
    Register dst;
    StackLocation src;
};

struct IndirectLoad {
    Register dst;
    Register src;
};

struct IndirectStore {
    Register dst;
    Register src;
};

struct PushI {
    int src;
    void* dst = nullptr;
};

struct Push {
    Register src;
    void* dst = nullptr;
};

using Instruction = std::variant<Mov, LoadI, StoreI, Store, Load, Jump, AddI, Add, SubI, Sub, AddMI, SubMI, Cmp,
                 CmpI, SetEAl, SetGAl, Label, JumpEq, Call, Lea, IndirectLoad, JumpGreater,
                 IndirectStore, PushI, Push, JumpLess, SetNeAl, SetLAl, ZeroExtend, LoadF, StoreF, CmpF>;

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