#include "alu.h"
#include "Common/Logger.h"
#include <iomanip>
#include <sstream>
#include <stdexcept>

ALU::ALU() {
    reset();
    LOG_DEBUG("ALU", "ALU initialized");
}

void ALU::reset() {
    operation = 0;
    operand_a = 0;
    operand_b = 0;
    result = 0;
    flags = 0;
    operation_valid = true;
    last_error.clear();
    LOG_DEBUG("ALU", "ALU reset");
}

void ALU::clear() {
    reset();
}

bool ALU::execute() {
    if (!is_valid_operation(operation)) {
        set_error("Invalid operation: 0x" + std::to_string(operation));
        return false;
    }
    
    operation_valid = true;
    last_error.clear();
    
    switch (operation) {
        case ALU_ADD:
            result = add(operand_a, operand_b);
            break;
        case ALU_SUB:
            result = subtract(operand_a, operand_b);
            break;
        case ALU_MUL:
            result = multiply(operand_a, operand_b);
            break;
        case ALU_DIV:
            result = divide(operand_a, operand_b);
            break;
        case ALU_MOD:
            result = modulo(operand_a, operand_b);
            break;
        case ALU_AND:
            result = logical_and(operand_a, operand_b);
            break;
        case ALU_OR:
            result = logical_or(operand_a, operand_b);
            break;
        case ALU_XOR:
            result = logical_xor(operand_a, operand_b);
            break;
        case ALU_NOT:
            result = logical_not(operand_a);
            break;
        case ALU_SHL:
            result = shift_left(operand_a, operand_b & 0x0F);
            break;
        case ALU_SHR:
            result = shift_right(operand_a, operand_b & 0x0F);
            break;
        case ALU_ROL:
            result = rotate_left(operand_a, operand_b & 0x0F);
            break;
        case ALU_ROR:
            result = rotate_right(operand_a, operand_b & 0x0F);
            break;
        case ALU_CMP:
            compare(operand_a, operand_b);
            result = 0; // 比较操作不产生结果
            break;
        case ALU_TEST:
            test(operand_a, operand_b);
            result = 0; // 测试操作不产生结果
            break;
        default:
            set_error("Unimplemented operation: 0x" + std::to_string(operation));
            return false;
    }
    
    LOG_DEBUG("ALU", get_operation_name() + ": " + std::to_string(operand_a) + " " + 
              get_operation_name() + " " + std::to_string(operand_b) + " = " + std::to_string(result));
    
    return operation_valid;
}

bool ALU::execute(BYTE op, WORD a, WORD b) {
    set_operation(op);
    set_operands(a, b);
    return execute();
}

// 算术运算实现
WORD ALU::add(WORD a, WORD b) {
    WORD result = a + b;
    update_flags_from_result(result);
    
    // 检查溢出和进位
    if (check_add_overflow(a, b, result)) {
        set_overflow_flag(true);
    }
    if (check_add_carry(a, b, result)) {
        set_carry_flag(true);
    }
    
    return result;
}

WORD ALU::subtract(WORD a, WORD b) {
    WORD result = a - b;
    update_flags_from_result(result);
    
    // 检查溢出和进位
    if (check_sub_overflow(a, b, result)) {
        set_overflow_flag(true);
    }
    if (check_sub_carry(a, b, result)) {
        set_carry_flag(true);
    }
    
    return result;
}

WORD ALU::multiply(WORD a, WORD b) {
    WORD result = a * b;
    update_flags_from_result(result);
    
    // 检查溢出
    if (check_mul_overflow(a, b, result)) {
        set_overflow_flag(true);
    }
    
    return result;
}

WORD ALU::divide(WORD a, WORD b) {
    if (b == 0) {
        set_error("Division by zero");
        set_error_flag(true);
        return 0;
    }
    
    WORD result = a / b;
    update_flags_from_result(result);
    return result;
}

WORD ALU::modulo(WORD a, WORD b) {
    if (b == 0) {
        set_error("Modulo by zero");
        set_error_flag(true);
        return 0;
    }
    
    WORD result = a % b;
    update_flags_from_result(result);
    return result;
}

// 逻辑运算实现
WORD ALU::logical_and(WORD a, WORD b) {
    WORD result = a & b;
    update_flags_from_result(result);
    return result;
}

WORD ALU::logical_or(WORD a, WORD b) {
    WORD result = a | b;
    update_flags_from_result(result);
    return result;
}

WORD ALU::logical_xor(WORD a, WORD b) {
    WORD result = a ^ b;
    update_flags_from_result(result);
    return result;
}

WORD ALU::logical_not(WORD a) {
    WORD result = ~a;
    update_flags_from_result(result);
    return result;
}

// 移位运算实现
WORD ALU::shift_left(WORD a, BYTE count) {
    if (count > 15) count = 15; // 限制移位数量
    
    WORD result = a << count;
    update_flags_from_result(result);
    
    // 检查进位（最高位被移出）
    if (count > 0 && (a & (1 << (16 - count)))) {
        set_carry_flag(true);
    }
    
    return result;
}

WORD ALU::shift_right(WORD a, BYTE count) {
    if (count > 15) count = 15; // 限制移位数量
    
    WORD result = a >> count;
    update_flags_from_result(result);
    
    // 检查进位（最低位被移出）
    if (count > 0 && (a & (1 << (count - 1)))) {
        set_carry_flag(true);
    }
    
    return result;
}

WORD ALU::rotate_left(WORD a, BYTE count) {
    if (count > 15) count = 15; // 限制移位数量
    
    WORD result = (a << count) | (a >> (16 - count));
    update_flags_from_result(result);
    
    // 检查进位（最高位被移出）
    if (count > 0 && (a & (1 << (16 - count)))) {
        set_carry_flag(true);
    }
    
    return result;
}

