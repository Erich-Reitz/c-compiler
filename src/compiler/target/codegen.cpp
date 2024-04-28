#include "../../../include/compiler/target/codegen.hpp"

namespace target {

class CodeGenContext {
   public:
    std::string Code = "";

    void AddInstructionNoIndent(const std::string& i) { Code += (i + "\n"); }

    void AddInstruction(const std::string& i) { Code += "\t" + i + "\n"; }
};

void MoveInstruction(const Mov mov, CodeGenContext& ctx) {
    if (std::get<HardcodedRegister>(mov.dst) ==
        std::get<HardcodedRegister>(mov.src))
        return;
    auto dst = std::get<HardcodedRegister>(mov.dst);
    auto src = std::get<HardcodedRegister>(mov.src);

    ctx.AddInstruction("mov " + to_asm(dst.reg, dst.size) + ", " + to_asm(src.reg, src.size)); 
}



std::string to_asm(StackLocation sl) {
      if (sl.is_computed) {
        if (std::holds_alternative<VirtualRegister>(sl.src)) {
            throw std::runtime_error("Cannot compute with virtual register");
        }
        const auto base = std::get<HardcodedRegister>(sl.src);
        const auto initalOffset = sl.offset;
        const auto scale = sl.scale;

        std::string scale_string = "";
        if (scale != 1) {
            scale_string = " * " + std::to_string(scale);
        }

        if (initalOffset == 0) {
            // don't return relative to rbp
            return " [0+" + to_asm(base.reg, 8) + scale_string + "]";
        }

        // if we are using it as an offset, then its size is 8.
        // TODO: issue a clear instruction
        return " [rbp-" + std::to_string(initalOffset) + " + " + to_asm(base.reg, 8) + scale_string + "]";
    }

    if (sl.offset >= 0) {
        return " [rbp - " + std::to_string(sl.offset) + "]";
    }
    return " [rbp + " + std::to_string(-sl.offset) + "]"; 
}

void generateASMForInstruction(const Instruction& is, CodeGenContext& ctx) {
    // std::cout << "codegen for " << is << std::endl;
    if (std::holds_alternative<Mov>(is)) {
        MoveInstruction(std::get<Mov>(is), ctx);
    } else if (std::holds_alternative<Jump>(is)) {
        const auto jump = std::get<Jump>(is);
        ctx.AddInstruction("jmp ." + jump.label);
    } else if (std::holds_alternative<LoadI>(is)) {
        const auto loadI = std::get<LoadI>(is);
        const auto dst = std::get<HardcodedRegister>(loadI.dst);
        ctx.AddInstruction("mov " + to_asm(dst.reg, dst.size) + ", " +
                           std::to_string(loadI.value));
    } else if (std::holds_alternative<StoreI>(is)) {
        const auto storeI = std::get<StoreI>(is);
        const auto sourcesizeString = std::string("dword");
        ctx.AddInstruction("mov " + sourcesizeString + to_asm(storeI.dst) + ", " +
                           std::to_string(storeI.value));
    } else if (std::holds_alternative<Store>(is)) {
        const auto store = std::get<Store>(is);
        const auto src = std::get<HardcodedRegister>(store.src);
        const auto sourcesizeString = src.size == 4 ? std::string("dword") : std::string("qword");
        ctx.AddInstruction("mov " + sourcesizeString + to_asm(store.dst) + ", " +
                           to_asm(src.reg, src.size));
    } else if (std::holds_alternative<Load>(is)) {
        const auto load = std::get<Load>(is);
        const auto dst = std::get<HardcodedRegister>(load.dst);
        const auto sourcesizeString = dst.size == 4 ? std::string("dword") : std::string("qword");
        ctx.AddInstruction("mov " + to_asm(dst.reg, dst.size) + ", " + sourcesizeString +
                           to_asm(load.src));
    } else if (std::holds_alternative<AddI>(is)) {
        const auto addI = std::get<AddI>(is);
        const auto dst = std::get<HardcodedRegister>(addI.dst);
        ctx.AddInstruction("add " + to_asm(dst.reg, dst.size) + ", " +
                           std::to_string(addI.value));
    } else if (std::holds_alternative<Add>(is)) {
        const auto add = std::get<Add>(is);
        const auto dst = std::get<HardcodedRegister>(add.dst);
        const auto src = std::get<HardcodedRegister>(add.src);
        ctx.AddInstruction("add " + to_asm(dst.reg, dst.size) + ", " +
                           to_asm(src.reg, src.size));
    } else if (std::holds_alternative<SubI>(is)) {
        const auto subI = std::get<SubI>(is);
        const auto dst = std::get<HardcodedRegister>(subI.dst);
        ctx.AddInstruction("sub " + to_asm(dst.reg, dst.size) + ", " +
                           std::to_string(subI.value));
    } else if (std::holds_alternative<Sub>(is)) {
        const auto sub = std::get<Sub>(is);
        const auto dst = std::get<HardcodedRegister>(sub.dst);
        const auto src = std::get<HardcodedRegister>(sub.src);
        ctx.AddInstruction("sub " + to_asm(dst.reg, dst.size) + ", " +
                           to_asm(src.reg, src.size));
    } else if (std::holds_alternative<Cmp>(is)) {
        const auto cmp = std::get<Cmp>(is);
        const auto left = std::get<HardcodedRegister>(cmp.dst);
        const auto right = std::get<HardcodedRegister>(cmp.src);
        ctx.AddInstruction("cmp " + to_asm(left.reg, left.size) + ", " +
                           to_asm(right.reg, right.size));
    } else if (std::holds_alternative<CmpI>(is)) {
        const auto cmpI = std::get<CmpI>(is);
        const auto left = std::get<HardcodedRegister>(cmpI.dst);
        ctx.AddInstruction("cmp " + to_asm(left.reg, left.size) + ", " +
                           std::to_string(cmpI.value));
    } else if (std::holds_alternative<SetEAl>(is)) {
        ctx.AddInstruction("sete al");
        const auto dst = std::get<HardcodedRegister>(std::get<SetEAl>(is).dst);
        ctx.AddInstruction("movzx " + to_asm(dst.reg, dst.size) + ", al");
    } else if (std::holds_alternative<JumpEq>(is)) {
        const auto jump = std::get<JumpEq>(is);
        ctx.AddInstruction("je ." + jump.label);
    } else if (std::holds_alternative<Label>(is)) {
        const auto label = std::get<Label>(is);
        ctx.AddInstructionNoIndent("." + label.name + ":");
    } else if (std::holds_alternative<Call>(is)) {
        const auto call = std::get<Call>(is);
        ctx.AddInstruction("call " + call.name);
    } else if (std::holds_alternative<Lea>(is)) {
        const auto lea = std::get<Lea>(is);
        const auto dst = std::get<HardcodedRegister>(lea.dst);
        const auto dstsize = dst.size;
        ctx.AddInstruction("lea " + to_asm(dst.reg, dstsize) + ", " + to_asm(lea.src));
    } else if (std::holds_alternative<IndirectLoad>(is)) {
        const auto imao = std::get<IndirectLoad>(is);
        const auto dst = std::get<HardcodedRegister>(imao.dst);
        const auto src = std::get<HardcodedRegister>(imao.src);
        const auto dstsize = dst.size;
        const auto srcsize = src.size;
        ctx.AddInstruction("mov " + to_asm(dst.reg, dstsize) + ", [" +
                           to_asm(src.reg, srcsize) + "]");
    } else if (std::holds_alternative<IndirectStore>(is)) {
        const auto imao = std::get<IndirectStore>(is);
        const auto dst = std::get<HardcodedRegister>(imao.dst);
        const auto src = std::get<HardcodedRegister>(imao.src);
        const auto dstsize = dst.size;
        const auto srcsize = src.size;
        ctx.AddInstruction("mov [" + to_asm(dst.reg, dstsize) + "], " +
                           to_asm(src.reg, srcsize));
    } else if (std::holds_alternative<JumpGreater>(is)) {
        const auto jump = std::get<JumpGreater>(is);
        ctx.AddInstruction("jg ." + jump.label);
    } else if (std::holds_alternative<SetGAl>(is)) {
        ctx.AddInstruction("setg al");
        const auto dst = std::get<HardcodedRegister>(std::get<SetGAl>(is).dst);
        ctx.AddInstruction("movzx " + to_asm(dst.reg, dst.size) + ", al");
    } else if (std::holds_alternative<PushI>(is)) {
        const auto pushI = std::get<PushI>(is);
        ctx.AddInstruction("push " + std::to_string(pushI.src));
    } else if (std::holds_alternative<Push>(is)) {
        const auto push = std::get<Push>(is);
        const auto src = std::get<HardcodedRegister>(push.src);
        ctx.AddInstruction("push " + to_asm(src.reg, 8));
    } else if (std::holds_alternative<AddMI>(is)) {
        const auto addMI = std::get<AddMI>(is);
        const StackLocation dst = addMI.dst;
        const auto v = addMI.value;
        const auto sourceSizeString = std::string("dword");
        ctx.AddInstruction("add " + sourceSizeString + to_asm(dst) + ", " + std::to_string(v));
    } else if (std::holds_alternative<SubMI>(is)) {
        const auto subMI = std::get<SubMI>(is);
        const StackLocation dst = subMI.dst;
        const auto v = subMI.value;
        std::string sourceSizeString = std::string("dword");
        ctx.AddInstruction("add " + sourceSizeString + to_asm(dst) + ", " + std::to_string(v));
    } else if (std::holds_alternative<JumpLess>(is)) {
        const auto jump = std::get<JumpLess>(is);
        ctx.AddInstruction("jl ." + jump.label);
    } else if (std::holds_alternative<SetNeAl>(is)) {

        ctx.AddInstruction("setne al");
        const auto dst = std::get<HardcodedRegister>(std::get<SetNeAl>(is).dst);
        ctx.AddInstruction("movzx " + to_asm(dst.reg, dst.size) + ", al");
    } else if (std::holds_alternative<SetLAl>(is)) {
        ctx.AddInstruction("setl al");
        const auto dst = std::get<HardcodedRegister>(std::get<SetLAl>(is).dst); 
        ctx.AddInstruction("movzx " + to_asm(dst.reg, dst.size) + ", al");
    } else if (std::holds_alternative<ZeroExtend>(is)) {
        const auto zeroExtend = std::get<ZeroExtend>(is);
        const auto dst = std::get<HardcodedRegister>(zeroExtend.dst);
        const auto src = std::get<HardcodedRegister>(zeroExtend.src);
        ctx.AddInstruction("movsx " + to_asm(dst.reg, dst.size) + ", " +
                           to_asm(src.reg, src.size));
    }
    else {
        throw std::runtime_error("Unsupported instruction type" + std::to_string(is.index()));
    }
}



void generateASMForFrame(const Frame& frame, CodeGenContext& ctx) {
    ctx.AddInstructionNoIndent(frame.name + ":");
    ctx.AddInstruction("push rbp");
    ctx.AddInstruction("mov rbp, rsp");
    ctx.AddInstruction("sub rsp, " + std::to_string(sixteenByteAlign(frame.size)));
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

[[nodiscard]] std::string Generate(const std::vector<Frame>& frames) {
    CodeGenContext ctx;
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