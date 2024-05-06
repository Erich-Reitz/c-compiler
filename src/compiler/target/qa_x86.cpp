#include "../../../include/compiler/target/qa_x86.hpp"

namespace target {

int sixteenByteAlign(int size) { return size % 16 == 0 ? size : size + (16 - (size % 16)); }

std::optional<VirtualRegister> get_src_register(const Instruction& ins) {
    return std::visit(
        [&](auto&& arg) -> std::optional<VirtualRegister> {
            if constexpr (HasRegisterSrc<decltype(arg)>) {
                const auto reg = arg.src;
                if (std::holds_alternative<VirtualRegister>(reg)) {
                    return std::get<VirtualRegister>(reg);
                }
                return std::nullopt;
            } else if constexpr (HasRegisterSrc<decltype(arg.src)>) {
                if (std::holds_alternative<VirtualRegister>(arg.src.src)) {
                    return std::get<VirtualRegister>(arg.src.src);
                }
                return std::nullopt;
            }
            return std::nullopt;
        },
        ins);
}

std::optional<VirtualRegister> get_dest_register(const Instruction& ins) {
    return std::visit(
        [](auto&& arg) -> std::optional<VirtualRegister> {
            if constexpr (HasRegisterDest<decltype(arg)>) {
                const auto reg = arg.dst;
                if (std::holds_alternative<VirtualRegister>(reg)) {
                    return std::get<VirtualRegister>(reg);
                }
                return std::nullopt;
            } else if constexpr (HasRegisterSrc<decltype(arg.dst)>) {
                if (std::holds_alternative<VirtualRegister>(arg.dst.src)) {
                    return std::get<VirtualRegister>(arg.dst.src);
                }
                return std::nullopt;
            }
            return std::nullopt;
        },
        ins);
}

std::optional<int> get_src_virtual_id_if_present(const Instruction& ins) {
    auto reg = get_src_register(ins);
    if (reg.has_value()) {
        return reg->id;
    }
    return std::nullopt;
}

std::optional<int> get_dest_virtual_id_if_present(const Instruction& ins) {
    auto reg = get_dest_register(ins);
    if (reg.has_value()) {
        return reg->id;
    }
    return std::nullopt;
}

void set_src_register(Instruction& ins, Register reg) {
    std::visit(
        [&reg](auto&& arg) {
            if constexpr (HasRegisterSrc<decltype(arg)>) {
                arg.src = reg;
            } else if constexpr (HasRegisterSrc<decltype(arg.src)>) {
                arg.src.src = reg;
            }
        },
        ins);
}
void set_dest_register(Instruction& ins, Register reg) {
    std::visit(
        [&reg](auto&& arg) {
            if constexpr (HasRegisterDest<decltype(arg)>) {
                arg.dst = reg;
            } else if constexpr (HasRegisterSrc<decltype(arg.dst)>) {
                arg.dst.src = reg;
            }
        },
        ins);
}

}  // namespace target