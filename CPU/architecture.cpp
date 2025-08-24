#include "architecture.h"
#include "toy.h"
#include "rv32.h"

Architecture* ArchitectureFactory::create_architecture(ArchitectureType type) {
    switch (type) {
        case TOY:
            return new ToyCPU();
        case RV32:
            return new RV32CPU();
        default:
            return nullptr;
    }
}

void ArchitectureFactory::destroy_architecture(Architecture* arch) {
    if (arch) {
        delete arch;
    }
}

std::string ArchitectureFactory::get_architecture_name(ArchitectureType type) {
    switch (type) {
        case TOY:
            return "Toy";
        case RV32:
            return "RV32";
        default:
            return "Unknown";
    }
}

std::string ArchitectureFactory::get_architecture_description(ArchitectureType type) {
    switch (type) {
        case TOY:
            return "16位字长，256个寄存器，64KB内存，支持字节和字操作";
        case RV32:
            return "32位RISC-V指令集，32个寄存器，64KB内存，现代RISC设计";
        default:
            return "未知架构";
    }
}
