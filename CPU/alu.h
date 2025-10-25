#ifndef __ALU_H_
#define __ALU_H_

#include <cstdint>
#include <string>
#include <vector>

// Toy架构基本类型定义
typedef uint16_t WORD;
typedef uint8_t BYTE;

// ALU操作码定义
#define ALU_ADD     0x01
#define ALU_SUB     0x02
#define ALU_MUL     0x03
#define ALU_DIV     0x04
#define ALU_MOD     0x05
#define ALU_AND     0x06
#define ALU_OR      0x07
#define ALU_XOR     0x08
#define ALU_NOT     0x09
#define ALU_SHL     0x0A
#define ALU_SHR     0x0B
#define ALU_ROL     0x0C
#define ALU_ROR     0x0D
#define ALU_CMP     0x0E
#define ALU_TEST    0x0F

// 标志位定义
#define ALU_FLAG_ZERO    0x1000  // 零标志
#define ALU_FLAG_EQUAL   0x0800  // 相等标志
#define ALU_FLAG_GREATER 0x0400  // 大于标志
#define ALU_FLAG_NEGATIVE 0x0200 // 负数标志
#define ALU_FLAG_CARRY   0x0100  // 进位标志
#define ALU_FLAG_OVERFLOW 0x0080 // 溢出标志
#define ALU_FLAG_ERROR   0x0001  // 错误标志

/**
 * ALU抽象类 - 算术逻辑单元
 * 提供统一的算术和逻辑运算接口
 */
class ALU {
private:
    BYTE operation;        // 当前操作码
    WORD operand_a;        // 操作数A
    WORD operand_b;        // 操作数B
    WORD result;           // 运算结果
    WORD flags;            // 标志寄存器
    
    // 内部状态
    bool operation_valid;  // 操作是否有效
    std::string last_error; // 最后的错误信息
    
public:
    ALU();
    ~ALU() = default;
    
    // 基本操作
    void reset();
    void clear();
    
    // 操作数设置
    void set_operand_a(WORD value) { operand_a = value; }
    void set_operand_b(WORD value) { operand_b = value; }
    void set_operands(WORD a, WORD b) { operand_a = a; operand_b = b; }
    
    // 操作码设置
    void set_operation(BYTE op) { operation = op; }
    BYTE get_operation() const { return operation; }
    
    // 结果获取
    WORD get_result() const { return result; }
    WORD get_flags() const { return flags; }
    void set_flags(WORD f) { flags = f; }
    
    // 标志位操作
    bool get_zero_flag() const { return (flags & ALU_FLAG_ZERO) != 0; }
    bool get_equal_flag() const { return (flags & ALU_FLAG_EQUAL) != 0; }
    bool get_greater_flag() const { return (flags & ALU_FLAG_GREATER) != 0; }
    bool get_negative_flag() const { return (flags & ALU_FLAG_NEGATIVE) != 0; }
    bool get_carry_flag() const { return (flags & ALU_FLAG_CARRY) != 0; }
    bool get_overflow_flag() const { return (flags & ALU_FLAG_OVERFLOW) != 0; }
    bool get_error_flag() const { return (flags & ALU_FLAG_ERROR) != 0; }
    
    void set_zero_flag(bool value) { 
        if (value) flags |= ALU_FLAG_ZERO; 
        else flags &= ~ALU_FLAG_ZERO; 
    }
    
    void set_equal_flag(bool value) { 
        if (value) flags |= ALU_FLAG_EQUAL; 
        else flags &= ~ALU_FLAG_EQUAL; 
    }
    
    void set_greater_flag(bool value) { 
        if (value) flags |= ALU_FLAG_GREATER; 
        else flags &= ~ALU_FLAG_GREATER; 
    }
    
    void set_negative_flag(bool value) { 
        if (value) flags |= ALU_FLAG_NEGATIVE; 
        else flags &= ~ALU_FLAG_NEGATIVE; 
    }
    
    void set_carry_flag(bool value) { 
        if (value) flags |= ALU_FLAG_CARRY; 
        else flags &= ~ALU_FLAG_CARRY; 
    }
    
    void set_overflow_flag(bool value) { 
        if (value) flags |= ALU_FLAG_OVERFLOW; 
        else flags &= ~ALU_FLAG_OVERFLOW; 
    }
    
    void set_error_flag(bool value) { 
        if (value) flags |= ALU_FLAG_ERROR; 
        else flags &= ~ALU_FLAG_ERROR; 
    }
    
    // 主要执行方法
    bool execute();
    bool execute(BYTE op, WORD a, WORD b);
    
    // 算术运算
    WORD add(WORD a, WORD b);
    WORD subtract(WORD a, WORD b);
    WORD multiply(WORD a, WORD b);
    WORD divide(WORD a, WORD b);
    WORD modulo(WORD a, WORD b);
    
    // 逻辑运算
    WORD logical_and(WORD a, WORD b);
    WORD logical_or(WORD a, WORD b);
    WORD logical_xor(WORD a, WORD b);
    WORD logical_not(WORD a);
    
    // 移位运算
    WORD shift_left(WORD a, BYTE count);
    WORD shift_right(WORD a, BYTE count);
    WORD rotate_left(WORD a, BYTE count);
    WORD rotate_right(WORD a, BYTE count);
    
    // 比较运算
    void compare(WORD a, WORD b);
    void test(WORD a, WORD b);
    
    // 状态查询
    bool is_operation_valid() const { return operation_valid; }
    const std::string& get_last_error() const { return last_error; }
    
    // 操作名称
    std::string get_operation_name() const;
    std::string get_operation_name(BYTE op) const;
    
    // 调试和诊断
    void print_alu_state();
    std::string get_alu_state_string();
    std::string get_flags_string();
    
    // 批量操作
    std::vector<WORD> execute_batch(const std::vector<std::tuple<BYTE, WORD, WORD>>& operations);
    
    // 验证操作
    bool is_valid_operation(BYTE op) const;
    bool is_arithmetic_operation(BYTE op) const;
    bool is_logical_operation(BYTE op) const;
    bool is_shift_operation(BYTE op) const;
    bool is_compare_operation(BYTE op) const;
    
private:
    // 内部辅助方法
    void update_flags_from_result(WORD result);
    void update_flags_from_comparison(WORD a, WORD b);
    void clear_flags();
    void set_error(const std::string& error_msg);
    
    // 溢出检测
    bool check_add_overflow(WORD a, WORD b, WORD result);
    bool check_sub_overflow(WORD a, WORD b, WORD result);
    bool check_mul_overflow(WORD a, WORD b, WORD result);
    
    // 进位检测
    bool check_add_carry(WORD a, WORD b, WORD result);
    bool check_sub_carry(WORD a, WORD b, WORD result);
};

#endif // __ALU_H_
