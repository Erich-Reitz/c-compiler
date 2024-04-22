#include "../include/qa_ir.hpp"

#include <cassert>
#include <stdexcept>

#include "../include/qa_x86.hpp"

namespace qa_ir {

std::ostream& operator<<(std::ostream& os, const Label& label) {
    os << label.name;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Operation& ins) {
    if (std::holds_alternative<Mov>(ins)) {
        const auto& mov = std::get<Mov>(ins);
        os << "mov " << mov.dst << ", " << mov.src;
    } else if (std::holds_alternative<Ret>(ins)) {
        const auto& ret = std::get<Ret>(ins);
        os << "ret " << ret.value;
    } else if (std::holds_alternative<Add>(ins)) {
        const auto& add = std::get<Add>(ins);
        os << "add " << add.dst << ", " << add.left << ", " << add.right;
    } else if (std::holds_alternative<Sub>(ins)) {
        const auto& sub = std::get<Sub>(ins);
        os << "sub " << sub.dst << ", " << sub.left << ", " << sub.right;
    } else if (std::holds_alternative<MovR>(ins)) {
        const auto& movr = std::get<MovR>(ins);
        os << "movr " << movr.dst << ", " << movr.src;
    } else if (std::holds_alternative<Addr>(ins)) {
        const auto& addr = std::get<Addr>(ins);
        os << "addr " << addr.dst << ", " << addr.src;
    } else if (std::holds_alternative<Deref>(ins)) {
        const auto& deref = std::get<Deref>(ins);
        os << "deref " << deref.dst << ", " << deref.src;
    } else if (std::holds_alternative<Equal>(ins)) {
        const auto& equal = std::get<Equal>(ins);
        os << "equal " << equal.dst << ", " << equal.left << ", "
           << equal.right;
    } else if (std::holds_alternative<ConditionalJumpEqual>(ins)) {
        const auto& cj = std::get<ConditionalJumpEqual>(ins);
        os << "cj " << cj.trueLabel << ", " << cj.falseLabel;
    } else if (std::holds_alternative<LabelDef>(ins)) {
        const auto& label = std::get<LabelDef>(ins);
        os << label.label << ":";
    } else if (std::holds_alternative<Compare>(ins)) {
        const auto& cmp = std::get<Compare>(ins);
        os << "cmp " << cmp.left << ", " << cmp.right;
    } else if (std::holds_alternative<Call>(ins)) {
        const auto& call = std::get<Call>(ins);
        os << "call " << call.name << ", ";
        for (const auto& arg : call.args) {
            os << arg << ", ";
        }
        os << call.dst;
    } else if (std::holds_alternative<Addr>(ins)) {
        const auto& addr = std::get<Addr>(ins);
        os << "addr " << addr.dst << ", " << addr.src;
    } else if (std::holds_alternative<Deref>(ins)) {
        const auto& deref = std::get<Deref>(ins);
        os << "deref " << deref.dst << ", " << deref.src;
    } else if (std::holds_alternative<DerefStore>(ins)) {
        const auto& derefstore = std::get<DerefStore>(ins);
        os << "derefstore " << derefstore.dst << ", " << derefstore.src;
    } else if (std::holds_alternative<GreaterThan>(ins)) {
        const auto& gt = std::get<GreaterThan>(ins);
        os << "gt " << gt.dst << ", " << gt.left << ", " << gt.right;
    } else if (std::holds_alternative<ConditionalJumpGreater>(ins)) {
        const auto& cj = std::get<ConditionalJumpGreater>(ins);
        os << "cjg " << cj.trueLabel << ", " << cj.falseLabel;
    } else if (std::holds_alternative<DefineStackPushed>(ins)) {
        const auto& dsp = std::get<DefineStackPushed>(ins);
        os << "DefineStackPushed " << dsp.name << ", " << dsp.size;
    } else if (std::holds_alternative<Jump>(ins)) {
        const auto& jmp = std::get<Jump>(ins);
        os << "jmp " << jmp.label;
    } else if (std::holds_alternative<ConditionalJumpLess>(ins)) {
        const auto& cj = std::get<ConditionalJumpLess>(ins);
        os << "cjl " << cj.trueLabel << ", " << cj.falseLabel;
    } else {
        throw std::runtime_error("Unknown instruction type " +
                                 std::to_string(ins.index()));
    }
    return os;
}

Label get_true_label(const CondJ& condj) {
    if (std::holds_alternative<ConditionalJumpEqual>(condj)) {
        return std::get<ConditionalJumpEqual>(condj).trueLabel;
    } else if (std::holds_alternative<ConditionalJumpGreater>(condj)) {
        return std::get<ConditionalJumpGreater>(condj).trueLabel;
    } else {
        throw std::runtime_error("Unknown conditional jump type " +
                                 std::to_string(condj.index()));
    }
}
Label get_false_label(const CondJ& condj) {
    if (std::holds_alternative<ConditionalJumpEqual>(condj)) {
        return std::get<ConditionalJumpEqual>(condj).falseLabel;
    } else if (std::holds_alternative<ConditionalJumpGreater>(condj)) {
        return std::get<ConditionalJumpGreater>(condj).falseLabel;
    } else {
        throw std::runtime_error("Unknown conditional jump type " +
                                 std::to_string(condj.index()));
    }
}

bool operator<(const Temp& lhs, const Temp& rhs) { return lhs.id < rhs.id; }

std::ostream& operator<<(std::ostream& os, const Temp& temp) {
    os << "t" << temp.id;
    return os;
}

bool operator<(const target::HardcodedRegister& lhs,
               const target::HardcodedRegister& rhs) {
    return lhs.reg < rhs.reg;
}

std::ostream& operator<<(std::ostream& os,
                         const target::HardcodedRegister& reg) {
    os << target::to_asm(reg.reg, reg.size);
    return os;
}

std::ostream& operator<<(std::ostream& os, const Value& v) {
    if (std::holds_alternative<Temp>(v)) {
        os << std::get<Temp>(v);
    } else if (std::holds_alternative<target::HardcodedRegister>(v)) {
        os << std::get<target::HardcodedRegister>(v);
    } else if (std::holds_alternative<Variable>(v)) {
        os << std::get<Variable>(v).name;
    } else if (std::holds_alternative<int>(v)) {
        os << std::get<int>(v);
    } else {
        throw std::runtime_error("Unknown value type");
    }
    return os;
}

[[nodiscard]] int SizeOf(Value v) {
    if (std::holds_alternative<Temp>(v)) {
        return std::get<Temp>(v).size;
    } else if (std::holds_alternative<target::HardcodedRegister>(v)) {
        return std::get<target::HardcodedRegister>(v).size;
    } else if (std::holds_alternative<Variable>(v)) {
        return std::get<Variable>(v).size;
    } else if (std::holds_alternative<int>(v)) {
        return 4;
    } else {
        throw std::runtime_error("Unknown value type");
    }
}

[[nodiscard]] int SizeOfWhatItPointsTo(Value v) {
    if (std::holds_alternative<Variable>(v)) {
        return std::get<Variable>(v).size;
    }
    throw std::runtime_error("Unknown value type");
}

}  // namespace qa_ir