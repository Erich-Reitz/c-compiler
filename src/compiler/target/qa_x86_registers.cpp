

#include "../../../include/compiler/target/qa_x86_registers.hpp"


#include <stdexcept>

namespace target {

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

std::string register_to_asm(Register v_reg) {
    if (std::holds_alternative<VirtualRegister>(v_reg)) {
        throw std::runtime_error("cannot convert VirtualRegister to asm"); 
    }
    const auto reg = std::get<HardcodedRegister>(v_reg); 

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
        case BaseRegister::XMM0:
            return "xmm0";
        case BaseRegister::XMM1:
            return "xmm1";
        case BaseRegister::XMM2:
            return "xmm2";
        case BaseRegister::XMM3:
            return "xmm3";
        case BaseRegister::XMM4:
            return "xmm4";
        case BaseRegister::XMM5:
            return "xmm5";
        case BaseRegister::XMM6:
            return "xmm6";
        case BaseRegister::XMM7:
            return "xmm7";
        default:
            throw std::runtime_error("register_to_asm: unknown register");
    }
}

[[nodiscard]] auto param_register_by_convention(int idx, int size) -> HardcodedRegister {
    if (static_cast<size_t>(idx) >= param_regs.size()) {
        throw std::runtime_error("param_register_by_convention: idx out of bounds");
    }
    return HardcodedRegister{.reg = param_regs[idx], .size = size};
}

}