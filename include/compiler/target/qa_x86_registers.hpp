#pragma once

#include <string>
#include <variant>
#include <vector>

#include "virtual_register.hpp"

namespace target {
const int address_size = 8;

// all registers used.
// x86-64 registers
enum class BaseRegister {
    AX,
    BX,
    CX,
    DX,
    SI,
    DI,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
    XMM0,
    XMM1,
    XMM2,
    XMM3,
    XMM4,
    XMM5,
    XMM6,
    XMM7
};

[[nodiscard]] auto is_float_register(BaseRegister p_reg) -> bool;

inline const std::vector<BaseRegister> float_regs = {
    BaseRegister::XMM0, BaseRegister::XMM1, BaseRegister::XMM2, BaseRegister::XMM3,
    BaseRegister::XMM4, BaseRegister::XMM5, BaseRegister::XMM6, BaseRegister::XMM7};

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

}  // namespace target