#include "../../../include/compiler/target/qa_x86.hpp"

namespace target {
    
int sixteenByteAlign(int size) { return size % 16 == 0 ? size : size + (16 - (size % 16)); }



[[nodiscard]] auto param_register_by_convention(int idx, int size) -> HardcodedRegister {
    if (static_cast<size_t>(idx) >= param_regs.size()) {
        throw std::runtime_error("param_register_by_convention: idx out of bounds");
    }
    return HardcodedRegister{.reg = param_regs[idx], .size = size};
}

std::ostream& operator<<(std::ostream& os, BaseRegister reg) {
    switch (reg) {
        case BaseRegister::AX:
            os << "ax";
            break;
        case BaseRegister::BX:
            os << "bx";
            break;
        case BaseRegister::CX:
            os << "cx";
            break;
        case BaseRegister::DX:
            os << "dx";
            break;
        case BaseRegister::SI:
            os << "si";
            break;
        case BaseRegister::DI:
            os << "di";
            break;
        case BaseRegister::R8:
            os << "r8";
            break;
        case BaseRegister::R9:
            os << "r9";
            break;
        case BaseRegister::R10:
            os << "r10";
            break;
        case BaseRegister::R11:
            os << "r11";
            break;
        case BaseRegister::R12:
            os << "r12";
            break;
        case BaseRegister::R13:
            os << "r13";
            break;
        case BaseRegister::R14:
            os << "r14";
            break;
        case BaseRegister::R15:
            os << "r15";
            break;
    }
    return os;
}

[[nodiscard]] std::string to_asm(HardcodedRegister reg) {
    const auto base = reg.reg;
    const auto size = reg.size;
    switch (base) {
        case BaseRegister::AX:
            return size == 4 ? "eax" : "rax";
        case BaseRegister::BX:
            return size == 4 ? "ebx" : "rbx";
        case BaseRegister::CX:
            return size == 4 ? "ecx" : "rcx";
        case BaseRegister::DX:
            return size == 4 ? "edx" : "rdx";
        case BaseRegister::SI:
            return size == 4 ? "esi" : "rsi";
        case BaseRegister::DI:
            return size == 4 ? "edi" : "rdi";
        case BaseRegister::R8:
            return size == 4 ? "r8d" : "r8";
        case BaseRegister::R9:
            return size == 4 ? "r9d" : "r9";
        case BaseRegister::R10:
            return size == 4 ? "r10d" : "r10";
        case BaseRegister::R11:
            return size == 4 ? "r11d" : "r11";
        case BaseRegister::R12:
            return size == 4 ? "r12d" : "r12";
        case BaseRegister::R13:
            return size == 4 ? "r13d" : "r13";
        case BaseRegister::R14:
            return size == 4 ? "r14d" : "r14";
        case BaseRegister::R15:
            return size == 4 ? "r15d" : "r15";
        default:
            throw std::runtime_error("to_asm not implemented");
    }
}

std::ostream& operator<<(std::ostream& os, const StackLocation& loc) {
    os << "rbp - " << loc.offset;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Register& reg) {
    if (std::holds_alternative<HardcodedRegister>(reg)) {
        const auto hardcoded = std::get<HardcodedRegister>(reg);
        os << to_asm(hardcoded.reg, hardcoded.size);
    }
    if (std::holds_alternative<VirtualRegister>(reg)) {
        const auto virtual_reg = std::get<VirtualRegister>(reg);
        os << "v" << virtual_reg.id;
    }
    return os;
}

bool operator<(const Register& lhs, const Register& rhs) {
    if (std::holds_alternative<HardcodedRegister>(lhs) &&
        std::holds_alternative<HardcodedRegister>(rhs)) {
        return std::get<HardcodedRegister>(lhs).reg < std::get<HardcodedRegister>(rhs).reg;
    }
    if (std::holds_alternative<VirtualRegister>(lhs) &&
        std::holds_alternative<VirtualRegister>(rhs)) {
        return std::get<VirtualRegister>(lhs).id < std::get<VirtualRegister>(rhs).id;
    }
    return false;
}
bool operator==(const HardcodedRegister& lhs, const HardcodedRegister& rhs) {
    return (lhs.reg == rhs.reg) && (lhs.size == rhs.size);
}


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
            }  else if constexpr (HasRegisterSrc<decltype(arg.dst)>) {
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