#include "../../../include/compiler/target/target.hpp"

namespace target {

int sixteenByteAlign(int size) { return size % 16 == 0 ? size : size + (16 - (size % 16)); }

[[nodiscard]] int SizeOf(const Location& loc) {
    if (std::holds_alternative<StackLocation>(loc)) {
        throw std::runtime_error("SizeOf called on a stack location");
    }
    if (std::holds_alternative<Register>(loc)) {
        const auto reg = std::get<Register>(loc);
        if (std::holds_alternative<HardcodedRegister>(reg)) {
            return std::get<HardcodedRegister>(reg).size;
        }
        return std::get<VirtualRegister>(reg).size;
    }
    throw std::runtime_error("SizeOf called on an unknown location");
}

[[nodiscard]] auto param_register_by_convention(int idx, int size) -> HardcodedRegister {
    if (static_cast<size_t>(idx) >= param_regs.size()) {
        throw std::runtime_error("param_register_by_convention: idx out of bounds");
    }
    return HardcodedRegister{.reg = param_regs[idx], .size = size};
}

}  // namespace target