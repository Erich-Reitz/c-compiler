#include "../../../include/compiler/qa_ir/qa_ir.hpp"

#include "../../../include/ast/ast.hpp"

namespace qa_ir {

[[nodiscard]] ast::DataType ResultingTypeForBinOp(ast::DataType lhs, ast::DataType rhs,
                                                  ast::BinOpKind op) {
    if (ast::is_comparison(op)) {
        return ast::DataType::int_type();
    }

    if (lhs.is_int_ptr() && rhs.is_int_ptr()) {
        if (op == ast::BinOpKind::Sub) {
            return ast::DataType::int_type();
        }
        throw std::runtime_error("Invalid operation on pointers");
    }
    if (lhs.is_int_ptr() && rhs.is_int()) {
        if (op == ast::BinOpKind::Add) {
            return ast::DataType{.base_type = ast::BaseType::POINTER,
                                 .points_to = ast::BaseType::INT,
                                 .array_size = 0,
                                 .indirect_level = 1};
        }
        throw std::runtime_error("Invalid operation on pointers");
    }

    if (lhs.is_int() && rhs.is_int_ptr()) {
        if (op == ast::BinOpKind::Add) {
            return ast::DataType{.base_type = ast::BaseType::POINTER,
                                 .points_to = ast::BaseType::INT,
                                 .array_size = 0,
                                 .indirect_level = 1};
        }
        throw std::runtime_error("Invalid operation on pointers");
    }

    if (lhs.is_int() && rhs.is_int()) {
        return ast::DataType::int_type();
    }
    if (lhs.is_float() && rhs.is_float()) {
        return ast::DataType::float_type();
    }
    if (lhs.is_float() && rhs.is_int()) {
        return ast::DataType::float_type();
    }
    if (lhs.is_int() && rhs.is_float()) {
        return ast::DataType::float_type();
    }

    if (lhs.base_type == ast::BaseType::ARRAY && rhs.base_type == ast::BaseType::INT) {
        return ast::DataType{.base_type = ast::BaseType::POINTER,
                             .points_to = lhs.points_to,
                             .array_size = 0,
                             .indirect_level = 1};
    }
    if (lhs.base_type == ast::BaseType::INT && rhs.base_type == ast::BaseType::ARRAY) {
        return ast::DataType{.base_type = ast::BaseType::POINTER,
                             .points_to = rhs.points_to,
                             .array_size = 0,
                             .indirect_level = 1};
    }

    std::cout << "lhs: " << lhs << " rhs: " << rhs << std::endl;
    throw std::runtime_error("Invalid types for binary operation");
}

[[nodiscard]] ast::DataType GetDataType(Value v) {
    if (std::holds_alternative<Temp>(v)) {
        return std::get<Temp>(v).type;
    } else if (std::holds_alternative<Variable>(v)) {
        return std::get<Variable>(v).type;
    } else if (std::holds_alternative<Immediate<int>>(v)) {
        return ast::DataType::int_type();
    } else if (std::holds_alternative<Immediate<float>>(v)) {
        return ast::DataType::float_type();
    }
    throw std::runtime_error("hardcoded registers have no type...");
}

bool operator<(const Temp& lhs, const Temp& rhs) { return lhs.id < rhs.id; }

std::ostream& operator<<(std::ostream& os, const Temp& temp) {
    if (temp.offset) {
        os << "qa_ir::Temp{id=" << temp.id << ", type=" << temp.type << ", offset=" << *temp.offset
           << "}";
        return os;
    }

    os << "qa_ir::Temp{id=" << temp.id << ", type=" << temp.type << ", offset=" << temp.offset
       << "}";
    return os;
}

bool operator<(const target::HardcodedRegister& lhs, const target::HardcodedRegister& rhs) {
    return lhs.reg < rhs.reg;
}

std::ostream& operator<<(std::ostream& os, const target::HardcodedRegister& reg) {
    os << target::register_to_asm(reg);
    return os;
}

std::ostream& operator<<(std::ostream& os, const Value& v) {
    if (std::holds_alternative<Temp>(v)) {
        os << std::get<Temp>(v);
    } else if (std::holds_alternative<target::HardcodedRegister>(v)) {
        os << std::get<target::HardcodedRegister>(v);
    } else if (std::holds_alternative<Variable>(v)) {
        auto var = std::get<Variable>(v);
        if (var.offset) {
            os << "Variable{name=" << var.name << ", type=" << var.type
               << ", offset=" << *var.offset << "}";
            return os;
        }
        os << "Variable{name=" << var.name << ", type=" << var.type << ", offset=" << var.offset
           << "}";
    } else if (std::holds_alternative<Immediate<int>>(v)) {
        os << std::get<Immediate<int>>(v).numerical_value;
    } else if (std::holds_alternative<Immediate<float>>(v)) {
        os << std::get<Immediate<float>>(v).numerical_value;
    }
    return os;
}
// TODO: for some things this behaves like you how you would expect SizeOfWhatItPointsTo to
int SizeOf(Value v) {
    if (std::holds_alternative<Temp>(v)) {
        const auto t = std::get<Temp>(v);
        return t.type.GetSize();
    } else if (std::holds_alternative<target::HardcodedRegister>(v)) {
        return std::get<target::HardcodedRegister>(v).size;
    } else if (std::holds_alternative<Variable>(v)) {
        const auto var = std::get<Variable>(v);
        const auto type = var.deduceTypeIncorporatingOffset();
        return type.GetSize();
    } else if (std::holds_alternative<Immediate<int>>(v)) {
        return 4;
    } else if (std::holds_alternative<Immediate<float>>(v)) {
        return 4;
    } else {
        throw std::runtime_error("Unknown value type");
    }
}

}  // namespace qa_ir