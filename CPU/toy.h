#ifndef __TOY_H_
#define __TOY_H_

#include "architecture.h"
#include "mmio.h"
#include "memory.h"
#include "register.h"
#include "alu.h"
#include "nes_optimizer.h"
#include "mmu.h"
#include "tlb.h"
#include "cache_manager.h"
#include <cstdint>
#include <iostream>
#include <vector>

// Toy架构基本类型定义
typedef uint16_t WORD;
typedef uint8_t BYTE;

// Toy架构常量定义
#define TOY_MEM_SIZE (64 * 1024)  // 64KB内存
#define TOY_REG_COUNT 256         // 256个寄存器


// 特权级别定义
#define PRIVILEGE_KERNEL 0        // 内核特权级别
#define PRIVILEGE_USER   1        // 用户特权级别
#define PRIVILEGE_MAX    1        // 最大特权级别


// 中断类型定义
enum InterruptType {
    INTERRUPT_NONE = 0,
    INTERRUPT_TIMER,              // 定时器中断
    INTERRUPT_CONSOLE,            // 控制台中断
    INTERRUPT_DISPLAY,            // 显示中断
    INTERRUPT_STORAGE,            // 存储中断
    INTERRUPT_AUDIO,              // 音频中断
    INTERRUPT_EXTERNAL,           // 外部中断
    INTERRUPT_MAX
};

// Toy架构标志位定义
#define BIT_MASK    0x0000
#define BIT_ZERO    0x1000
#define BIT_EQ      0x0800
#define BIT_GT      0x0400
#define BIT_NEG     0x0200
#define BIT_ERR     0x0001
#define BIT_IF      0x0080  // 中断使能标志
#define BIT_PRIV    0x0040  // 特权级别标志 (0=内核, 1=用户)

// Toy架构寻址模式
#define MR_A        0x00  // 0000 寄存器寻址/立即数寻址
#define MR_B        0x40  // 0100 直接寻址
#define MR_INDIRECT 0x20  // 0010 寄存器间接寻址 [$reg]
#define MR_PC_REL   0x60  // 0110 PC相对寻址 [PC+offset]
#define MR_BYTE     0x80  // 字节/字操作
#define REG_SRC_MASK 0xE0
#define REG_DST_MASK 0x1C

// Toy架构指令集 - 与Asm模块保持一致
// 使用100以上的数值范围避免与ASCII码冲突
enum TOY_CODE {
    // 基础指令 (100-109)
    HALT = 100,
    ADD = 101, SUB = 102, MUL = 103, DIV = 104, MOD = 105, CMP = 106,  // 整数运算
    NEG = 107,                           // 一元运算
    LOOP = 108,                          // 循环指令
    
    // 位运算指令 (110-119)
    AND = 110, OR = 111, XOR = 112, NOT = 113,  // 位运算
    SHL = 114, SHR = 115, SAL = 116, SAR = 117, SRL = 118, SRR = 119,  // 移位运算
    
    // 数据传输指令 (120-129)
    MOV = 120, IN = 121, OUT = 122,                  // 数据传输
    LOAD = 123, STORE = 124, LEA = 125,              // 内存访问
    PUSH = 127, POP = 128,                     // 栈操作
    
    // 跳转指令 (130-139)
    JMP = 130, JNE = 131, JG = 132, JE = 133, JB = 134, JGE = 135, JBE = 136, // 跳转指令
    CALL = 137, RET = 138,                     // 函数调用
    
    // 数学运算指令 (140-149)
    INC = 140, DEC = 141, ABS = 142,  // 数学运算
    
    // 条件设置指令 (150-159)
    SETZ = 150, SETNZ = 151, SETG = 152, SETL = 153, SETGE = 154, SETLE = 155,  // 条件设置指令
    
    // 字符串操作指令 (160-169)
    STRLEN = 160, STRCPY = 161, STRCMP = 162, STRCHR = 163,  // 字符串操作
    
    // 中断和系统调用指令 (170-179)
    INT_INST = 170, IRET = 171, CLI_INST = 172, STI_INST = 173,     // 中断指令
    SYSCALL = 174, HLT = 175,                                     // 系统调用和停机指令
    // 关键字
    ID = 256, INT, STRING, COMMENT, END, LABEL, DATA, CODE, STACK, VAR, FS, GS,
    // 控制流语句
    IF, ELSE, WHILE, FOR, DO, SWITCH, CASE, DEFAULT,
    BREAK, CONTINUE
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
            case ADD:
            case '+': R = RA + RB; break;
            case SUB:
            case '-': R = RA - RB; break;
            case MUL:
            case '*': R = RA * RB; break;
            case DIV:
            case '/': R = RA / RB; break;
            case MOD:
            case '%': R = RA % RB; break;
            case CMP:
            case '<':
            case '>':
            case '=': R = RA == RB; break;
            case NEG: R = 0 - RA; break;
            case AND: R = RA & RB; break;
            case OR:  R = RA | RB; break;
            case XOR: R = RA ^ RB; break;
            case NOT: R = ~RA; break;
            case INC: R = RA + 1; break;
            case DEC: R = RA - 1; break;
            default: FR |= BIT_ERR; break;
        }
        
        // 更新标志位
        FR &= ~BIT_ZERO;
        FR |= (R == 0) ? BIT_ZERO : BIT_MASK;
        FR &= ~BIT_NEG;
        FR |= (static_cast<int16_t>(R) < 0) ? BIT_NEG : BIT_MASK;
        FR &= ~BIT_EQ;
        FR |= (RA == RB) ? BIT_EQ : BIT_MASK;
    }
};


