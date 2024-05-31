

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

[[nodiscard]] auto param_register_by_convention(int idx, int size) -> HardcodedRegister {
    if (static_cast<size_t>(idx) >= param_regs.size()) {
        throw std::runtime_error("param_register_by_convention: idx out of bounds");
    }
    return HardcodedRegister{.reg = param_regs[idx], .size = size};
}

}  // namespace target