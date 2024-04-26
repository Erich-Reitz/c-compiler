#include "../../include/compiler/qa_ir.hpp"

#include <cassert>
#include <stdexcept>

#include "../../include/compiler/qa_x86.hpp"

namespace qa_ir {

std::ostream& operator<<(std::ostream& os, const Operation& ins) {
    return std::visit([&os](const auto& ins) -> std::ostream& { return os << ins; }, ins);
}

std::ostream& operator<<(std::ostream& os, const Label& label) {
    os << label.name;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Mov& mov) {
    os << "mov dst=" << mov.dst << ", src=" << mov.src;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Ret& ret) {
    os << "ret value=" << ret.value;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Add& add) {
    os << "add dst=" << add.dst << ", left=" << add.left << ", right=" << add.right;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Sub& sub) {
    os << "sub dst=" << sub.dst << ", left=" << sub.left << ", right=" << sub.right;
    return os;
}

std::ostream& operator<<(std::ostream& os, const MovR& movr) {
    os << "movr dst=" << movr.dst << ", src=" << movr.src;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Addr& addr) {
    os << "addr dst=" << addr.dst << ", src=" << addr.src;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Deref& deref) {
    os << "deref dst=" << deref.dst << ", src=" << deref.src << ", depth=" << deref.depth;
    return os;
}

std::ostream& operator<<(std::ostream& os, const StoreAddr& storeaddr) {
    os << "storeaddr dst=" << storeaddr.dst << ", src=" << storeaddr.src;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Compare& cmp) {
    os << "cmp left=" << cmp.left << ", right=" << cmp.right;
    return os;
}
std::ostream& operator<<(std::ostream& os, const Equal& eq) {
    os << "eq dst=" << eq.dst << ", left=" << eq.left << ", right=" << eq.right;
    return os;
}

std::ostream& operator<<(std::ostream& os, const NotEqual& neq) {
    os << "neq dst=" << neq.dst << ", left=" << neq.left << ", right=" << neq.right;
    return os;
}

std::ostream& operator<<(std::ostream& os, const GreaterThan& gt) {
    os << "gt dst=" << gt.dst << ", left=" << gt.left << ", right=" << gt.right;
    return os;
}

std::ostream& operator<<(std::ostream& os, const LessThan& lt) {
    os << "lt dst=" << lt.dst << ", left=" << lt.left << ", right=" << lt.right;
    return os;
}

std::ostream& operator<<(std::ostream& os, const ConditionalJumpEqual& cje) {
    os << "cje trueLabel=" << cje.trueLabel << ", falseLabel=" << cje.falseLabel;
    return os;
}

std::ostream& operator<<(std::ostream& os, const ConditionalJumpNotEqual& cjne) {
    os << "cjne trueLabel=" << cjne.trueLabel << ", falseLabel=" << cjne.falseLabel;
    return os;
}

std::ostream& operator<<(std::ostream& os, const ConditionalJumpGreater& cjg) {
    os << "cjg trueLabel=" << cjg.trueLabel << ", falseLabel=" << cjg.falseLabel;
    return os;
}

std::ostream& operator<<(std::ostream& os, const ConditionalJumpLess& cjl) {
    os << "cjl trueLabel=" << cjl.trueLabel << ", falseLabel=" << cjl.falseLabel;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Jump& jump) {
    os << "jump label=" << jump.label;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Call& call) {
    os << "call name=" << call.name << ", args=[";
    for (const auto& arg : call.args) {
        os << arg << ", ";
    }
    os << "], dst=" << call.dst;
    return os;
}

std::ostream& operator<<(std::ostream& os, const LabelDef& labeldef) {
    os << "labeldef label=" << labeldef.label;
    return os;
}

std::ostream& operator<<(std::ostream& os, const DerefStore& derefstore) {
    os << "derefstore dst=" << derefstore.dst << ", src=" << derefstore.src;
    return os;
}

std::ostream& operator<<(std::ostream& os, const DefineStackPushed& dsp) {
    os << "define_stack_pushed name=" << dsp.name << ", size=" << dsp.size;
    return os;
}

bool operator<(const Temp& lhs, const Temp& rhs) { return lhs.id < rhs.id; }

std::ostream& operator<<(std::ostream& os, const Temp& temp) {
    os << "t" << temp.id;
    return os;
}

bool operator<(const target::HardcodedRegister& lhs, const target::HardcodedRegister& rhs) {
    return lhs.reg < rhs.reg;
}

std::ostream& operator<<(std::ostream& os, const target::HardcodedRegister& reg) {
    os << target::to_asm(reg.reg, reg.size);
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
    } else if (std::holds_alternative<ConstInt>(v)) {
        os << std::get<ConstInt>(v).numerical_value;
    } else {
        throw std::runtime_error("Unknown value type");
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
    } else if (std::holds_alternative<ConstInt>(v)) {
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