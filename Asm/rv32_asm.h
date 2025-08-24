#ifndef __RV32_ASM_H_
#define __RV32_ASM_H_

#include <string>
#include <map>
#include <vector>
#include <fstream>

// RV32汇编指令类型
enum RV32AsmType {
    ASM_R_TYPE,  // 寄存器-寄存器
    ASM_I_TYPE,  // 立即数
    ASM_S_TYPE,  // 存储
    ASM_B_TYPE,  // 分支
    ASM_U_TYPE,  // 上层立即数
    ASM_J_TYPE   // 跳转
};

// RV32汇编指令结构
struct RV32AsmInstruction {
    std::string mnemonic;     // 指令助记符
    RV32AsmType type;         // 指令类型
    std::string rd;           // 目标寄存器
    std::string rs1;          // 源寄存器1
    std::string rs2;          // 源寄存器2
    int immediate;            // 立即数
    std::string label;        // 标签
    uint32_t address;         // 指令地址
};

// RV32汇编器类
class RV32Assembler {
private:
    std::map<std::string, uint32_t> labels;           // 标签表
    std::vector<RV32AsmInstruction> instructions;     // 指令列表
    uint32_t current_address;                         // 当前地址
    
public:
    RV32Assembler();
    
    // 汇编器主要功能
    bool parse_file(const std::string& filename);
    bool assemble(const std::string& output_file);
    void add_instruction(const RV32AsmInstruction& inst);
    void add_label(const std::string& name, uint32_t address);
    
private:
    // 指令解析
    bool parse_line(const std::string& line);
    bool parse_r_type(const std::string& mnemonic, const std::vector<std::string>& operands);
    bool parse_i_type(const std::string& mnemonic, const std::vector<std::string>& operands);
    bool parse_s_type(const std::string& mnemonic, const std::vector<std::string>& operands);
    bool parse_b_type(const std::string& mnemonic, const std::vector<std::string>& operands);
    bool parse_u_type(const std::string& mnemonic, const std::vector<std::string>& operands);
    bool parse_j_type(const std::string& mnemonic, const std::vector<std::string>& operands);
    
    // 指令编码
    uint32_t encode_r_type(const RV32AsmInstruction& inst);
    uint32_t encode_i_type(const RV32AsmInstruction& inst);
    uint32_t encode_s_type(const RV32AsmInstruction& inst);
    uint32_t encode_b_type(const RV32AsmInstruction& inst);
    uint32_t encode_u_type(const RV32AsmInstruction& inst);
    uint32_t encode_j_type(const RV32AsmInstruction& inst);
    
    // 辅助函数
    int parse_register(const std::string& reg);
    int parse_immediate(const std::string& imm);
    std::vector<std::string> split_string(const std::string& str, char delimiter);
    void resolve_labels();
};

#endif
