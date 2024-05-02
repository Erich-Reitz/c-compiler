#include "../../../include/compiler/target/codegenCtx.hpp"

namespace target {
void CodegenContext::AddInstructionNoIndent(const std::string& i) { Code += (i + "\n"); }

void CodegenContext::AddInstruction(const std::string& i) { Code += "\t" + i + "\n"; }


//  ideally the assembly looks like this.
// section .data
// a: dd 5.000
// b: dd 1.0000
// c: dd 5.50000

std::string CodegenContext::DefineFloatConstant(float value) {
    const auto name = "float_constant_" + std::to_string(float_counter++);
    const auto ins = name + ": dd " + std::to_string(value);
    DataSection += ins + "\n";
    return name;
}
}