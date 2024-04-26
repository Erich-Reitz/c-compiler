#include "../../include/compiler/codegen.hpp"

#include <iostream>

namespace codegen {
class Ctx {
   public:
    std::string Code = "";

    void AddInstructionNoIndent(const std::string& i) { Code += (i + "\n"); }

    void AddInstruction(const std::string& i) { Code += "\t" + i + "\n"; }
};

void MoveInstruction(const target::Mov mov, Ctx& ctx) {
    if (std::get<target::HardcodedRegister>(mov.dst) ==
        std::get<target::HardcodedRegister>(mov.src))
        return;
    ctx.AddInstruction("mov " + target::to_asm(mov));
}

std::string to_asm_special(target::StackLocation sl) {
       if (sl.is_computed) {
        if (std::holds_alternative<target::VirtualRegister>(sl.src)) {
            throw std::runtime_error("Cannot compute with virtual register");
        }
        const auto base = std::get<target::HardcodedRegister>(sl.src);
        const auto initalOffset = sl.offset;
        const auto scale = sl.scale;
        // if we are using it as an offset, then its size is 8.
        // TODO: issue a clear instruction
        return " [rbp-" + std::to_string(initalOffset) + " + " + target::to_asm(base.reg, 8) + " * " + std::to_string(scale) + "]";
    }

    if (sl.offset >= 0) {
        return " [rbp - " + std::to_string(sl.offset) + "]";
    }
    return " [rbp + " + std::to_string(-sl.offset) + "]"; 
}

std::string to_asm(target::StackLocation sl) {
    if (sl.offset >= 0) {
        return " [rbp - " + std::to_string(sl.offset) + "]";
    }
    return " [rbp + " + std::to_string(-sl.offset) + "]";
}

void generateASMForInstruction(const target::Instruction& is, Ctx& ctx) {
    // std::cout << "codegen for " << is << std::endl;
    if (std::holds_alternative<target::Mov>(is)) {
        MoveInstruction(std::get<target::Mov>(is), ctx);
    } else if (std::holds_alternative<target::Jump>(is)) {
        const auto jump = std::get<target::Jump>(is);
        ctx.AddInstruction("jmp ." + jump.label);
    } else if (std::holds_alternative<target::LoadI>(is)) {
        const auto loadI = std::get<target::LoadI>(is);
        const auto dst = std::get<target::HardcodedRegister>(loadI.dst);
        ctx.AddInstruction("mov " + target::to_asm(dst.reg, dst.size) + ", " +
                           std::to_string(loadI.value));
    } else if (std::holds_alternative<target::StoreI>(is)) {
        const auto storeI = std::get<target::StoreI>(is);
        const auto sourcesizeString = std::string("dword");
        ctx.AddInstruction("mov " + sourcesizeString + to_asm(storeI.dst) + ", " +
                           std::to_string(storeI.value));
    } else if (std::holds_alternative<target::Store>(is)) {
        const auto store = std::get<target::Store>(is);
        const auto src = std::get<target::HardcodedRegister>(store.src);
        const auto sourcesizeString = src.size == 4 ? std::string("dword") : std::string("qword");
        ctx.AddInstruction("mov " + sourcesizeString + to_asm_special(store.dst) + ", " +
                           target::to_asm(src.reg, src.size));
    } else if (std::holds_alternative<target::Load>(is)) {
        const auto load = std::get<target::Load>(is);
        const auto dst = std::get<target::HardcodedRegister>(load.dst);
        const auto sourcesizeString = dst.size == 4 ? std::string("dword") : std::string("qword");
        ctx.AddInstruction("mov " + target::to_asm(dst.reg, dst.size) + ", " + sourcesizeString +
                           to_asm(load.src));
    } else if (std::holds_alternative<target::AddI>(is)) {
        const auto addI = std::get<target::AddI>(is);
        const auto dst = std::get<target::HardcodedRegister>(addI.dst);
        ctx.AddInstruction("add " + target::to_asm(dst.reg, dst.size) + ", " +
                           std::to_string(addI.value));
    } else if (std::holds_alternative<target::Add>(is)) {
        const auto add = std::get<target::Add>(is);
        const auto dst = std::get<target::HardcodedRegister>(add.dst);
        const auto src = std::get<target::HardcodedRegister>(add.src);
        ctx.AddInstruction("add " + target::to_asm(dst.reg, dst.size) + ", " +
                           target::to_asm(src.reg, src.size));
    } else if (std::holds_alternative<target::SubI>(is)) {
        const auto subI = std::get<target::SubI>(is);
        const auto dst = std::get<target::HardcodedRegister>(subI.dst);
        ctx.AddInstruction("sub " + target::to_asm(dst.reg, dst.size) + ", " +
                           std::to_string(subI.value));
    } else if (std::holds_alternative<target::Sub>(is)) {
        const auto sub = std::get<target::Sub>(is);
        const auto dst = std::get<target::HardcodedRegister>(sub.dst);
        const auto src = std::get<target::HardcodedRegister>(sub.src);
        ctx.AddInstruction("sub " + target::to_asm(dst.reg, dst.size) + ", " +
                           target::to_asm(src.reg, src.size));
    } else if (std::holds_alternative<target::Cmp>(is)) {
        const auto cmp = std::get<target::Cmp>(is);
        const auto left = std::get<target::HardcodedRegister>(cmp.dst);
        const auto right = std::get<target::HardcodedRegister>(cmp.src);
        ctx.AddInstruction("cmp " + target::to_asm(left.reg, left.size) + ", " +
                           target::to_asm(right.reg, right.size));
    } else if (std::holds_alternative<target::CmpI>(is)) {
        const auto cmpI = std::get<target::CmpI>(is);
        const auto left = std::get<target::HardcodedRegister>(cmpI.dst);
        ctx.AddInstruction("cmp " + target::to_asm(left.reg, left.size) + ", " +
                           std::to_string(cmpI.value));
    } else if (std::holds_alternative<target::SetEAl>(is)) {
        const auto SetEAl = std::get<target::SetEAl>(is);
        ctx.AddInstruction("sete al");
        const auto dst = std::get<target::HardcodedRegister>(SetEAl.dst);
        ctx.AddInstruction("movzx " + target::to_asm(dst.reg, dst.size) + ", al");
    } else if (std::holds_alternative<target::JumpEq>(is)) {
        const auto jump = std::get<target::JumpEq>(is);
        ctx.AddInstruction("je ." + jump.label);
    } else if (std::holds_alternative<target::Label>(is)) {
        const auto label = std::get<target::Label>(is);
        ctx.AddInstructionNoIndent("." + label.name + ":");
    } else if (std::holds_alternative<target::Call>(is)) {
        const auto call = std::get<target::Call>(is);
        ctx.AddInstruction("call " + call.name);
    } else if (std::holds_alternative<target::Lea>(is)) {
        const auto lea = std::get<target::Lea>(is);
        const auto dst = std::get<target::HardcodedRegister>(lea.dst);
        const auto dstsize = dst.size;
        ctx.AddInstruction("lea " + target::to_asm(dst.reg, dstsize) + ", [rbp - " +
                           std::to_string(lea.src.offset) + "]");
    } else if (std::holds_alternative<target::IndirectLoad>(is)) {
        const auto imao = std::get<target::IndirectLoad>(is);
        const auto dst = std::get<target::HardcodedRegister>(imao.dst);
        const auto src = std::get<target::HardcodedRegister>(imao.src);
        const auto dstsize = dst.size;
        const auto srcsize = src.size;
        ctx.AddInstruction("mov " + target::to_asm(dst.reg, dstsize) + ", [" +
                           target::to_asm(src.reg, srcsize) + "]");
    } else if (std::holds_alternative<target::IndirectStore>(is)) {
        const auto imao = std::get<target::IndirectStore>(is);
        const auto dst = std::get<target::HardcodedRegister>(imao.dst);
        const auto src = std::get<target::HardcodedRegister>(imao.src);
        const auto dstsize = dst.size;
        const auto srcsize = src.size;
        ctx.AddInstruction("mov [" + target::to_asm(dst.reg, dstsize) + "], " +
                           target::to_asm(src.reg, srcsize));
    } else if (std::holds_alternative<target::JumpGreater>(is)) {
        const auto jump = std::get<target::JumpGreater>(is);
        ctx.AddInstruction("jg ." + jump.label);
    } else if (std::holds_alternative<target::SetGAl>(is)) {
        const auto SetGAl = std::get<target::SetGAl>(is);
        ctx.AddInstruction("setg al");
        const auto dst = std::get<target::HardcodedRegister>(SetGAl.dst);
        ctx.AddInstruction("movzx " + target::to_asm(dst.reg, dst.size) + ", al");
    } else if (std::holds_alternative<target::PushI>(is)) {
        const auto pushI = std::get<target::PushI>(is);
        ctx.AddInstruction("push " + std::to_string(pushI.src));
    } else if (std::holds_alternative<target::Push>(is)) {
        const auto push = std::get<target::Push>(is);
        const auto src = std::get<target::HardcodedRegister>(push.src);
        ctx.AddInstruction("push " + target::to_asm(src.reg, 8));
    } else if (std::holds_alternative<target::AddMI>(is)) {
        const auto addMI = std::get<target::AddMI>(is);
        const target::StackLocation dst = addMI.dst;
        const auto v = addMI.value;
        const auto sourceSizeString = std::string("dword");
        ctx.AddInstruction("add " + sourceSizeString + to_asm(dst) + ", " + std::to_string(v));
    } else if (std::holds_alternative<target::SubMI>(is)) {
        const auto subMI = std::get<target::SubMI>(is);
        const target::StackLocation dst = subMI.dst;
        const auto v = subMI.value;
        std::string sourceSizeString = std::string("dword");
        ctx.AddInstruction("add " + sourceSizeString + to_asm(dst) + ", " + std::to_string(v));
    } else if (std::holds_alternative<target::JumpLess>(is)) {
        const auto jump = std::get<target::JumpLess>(is);
        ctx.AddInstruction("jl ." + jump.label);
    } else if (std::holds_alternative<target::SetNeAl>(is)) {
        const auto SetNeAl = std::get<target::SetNeAl>(is);
        ctx.AddInstruction("setne al");
        const auto dst = std::get<target::HardcodedRegister>(SetNeAl.dst);
        ctx.AddInstruction("movzx " + target::to_asm(dst.reg, dst.size) + ", al");
    } else if (std::holds_alternative<target::SetLAl>(is)) {
        const auto SetLAl = std::get<target::SetLAl>(is);
        ctx.AddInstruction("setl al");
        const auto dst = std::get<target::HardcodedRegister>(SetLAl.dst);
        ctx.AddInstruction("movzx " + target::to_asm(dst.reg, dst.size) + ", al");
    } else if (std::holds_alternative<target::ZeroExtend>(is)) {
        const auto zeroExtend = std::get<target::ZeroExtend>(is);
        const auto dst = std::get<target::HardcodedRegister>(zeroExtend.dst);
        const auto src = std::get<target::HardcodedRegister>(zeroExtend.src);
        ctx.AddInstruction("movsx " + target::to_asm(dst.reg, dst.size) + ", " +
                           target::to_asm(src.reg, src.size));
    }
    else {
        throw std::runtime_error("Unsupported instruction type" + std::to_string(is.index()));
    }
}



void generateASMForFrame(const target::Frame& frame, Ctx& ctx) {
    ctx.AddInstructionNoIndent(frame.name + ":");
    ctx.AddInstruction("push rbp");
    ctx.AddInstruction("mov rbp, rsp");
    ctx.AddInstruction("sub rsp, " + std::to_string(target::sixteenByteAlign(frame.size)));
    for (const auto& is : frame.instructions) {
        try {
            generateASMForInstruction(is, ctx);
        } catch (const std::exception& e) {
            std::cerr << "Error generating ASM for operation: " << e.what() << std::endl;
            throw;
        }
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
    Ctx ctx;
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