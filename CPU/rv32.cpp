#include "rv32.h"
#include <fstream>
#include <cstring>
#include <cstdio>

RV32CPU::RV32CPU() {
    reset();
}

void RV32CPU::reset() {
    // 清零寄存器
    memset(regs, 0, sizeof(regs));
    
    // 清零内存
    memset(memory, 0, sizeof(memory));
    
    // 初始化程序计数器
    pc = 0x1000; // 程序起始地址
    
    // 初始化栈指针 (x2 = sp)
    sp = RV32_MEM_SIZE - 4; // 栈从内存顶部开始
    regs[2] = sp;
}

void RV32CPU::load_program(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return;
    }
    
    // 读取程序到内存
    word_t addr = 0x1000; // 程序加载地址
    while (file.good() && addr < RV32_MEM_SIZE - 4) {
        word_t instruction;
        file.read(reinterpret_cast<char*>(&instruction), sizeof(instruction));
        if (file.gcount() == sizeof(instruction)) {
            write_memory(addr, instruction);
            addr += 4;
        }
    }
    
    file.close();
    std::cout << "程序已加载到地址 0x1000" << std::endl;
}

void RV32CPU::execute() {
    std::cout << "开始执行RV32程序..." << std::endl;
    
    while (pc < RV32_MEM_SIZE - 4) {
        // 读取指令
        word_t instruction_raw = read_memory(pc);
        RV32Instruction inst;
        inst.raw = instruction_raw;
        
        // 检查是否是系统调用 (ECALL)
        if (inst.r_type.opcode == OP_SYSTEM && inst.i_type.funct3 == 0) {
            std::cout << "遇到系统调用，程序结束" << std::endl;
            break;
        }
        
        // 执行指令
        execute_instruction(inst);
        
        // 更新程序计数器 (除了分支和跳转指令)
        if (inst.r_type.opcode != OP_BRANCH && 
            inst.r_type.opcode != OP_JAL && 
            inst.r_type.opcode != OP_JALR) {
            pc += 4;
        }
    }
}

void RV32CPU::execute_instruction(RV32Instruction& inst) {
    switch (inst.r_type.opcode) {
        case OP_LOAD:
            execute_load(inst);
            break;
        case OP_STORE:
            execute_store(inst);
            break;
        case OP_OP_IMM:
            execute_op_imm(inst);
            break;
        case OP_OP:
            execute_op(inst);
            break;
        case OP_BRANCH:
            execute_branch(inst);
            break;
        case OP_JALR:
            execute_jalr(inst);
            break;
        case OP_JAL:
            execute_jal(inst);
            break;
        case OP_LUI:
            execute_lui(inst);
            break;
        case OP_AUIPC:
            execute_auipc(inst);
            break;
        case OP_SYSTEM:
            execute_system(inst);
            break;
        default:
            std::cerr << "未知操作码: 0x" << std::hex << inst.r_type.opcode << std::endl;
            break;
    }
}

void RV32CPU::execute_load(RV32Instruction& inst) {
    word_t addr = regs[inst.i_type.rs1] + sign_extend(inst.i_type.imm, 12);
    word_t data = 0;
    
    switch (inst.i_type.funct3) {
        case F3_LB:  // 加载字节
            data = sign_extend(read_memory(addr) & 0xFF, 8);
            break;
        case F3_LH:  // 加载半字
            data = sign_extend(read_memory(addr) & 0xFFFF, 16);
            break;
        case F3_LW:  // 加载字
            data = read_memory(addr);
            break;
        case F3_LBU: // 加载无符号字节
            data = read_memory(addr) & 0xFF;
            break;
        case F3_LHU: // 加载无符号半字
            data = read_memory(addr) & 0xFFFF;
            break;
        default:
            std::cerr << "未知加载指令" << std::endl;
            return;
    }
    
    if (inst.i_type.rd != 0) { // x0寄存器不能写入
        regs[inst.i_type.rd] = data;
    }
    
    std::cout << "LOAD: x" << inst.i_type.rd << " = memory[" << std::hex << addr << "] = " << std::dec << data << std::endl;
}

void RV32CPU::execute_store(RV32Instruction& inst) {
    word_t addr = regs[inst.s_type.rs1] + sign_extend((inst.s_type.imm_11_5 << 5) | inst.s_type.imm_4_0, 12);
    word_t data = regs[inst.s_type.rs2];
    
    switch (inst.s_type.funct3) {
        case F3_SB: // 存储字节
            write_memory(addr, data & 0xFF);
            break;
        case F3_SH: // 存储半字
            write_memory(addr, data & 0xFFFF);
            break;
        case F3_SW: // 存储字
            write_memory(addr, data);
            break;
        default:
            std::cerr << "未知存储指令" << std::endl;
            return;
    }
    
    std::cout << "STORE: memory[" << std::hex << addr << "] = x" << inst.s_type.rs2 << " = " << std::dec << data << std::endl;
}

