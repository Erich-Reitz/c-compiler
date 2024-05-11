#include "../../../include/compiler/target/qa_x86.hpp"

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
}  // namespace target