#pragma once

#include <memory>
#include <string>

namespace ast {

struct DataType {
    std::string name = "";
    int size = 0;
    bool is_pointer = false;
    int points_to_size = 0;
    bool is_array = false;
};
}  // namespace ast