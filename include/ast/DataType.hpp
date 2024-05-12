#pragma once

#include <iostream>
#include <memory>
#include <string>

namespace ast {
enum BaseType {
    NONE,
    INT,
    FLOAT,
    POINTER,
    ARRAY,
};

[[nodiscard]] auto SizeOf(BaseType base_type) -> int;

struct DataType {
    BaseType base_type;
    BaseType points_to = BaseType::NONE;
    int array_size = 0;
    int indirect_level = 0;

    DataType static int_type() { return DataType{.base_type = BaseType::INT}; }

    DataType static float_type() { return DataType{.base_type = BaseType::FLOAT}; }

    DataType static array_type(int size, BaseType points_to) {
        return DataType{.base_type = BaseType::ARRAY,
                        .points_to = points_to,
                        .array_size = size,
                        .indirect_level = 1};
    }

    [[nodiscard]] auto is_int() const -> bool { return base_type == BaseType::INT; };

    [[nodiscard]] auto is_int_ptr() const -> bool {
        return base_type == BaseType::POINTER && points_to == BaseType::INT;
    }

    [[nodiscard]] auto is_float() const -> bool { return base_type == BaseType::FLOAT; }

    [[nodiscard]] auto is_float_ptr() const -> bool {
        return base_type == BaseType::POINTER && points_to == BaseType::FLOAT;
    }

    [[nodiscard]] auto GetSize() const -> int {
        if (base_type == BaseType::INT) {
            return 4;
        } else if (base_type == BaseType::FLOAT) {
            return 4;
        } else if (base_type == BaseType::POINTER) {
            return 8;
        } else if (base_type == BaseType::ARRAY) {
            return array_size * SizeOf(points_to);
        }
        return 0;
    }
};

[[nodiscard]] auto dereference_type(const DataType& dt) -> DataType;
[[nodiscard]] auto decay_array_type(const DataType& dt) -> DataType;
// [[nodiscard]] auto reference_type(const DataType &dt) -> DataType;
[[nodiscard]] auto dt_to_string(const DataType& dt) -> std::string;

std::ostream& operator<<(std::ostream& os, const DataType& dt);
}  // namespace ast