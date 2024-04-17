#include <stdexcept>

#include "operation.hpp"

namespace qa_ir {

std::ostream &operator<<(std::ostream &os, const Label &label) {
    os << label.name;
    return os;
}

std::ostream &operator<<(std::ostream &os, const Operation &ins) {
    if (std::holds_alternative<Mov>(ins)) {
        const auto &mov = std::get<Mov>(ins);
        os << "mov " << mov.dst << ", " << mov.src;
    } else if (std::holds_alternative<Ret>(ins)) {
        const auto &ret = std::get<Ret>(ins);
        os << "ret " << ret.value;
    } else if (std::holds_alternative<Add>(ins)) {
        const auto &add = std::get<Add>(ins);
        os << "add " << add.dst << ", " << add.left << ", " << add.right;
    } else if (std::holds_alternative<Sub>(ins)) {
        const auto &sub = std::get<Sub>(ins);
        os << "sub " << sub.dst << ", " << sub.left << ", " << sub.right;
    } else if (std::holds_alternative<MovR>(ins)) {
        const auto &movr = std::get<MovR>(ins);
        os << "movr " << movr.dst << ", " << movr.src;
    } else if (std::holds_alternative<Addr>(ins)) {
        const auto &addr = std::get<Addr>(ins);
        os << "addr " << addr.dst << ", " << addr.src;
    } else if (std::holds_alternative<Deref>(ins)) {
        const auto &deref = std::get<Deref>(ins);
        os << "deref " << deref.dst << ", " << deref.src;
    } else if (std::holds_alternative<StoreAddr>(ins)) {
        const auto &storeaddr = std::get<StoreAddr>(ins);
        os << "storeaddr " << storeaddr.dst << ", " << storeaddr.src;
    } else if (std::holds_alternative<Equal>(ins)) {
        const auto &equal = std::get<Equal>(ins);
        os << "equal " << equal.dst << ", " << equal.left << ", " << equal.right;
    } else if (std::holds_alternative<ConditionalJumpEqual>(ins)) {
        const auto &cj = std::get<ConditionalJumpEqual>(ins);
        os << "cj " << cj.trueLabel << ", " << cj.falseLabel;
    } else if (std::holds_alternative<LabelDef>(ins)) {
        const auto &label = std::get<LabelDef>(ins);
        os << label.label << ":";
    } else if (std::holds_alternative<Compare>(ins)) {
        const auto &cmp = std::get<Compare>(ins);
        os << "cmp " << cmp.left << ", " << cmp.right;
    } else if (std::holds_alternative<Call>(ins)) {
        const auto &call = std::get<Call>(ins);
        os << "call " << call.name << ", ";
        for (const auto &arg : call.args) {
            os << arg << ", ";
        }
        os << call.dst;
    } else if (std::holds_alternative<Addr>(ins)) {
        const auto &addr = std::get<Addr>(ins);
        os << "addr " << addr.dst << ", " << addr.src;
    } else if (std::holds_alternative<Deref>(ins)) {
        const auto &deref = std::get<Deref>(ins);
        os << "deref " << deref.dst << ", " << deref.src;
    } else if (std::holds_alternative<StoreAddr>(ins)) {
        const auto &storeaddr = std::get<StoreAddr>(ins);
        os << "storeaddr " << storeaddr.dst << ", " << storeaddr.src;
    } else if (std::holds_alternative<DerefStore>(ins)) {
        const auto &derefstore = std::get<DerefStore>(ins);
        os << "derefstore " << derefstore.dst << ", " << derefstore.src;
    } else if (std::holds_alternative<GreaterThan>(ins)) {
        const auto &gt = std::get<GreaterThan>(ins);
        os << "gt " << gt.dst << ", " << gt.left << ", " << gt.right;
    } else if (std::holds_alternative<ConditionalJumpGreater>(ins)) {
        const auto &cj = std::get<ConditionalJumpGreater>(ins);
        os << "cjg " << cj.trueLabel << ", " << cj.falseLabel;
    } else if (std::holds_alternative<DefineStackPushed>(ins)) {
        const auto &dsp = std::get<DefineStackPushed>(ins);
        os << "DefineStackPushed " << dsp.name << ", " << dsp.size;
    } else {
        throw std::runtime_error("Unknown instruction type " +
                                 std::to_string(ins.index()));
    }
    return os;
}

Label get_true_label(const CondJ &condj) {
    if (std::holds_alternative<ConditionalJumpEqual>(condj)) {
        return std::get<ConditionalJumpEqual>(condj).trueLabel;
    } else if (std::holds_alternative<ConditionalJumpGreater>(condj)) {
        return std::get<ConditionalJumpGreater>(condj).trueLabel;
    } else {
        throw std::runtime_error("Unknown conditional jump type " +
                                 std::to_string(condj.index()));
    }
}
Label get_false_label(const CondJ &condj) {
    if (std::holds_alternative<ConditionalJumpEqual>(condj)) {
        return std::get<ConditionalJumpEqual>(condj).falseLabel;
    } else if (std::holds_alternative<ConditionalJumpGreater>(condj)) {
        return std::get<ConditionalJumpGreater>(condj).falseLabel;
    } else {
        throw std::runtime_error("Unknown conditional jump type " +
                                 std::to_string(condj.index()));
    }
}

} // namespace qa_ir