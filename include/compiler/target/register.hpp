#pragma once

#include <string>
#include <variant>

#include "qa_x86_registers.hpp"
#include "virtual_register.hpp"

namespace target {

using Register = std::variant<HardcodedRegister, VirtualRegister>;

[[nodiscard]] auto is_float_register(Register p_reg) -> bool;

[[nodiscard]] std::string register_to_asm(Register reg);
// comparison operators for registers
bool operator<(const Register& lhs, const Register& rhs);
}  // namespace target