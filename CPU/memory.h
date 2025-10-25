#ifndef __MEMORY_H_
#define __MEMORY_H_

#include <cstdint>
#include <cstring>
#include <string>

// Toy架构基本类型定义
typedef uint16_t WORD;
typedef uint8_t BYTE;

// 前向声明
struct PageTableEntry;

// Toy架构常量定义
#define TOY_MEM_SIZE (64 * 1024)  // 64KB内存
#define TOY_MMIO_BASE 0xF000      // MMIO基地址
#define TOY_MMIO_SIZE 0x1000      // MMIO大小

// 前向声明
class MMIODevice;

/**
 * Memory抽象类 - 封装内存访问操作
 * 提供统一的内存读写接口，支持RAM和MMIO访问
 */
class Memory {
private:
    BYTE RAM[TOY_MEM_SIZE];         // 主内存
    MMIODevice* mmio_devices[16];   // MMIO设备数组
    WORD instruction_pointer;       // 指令指针（用于顺序读取）
    
public:
    Memory();
    ~Memory() = default;
    
    // 基本内存操作
    void reset();
    void clear();
    
    // 字节访问
    BYTE read_byte(WORD address);
    void write_byte(WORD address, BYTE value);
    
    // 字访问
    WORD read_word(WORD address);
    void write_word(WORD address, WORD value);
    
    // 顺序读取（用于指令解码）
    BYTE read_next_byte();
    WORD read_next_word();
    
    // 批量操作
    void read_block(WORD address, BYTE* buffer, WORD size);
    void write_block(WORD address, const BYTE* buffer, WORD size);
    
    // 内存复制
    void copy_memory(WORD dest, WORD src, WORD size);
    
    // 内存填充
    void fill_memory(WORD address, BYTE value, WORD size);
    
    // 内存比较
    bool compare_memory(WORD addr1, WORD addr2, WORD size);
    
    // 内存搜索
    WORD find_byte(WORD start_addr, BYTE value, WORD max_size);
    WORD find_word(WORD start_addr, WORD value, WORD max_size);
    
    // 内存转储
    std::string dump_memory_hex(WORD start, WORD end);
    std::string dump_memory_ascii(WORD start, WORD end);
    
    // MMIO设备管理
    void register_mmio_device(int device_id, MMIODevice* device);
    MMIODevice* get_mmio_device(int device_id);
    
    // 地址验证
    bool is_valid_address(WORD address);
    bool is_mmio_address(WORD address);
    bool is_ram_address(WORD address);
    
    // 内存统计
    WORD get_memory_size() const { return static_cast<WORD>(TOY_MEM_SIZE); }
    WORD get_mmio_base() const { return TOY_MMIO_BASE; }
    WORD get_mmio_size() const { return TOY_MMIO_SIZE; }
    
    // 指令指针管理
    void set_instruction_pointer(WORD ip) { instruction_pointer = ip; }
    WORD get_instruction_pointer() const { return instruction_pointer; }
    
    // 调试和诊断
    void print_memory_info();
    void validate_memory_integrity();
    
    // 页表支持
    PageTableEntry read_page_table_entry(WORD address);
    void write_page_table_entry(WORD address, const PageTableEntry& entry);
    PageTableEntry* get_page_table_entry_ptr(WORD address);
    
private:
    // 内部辅助方法
    WORD read_mmio(WORD address);
    void write_mmio(WORD address, WORD value);
    WORD get_mmio_device_id(WORD address);
    WORD get_mmio_device_offset(WORD address);
};

#endif // __MEMORY_H_
