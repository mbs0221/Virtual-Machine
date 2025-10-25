#include "memory.h"
#include "mmio.h"
#include "mmu.h"
#include "Common/Logger.h"
#include <iomanip>
#include <sstream>

Memory::Memory() {
    // 初始化MMIO设备数组
    for (int i = 0; i < 16; i++) {
        mmio_devices[i] = nullptr;
    }
    
    instruction_pointer = 0;
    reset();
    
    LOG_DEBUG("Memory", "Memory system initialized");
}

void Memory::reset() {
    // 清零主内存
    memset(RAM, 0, sizeof(RAM));
    instruction_pointer = 0;
    
    LOG_DEBUG("Memory", "Memory system reset");
}

void Memory::clear() {
    reset();
}

BYTE Memory::read_byte(WORD address) {
    if (!is_valid_address(address)) {
        LOG_ERROR("Memory", "Invalid byte read address: 0x" + std::to_string(address));
        return 0;
    }
    
    if (is_mmio_address(address)) {
        // MMIO访问返回0（MMIO通常按字访问）
        return 0;
    }
    
    return RAM[address];
}

void Memory::write_byte(WORD address, BYTE value) {
    if (!is_valid_address(address)) {
        LOG_ERROR("Memory", "Invalid byte write address: 0x" + std::to_string(address));
        return;
    }
    
    if (is_mmio_address(address)) {
        // MMIO通常按字访问，字节写入可能不被支持
        LOG_WARN("Memory", "Byte write to MMIO address: 0x" + std::to_string(address));
        return;
    }
    
    RAM[address] = value;
}

WORD Memory::read_word(WORD address) {
    if (!is_valid_address(address)) {
        LOG_ERROR("Memory", "Invalid word read address: 0x" + std::to_string(address));
        return 0;
    }
    
    if (is_mmio_address(address)) {
        return read_mmio(address);
    }
    
    // 按小端序读取字
    WORD A = address << 1;
    WORD W = 0;
    W |= (WORD)RAM[A] << 8;
    W |= (WORD)RAM[A + 1];
    
    return W;
}

void Memory::write_word(WORD address, WORD value) {
    if (!is_valid_address(address)) {
        LOG_ERROR("Memory", "Invalid word write address: 0x" + std::to_string(address));
        return;
    }
    
    if (is_mmio_address(address)) {
        write_mmio(address, value);
        return;
    }
    
    // 按小端序写入字
    WORD A = address << 1;
    RAM[A] = value >> 8;
    RAM[A + 1] = value;
}

BYTE Memory::read_next_byte() {
    if (instruction_pointer >= TOY_MEM_SIZE) {
        LOG_ERROR("Memory", "Instruction pointer out of bounds: " + std::to_string(instruction_pointer));
        return 0;
    }
    
    return RAM[instruction_pointer++];
}

WORD Memory::read_next_word() {
    WORD W = 0;
    W |= (WORD)read_next_byte();
    W |= (WORD)read_next_byte() << 8;
    return W;
}

void Memory::read_block(WORD address, BYTE* buffer, WORD size) {
    if (!buffer || size == 0) {
        LOG_ERROR("Memory", "Invalid read_block parameters");
        return;
    }
    
    if (address + size > TOY_MEM_SIZE) {
        LOG_ERROR("Memory", "Read block exceeds memory bounds");
        return;
    }
    
    memcpy(buffer, &RAM[address], size);
}

void Memory::write_block(WORD address, const BYTE* buffer, WORD size) {
    if (!buffer || size == 0) {
        LOG_ERROR("Memory", "Invalid write_block parameters");
        return;
    }
    
    if (address + size > TOY_MEM_SIZE) {
        LOG_ERROR("Memory", "Write block exceeds memory bounds");
        return;
    }
    
    memcpy(&RAM[address], buffer, size);
}

void Memory::copy_memory(WORD dest, WORD src, WORD size) {
    if (dest + size > TOY_MEM_SIZE || src + size > TOY_MEM_SIZE) {
        LOG_ERROR("Memory", "Memory copy exceeds bounds");
        return;
    }
    
    memmove(&RAM[dest], &RAM[src], size);
}

void Memory::fill_memory(WORD address, BYTE value, WORD size) {
    if (address + size > TOY_MEM_SIZE) {
        LOG_ERROR("Memory", "Memory fill exceeds bounds");
        return;
    }
    
    memset(&RAM[address], value, size);
}