void RV32CPU::execute_op_imm(RV32Instruction& inst) {
    word_t rs1_val = regs[inst.i_type.rs1];
    word_t imm = sign_extend(inst.i_type.imm, 12);
    word_t result = 0;
    
    switch (inst.i_type.funct3) {
        case F3_ADDI:  // 加法立即数
            result = rs1_val + imm;
            break;
        case F3_SLTI:  // 小于立即数
            result = (static_cast<int32_t>(rs1_val) < static_cast<int32_t>(imm)) ? 1 : 0;
            break;
        case F3_SLTIU: // 小于无符号立即数
            result = (rs1_val < static_cast<word_t>(imm)) ? 1 : 0;
            break;
        case F3_XORI:  // 异或立即数
            result = rs1_val ^ imm;
            break;
        case F3_ORI:   // 或立即数
            result = rs1_val | imm;
            break;
        case F3_ANDI:  // 与立即数
            result = rs1_val & imm;
            break;
        case F3_SLLI:  // 逻辑左移立即数
            result = rs1_val << (imm & 0x1F);
            break;
        case F3_SRLI:  // 逻辑右移立即数
            if ((inst.i_type.imm >> 10) & 1) { // SRAI
                result = static_cast<int32_t>(rs1_val) >> (imm & 0x1F);
            } else { // SRLI
                result = rs1_val >> (imm & 0x1F);
            }
            break;
        default:
            std::cerr << "未知立即数运算指令" << std::endl;
            return;
    }
    
    if (inst.i_type.rd != 0) {
        regs[inst.i_type.rd] = result;
    }
    
    std::cout << "OP-IMM: x" << inst.i_type.rd << " = x" << inst.i_type.rs1 << " op " << std::dec << imm << " = " << result << std::endl;
}

void RV32CPU::execute_op(RV32Instruction& inst) {
    word_t rs1_val = regs[inst.r_type.rs1];
    word_t rs2_val = regs[inst.r_type.rs2];
    word_t result = 0;
    
    switch (inst.r_type.funct3) {
        case F3_ADD:   // 加法/减法
            if ((inst.r_type.funct7 >> 5) & 1) { // SUB
                result = rs1_val - rs2_val;
            } else { // ADD
                result = rs1_val + rs2_val;
            }
            break;
        case F3_SLL:   // 逻辑左移
            result = rs1_val << (rs2_val & 0x1F);
            break;
        case F3_SLT:   // 小于
            result = (static_cast<int32_t>(rs1_val) < static_cast<int32_t>(rs2_val)) ? 1 : 0;
            break;
        case F3_SLTU:  // 小于无符号
            result = (rs1_val < rs2_val) ? 1 : 0;
            break;
        case F3_XOR:   // 异或
            result = rs1_val ^ rs2_val;
            break;
        case F3_SRL:   // 逻辑右移/算术右移
            if ((inst.r_type.funct7 >> 5) & 1) { // SRA
                result = static_cast<int32_t>(rs1_val) >> (rs2_val & 0x1F);
            } else { // SRL
                result = rs1_val >> (rs2_val & 0x1F);
            }
            break;
        case F3_OR:    // 或
            result = rs1_val | rs2_val;
            break;
        case F3_AND:   // 与
            result = rs1_val & rs2_val;
            break;
        default:
            std::cerr << "未知寄存器运算指令" << std::endl;
            return;
    }
    
    if (inst.r_type.rd != 0) {
        regs[inst.r_type.rd] = result;
    }
    
    std::cout << "OP: x" << inst.r_type.rd << " = x" << inst.r_type.rs1 << " op x" << inst.r_type.rs2 << " = " << result << std::endl;
}

void RV32CPU::execute_branch(RV32Instruction& inst) {
    word_t rs1_val = regs[inst.b_type.rs1];
    word_t rs2_val = regs[inst.b_type.rs2];
    word_t offset = sign_extend((inst.b_type.imm_12 << 12) | 
                               (inst.b_type.imm_11 << 11) | 
                               (inst.b_type.imm_10_5 << 5) | 
                               (inst.b_type.imm_4_1 << 1), 13);
    
    bool taken = false;
    
    switch (inst.b_type.funct3) {
        case F3_BEQ:   // 相等分支
            taken = (rs1_val == rs2_val);
            break;
        case F3_BNE:   // 不相等分支
            taken = (rs1_val != rs2_val);
            break;
        case F3_BLT:   // 小于分支
            taken = (static_cast<int32_t>(rs1_val) < static_cast<int32_t>(rs2_val));
            break;
        case F3_BGE:   // 大于等于分支
            taken = (static_cast<int32_t>(rs1_val) >= static_cast<int32_t>(rs2_val));
            break;
        case F3_BLTU:  // 小于无符号分支
            taken = (rs1_val < rs2_val);
            break;
        case F3_BGEU:  // 大于等于无符号分支
            taken = (rs1_val >= rs2_val);
            break;
        default:
            std::cerr << "未知分支指令" << std::endl;
            return;
    }
    
    if (taken) {
        pc = pc + offset;
        std::cout << "BRANCH: 跳转到 " << std::hex << pc << std::endl;
    } else {
        pc += 4;
        std::cout << "BRANCH: 不跳转" << std::endl;
    }
}

