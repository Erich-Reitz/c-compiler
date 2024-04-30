#include "../../../include/compiler/target/codegen.hpp"
#include "../../../include/compiler/target/qa_x86.hpp"


#include <iostream>

namespace target {

class CodegenContext {
   public:
    std::string Code = "";

    void AddInstructionNoIndent(const std::string& i) { Code += (i + "\n"); }

    void AddInstruction(const std::string& i) { Code += "\t" + i + "\n"; }
};


template<target::HasToAsmMethod T>
auto generate_asm(const T& obj) -> std::string {
    return obj.to_asm(); 
} 

void generateASMForFrame(const target::Frame& frame, CodegenContext& ctx) {
    ctx.AddInstructionNoIndent(frame.name + ":");
    ctx.AddInstruction("push rbp");
    ctx.AddInstruction("mov rbp, rsp");
    ctx.AddInstruction("sub rsp, " + std::to_string(target::sixteenByteAlign(frame.size)));
    for (const auto& v_is : frame.instructions) {
        const auto instrct_str = std::visit([](auto &&arg) {return generate_asm(arg); }, v_is ) ; 
        ctx.AddInstructionNoIndent(instrct_str); 
    }
    ctx.AddInstructionNoIndent(".end:");
    if (frame.size > 0) {
        ctx.AddInstruction("leave");
    } else {
        ctx.AddInstruction("pop rbp");
    }
    ctx.AddInstruction("ret");
}

[[nodiscard]] std::string Generate(const std::vector<target::Frame>& frames) {
    CodegenContext ctx;
    ctx.AddInstructionNoIndent("section .text");
    ctx.AddInstructionNoIndent("global _start");
    for (const auto& frame : frames) {
        generateASMForFrame(frame, ctx);
    }
    ctx.AddInstructionNoIndent("_start:");
    ctx.AddInstruction("call main");
    ctx.AddInstruction("mov edi, eax");
    ctx.AddInstruction("mov eax, 60");
    ctx.AddInstruction("syscall");
    return ctx.Code;
}
}  // namespace codegen