#include "toy.h"
#include <fstream>
#include <cstring>
#include <cstdio>

ToyCPU::ToyCPU() {
    reset();
}

void ToyCPU::reset() {
    // 清零寄存器
    memset(REG, 0, sizeof(REG));
    
    // 清零内存
    memset(RAM, 0, sizeof(RAM));
    
    // 初始化寄存器
    SP = TOY_MEM_SIZE - 4;
    BP = SI = DI = 0;
    CS = DS = ES = SS = FS = GS = 0;
    IP = 0;
    IBUS = DBUS = ABUS = 0;
    
    // 清零ALU
    alu.OP = 0;
    alu.RA = alu.RB = alu.R = 0;
    alu.FR = 0;
    
    // 初始化状态
    running = true;
    instruction_count = 0;
}

void ToyCPU::load_program(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return;
    }
    
    // 读取程序头信息
    WORD DS, CS, LENGTH;
    file.read(reinterpret_cast<char*>(&DS), sizeof(WORD));
    file.read(reinterpret_cast<char*>(&CS), sizeof(WORD));
    file.read(reinterpret_cast<char*>(&LENGTH), sizeof(WORD));
    
    // 读取程序到内存
    for (int i = 0; i < LENGTH; i++) {
        file.read(reinterpret_cast<char*>(&RAM[i]), sizeof(BYTE));
    }
    
    file.close();
    
    // 设置段寄存器
    this->DS = DS;
    this->CS = CS;
    IP = CS;
    
    std::cout << "Toy程序已加载" << std::endl;
    std::cout << "DS: " << DS << ", CS: " << CS << ", LENGTH: " << LENGTH << std::endl;
}

void ToyCPU::execute() {
    std::cout << "开始执行Toy程序..." << std::endl;
    
    while (IP < TOY_MEM_SIZE - 4 && running) {
        execute_instruction();
        instruction_count++;
    }
}

void ToyCPU::execute_instruction() {
    // 读取指令
    BYTE OP = RAM[IP++];
    if (OP == HALT) {
        std::cout << "遇到HALT指令，程序结束" << std::endl;
        running = false;
        return;
    }
    
    // 解码指令
    BYTE TYPE = OP & MR_BYTE;
    BYTE MR = OP & MR_B;
    OP &= (~MR_BYTE);
    OP &= (~MR_B);
    
    // 执行指令
    switch (OP) {
        case ADD:
        case SUB:
        case MUL:
        case DIV:
        case MOD:
        case CMP:
        case NEG:
            execute_arithmetic();
            break;
        case JE:
        case JNE:
        case JG:
        case JB:
        case JGE:
        case JBE:
            execute_jump();
            break;
        case PUSH:
        case POP:
            execute_stack();
            break;
        case LOAD:
        case STORE:
            execute_memory();
            break;
        case CALL:
        case RET:
            execute_function();
            break;
        default:
            std::cerr << "未知操作码: 0x" << std::hex << OP << std::endl;
            break;
    }
}

void ToyCPU::execute_arithmetic() {
    BYTE OP = RAM[IP-1];
    
    BYTE TYPE = OP & MR_BYTE;
    OP &= (~MR_BYTE);
    OP &= (~MR_B);
    
    alu.OP = OP;
    
    if (TYPE == MR_BYTE) {
        // 字节操作
        BYTE ABUS = ReadB();
        alu.RA = REG[ABUS];
        ABUS = ReadB();
        alu.RB = REG[ABUS];
        alu.execute();
        ABUS = ReadB();
        REG[ABUS] = alu.R;
        
        std::cout << "BYTE: " << (int)alu.R << " = " << (int)alu.RA << " OP " << (int)alu.RB << std::endl;
    } else {
        // 字操作
        BYTE ABUS = ReadB();
        ABUS <<= 1;
        alu.RA = (REG[ABUS] << 8) | REG[ABUS + 1];
        ABUS = ReadB();
        ABUS <<= 1;
        alu.RB = (REG[ABUS] << 8) | REG[ABUS + 1];
        alu.execute();
        ABUS = ReadB();
        ABUS <<= 1;
        REG[ABUS] = alu.R >> 8;
        REG[ABUS + 1] = alu.R;
        
        std::cout << "WORD: " << alu.R << " = " << alu.RA << " OP " << alu.RB << std::endl;
    }
}

void ToyCPU::execute_jump() {
    BYTE OP = RAM[IP-1];
    
    BYTE TYPE = OP & MR_BYTE;
    OP &= (~MR_BYTE);
    OP &= (~MR_B);
    
    switch (OP) {
        case JE:
            if (alu.FR & BIT_EQ) {
                IP = ReadW();
                std::cout << "JE 跳转到 " << std::dec << IP << std::endl;
            } else {
                IP += 2;
            }
            break;
        case JNE:
            if (alu.FR & BIT_EQ) {
                IP += 2;
            } else {
                IP = ReadW();
                std::cout << "JNE 跳转到 " << std::dec << IP << std::endl;
            }
            break;
        default:
            std::cerr << "未实现的分支指令: " << OP << std::endl;
            break;
    }
}

void ToyCPU::execute_stack() {
    BYTE OP = RAM[IP-1];
    
    switch (OP) {
        case PUSH:
            {
                BYTE ABUS = ReadB();
                std::cout << "PUSH: 寄存器 " << (int)ABUS << " 压栈" << std::endl;
                RAM[SP--] = REG[ABUS];
            }
            break;
        case POP:
            {
                BYTE ABUS = ReadB();
                std::cout << "POP: 寄存器 " << (int)ABUS << " 出栈" << std::endl;
                REG[ABUS] = RAM[SP++];
            }
            break;
    }
}