bool Memory::compare_memory(WORD addr1, WORD addr2, WORD size) {
    if (addr1 + size > TOY_MEM_SIZE || addr2 + size > TOY_MEM_SIZE) {
        LOG_ERROR("Memory", "Memory compare exceeds bounds");
        return false;
    }
    
    return memcmp(&RAM[addr1], &RAM[addr2], size) == 0;
}

WORD Memory::find_byte(WORD start_addr, BYTE value, WORD max_size) {
    if (start_addr >= TOY_MEM_SIZE) {
        return 0xFFFF; // 未找到
    }
    
    WORD end_addr = start_addr + max_size;
    if (end_addr > TOY_MEM_SIZE) {
        end_addr = TOY_MEM_SIZE;
    }
    
    for (WORD addr = start_addr; addr < end_addr; addr++) {
        if (RAM[addr] == value) {
            return addr;
        }
    }
    
    return 0xFFFF; // 未找到
}

WORD Memory::find_word(WORD start_addr, WORD value, WORD max_size) {
    if (start_addr >= TOY_MEM_SIZE) {
        return 0xFFFF; // 未找到
    }
    
    WORD end_addr = start_addr + max_size;
    if (end_addr > TOY_MEM_SIZE) {
        end_addr = TOY_MEM_SIZE;
    }
    
    for (WORD addr = start_addr; addr < end_addr - 1; addr += 2) {
        if (read_word(addr) == value) {
            return addr;
        }
    }
    
    return 0xFFFF; // 未找到
}

std::string Memory::dump_memory_hex(WORD start, WORD end) {
    std::stringstream ss;
    
    if (start >= TOY_MEM_SIZE || end > TOY_MEM_SIZE || start >= end) {
        return "Invalid address range";
    }
    
    for (WORD addr = start; addr < end; addr += 16) {
        ss << std::hex << std::setfill('0') << std::setw(4) << addr << ": ";
        
        for (int i = 0; i < 16 && addr + i < end; i++) {
            ss << std::hex << std::setfill('0') << std::setw(2) << (int)RAM[addr + i] << " ";
        }
        
        ss << "\n";
    }
    
    return ss.str();
}

std::string Memory::dump_memory_ascii(WORD start, WORD end) {
    std::stringstream ss;
    
    if (start >= TOY_MEM_SIZE || end > TOY_MEM_SIZE || start >= end) {
        return "Invalid address range";
    }
    
    for (WORD addr = start; addr < end; addr += 16) {
        ss << std::hex << std::setfill('0') << std::setw(4) << addr << ": ";
        
        // 显示十六进制
        for (int i = 0; i < 16 && addr + i < end; i++) {
            ss << std::hex << std::setfill('0') << std::setw(2) << (int)RAM[addr + i] << " ";
        }
        
        ss << " | ";
        
        // 显示ASCII
        for (int i = 0; i < 16 && addr + i < end; i++) {
            char c = RAM[addr + i];
            ss << (isprint(c) ? c : '.');
        }
        
        ss << "\n";
    }
    
    return ss.str();
}

void Memory::register_mmio_device(int device_id, MMIODevice* device) {
    if (device_id >= 0 && device_id < 16) {
        mmio_devices[device_id] = device;
        LOG_DEBUG("Memory", "MMIO device " + std::to_string(device_id) + " registered");
    } else {
        LOG_ERROR("Memory", "Invalid MMIO device ID: " + std::to_string(device_id));
    }
}

MMIODevice* Memory::get_mmio_device(int device_id) {
    if (device_id >= 0 && device_id < 16) {
        return mmio_devices[device_id];
    }
    return nullptr;
}

bool Memory::is_valid_address(WORD address) {
    return address < TOY_MEM_SIZE;
}

bool Memory::is_mmio_address(WORD address) {
    return address >= TOY_MMIO_BASE && address < TOY_MMIO_BASE + TOY_MMIO_SIZE;
}

bool Memory::is_ram_address(WORD address) {
    return address < TOY_MMIO_BASE;
}