// Toy架构CPU类 - 继承自Architecture
class ToyCPU : public Architecture {
private:
    RegisterFile registers;         // 寄存器文件抽象
    WORD dataSize, codeSize;        // 数据段和代码段大小
    WORD IN_PORT[TOY_REG_COUNT], OUT_PORT[TOY_REG_COUNT]; // I/O端口
    Memory memory;                  // 内存抽象
    ALU alu;                        // ALU抽象
    NESOptimizer* nes_optimizer;    // NES游戏优化器
    
    // MMIO和中断支持
    MMIODevice* mmio_devices[16];   // MMIO设备数组
    WORD interrupt_vectors[TOY_INTERRUPT_VECTORS]; // 中断向量表
    bool interrupts_enabled;        // 中断使能标志
    bool in_interrupt;              // 中断处理状态
    WORD saved_ip, saved_flags;     // 中断保存的寄存器
    
    // 异常和特权处理
    BYTE current_privilege_level;   // 当前特权级别
    ExceptionType pending_exception; // 待处理异常
    WORD exception_vectors[EXCEPTION_MAX]; // 异常向量表
    WORD saved_privilege_level;     // 保存的特权级别
    bool exception_enabled;         // 异常处理使能
    
    // MMU和虚拟内存支持
    MMU* mmu;                       // MMU实例
    TLB* tlb;                       // TLB实例
    
    // 缓存系统
    CacheManager* cache_manager;    // 缓存管理器
    
    // 状态信息
    bool running;
    uint32_t instruction_count;
    
public:
    ToyCPU();
    ~ToyCPU() override;
    
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
    uint32_t get_pc() const override { return registers.get_instruction_pointer(); }
    uint32_t get_instruction_count() const override { return instruction_count; }
    
    // MMIO和中断管理
    void register_mmio_device(int device_id, MMIODevice* device);
    void set_interrupt_vector(BYTE vector, WORD handler_address);
    void enable_interrupts() { interrupts_enabled = true; }
    void disable_interrupts() { interrupts_enabled = false; }
    bool are_interrupts_enabled() const { return interrupts_enabled; }
    
    // 异常和特权管理
    void set_exception_vector(ExceptionType type, WORD handler_address);
    void set_privilege_level(BYTE level);
    BYTE get_privilege_level() const { return current_privilege_level; }
    void enable_exceptions() { exception_enabled = true; }
    void disable_exceptions() { exception_enabled = false; }
    bool are_exceptions_enabled() const { return exception_enabled; }
    void raise_exception(ExceptionType type);
    void handle_exception(ExceptionType type);
    
    // MMU和虚拟内存管理
    void enable_mmu() { if (mmu) mmu->enable(); }
    void disable_mmu() { if (mmu) mmu->disable(); }
    bool is_mmu_enabled() const { return mmu ? mmu->is_enabled() : false; }
    void set_pgd_base(WORD base) { if (mmu) mmu->set_pgd_base(base); }
    WORD get_pgd_base() const { return mmu ? mmu->get_pgd_base() : 0; }
    WORD translate_address(WORD virtual_addr, bool is_write = false, bool is_execute = false);
    void handle_page_fault(WORD virtual_addr, BYTE fault_flags);
    void invalidate_tlb();
    void invalidate_tlb_entry(WORD virtual_addr);
    
    // 缓存管理
    void enable_cache() { if (cache_manager) cache_manager->enable(); }
    void disable_cache() { if (cache_manager) cache_manager->disable(); }
    bool is_cache_enabled() const { return cache_manager ? cache_manager->is_enabled() : false; }
    void flush_cache() { if (cache_manager) cache_manager->flush_all(); }
    void invalidate_cache() { if (cache_manager) cache_manager->invalidate_all(); }
    void print_cache_stats() { if (cache_manager) cache_manager->print_all_stats(); }
    
    // 设备访问
    ConsoleDevice* get_console_device() const;
    TimerDevice* get_timer_device() const;
    
private:
    // 内存访问（通过Memory抽象）
    BYTE ReadB();
    BYTE ReadB(WORD addr);
    WORD ReadW();
    WORD ReadW(WORD addr);
    void WriteW(WORD addr, WORD data);
    
    // 指令执行
    void execute_instruction();
    
    // 指令执行函数
    void execute_arithmetic();
    void execute_bitwise();
    void execute_math();
    void execute_conditional();
    void execute_string();
    void execute_shift();
    void execute_jump();
    void execute_memory();
    void execute_stack();
    void execute_mov();
    void execute_io();
    void execute_function();
    
    // 辅助函数
    const char* get_op_name(BYTE op);
    const char* get_instruction_name(BYTE op);
    void print_instruction_info(BYTE op, const char* details = "");
    void print_detailed_instruction_info(BYTE op, const std::vector<WORD>& operands);
    
    // 寻址模式解析
    WORD resolve_address(BYTE addressing_mode, WORD operand);
    const char* get_addressing_mode_name(BYTE mode);
    
    // 执行模式控制
    void execute_sequential_mode();
    void update_devices();
    
    // MMIO和中断处理
    WORD read_mmio(WORD address);
    void write_mmio(WORD address, WORD value);
    void check_interrupts();
    void handle_interrupt(BYTE vector);
    void execute_interrupt_instruction();
    
    // 异常处理
    void check_exceptions();
    void execute_syscall(BYTE syscall_number);
    void execute_privilege_instruction();
    bool check_privilege(BYTE required_level) const;
    void switch_to_kernel_mode();
    void switch_to_user_mode();
    
    
    // NES游戏优化
    void enable_nes_mode(bool enable);
    bool is_nes_mode_enabled() const;
    void optimize_for_nes_game();
    void print_nes_performance_report();
};

#endif
