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

[[nodiscard]] std::string to_asm(BaseRegister reg, int size);

bool operator==(const HardcodedRegister& lhs, const HardcodedRegister& rhs);

struct VirtualRegister {
    int id;
    int size;
};
using Register = std::variant<HardcodedRegister, VirtualRegister>;

struct StackLocation {
    int offset;
    bool is_computed = false;
    Register src;
    int scale;
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



[[nodiscard]] std::string to_asm(const Mov& mov);

struct LoadI {
    Register dst;
    int value;
    void *src;
    
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
    void* src;
};

struct Store {
    StackLocation dst;
    Register src;
};

struct JumpGreater {
    std::string label;
    void* src;
    void *dst;
};

struct JumpLess {
    std::string label;
    void* src;
    void *dst;
};

struct Jump {
    std::string label;
    void* src;
    void *dst;
};

struct JumpEq {
    std::string label;
    void* src;
    void *dst;
};

struct AddI {
    Register dst;
    int value;
    void* src;
};

struct AddMI {
    StackLocation dst;
    int value;
    void* src;
};

struct SubMI {
    StackLocation dst;
    int value;
    void* src;
};

struct SubI {
    Register dst;
    int value;
    void* src;
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
    void* src;
};

struct SetEAl {
    Register dst;
    void* src;
};

struct SetGAl {
    Register dst;
    void* src;
};

struct SetNeAl {
    Register dst;
void* src;
    };

struct SetLAl {
    Register dst;
    void* src;
};

struct Label {
    std::string name;
    void* src;
    void *dst;
};

struct Call {
    std::string name;
    Register dst;
    void* src;
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
    void *dst;
};

struct Push {
    Register src;
    void *dst;
};

using Instruction =
    std::variant<Mov, LoadI, StoreI, Store, Load, Jump, AddI, Add, SubI, Sub, AddMI, SubMI, Cmp,
                 CmpI, SetEAl, SetGAl, Label, JumpEq, Call, Lea, IndirectLoad, JumpGreater,
                 IndirectStore, PushI, Push, JumpLess, SetNeAl, SetLAl, ZeroExtend>;

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