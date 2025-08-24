#ifndef __RV32_H_
#define __RV32_H_

#include "architecture.h"
#include <cstdint>
#include <iostream>

// RV32基本类型定义
typedef uint32_t word_t;
typedef uint16_t half_t;
typedef uint8_t byte_t;

// RV32寄存器定义
#define RV32_REG_COUNT 32
#define RV32_MEM_SIZE (64 * 1024) // 64KB内存

// RV32指令格式
struct RV32Instruction {
    uint32_t raw;
    
    // R-type指令 (register-register)
    struct {
        uint32_t funct7 : 7;
        uint32_t rs2 : 5;
        uint32_t rs1 : 5;
        uint32_t funct3 : 3;
        uint32_t rd : 5;
        uint32_t opcode : 7;
    } r_type;
    
    // I-type指令 (immediate)
    struct {
        uint32_t imm : 12;
        uint32_t rs1 : 5;
        uint32_t funct3 : 3;
        uint32_t rd : 5;
        uint32_t opcode : 7;
    } i_type;
    
    // S-type指令 (store)
    struct {
        uint32_t imm_11_5 : 7;
        uint32_t rs2 : 5;
        uint32_t rs1 : 5;
        uint32_t funct3 : 3;
        uint32_t imm_4_0 : 5;
        uint32_t opcode : 7;
    } s_type;
    
    // B-type指令 (branch)
    struct {
        uint32_t imm_12 : 1;
        uint32_t imm_10_5 : 6;
        uint32_t rs2 : 5;
        uint32_t rs1 : 5;
        uint32_t funct3 : 3;
        uint32_t imm_4_1 : 4;
        uint32_t imm_11 : 1;
        uint32_t opcode : 7;
    } b_type;
    
    // U-type指令 (upper immediate)
    struct {
        uint32_t imm : 20;
        uint32_t rd : 5;
        uint32_t opcode : 7;
    } u_type;
    
    // J-type指令 (jump)
    struct {
        uint32_t imm_20 : 1;
        uint32_t imm_10_1 : 10;
        uint32_t imm_11 : 1;
        uint32_t imm_19_12 : 8;
        uint32_t rd : 5;
        uint32_t opcode : 7;
    } j_type;
};

// RV32操作码
enum RV32Opcode {
    // 加载和存储
    OP_LOAD  = 0x03,
    OP_STORE = 0x23,
    
    // 立即数运算
    OP_OP_IMM = 0x13,
    
    // 寄存器运算
    OP_OP = 0x33,
    
    // 分支
    OP_BRANCH = 0x63,
    
    // 跳转
    OP_JALR = 0x67,
    OP_JAL = 0x6F,
    
    // 上层立即数
    OP_LUI = 0x37,
    OP_AUIPC = 0x17,
    
    // 系统
    OP_SYSTEM = 0x73
};

// RV32 funct3编码
enum RV32Funct3 {
    // 加载指令
    F3_LB  = 0x0,  // 加载字节
    F3_LH  = 0x1,  // 加载半字
    F3_LW  = 0x2,  // 加载字
    F3_LBU = 0x4,  // 加载无符号字节
    F3_LHU = 0x5,  // 加载无符号半字
    
    // 存储指令
    F3_SB = 0x0,   // 存储字节
    F3_SH = 0x1,   // 存储半字
    F3_SW = 0x2,   // 存储字
    
    // 立即数运算
    F3_ADDI  = 0x0,  // 加法立即数
    F3_SLTI  = 0x2,  // 小于立即数
    F3_SLTIU = 0x3,  // 小于无符号立即数
    F3_XORI  = 0x4,  // 异或立即数
    F3_ORI   = 0x6,  // 或立即数
    F3_ANDI  = 0x7,  // 与立即数
    F3_SLLI  = 0x1,  // 逻辑左移立即数
    F3_SRLI  = 0x5,  // 逻辑右移立即数
    F3_SRAI  = 0x5,  // 算术右移立即数
    
    // 寄存器运算
    F3_ADD  = 0x0,   // 加法
    F3_SUB  = 0x0,   // 减法
    F3_SLL  = 0x1,   // 逻辑左移
    F3_SLT  = 0x2,   // 小于
    F3_SLTU = 0x3,   // 小于无符号
    F3_XOR  = 0x4,   // 异或
    F3_SRL  = 0x5,   // 逻辑右移
    F3_SRA  = 0x5,   // 算术右移
    F3_OR   = 0x6,   // 或
    F3_AND  = 0x7,   // 与
    
    // 分支指令
    F3_BEQ  = 0x0,   // 相等分支
    F3_BNE  = 0x1,   // 不相等分支
    F3_BLT  = 0x4,   // 小于分支
    F3_BGE  = 0x5,   // 大于等于分支
    F3_BLTU = 0x6,   // 小于无符号分支
    F3_BGEU = 0x7    // 大于等于无符号分支
};

// RV32 funct7编码
enum RV32Funct7 {
    F7_ADD = 0x00,   // 加法
    F7_SUB = 0x20,   // 减法
    F7_SLL = 0x00,   // 逻辑左移
    F7_SLT = 0x00,   // 小于
    F7_SLTU = 0x00,  // 小于无符号
    F7_XOR = 0x00,   // 异或
    F7_SRL = 0x00,   // 逻辑右移
    F7_SRA = 0x20,   // 算术右移
    F7_OR = 0x00,    // 或
    F7_AND = 0x00    // 与
};

// RV32 CPU类 - 继承自Architecture
class RV32CPU : public Architecture {
private:
    word_t regs[RV32_REG_COUNT];  // 寄存器文件
    byte_t memory[RV32_MEM_SIZE]; // 内存
    word_t pc;                    // 程序计数器
    word_t sp;                    // 栈指针 (x2)
    
    // 状态信息
    bool running;
    uint32_t instruction_count;
    
public:
    RV32CPU();
    ~RV32CPU() override = default;
    
    // Architecture接口实现
    void reset() override;
    void load_program(const char* filename) override;
    void execute() override;
    void dump_registers() override;
    void dump_memory(uint32_t start, uint32_t end) override;
    
    // 架构信息
    std::string get_name() const override { return "RV32"; }
    std::string get_description() const override { 
        return "32位RISC-V指令集，32个寄存器，64KB内存，现代RISC设计"; 
    }
    uint32_t get_word_size() const override { return 32; }
    uint32_t get_register_count() const override { return RV32_REG_COUNT; }
    uint32_t get_memory_size() const override { return RV32_MEM_SIZE; }
    
    // 状态查询
    bool is_running() const override { return running; }
    uint32_t get_pc() const override { return pc; }
    uint32_t get_instruction_count() const override { return instruction_count; }
    
private:
    word_t read_memory(word_t addr);
    void write_memory(word_t addr, word_t data);
    word_t sign_extend(uint32_t value, int bits);
    void execute_instruction(RV32Instruction& inst);
    
    // 指令执行函数
    void execute_load(RV32Instruction& inst);
    void execute_store(RV32Instruction& inst);
    void execute_op_imm(RV32Instruction& inst);
    void execute_op(RV32Instruction& inst);
    void execute_branch(RV32Instruction& inst);
    void execute_jalr(RV32Instruction& inst);
    void execute_jal(RV32Instruction& inst);
    void execute_lui(RV32Instruction& inst);
    void execute_auipc(RV32Instruction& inst);
    void execute_system(RV32Instruction& inst);
};

#endif
