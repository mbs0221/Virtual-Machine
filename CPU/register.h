#ifndef __REGISTER_H_
#define __REGISTER_H_

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

// Toy架构基本类型定义
typedef uint16_t WORD;
typedef uint8_t BYTE;

// Toy架构常量定义
#define TOY_REG_COUNT 256         // 256个寄存器

// 特殊寄存器索引定义
#define REG_SP 0                  // 栈指针
#define REG_BP 1                  // 基指针
#define REG_SI 2                  // 源索引
#define REG_DI 3                  // 目标索引
#define REG_CS 4                  // 代码段
#define REG_DS 5                  // 数据段
#define REG_ES 6                  // 扩展段
#define REG_SS 7                  // 栈段
#define REG_FS 8                  // 文件段
#define REG_GS 9                  // 全局段
#define REG_IP 10                 // 指令指针
#define REG_IBUS 11               // 指令总线
#define REG_DBUS 12               // 数据总线
#define REG_ABUS 13               // 地址总线

// 标志位定义
#define BIT_MASK    0x0000
#define BIT_ZERO    0x1000
#define BIT_EQ      0x0800
#define BIT_GT      0x0400
#define BIT_NEG     0x0200
#define BIT_ERR     0x0001

/**
 * RegisterFile抽象类 - 统一管理寄存器访问
 * 提供统一的寄存器读写接口，支持通用寄存器和特殊寄存器
 */
class RegisterFile {
private:
    BYTE registers[TOY_REG_COUNT];    // 通用寄存器数组
    WORD flags;                       // 标志寄存器
    WORD instruction_pointer;         // 指令指针
    WORD stack_pointer;               // 栈指针
    WORD base_pointer;                // 基指针
    WORD source_index;                // 源索引
    WORD destination_index;           // 目标索引
    WORD code_segment;                // 代码段
    WORD data_segment;                // 数据段
    WORD extra_segment;               // 扩展段
    WORD stack_segment;               // 栈段
    WORD file_segment;                // 文件段
    WORD global_segment;              // 全局段
    WORD instruction_bus;             // 指令总线
    WORD data_bus;                    // 数据总线
    WORD address_bus;                 // 地址总线
    
public:
    RegisterFile();
    ~RegisterFile() = default;
    
    // 基本寄存器操作
    void reset();
    void clear();
    
    // 通用寄存器访问
    BYTE read_byte(BYTE reg_index);
    void write_byte(BYTE reg_index, BYTE value);
    WORD read_word(BYTE reg_index);
    void write_word(BYTE reg_index, WORD value);
    
    // 特殊寄存器访问
    WORD get_instruction_pointer() const { return instruction_pointer; }
    void set_instruction_pointer(WORD ip) { instruction_pointer = ip; }
    
    WORD get_stack_pointer() const { return stack_pointer; }
    void set_stack_pointer(WORD sp) { stack_pointer = sp; }
    
    WORD get_base_pointer() const { return base_pointer; }
    void set_base_pointer(WORD bp) { base_pointer = bp; }
    
    WORD get_source_index() const { return source_index; }
    void set_source_index(WORD si) { source_index = si; }
    
    WORD get_destination_index() const { return destination_index; }
    void set_destination_index(WORD di) { destination_index = di; }
    
    // 段寄存器访问
    WORD get_code_segment() const { return code_segment; }
    void set_code_segment(WORD cs) { code_segment = cs; }
    
    WORD get_data_segment() const { return data_segment; }
    void set_data_segment(WORD ds) { data_segment = ds; }
    
    WORD get_extra_segment() const { return extra_segment; }
    void set_extra_segment(WORD es) { extra_segment = es; }
    
    WORD get_stack_segment() const { return stack_segment; }
    void set_stack_segment(WORD ss) { stack_segment = ss; }
    
    WORD get_file_segment() const { return file_segment; }
    void set_file_segment(WORD fs) { file_segment = fs; }
    
    WORD get_global_segment() const { return global_segment; }
    void set_global_segment(WORD gs) { global_segment = gs; }
    
    // 总线访问
    WORD get_instruction_bus() const { return instruction_bus; }
    void set_instruction_bus(WORD ibus) { instruction_bus = ibus; }
    
    WORD get_data_bus() const { return data_bus; }
    void set_data_bus(WORD dbus) { data_bus = dbus; }
    
    WORD get_address_bus() const { return address_bus; }
    void set_address_bus(WORD abus) { address_bus = abus; }
    
    // 标志寄存器操作
    WORD get_flags() const { return flags; }
    void set_flags(WORD f) { flags = f; }
    
    // 标志位操作
    bool get_zero_flag() const { return (flags & BIT_ZERO) != 0; }
    void set_zero_flag(bool value) { 
        if (value) flags |= BIT_ZERO; 
        else flags &= ~BIT_ZERO; 
    }
    
    bool get_equal_flag() const { return (flags & BIT_EQ) != 0; }
    void set_equal_flag(bool value) { 
        if (value) flags |= BIT_EQ; 
        else flags &= ~BIT_EQ; 
    }
    
    bool get_greater_flag() const { return (flags & BIT_GT) != 0; }
    void set_greater_flag(bool value) { 
        if (value) flags |= BIT_GT; 
        else flags &= ~BIT_GT; 
    }
    
    bool get_negative_flag() const { return (flags & BIT_NEG) != 0; }
    void set_negative_flag(bool value) { 
        if (value) flags |= BIT_NEG; 
        else flags &= ~BIT_NEG; 
    }
    
    bool get_error_flag() const { return (flags & BIT_ERR) != 0; }
    void set_error_flag(bool value) { 
        if (value) flags |= BIT_ERR; 
        else flags &= ~BIT_ERR; 
    }
    
    // 批量操作
    void read_register_block(BYTE start_reg, BYTE* buffer, BYTE count);
    void write_register_block(BYTE start_reg, const BYTE* buffer, BYTE count);
    
    // 寄存器复制
    void copy_register(BYTE dest_reg, BYTE src_reg);
    void swap_registers(BYTE reg1, BYTE reg2);
    
    // 寄存器比较
    bool compare_registers(BYTE reg1, BYTE reg2);
    
    // 寄存器统计
    BYTE get_register_count() const { return static_cast<BYTE>(TOY_REG_COUNT); }
    WORD get_register_value(BYTE reg_index) const;
    void set_register_value(BYTE reg_index, WORD value);
    
    // 调试和诊断
    void print_register_info();
    void print_register_dump();
    std::string get_register_dump_string();
    std::string get_special_registers_string();
    
    // 寄存器验证
    bool is_valid_register(BYTE reg_index) const;
    bool is_special_register(BYTE reg_index) const;
    
    // 寄存器名称
    std::string get_register_name(BYTE reg_index) const;
    std::string get_special_register_name(BYTE reg_index) const;
    
private:
    // 内部辅助方法
    void initialize_special_registers();
    void update_flags_from_value(WORD value);
    void update_flags_from_comparison(WORD value1, WORD value2);
};

#endif // __REGISTER_H_
