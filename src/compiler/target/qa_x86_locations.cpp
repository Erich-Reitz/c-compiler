#include "../../../include/compiler/target/qa_x86_locations.hpp"

#include <stdexcept>

namespace target {
    std::string stack_location_at_asm(target::StackLocation sl) {
      if (sl.is_computed) {
        if (std::holds_alternative<target::VirtualRegister>(sl.src)) {
            throw std::runtime_error("Cannot compute with virtual register");
        }
        const auto base = std::get<target::HardcodedRegister>(sl.src);
        const auto initalOffset = sl.offset;
        const auto scale = sl.scale;

        std::string scale_string = "";
        if (scale != 1) {
            scale_string = " * " + std::to_string(scale);
        }
        const auto base_register = HardcodedRegister{.reg = base.reg, .size = 8}; 
        if (initalOffset == 0) {
            // don't return relative to rbp
            //TODO: i dunno
            

            return " [0+" + register_to_asm(base_register) + scale_string + "]";
        }

        // if we are using it as an offset, then its size is 8.
        // TODO: issue a clear instruction
        return " [rbp-" + std::to_string(initalOffset) + " + " + register_to_asm(base_register) + scale_string + "]";
    }

    if (sl.offset >= 0) {
        return " [rbp - " + std::to_string(sl.offset) + "]";
    }
    return " [rbp + " + std::to_string(-sl.offset) + "]"; 
}

}