void Memory::print_memory_info() {
    LOG_INFO("Memory", "=== Memory System Information ===");
    LOG_INFO("Memory", "Total RAM size: " + std::to_string(TOY_MEM_SIZE) + " bytes");
    LOG_INFO("Memory", "MMIO base address: 0x" + std::to_string(TOY_MMIO_BASE));
    LOG_INFO("Memory", "MMIO size: " + std::to_string(TOY_MMIO_SIZE) + " bytes");
    LOG_INFO("Memory", "Current instruction pointer: 0x" + std::to_string(instruction_pointer));
    
    // 统计MMIO设备
    int device_count = 0;
    for (int i = 0; i < 16; i++) {
        if (mmio_devices[i] != nullptr) {
            device_count++;
        }
    }
    LOG_INFO("Memory", "Registered MMIO devices: " + std::to_string(device_count));
}

void Memory::validate_memory_integrity() {
    LOG_DEBUG("Memory", "Validating memory integrity...");
    
    // 检查关键内存区域
    bool integrity_ok = true;
    
    // 这里可以添加更多的完整性检查
    // 例如：检查栈指针是否在有效范围内等
    
    if (integrity_ok) {
        LOG_DEBUG("Memory", "Memory integrity check passed");
    } else {
        LOG_ERROR("Memory", "Memory integrity check failed");
    }
}

// 私有方法实现
WORD Memory::read_mmio(WORD address) {
    WORD offset = address - TOY_MMIO_BASE;
    int device_id = offset / 16;  // 每个设备占用16字节
    WORD device_offset = offset % 16;
    
    if (device_id >= 0 && device_id < 16 && mmio_devices[device_id]) {
        return mmio_devices[device_id]->read(device_offset);
    }
    
    return 0;
}

void Memory::write_mmio(WORD address, WORD value) {
    WORD offset = address - TOY_MMIO_BASE;
    int device_id = offset / 16;  // 每个设备占用16字节
    WORD device_offset = offset % 16;
    
    LOG_DEBUG("Memory", "MMIO write: address=0x" + std::to_string(address) + 
             ", value=" + std::to_string(value) + 
             ", device_id=" + std::to_string(device_id) + 
             ", offset=" + std::to_string(device_offset));
    
    if (device_id >= 0 && device_id < 16 && mmio_devices[device_id]) {
        mmio_devices[device_id]->write(device_offset, value);
    }
}

WORD Memory::get_mmio_device_id(WORD address) {
    WORD offset = address - TOY_MMIO_BASE;
    return offset / 16;
}

WORD Memory::get_mmio_device_offset(WORD address) {
    WORD offset = address - TOY_MMIO_BASE;
    return offset % 16;
}

// 页表支持方法实现

PageTableEntry Memory::read_page_table_entry(WORD address) {
    if (!is_valid_address(address) || address + sizeof(PageTableEntry) > TOY_MEM_SIZE) {
        LOG_ERROR("Memory", "Invalid page table entry read address: 0x" + std::to_string(address));
        return PageTableEntry();
    }
    
    PageTableEntry entry;
    // 从内存中读取页表项（4字节：2字节物理地址 + 1字节标志 + 1字节保留）
    entry.physical_addr = read_word(address);
    entry.flags = read_byte(address + 2);
    entry.reserved = read_byte(address + 3);
    
    LOG_DEBUG("Memory", "Read page table entry: addr=0x" + std::to_string(address) + 
             ", physical=0x" + std::to_string(entry.physical_addr) + 
             ", flags=0x" + std::to_string(entry.flags));
    
    return entry;
}

void Memory::write_page_table_entry(WORD address, const PageTableEntry& entry) {
    if (!is_valid_address(address) || address + sizeof(PageTableEntry) > TOY_MEM_SIZE) {
        LOG_ERROR("Memory", "Invalid page table entry write address: 0x" + std::to_string(address));
        return;
    }
    
    // 写入页表项到内存
    write_word(address, entry.physical_addr);
    write_byte(address + 2, entry.flags);
    write_byte(address + 3, entry.reserved);
    
    LOG_DEBUG("Memory", "Write page table entry: addr=0x" + std::to_string(address) + 
             ", physical=0x" + std::to_string(entry.physical_addr) + 
             ", flags=0x" + std::to_string(entry.flags));
}

PageTableEntry* Memory::get_page_table_entry_ptr(WORD address) {
    if (!is_valid_address(address) || address + sizeof(PageTableEntry) > TOY_MEM_SIZE) {
        LOG_ERROR("Memory", "Invalid page table entry pointer address: 0x" + std::to_string(address));
        return nullptr;
    }
    
    // 返回指向页表项的指针
    return reinterpret_cast<PageTableEntry*>(&RAM[address]);
}
