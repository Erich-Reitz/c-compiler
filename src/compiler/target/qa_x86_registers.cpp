

#include "../../../include/compiler/target/qa_x86_registers.hpp"

#include <algorithm>
#include <stdexcept>

namespace target {

bool operator==(const HardcodedRegister& lhs, const HardcodedRegister& rhs) {
    return (lhs.reg == rhs.reg) && (lhs.size == rhs.size);
}

[[nodiscard]] auto is_float_register(BaseRegister p_reg) -> bool {
    return std::find(float_regs.begin(), float_regs.end(), p_reg) != float_regs.end();
}

}  // namespace target