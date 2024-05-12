#include "../../../include/compiler/qa_ir/qa_ir_operations.hpp"

namespace qa_ir {

std::ostream& operator<<(std::ostream& os, const Operation& ins) {
    return std::visit([&os](const auto& v_ins) -> std::ostream& { return os << v_ins; }, ins);
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

std::ostream& operator<<(std::ostream& os, const DefineArray& arr) {
    os << "define_array name=" << arr.name << ", type=" << arr.type;
    return os;
}

template <ast::BaseType T, ast::BaseType U>
std::ostream& operator<<(std::ostream& os, const Add<T, U>& add) {
    os << "add dst=" << add.dst << ", left=" << add.left << ", right=" << add.right;
    return os;
}

template <ast::BaseType T, ast::BaseType U>
std::ostream& operator<<(std::ostream& os, const Sub<T, U>& sub) {
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

template <ast::BaseType T, ast::BaseType U>
std::ostream& operator<<(std::ostream& os, const Compare<T, U>& cmp) {
    os << "cmp left=" << cmp.left << ", right=" << cmp.right;
    return os;
}

template <ast::BaseType T, ast::BaseType U>
std::ostream& operator<<(std::ostream& os, const Equal<T, U>& eq) {
    os << "eq dst=" << eq.dst << ", left=" << eq.left << ", right=" << eq.right;
    return os;
}

template <ast::BaseType T, ast::BaseType U>
std::ostream& operator<<(std::ostream& os, const NotEqual<T, U>& neq) {
    os << "neq dst=" << neq.dst << ", left=" << neq.left << ", right=" << neq.right;
    return os;
}

template <ast::BaseType T, ast::BaseType U>
std::ostream& operator<<(std::ostream& os, const GreaterThan<T, U>& gt) {
    os << "gt dst=" << gt.dst << ", left=" << gt.left << ", right=" << gt.right;
    return os;
}

template <ast::BaseType T, ast::BaseType U>
std::ostream& operator<<(std::ostream& os, const LessThan<T, U>& lt) {
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

std::ostream& operator<<(std::ostream& os, const PointerOffset& pointer_offset) {
    os << "pointer_offset dst=" << pointer_offset.dst << ", src=" << pointer_offset.base
       << ", offset=" << pointer_offset.offset;
    return os;
}

}  // namespace qa_ir