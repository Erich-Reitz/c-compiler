#include "../include/qa_x86.hpp"

namespace target {
[[nodiscard]] std::string to_asm(const Mov& mov) {
    auto dst = std::get<HardcodedRegister>(mov.dst);
    auto src = std::get<HardcodedRegister>(mov.src);
    return to_asm(dst.reg, dst.size) + ", " + to_asm(src.reg, src.size);
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

[[nodiscard]] std::string to_asm(BaseRegister reg, int size) {
    switch (reg) {
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

std::ostream& operator<<(std::ostream& os, const Instruction& ins) {
    if (std::holds_alternative<Mov>(ins)) {
        const auto mov = std::get<Mov>(ins);
        os << "mov " << mov.src << " -> " << mov.dst;
    } else if (std::holds_alternative<Load>(ins)) {
        const auto load = std::get<Load>(ins);
        os << "load " << load.src << " -> " << load.dst;
    } else if (std::holds_alternative<Store>(ins)) {
        const auto store = std::get<Store>(ins);
        os << "store " << store.src << " -> " << store.dst;
    } else if (std::holds_alternative<LoadI>(ins)) {
        const auto loadI = std::get<LoadI>(ins);
        os << "loadI " << loadI.value << " -> " << loadI.dst;
    } else if (std::holds_alternative<StoreI>(ins)) {
        const auto storeI = std::get<StoreI>(ins);
        os << "storeI " << storeI.value << " -> " << storeI.dst;
    } else if (std::holds_alternative<Jump>(ins)) {
        const auto jump = std::get<Jump>(ins);
        os << "jump " << jump.label;
    } else if (std::holds_alternative<AddI>(ins)) {
        const auto addI = std::get<AddI>(ins);
        os << "addI " << addI.value << " -> " << addI.dst;
    } else if (std::holds_alternative<Add>(ins)) {
        const auto add = std::get<Add>(ins);
        os << "add " << add.src << " -> " << add.dst;
    } else if (std::holds_alternative<Sub>(ins)) {
        const auto sub = std::get<Sub>(ins);
        os << "sub " << sub.src << " -> " << sub.dst;
    } else if (std::holds_alternative<SubI>(ins)) {
        const auto subI = std::get<SubI>(ins);
        os << "subI " << subI.value << " -> " << subI.dst;
    } else if (std::holds_alternative<CmpI>(ins)) {
        const auto cmpI = std::get<CmpI>(ins);
        os << "cmpI " << cmpI.value << " to -> " << cmpI.dst;
    } else if (std::holds_alternative<SetEAl>(ins)) {
        const auto setAl = std::get<SetEAl>(ins);
        os << "SetEAl " << setAl.dst;
    } else if (std::holds_alternative<Cmp>(ins)) {
        const auto cmp = std::get<Cmp>(ins);
        os << "cmp " << cmp.dst << " -> " << cmp.src;
    } else if (std::holds_alternative<Label>(ins)) {
        const auto label = std::get<Label>(ins);
        os << label.name << ":";
    } else if (std::holds_alternative<JumpEq>(ins)) {
        const auto jumpEq = std::get<JumpEq>(ins);
        os << "je " << jumpEq.label;
    } else if (std::holds_alternative<Call>(ins)) {
        const auto call = std::get<Call>(ins);
        os << "call " << call.name << " -> " << call.dst;
    } else if (std::holds_alternative<Lea>(ins)) {
        const auto lea = std::get<Lea>(ins);
        os << "lea " << lea.src << " -> " << lea.dst;
    } else if (std::holds_alternative<IndirectLoad>(ins)) {
        const auto mem = std::get<IndirectLoad>(ins);
        os << "mem " << mem.src << " -> " << mem.dst;
    } else if (std::holds_alternative<IndirectStore>(ins)) {
        const auto mem = std::get<IndirectStore>(ins);
        os << "mov " << mem.src << " -> " << mem.dst;
    } else if (std::holds_alternative<JumpGreater>(ins)) {
        const auto jumpGreater = std::get<JumpGreater>(ins);
        os << "jg " << jumpGreater.label;
    } else if (std::holds_alternative<SetGAl>(ins)) {
        const auto setAl = std::get<SetGAl>(ins);
        os << "SetGAl " << setAl.dst;
    } else if (std::holds_alternative<PushI>(ins)) {
        const auto pushI = std::get<PushI>(ins);
        os << "pushI " << pushI.src;
    } else if (std::holds_alternative<Push>(ins)) {
        const auto push = std::get<Push>(ins);
        os << "push " << push.src;
    } else if (std::holds_alternative<AddMI>(ins)) {
        const auto addMI = std::get<AddMI>(ins);
        os << "addMI " << addMI.value << " -> " << addMI.dst;
    } else if (std::holds_alternative<SubMI>(ins)) {
        const auto subMI = std::get<SubMI>(ins);
        os << "subMI " << subMI.value << " -> " << subMI.dst;
    } else if (std::holds_alternative<JumpLess>(ins)) {
        const auto jumpLess = std::get<JumpLess>(ins);
        os << "jl " << jumpLess.label;
    } else if (std::holds_alternative<SetNeAl>(ins)) {
        const auto setAl = std::get<SetNeAl>(ins);
        os << "SetNeAl " << setAl.dst;
    } else {
        throw std::runtime_error("Unsupported instruction type");
    }
    return os;
}

bool operator<(const Register& lhs, const Register& rhs) {
    if (std::holds_alternative<HardcodedRegister>(lhs) &&
        std::holds_alternative<HardcodedRegister>(rhs)) {
        return std::get<HardcodedRegister>(lhs).reg <
               std::get<HardcodedRegister>(rhs).reg;
    }
    if (std::holds_alternative<VirtualRegister>(lhs) &&
        std::holds_alternative<VirtualRegister>(rhs)) {
        return std::get<VirtualRegister>(lhs).id <
               std::get<VirtualRegister>(rhs).id;
    }
    return false;
}
bool operator==(const HardcodedRegister& lhs, const HardcodedRegister& rhs) {
    return lhs.reg == rhs.reg;
}

std::optional<VirtualRegister> get_src_register(const Instruction& ins) {
    return std::visit(
        [](auto&& arg) -> std::optional<VirtualRegister> {
            if constexpr (HasRegisterSrc<decltype(arg)>) {
                const auto reg = arg.src;
                if (std::holds_alternative<VirtualRegister>(reg)) {
                    return std::get<VirtualRegister>(reg);
                }
                return std::nullopt;
            } else {
                return std::nullopt;
            }
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
            } else {
                return std::nullopt;
            }
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
            }
        },
        ins);
}
void set_dest_register(Instruction& ins, Register reg) {
    std::visit(
        [&reg](auto&& arg) {
            if constexpr (HasRegisterDest<decltype(arg)>) {
                arg.dst = reg;
            }
        },
        ins);
}

}  // namespace target