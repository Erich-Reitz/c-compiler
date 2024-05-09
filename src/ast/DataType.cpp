#include "../../include/ast/DataType.hpp"

#include <cassert>
namespace ast {

[[nodiscard]] auto dt_to_string(const DataType& dt) -> std::string {
    std::string result;
    switch (dt.base_type) {
        case BaseType::INT:
            result = "int";
            break;
        case BaseType::FLOAT:
            result = "float";
            break;
        case BaseType::POINTER:
            result = dt_to_string(DataType{.base_type = dt.points_to}) + "*";
            break;
        case BaseType::ARRAY:
            result = dt_to_string(DataType{.base_type = dt.points_to}) + "[" +
                     std::to_string(dt.array_size) + "]";
            break;
        default:
            result = "unknown";
            break;
    }
    return result;
}

[[nodiscard]] auto SizeOf(BaseType base_type) -> int {
    if (base_type == BaseType::INT) {
        return 4;
    } else if (base_type == BaseType::FLOAT) {
        return 4;
    } else if (base_type == BaseType::POINTER) {
        return 8;
    }
    throw std::runtime_error("Unsupported base type");
}

[[nodiscard]] auto dereference_type(const DataType& dt) -> DataType {
    const auto levels = dt.indirect_level - 1;
    if (levels < 0) {
        std::cout << "dt: " << dt << std::endl;
        throw std::runtime_error("Cannot dereference a non-pointer type");
    }
    if (levels == 0) {
        return DataType{.base_type = dt.points_to};
    }
    return DataType{
        .base_type = BaseType::POINTER, .points_to = dt.points_to, .indirect_level = levels};
}

[[nodiscard]] auto decay_array_type(const DataType& dt) -> DataType {
    assert(dt.base_type == BaseType::ARRAY);
    return DataType{.base_type = BaseType::POINTER, .points_to = dt.points_to, .indirect_level = 1};
}

std::ostream& operator<<(std::ostream& os, const DataType& dt) {
    os << dt_to_string(dt);
    return os;
}

}  // namespace ast