void ToyCPU::execute_memory() {
    BYTE OP = RAM[IP-1];
    
    BYTE TYPE = OP & MR_BYTE;
    BYTE MR = OP & MR_B;
    OP &= (~MR_BYTE);
    OP &= (~MR_B);
    
    BYTE ABUS = ReadB();
    
    switch (OP) {
        case LOAD:
            if (TYPE == MR_BYTE) {
                WORD DBUS;
                switch (MR) {
                    case MR_A:
                        DBUS = ReadB();
                        break;
                    case MR_B:
                        DBUS = ReadB(ReadB());
                        break;
                    default:
                        std::cerr << "LOAD错误: " << OP << std::endl;
                        break;
                }
                REG[ABUS] = DBUS;
                std::cout << "LOADB: 寄存器 " << (int)ABUS << " = " << (int)DBUS << std::endl;
            } else {
                WORD DBUS;
                switch (MR) {
                    case MR_A:
                        DBUS = ReadW();
                        break;
                    case MR_B:
                        DBUS = ReadW(ReadW());
                        break;
                    default:
                        std::cerr << "LOAD错误: " << OP << std::endl;
                        break;
                }
                ABUS <<= 1;
                REG[ABUS] = DBUS >> 8;
                REG[ABUS + 1] = DBUS;
                std::cout << "LOADW: 寄存器 " << (int)ABUS << " = " << DBUS << std::endl;
            }
            break;
        case STORE:
            if (TYPE == MR_BYTE) {
                WORD DBUS = REG[ABUS];
                switch (MR) {
                    case MR_B:
                        ABUS = ReadW();
                        break;
                    default:
                        std::cerr << "STORE错误" << std::endl;
                        break;
                }
                RAM[ABUS] = DBUS;
                std::cout << "STOREB: 内存[" << ABUS << "] = " << (int)DBUS << std::endl;
            } else {
                ABUS <<= 1;
                WORD DBUS = (REG[ABUS] << 8) | REG[ABUS + 1];
                switch (MR) {
                    case MR_B:
                        ABUS = ReadW();
                        break;
                    default:
                        std::cerr << "STORE错误" << std::endl;
                        break;
                }
                ABUS <<= 1;
                RAM[ABUS] = DBUS >> 8;
                RAM[ABUS + 1] = DBUS;
                std::cout << "STOREW: 内存[" << ABUS << "] = " << DBUS << std::endl;
            }
            break;
    }
}

void ToyCPU::execute_function() {
    BYTE OP = RAM[IP-1];
    
    switch (OP) {
        case CALL:
            std::cout << "CALL: 函数调用" << std::endl;
            // 将返回地址压栈
            SP -= 2;
            RAM[SP] = (IP + 2) >> 8;
            RAM[SP + 1] = (IP + 2) & 0xFF;
            // 跳转到函数地址
            IP = ReadW();
            break;
        case RET:
            std::cout << "RET: 函数返回" << std::endl;
            // 从栈中弹出返回地址
            IP = (RAM[SP] << 8) | RAM[SP + 1];
            SP += 2;
            break;
    }
}

BYTE ToyCPU::ReadB() {
    return RAM[IP++];
}

BYTE ToyCPU::ReadB(WORD addr) {
    return RAM[addr];
}

WORD ToyCPU::ReadW() {
    WORD W = 0;
    W |= (WORD)RAM[IP++] << 8;
    W |= (WORD)RAM[IP++];
    return W;
}

WORD ToyCPU::ReadW(WORD addr) {
    WORD A, W;
    A = addr << 1;
    W = 0;
    W |= (WORD)RAM[A] << 8;
    W |= (WORD)RAM[A + 1];
    return W;
}

void ToyCPU::WriteW(WORD addr, WORD data) {
    WORD A;
    A = addr << 1;
    RAM[A] = data >> 8;
    RAM[A + 1] = data;
}

void ToyCPU::dump_registers() {
    std::cout << "\n=== Toy寄存器状态 ===" << std::endl;
    std::cout << "SP: " << SP << std::endl;
    std::cout << "BP: " << BP << std::endl;
    std::cout << "SI: " << SI << std::endl;
    std::cout << "DI: " << DI << std::endl;
    std::cout << "CS: " << CS << std::endl;
    std::cout << "DS: " << DS << std::endl;
    std::cout << "ES: " << ES << std::endl;
    std::cout << "SS: " << SS << std::endl;
    std::cout << "FS: " << FS << std::endl;
    std::cout << "GS: " << GS << std::endl;
    std::cout << "IP: " << IP << std::endl;
    
    // 显示前16个寄存器
    for (int i = 0; i < 16; i += 4) {
        printf("R%02d: %02x  R%02d: %02x  R%02d: %02x  R%02d: %02x\n",
               i, REG[i], i+1, REG[i+1], i+2, REG[i+2], i+3, REG[i+3]);
    }
}

void ToyCPU::dump_memory(uint32_t start, uint32_t end) {
    std::cout << "\n=== Toy内存状态 (" << std::hex << start << "-" << end << ") ===" << std::endl;
    for (uint32_t addr = start; addr < end; addr += 16) {
        printf("%04x: ", addr);
        for (int i = 0; i < 16 && addr + i < end; i++) {
            printf("%02x ", RAM[addr + i]);
        }
        std::cout << std::endl;
    }
}
