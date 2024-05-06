#include "../../../include/compiler/target/codegen.hpp"

#include <iostream>

#include "../../../include/compiler/target/codegenCtx.hpp"
#include "../../../include/compiler/target/qa_x86.hpp"
#include "../../../include/compiler/target/qa_x86_instructions.hpp"

namespace target {

template <target::HasToAsmMethod T>
auto generate_asm(const T& obj, CodegenContext& ctx) -> void {
    obj.to_asm(ctx);
}

void generateASMForFrame(const target::Frame& frame, CodegenContext& ctx) {
    ctx.AddInstructionNoIndent(frame.name + ":");
    ctx.AddInstruction("push rbp");
    ctx.AddInstruction("mov rbp, rsp");
    ctx.AddInstruction("sub rsp, " + std::to_string(target::sixteenByteAlign(frame.size)));
    for (const auto& v_is : frame.instructions) {
        std::visit([&ctx](auto&& arg) { generate_asm(arg, ctx); }, v_is);
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
    return ctx.DataSection + ctx.Code;
}
}  // namespace target