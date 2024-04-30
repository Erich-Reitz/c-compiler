#include "../../../include/compiler/qa_ir/qa_ir.hpp"

namespace qa_ir {


bool operator<(const Temp& lhs, const Temp& rhs) { return lhs.id < rhs.id; }

std::ostream& operator<<(std::ostream& os, const Temp& temp) {
    os << "t" << temp.id;
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
        os << "qa_ir::Variable{name=" << var.name << ", size=" << var.type.size; 
        if (var.offset) {
            os << ", offset=" << *var.offset ;
        }
        os << "}";
    } else if (std::holds_alternative<Immediate<int>>(v)) {
        os << std::get<Immediate<int>>(v).numerical_value;
    } else if (std::holds_alternative<Immediate<float>>(v)) {
        os << std::get<Immediate<float>>(v).numerical_value;
    }
    return os;
}

// TODO: for some things this behaves like you how you would expect SizeOfWhatItPointsTo to
[[nodiscard]] int SizeOf(Value v) {
    if (std::holds_alternative<Temp>(v)) {
        return std::get<Temp>(v).size;
    } else if (std::holds_alternative<target::HardcodedRegister>(v)) {
        return std::get<target::HardcodedRegister>(v).size;
    } else if (std::holds_alternative<Variable>(v)) {
        if (std::get<Variable>(v).offset) {
            return std::get<Variable>(v).type.points_to_size; 
        }

        return std::get<Variable>(v).type.size; 
    } else if (std::holds_alternative<Immediate<int>>(v)) {
        return 4;
    } else {
        throw std::runtime_error("Unknown value type");
    }
}

[[nodiscard]] int SizeOfWhatItPointsTo(Value v) {
    if (std::holds_alternative<Variable>(v)) {
        return std::get<Variable>(v).type.size; 
    }
    throw std::runtime_error("Unknown value type");
}

}  // namespace qa_ir