void RV32CPU::execute_jalr(RV32Instruction& inst) {
    word_t target = regs[inst.i_type.rs1] + sign_extend(inst.i_type.imm, 12);
    target &= ~1; // 清除最低位
    
    if (inst.i_type.rd != 0) {
        regs[inst.i_type.rd] = pc + 4;
    }
    
    pc = target;
    std::cout << "JALR: 跳转到 " << std::hex << pc << ", 返回地址保存到 x" << inst.i_type.rd << std::endl;
}

void RV32CPU::execute_jal(RV32Instruction& inst) {
    word_t offset = sign_extend((inst.j_type.imm_20 << 20) | 
                               (inst.j_type.imm_19_12 << 12) | 
                               (inst.j_type.imm_11 << 11) | 
                               (inst.j_type.imm_10_1 << 1), 21);
    
    if (inst.j_type.rd != 0) {
        regs[inst.j_type.rd] = pc + 4;
    }
    
    pc = pc + offset;
    std::cout << "JAL: 跳转到 " << std::hex << pc << ", 返回地址保存到 x" << inst.j_type.rd << std::endl;
}

void RV32CPU::execute_lui(RV32Instruction& inst) {
    word_t imm = inst.u_type.imm << 12;
    
    if (inst.u_type.rd != 0) {
        regs[inst.u_type.rd] = imm;
    }
    
    std::cout << "LUI: x" << inst.u_type.rd << " = " << std::hex << imm << std::endl;
}

void RV32CPU::execute_auipc(RV32Instruction& inst) {
    word_t imm = inst.u_type.imm << 12;
    word_t result = pc + imm;
    
    if (inst.u_type.rd != 0) {
        regs[inst.u_type.rd] = result;
    }
    
    std::cout << "AUIPC: x" << inst.u_type.rd << " = PC + " << std::hex << imm << " = " << result << std::endl;
}

void RV32CPU::execute_system(RV32Instruction& inst) {
    if (inst.i_type.funct3 == 0) {
        std::cout << "ECALL: 系统调用" << std::endl;
    } else {
        std::cerr << "未知系统指令" << std::endl;
    }
}

word_t RV32CPU::read_memory(word_t addr) {
    if (addr >= RV32_MEM_SIZE - 3) {
        std::cerr << "内存访问越界: " << std::hex << addr << std::endl;
        return 0;
    }
    
    // 小端序读取
    return (static_cast<word_t>(memory[addr + 3]) << 24) |
           (static_cast<word_t>(memory[addr + 2]) << 16) |
           (static_cast<word_t>(memory[addr + 1]) << 8) |
           static_cast<word_t>(memory[addr]);
}

void RV32CPU::write_memory(word_t addr, word_t data) {
    if (addr >= RV32_MEM_SIZE - 3) {
        std::cerr << "内存访问越界: " << std::hex << addr << std::endl;
        return;
    }
    
    // 小端序写入
    memory[addr] = data & 0xFF;
    memory[addr + 1] = (data >> 8) & 0xFF;
    memory[addr + 2] = (data >> 16) & 0xFF;
    memory[addr + 3] = (data >> 24) & 0xFF;
}

word_t RV32CPU::sign_extend(uint32_t value, int bits) {
    if (value & (1 << (bits - 1))) {
        return value | (0xFFFFFFFF << bits);
    }
    return value;
}

void RV32CPU::dump_registers() {
    std::cout << "\n=== 寄存器状态 ===" << std::endl;
    for (int i = 0; i < 32; i += 4) {
        printf("x%02d: %08x  x%02d: %08x  x%02d: %08x  x%02d: %08x\n",
               i, regs[i], i+1, regs[i+1], i+2, regs[i+2], i+3, regs[i+3]);
    }
    std::cout << "PC: " << std::hex << pc << std::endl;
}

void RV32CPU::dump_memory(word_t start, word_t end) {
    std::cout << "\n=== 内存状态 (" << std::hex << start << "-" << end << ") ===" << std::endl;
    for (word_t addr = start; addr < end; addr += 16) {
        printf("%08x: ", addr);
        for (int i = 0; i < 16 && addr + i < end; i++) {
            printf("%02x ", memory[addr + i]);
        }
        std::cout << std::endl;
    }
}
