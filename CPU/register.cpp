#include "register.h"
#include "Common/Logger.h"
#include <iomanip>
#include <sstream>

RegisterFile::RegisterFile() {
    reset();
    LOG_DEBUG("RegisterFile", "Register file initialized");
}

void RegisterFile::reset() {
    // 清零所有通用寄存器
    memset(registers, 0, sizeof(registers));
    
    // 初始化特殊寄存器
    initialize_special_registers();
    
    // 清零标志寄存器
    flags = 0;
    
    LOG_DEBUG("RegisterFile", "Register file reset");
}

void RegisterFile::clear() {
    reset();
}

BYTE RegisterFile::read_byte(BYTE reg_index) {
    if (!is_valid_register(reg_index)) {
        LOG_ERROR("RegisterFile", "Invalid register index: " + std::to_string(reg_index));
        return 0;
    }
    
    if (is_special_register(reg_index)) {
        // 特殊寄存器按字访问，返回低字节
        WORD value = get_register_value(reg_index);
        return value & 0xFF;
    }
    
    return registers[reg_index];
}

void RegisterFile::write_byte(BYTE reg_index, BYTE value) {
    if (!is_valid_register(reg_index)) {
        LOG_ERROR("RegisterFile", "Invalid register index: " + std::to_string(reg_index));
        return;
    }
    
    if (is_special_register(reg_index)) {
        // 特殊寄存器按字访问，更新低字节
        WORD current_value = get_register_value(reg_index);
        WORD new_value = (current_value & 0xFF00) | value;
        set_register_value(reg_index, new_value);
    } else {
        registers[reg_index] = value;
    }
}

WORD RegisterFile::read_word(BYTE reg_index) {
    if (!is_valid_register(reg_index)) {
        LOG_ERROR("RegisterFile", "Invalid register index: " + std::to_string(reg_index));
        return 0;
    }
    
    if (is_special_register(reg_index)) {
        return get_register_value(reg_index);
    }
    
    // 通用寄存器按字节存储，需要组合成字
    WORD high_byte = registers[reg_index];
    WORD low_byte = (reg_index + 1 < TOY_REG_COUNT) ? registers[reg_index + 1] : 0;
    return (high_byte << 8) | low_byte;
}

void RegisterFile::write_word(BYTE reg_index, WORD value) {
    if (!is_valid_register(reg_index)) {
        LOG_ERROR("RegisterFile", "Invalid register index: " + std::to_string(reg_index));
        return;
    }
    
    if (is_special_register(reg_index)) {
        set_register_value(reg_index, value);
    } else {
        // 通用寄存器按字节存储
        registers[reg_index] = (value >> 8) & 0xFF;
        if (reg_index + 1 < TOY_REG_COUNT) {
            registers[reg_index + 1] = value & 0xFF;
        }
    }
}

void RegisterFile::read_register_block(BYTE start_reg, BYTE* buffer, BYTE count) {
    if (!buffer || count == 0) {
        LOG_ERROR("RegisterFile", "Invalid read_register_block parameters");
        return;
    }
    
    if (start_reg + count > TOY_REG_COUNT) {
        LOG_ERROR("RegisterFile", "Register block read exceeds bounds");
        return;
    }
    
    for (BYTE i = 0; i < count; i++) {
        buffer[i] = read_byte(start_reg + i);
    }
}

void RegisterFile::write_register_block(BYTE start_reg, const BYTE* buffer, BYTE count) {
    if (!buffer || count == 0) {
        LOG_ERROR("RegisterFile", "Invalid write_register_block parameters");
        return;
    }
    
    if (start_reg + count > TOY_REG_COUNT) {
        LOG_ERROR("RegisterFile", "Register block write exceeds bounds");
        return;
    }
    
    for (BYTE i = 0; i < count; i++) {
        write_byte(start_reg + i, buffer[i]);
    }
}

void RegisterFile::copy_register(BYTE dest_reg, BYTE src_reg) {
    if (!is_valid_register(dest_reg) || !is_valid_register(src_reg)) {
        LOG_ERROR("RegisterFile", "Invalid register indices for copy");
        return;
    }
    
    WORD value = read_word(src_reg);
    write_word(dest_reg, value);
}

void RegisterFile::swap_registers(BYTE reg1, BYTE reg2) {
    if (!is_valid_register(reg1) || !is_valid_register(reg2)) {
        LOG_ERROR("RegisterFile", "Invalid register indices for swap");
        return;
    }
    
    WORD value1 = read_word(reg1);
    WORD value2 = read_word(reg2);
    write_word(reg1, value2);
    write_word(reg2, value1);
}

bool RegisterFile::compare_registers(BYTE reg1, BYTE reg2) {
    if (!is_valid_register(reg1) || !is_valid_register(reg2)) {
        LOG_ERROR("RegisterFile", "Invalid register indices for compare");
        return false;
    }
    
    WORD value1 = read_word(reg1);
    WORD value2 = read_word(reg2);
    return value1 == value2;
}

WORD RegisterFile::get_register_value(BYTE reg_index) const {
    switch (reg_index) {
        case REG_SP: return stack_pointer;
        case REG_BP: return base_pointer;
        case REG_SI: return source_index;
        case REG_DI: return destination_index;
        case REG_CS: return code_segment;
        case REG_DS: return data_segment;
        case REG_ES: return extra_segment;
        case REG_SS: return stack_segment;
        case REG_FS: return file_segment;
        case REG_GS: return global_segment;
        case REG_IP: return instruction_pointer;
        case REG_IBUS: return instruction_bus;
        case REG_DBUS: return data_bus;
        case REG_ABUS: return address_bus;
        default:
            if (reg_index < 255) {  // BYTE最大值是255
                // 通用寄存器按字节存储，需要组合成字
                WORD high_byte = registers[reg_index];
                WORD low_byte = (reg_index + 1 < TOY_REG_COUNT) ? registers[reg_index + 1] : 0;
                return (high_byte << 8) | low_byte;
            }
            return 0;
    }
}

