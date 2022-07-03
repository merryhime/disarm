// SPDX-FileCopyrightText: 2022 merryhime
// SPDX-License-Identifier: MIT

#include <cstdint>
#include <cstdio>
#include <string>

#include <llvm-c/Disassembler.h>
#include <llvm-c/Target.h>

std::string DisassembleAArch32(bool is_thumb, std::uint32_t pc, const std::uint8_t* instructions, size_t length) {
    std::string result;

    LLVMInitializeARMTargetInfo();
    LLVMInitializeARMTargetMC();
    LLVMInitializeARMDisassembler();
    LLVMDisasmContextRef llvm_ctx = LLVMCreateDisasm(is_thumb ? "thumbv8-arm" : "armv8-arm", nullptr, 0, nullptr, nullptr);
    LLVMSetDisasmOptions(llvm_ctx, LLVMDisassembler_Option_AsmPrinterVariant);

    char buffer[160];
    while (length) {
        size_t inst_size = LLVMDisasmInstruction(llvm_ctx, const_cast<std::uint8_t*>(instructions), length, pc, buffer, sizeof(buffer));

        result += inst_size > 0 ? buffer : "<invalid instruction>";
        result += '\n';

        if (inst_size == 0)
            inst_size = is_thumb ? 2 : 4;
        if (length <= inst_size)
            break;

        pc += inst_size;
        instructions += inst_size;
        length -= inst_size;
    }

    LLVMDisasmDispose(llvm_ctx);

    return result;
}

std::string DisassembleAArch64(std::uint64_t pc, std::uint32_t instruction) {
    std::string result;

    LLVMInitializeAArch64TargetInfo();
    LLVMInitializeAArch64TargetMC();
    LLVMInitializeAArch64Disassembler();
    LLVMDisasmContextRef llvm_ctx = LLVMCreateDisasm("aarch64", nullptr, 0, nullptr, nullptr);
    LLVMSetDisasmOptions(llvm_ctx, LLVMDisassembler_Option_AsmPrinterVariant);

    char buffer[160];
    size_t inst_size = LLVMDisasmInstruction(llvm_ctx, (std::uint8_t*)&instruction, sizeof(instruction), pc, buffer, sizeof(buffer));
    result = inst_size > 0 ? buffer : "<invalid instruction>";
    result += '\n';

    LLVMDisasmDispose(llvm_ctx);

    return result;
}

void PrintA32Instruction(std::uint32_t instruction) {
    const std::string dis = DisassembleAArch32(false, 0, (std::uint8_t*)&instruction, sizeof(instruction));
    std::printf("%s\n", dis.c_str());
}

void PrintA64Instruction(std::uint32_t instruction) {
    const std::string dis = DisassembleAArch64(0, instruction);
    std::printf("%s\n", dis.c_str());
}

void PrintThumbInstruction(std::uint32_t instruction) {
    const size_t inst_size = (instruction >> 16) == 0 ? 2 : 4;
    const std::string dis = DisassembleAArch32(true, 0, (std::uint8_t*)&instruction, inst_size);
    std::printf("%s\n", dis.c_str());
}

int main(int argc, char** argv) {
    if (argc < 3 || argc > 4) {
        std::printf("usage: %s <a32/a64/thumb> <instruction_in_hex>\n", argv[0]);
        return 1;
    }

    const char* const hex_instruction = [argv] {
        if (strlen(argv[2]) > 2 && argv[2][0] == '0' && argv[2][1] == 'x') {
            return argv[2] + 2;
        }
        return argv[2];
    }();

    if (strlen(hex_instruction) > 8) {
        std::printf("hex string too long\n");
        return 1;
    }

    const std::uint32_t instruction = std::strtol(hex_instruction, nullptr, 16);

    if (std::strcmp(argv[1], "a32") == 0) {
        PrintA32Instruction(instruction);
    } else if (std::strcmp(argv[1], "a64") == 0) {
        PrintA64Instruction(instruction);
    } else if (std::strcmp(argv[1], "t32") == 0 || strcmp(argv[1], "t16") == 0 || strcmp(argv[1], "thumb") == 0) {
        PrintThumbInstruction(instruction);
    } else {
        std::printf("Invalid mode: %s\nValid values: a32, a64, thumb\n", argv[1]);
        return 1;
    }

    return 0;
}