WORD ALU::rotate_right(WORD a, BYTE count) {
    if (count > 15) count = 15; // 限制移位数量
    
    WORD result = (a >> count) | (a << (16 - count));
    update_flags_from_result(result);
    
    // 检查进位（最低位被移出）
    if (count > 0 && (a & (1 << (count - 1)))) {
        set_carry_flag(true);
    }
    
    return result;
}

// 比较运算实现
void ALU::compare(WORD a, WORD b) {
    update_flags_from_comparison(a, b);
    LOG_DEBUG("ALU", "Compare: " + std::to_string(a) + " vs " + std::to_string(b));
}

void ALU::test(WORD a, WORD b) {
    WORD result = a & b;
    update_flags_from_result(result);
    LOG_DEBUG("ALU", "Test: " + std::to_string(a) + " & " + std::to_string(b) + " = " + std::to_string(result));
}

// 状态查询实现
std::string ALU::get_operation_name() const {
    return get_operation_name(operation);
}

std::string ALU::get_operation_name(BYTE op) const {
    switch (op) {
        case ALU_ADD: return "ADD";
        case ALU_SUB: return "SUB";
        case ALU_MUL: return "MUL";
        case ALU_DIV: return "DIV";
        case ALU_MOD: return "MOD";
        case ALU_AND: return "AND";
        case ALU_OR: return "OR";
        case ALU_XOR: return "XOR";
        case ALU_NOT: return "NOT";
        case ALU_SHL: return "SHL";
        case ALU_SHR: return "SHR";
        case ALU_ROL: return "ROL";
        case ALU_ROR: return "ROR";
        case ALU_CMP: return "CMP";
        case ALU_TEST: return "TEST";
        default: return "UNKNOWN";
    }
}

void ALU::print_alu_state() {
    LOG_INFO("ALU", "=== ALU State ===");
    LOG_INFO("ALU", get_alu_state_string());
    LOG_INFO("ALU", "Flags: " + get_flags_string());
}

std::string ALU::get_alu_state_string() {
    std::stringstream ss;
    ss << "Op: " << get_operation_name() << " (0x" << std::hex << (int)operation << std::dec << ")";
    ss << ", A: " << operand_a;
    ss << ", B: " << operand_b;
    ss << ", Result: " << result;
    ss << ", Valid: " << (operation_valid ? "Yes" : "No");
    return ss.str();
}

std::string ALU::get_flags_string() {
    std::stringstream ss;
    ss << "Z:" << (get_zero_flag() ? "1" : "0");
    ss << " E:" << (get_equal_flag() ? "1" : "0");
    ss << " G:" << (get_greater_flag() ? "1" : "0");
    ss << " N:" << (get_negative_flag() ? "1" : "0");
    ss << " C:" << (get_carry_flag() ? "1" : "0");
    ss << " O:" << (get_overflow_flag() ? "1" : "0");
    ss << " E:" << (get_error_flag() ? "1" : "0");
    return ss.str();
}

std::vector<WORD> ALU::execute_batch(const std::vector<std::tuple<BYTE, WORD, WORD>>& operations) {
    std::vector<WORD> results;
    results.reserve(operations.size());
    
    for (const auto& op : operations) {
        BYTE op_code = std::get<0>(op);
        WORD a = std::get<1>(op);
        WORD b = std::get<2>(op);
        
        if (execute(op_code, a, b)) {
            results.push_back(result);
        } else {
            results.push_back(0);
            LOG_ERROR("ALU", "Batch operation failed: " + get_last_error());
        }
    }
    
    return results;
}

// 验证操作实现
bool ALU::is_valid_operation(BYTE op) const {
    return op >= ALU_ADD && op <= ALU_TEST;
}

bool ALU::is_arithmetic_operation(BYTE op) const {
    return op >= ALU_ADD && op <= ALU_MOD;
}

bool ALU::is_logical_operation(BYTE op) const {
    return op >= ALU_AND && op <= ALU_NOT;
}

bool ALU::is_shift_operation(BYTE op) const {
    return op >= ALU_SHL && op <= ALU_ROR;
}

bool ALU::is_compare_operation(BYTE op) const {
    return op == ALU_CMP || op == ALU_TEST;
}

// 私有方法实现
void ALU::update_flags_from_result(WORD result) {
    set_zero_flag(result == 0);
    set_negative_flag((result & 0x8000) != 0);
}

void ALU::update_flags_from_comparison(WORD a, WORD b) {
    set_equal_flag(a == b);
    set_greater_flag(a > b);
    set_negative_flag(a < b);
    set_zero_flag(a == b);
}

void ALU::clear_flags() {
    flags = 0;
}

void ALU::set_error(const std::string& error_msg) {
    last_error = error_msg;
    operation_valid = false;
    set_error_flag(true);
    LOG_ERROR("ALU", error_msg);
}

bool ALU::check_add_overflow(WORD a, WORD b, WORD result) {
    // 检查有符号加法溢出
    return ((a ^ result) & (b ^ result) & 0x8000) != 0;
}

bool ALU::check_sub_overflow(WORD a, WORD b, WORD result) {
    // 检查有符号减法溢出
    return ((a ^ b) & (a ^ result) & 0x8000) != 0;
}

bool ALU::check_mul_overflow(WORD a, WORD b, WORD result) {
    // 检查乘法溢出（简化版本）
    return (a != 0 && b != 0 && result / a != b);
}

bool ALU::check_add_carry(WORD a, WORD b, WORD result) {
    // 检查无符号加法进位
    return result < a;
}

bool ALU::check_sub_carry(WORD a, WORD b, WORD result) {
    // 检查无符号减法借位
    return a < b;
}
