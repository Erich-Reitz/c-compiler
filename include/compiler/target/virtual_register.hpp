#pragma once

namespace target {
enum class VirtualRegisterKind { INT, FLOAT };

struct VirtualRegister {
    int id = -1;
    int size = -1;
    VirtualRegisterKind kind = VirtualRegisterKind::INT;
};
}  // namespace target