void RegisterFile::set_register_value(BYTE reg_index, WORD value) {
    switch (reg_index) {
        case REG_SP: stack_pointer = value; break;
        case REG_BP: base_pointer = value; break;
        case REG_SI: source_index = value; break;
        case REG_DI: destination_index = value; break;
        case REG_CS: code_segment = value; break;
        case REG_DS: data_segment = value; break;
        case REG_ES: extra_segment = value; break;
        case REG_SS: stack_segment = value; break;
        case REG_FS: file_segment = value; break;
        case REG_GS: global_segment = value; break;
        case REG_IP: instruction_pointer = value; break;
        case REG_IBUS: instruction_bus = value; break;
        case REG_DBUS: data_bus = value; break;
        case REG_ABUS: address_bus = value; break;
        default:
            if (reg_index < 255) {  // BYTE最大值是255
                // 通用寄存器按字节存储
                registers[reg_index] = (value >> 8) & 0xFF;
                if (reg_index + 1 < 255) {  // BYTE最大值是255
                    registers[reg_index + 1] = value & 0xFF;
                }
            }
            break;
    }
}

void RegisterFile::print_register_info() {
    LOG_INFO("RegisterFile", "=== Register File Information ===");
    LOG_INFO("RegisterFile", "Total registers: " + std::to_string(TOY_REG_COUNT));
    LOG_INFO("RegisterFile", "Special registers: 14");
    LOG_INFO("RegisterFile", "General purpose registers: " + std::to_string(TOY_REG_COUNT - 14));
    LOG_INFO("RegisterFile", "Current flags: 0x" + std::to_string(flags));
}

void RegisterFile::print_register_dump() {
    LOG_INFO("RegisterFile", "=== Register Dump ===");
    LOG_INFO("RegisterFile", get_register_dump_string());
    LOG_INFO("RegisterFile", get_special_registers_string());
}

std::string RegisterFile::get_register_dump_string() {
    std::stringstream ss;
    
    // 显示前16个通用寄存器，每行8个
    ss << "R00-R07: ";
    for (int i = 0; i < 8; i++) {
        ss << std::to_string(read_word(i)) << " ";
    }
    ss << "\n";
    
    ss << "R08-R15: ";
    for (int i = 8; i < 16; i++) {
        ss << std::to_string(read_word(i)) << " ";
    }
    ss << "\n";
    
    return ss.str();
}

std::string RegisterFile::get_special_registers_string() {
    std::stringstream ss;
    
    ss << "SP:" << std::to_string(stack_pointer) << " ";
    ss << "BP:" << std::to_string(base_pointer) << " ";
    ss << "SI:" << std::to_string(source_index) << " ";
    ss << "DI:" << std::to_string(destination_index) << " ";
    ss << "CS:" << std::to_string(code_segment) << " ";
    ss << "DS:" << std::to_string(data_segment) << " ";
    ss << "IP:" << std::to_string(instruction_pointer);
    
    return ss.str();
}

bool RegisterFile::is_valid_register(BYTE reg_index) const {
    return reg_index < 255;  // BYTE最大值是255
}

bool RegisterFile::is_special_register(BYTE reg_index) const {
    return reg_index <= REG_ABUS;  // 特殊寄存器范围是0-13
}

std::string RegisterFile::get_register_name(BYTE reg_index) const {
    if (is_special_register(reg_index)) {
        return get_special_register_name(reg_index);
    }
    
    std::stringstream ss;
    ss << "R" << std::setfill('0') << std::setw(2) << (int)reg_index;
    return ss.str();
}

std::string RegisterFile::get_special_register_name(BYTE reg_index) const {
    switch (reg_index) {
        case REG_SP: return "SP";
        case REG_BP: return "BP";
        case REG_SI: return "SI";
        case REG_DI: return "DI";
        case REG_CS: return "CS";
        case REG_DS: return "DS";
        case REG_ES: return "ES";
        case REG_SS: return "SS";
        case REG_FS: return "FS";
        case REG_GS: return "GS";
        case REG_IP: return "IP";
        case REG_IBUS: return "IBUS";
        case REG_DBUS: return "DBUS";
        case REG_ABUS: return "ABUS";
        default: return "UNKNOWN";
    }
}

// 私有方法实现
void RegisterFile::initialize_special_registers() {
    // 初始化特殊寄存器为默认值
    stack_pointer = 0xFFFC;  // 栈指针初始化为接近内存顶部
    base_pointer = 0;
    source_index = 0;
    destination_index = 0;
    code_segment = 0;
    data_segment = 0;
    extra_segment = 0;
    stack_segment = 0;
    file_segment = 0;
    global_segment = 0;
    instruction_pointer = 0;
    instruction_bus = 0;
    data_bus = 0;
    address_bus = 0;
}

void RegisterFile::update_flags_from_value(WORD value) {
    // 更新标志位基于值
    set_zero_flag(value == 0);
    set_negative_flag((value & 0x8000) != 0);
}

void RegisterFile::update_flags_from_comparison(WORD value1, WORD value2) {
    // 更新标志位基于比较
    set_equal_flag(value1 == value2);
    set_greater_flag(value1 > value2);
    set_negative_flag(value1 < value2);
}
