#ifndef __TOY_H_
#define __TOY_H_

#include "architecture.h"
#include <cstdint>
#include <iostream>

// Toy架构基本类型定义
typedef uint16_t WORD;
typedef uint8_t BYTE;

// Toy架构常量定义
#define TOY_MEM_SIZE (64 * 1024)  // 64KB内存
#define TOY_REG_COUNT 256         // 256个寄存器

// Toy架构标志位定义
#define BIT_MASK    0x0000
#define BIT_ZERO    0x1000
#define BIT_EQ      0x0800
#define BIT_GT      0x0400
#define BIT_NEG     0x0200
#define BIT_ERR     0x0001

// Toy架构寻址模式
#define MR_A        0x00  // 0000 寄存器寻址
#define MR_B        0x40  // 0100 直接寻址
#define MR_BYTE     0x80  // 字节/字操作
#define REG_SRC_MASK 0xE0
#define REG_DST_MASK 0x1C

// Toy架构指令集
enum TOY_CODE {
    HALT,
    ADD, SUB, MUL, DIV, MOD, CMP,  // 整数运算
    SHL, SHR, SAL, SAR, SRL, SRR,  // 移位运算
    MOV, IN, OUT,                  // 数据传输
    LOAD, STORE,                   // 内存访问
    PUSH, POP,                     // 栈操作
    JMP, JNE, JG, JE, JB, JGE, JBE, // 跳转指令
    CALL, RET,                     // 函数调用
    NEG,                           // 一元运算
    LOOP                           // 循环指令
};

// Toy架构ALU类
class ToyALU {
public:
    BYTE OP;
    WORD RA, RB;
    WORD R;
    WORD FR;
    
    ToyALU() {
        OP = 0;
        RA = RB = R = 0;
        FR = 0;
    }
    
    void execute() {
        switch (OP) {
            case ADD: R = RA + RB; break;
            case SUB: R = RA - RB; break;
            case MUL: R = RA * RB; break;
            case DIV: R = RA / RB; break;
            case MOD: R = RA % RB; break;
            case CMP: R = RA == RB; break;
            case NEG: R = 0 - RA; break;
            default: FR |= BIT_ERR; break;
        }
        
        // 更新标志位
        FR &= ~BIT_ZERO;
        FR |= (R == 0) ? BIT_ZERO : BIT_MASK;
        FR &= ~BIT_NEG;
        FR |= (R < 0) ? BIT_NEG : BIT_MASK;
        FR &= ~BIT_EQ;
        FR |= (RA == RB) ? BIT_EQ : BIT_MASK;
    }
};

// Toy架构CPU类 - 继承自Architecture
class ToyCPU : public Architecture {
private:
    BYTE REG[TOY_REG_COUNT];        // 寄存器文件
    WORD SP, BP, SI, DI;            // 通用寄存器
    WORD CS, DS, ES, SS, FS, GS;    // 段寄存器
    WORD IN[TOY_REG_COUNT], OUT[TOY_REG_COUNT]; // I/O端口
    WORD IP;                        // 指令指针
    WORD IBUS, DBUS, ABUS;          // 内部总线
    BYTE RAM[TOY_MEM_SIZE];         // 内存
    ToyALU alu;                     // ALU
    
    // 状态信息
    bool running;
    uint32_t instruction_count;
    
public:
    ToyCPU();
    ~ToyCPU() override = default;
    
    // Architecture接口实现
    void reset() override;
    void load_program(const char* filename) override;
    void execute() override;
    void dump_registers() override;
    void dump_memory(uint32_t start, uint32_t end) override;
    
    // 架构信息
    std::string get_name() const override { return "Toy"; }
    std::string get_description() const override { 
        return "16位字长，256个寄存器，64KB内存，支持字节和字操作"; 
    }
    uint32_t get_word_size() const override { return 16; }
    uint32_t get_register_count() const override { return TOY_REG_COUNT; }
    uint32_t get_memory_size() const override { return TOY_MEM_SIZE; }
    
    // 状态查询
    bool is_running() const override { return running; }
    uint32_t get_pc() const override { return IP; }
    uint32_t get_instruction_count() const override { return instruction_count; }
    
private:
    // 内存访问
    BYTE ReadB();
    BYTE ReadB(WORD addr);
    WORD ReadW();
    WORD ReadW(WORD addr);
    void WriteW(WORD addr, WORD data);
    
    // 指令执行
    void execute_instruction();
    
    // 指令执行函数
    void execute_arithmetic();
    void execute_jump();
    void execute_memory();
    void execute_stack();
    void execute_function();
};

#endif
