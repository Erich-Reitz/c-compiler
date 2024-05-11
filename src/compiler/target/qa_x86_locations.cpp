#include "../../../include/compiler/target/qa_x86_locations.hpp"

#include <stdexcept>

namespace target {
std::string computed_stack_location(target::StackLocation sl, std::string base_register_repr) {
    const auto initalOffset = sl.offset;
    const auto scale = sl.scale;
    std::string scale_string = "";

    if (scale != 1) {
        scale_string = " * " + std::to_string(scale);
    }

    if (initalOffset == 0) {
        // don't return relative to rbp
        // TODO: i dunno
        if (sl.offest_from_base != 0) {
            return " [" + base_register_repr + " + " + std::to_string(sl.offest_from_base) + " " +
                   scale_string + "]";
        }
        return " [" + base_register_repr + scale_string + "]";
    }

    // if we are using it as an offset, then its size is 8.
    // TODO: issue a clear instruction
    const auto result =
        " [rbp-" + std::to_string(initalOffset) + " + " + base_register_repr + scale_string + "]";

    return result;
}

std::string stack_location_at_asm(target::StackLocation sl) {
    if (sl.is_computed) {
        HardcodedRegister base;
        if (std::holds_alternative<target::VirtualRegister>(sl.src)) {
            const auto virtual_base = std::get<target::VirtualRegister>(sl.src);
            const auto virtual_base_str = register_to_asm(virtual_base);
            return computed_stack_location(sl, virtual_base_str);
        }
        base = std::get<target::HardcodedRegister>(sl.src);
        const auto base_register = HardcodedRegister{.reg = base.reg, .size = 8};
        const auto base_register_asm = register_to_asm(base_register);
        return computed_stack_location(sl, base_register_asm);
    }

    if (sl.offset >= 0) {
        return " [rbp - " + std::to_string(sl.offset) + "]";
    }
    return " [rbp + " + std::to_string(-sl.offset) + "]";
}

}  // namespace target