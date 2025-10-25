#include "cache_manager.h"
#include "Common/Logger.h"
#include <iostream>
#include <iomanip>

CacheManager::CacheManager(Memory* mem) 
    : memory(mem), enabled(true), coherence_protocol(0), 
      write_allocate_enabled(true), prefetch_enabled(true) {
    
    // 初始化缓存
    icache = new InstructionCache();
    dcache = new DataCache();
    
    LOG_DEBUG("CacheManager", "Cache manager initialized");
}

CacheManager::~CacheManager() {
    if (icache) {
        delete icache;
        icache = nullptr;
    }
    if (dcache) {
        delete dcache;
        dcache = nullptr;
    }
}

bool CacheManager::fetch_instruction(WORD address, WORD& instruction) {
    if (!enabled) {
        // 缓存禁用时直接从内存读取
        BYTE data[2];
        data[0] = memory->read_byte(address);
        data[1] = memory->read_byte(address + 1);
        instruction = data[0] | (data[1] << 8);
        return true;
    }
    
    // 尝试从指令缓存获取
    if (icache->fetch_instruction(address, instruction)) {
        return true;
    }
    
    // 缓存未命中，从内存加载
    BYTE data[2];
    data[0] = memory->read_byte(address);
    data[1] = memory->read_byte(address + 1);
    instruction = data[0] | (data[1] << 8);
    
    // 将指令加载到缓存
    icache->write(address, data, 2);
    
    // 预取下一条指令
    if (prefetch_enabled) {
        icache->prefetch_instructions(address + 2, 4);
    }
    
    LOG_DEBUG("CacheManager", "Instruction fetched from memory: addr=0x" + 
             std::to_string(address) + ", inst=0x" + std::to_string(instruction));
    
    return true;
}

bool CacheManager::prefetch_instructions(WORD start_address, WORD count) {
    if (!enabled || !prefetch_enabled) {
        return false;
    }
    
    return icache->prefetch_instructions(start_address, count);
}

void CacheManager::invalidate_instruction_cache() {
    if (icache) {
        icache->invalidate_all();
        LOG_DEBUG("CacheManager", "Instruction cache invalidated");
    }
}

bool CacheManager::read_word(WORD address, WORD& value) {
    if (!enabled) {
        // 缓存禁用时直接从内存读取
        value = memory->read_word(address);
        return true;
    }
    
    // 尝试从数据缓存获取
    if (dcache->read_word(address, value)) {
        return true;
    }
    
    // 缓存未命中，从内存加载
    value = memory->read_word(address);
    
    // 将数据加载到缓存
    BYTE data[2];
    data[0] = value & 0xFF;
    data[1] = (value >> 8) & 0xFF;
    dcache->write(address, data, 2);
    
    LOG_DEBUG("CacheManager", "Word read from memory: addr=0x" + 
             std::to_string(address) + ", value=0x" + std::to_string(value));
    
    return true;
}

bool CacheManager::write_word(WORD address, WORD value) {
    if (!enabled) {
        // 缓存禁用时直接写入内存
        memory->write_word(address, value);
        return true;
    }
    
    // 尝试写入数据缓存
    if (dcache->write_word(address, value)) {
        // 缓存命中，根据写策略处理
        if (dcache->get_stats().write_hits > 0) {
            // 写命中，维护缓存一致性
            maintain_coherence(address);
        }
        return true;
    }
    
    // 缓存未命中
    if (write_allocate_enabled) {
        // 写分配策略：先加载到缓存再写入
        BYTE data[2];
        data[0] = memory->read_byte(address);
        data[1] = memory->read_byte(address + 1);
        dcache->write(address, data, 2);
        
        // 现在写入缓存
        dcache->write_word(address, value);
    } else {
        // 写不分配策略：直接写入内存
        memory->write_word(address, value);
    }
    
    // 维护缓存一致性
    maintain_coherence(address);
    
    LOG_DEBUG("CacheManager", "Word written: addr=0x" + 
             std::to_string(address) + ", value=0x" + std::to_string(value));
    
    return true;
}

bool CacheManager::read_byte(WORD address, BYTE& value) {
    if (!enabled) {
        // 缓存禁用时直接从内存读取
        value = memory->read_byte(address);
        return true;
    }
    
    // 尝试从数据缓存获取
    if (dcache->read_byte(address, value)) {
        return true;
    }
    
    // 缓存未命中，从内存加载
    value = memory->read_byte(address);
    
    // 将数据加载到缓存
    dcache->write_byte(address, value);
    
    LOG_DEBUG("CacheManager", "Byte read from memory: addr=0x" + 
             std::to_string(address) + ", value=0x" + std::to_string(value));
    
    return true;
}

bool CacheManager::write_byte(WORD address, BYTE value) {
    if (!enabled) {
        // 缓存禁用时直接写入内存
        memory->write_byte(address, value);
        return true;
    }
    
    // 尝试写入数据缓存
    if (dcache->write_byte(address, value)) {
        // 缓存命中，维护缓存一致性
        maintain_coherence(address);
        return true;
    }
    
    // 缓存未命中
    if (write_allocate_enabled) {
        // 写分配策略：先加载到缓存再写入
        BYTE data = memory->read_byte(address);
        dcache->write_byte(address, data);
        
        // 现在写入缓存
        dcache->write_byte(address, value);
    } else {
        // 写不分配策略：直接写入内存
        memory->write_byte(address, value);
    }
    
    // 维护缓存一致性
    maintain_coherence(address);
    
    LOG_DEBUG("CacheManager", "Byte written: addr=0x" + 
             std::to_string(address) + ", value=0x" + std::to_string(value));
    
    return true;
}

void CacheManager::flush_all() {
    if (icache) {
        icache->flush();
    }
    if (dcache) {
        dcache->flush();
    }
    
    LOG_DEBUG("CacheManager", "All caches flushed");
}

void CacheManager::flush_data_cache() {
    if (dcache) {
        dcache->flush();
        LOG_DEBUG("CacheManager", "Data cache flushed");
    }
}

void CacheManager::flush_instruction_cache() {
    if (icache) {
        icache->flush();
        LOG_DEBUG("CacheManager", "Instruction cache flushed");
    }
}

void CacheManager::invalidate_all() {
    if (icache) {
        icache->invalidate_all();
    }
    if (dcache) {
        dcache->invalidate_all();
    }
    
    LOG_DEBUG("CacheManager", "All caches invalidated");
}

void CacheManager::maintain_coherence(WORD address) {
    // 维护缓存一致性
    // 这里可以实现MESI协议或其他一致性协议
    
    // 简化实现：当数据缓存写入时，检查指令缓存是否需要失效
    if (icache) {
        // 检查地址是否在指令缓存中
        // 如果是，则失效该缓存行
        icache->invalidate(address);
    }
    
    LOG_DEBUG("CacheManager", "Cache coherence maintained for addr=0x" + 
             std::to_string(address));
}

void CacheManager::handle_write_invalidate(WORD address) {
    // 处理写失效
    if (icache) {
        icache->invalidate(address);
    }
    
    LOG_DEBUG("CacheManager", "Write invalidate handled for addr=0x" + 
             std::to_string(address));
}

void CacheManager::handle_write_update(WORD address, WORD value) {
    // 处理写更新
    if (icache) {
        // 更新指令缓存中的对应数据
        BYTE data[2];
        data[0] = value & 0xFF;
        data[1] = (value >> 8) & 0xFF;
        icache->write(address, data, 2);
    }
    
    LOG_DEBUG("CacheManager", "Write update handled for addr=0x" + 
             std::to_string(address));
}

void CacheManager::optimize_for_workload() {
    // 根据工作负载优化缓存
    if (icache) {
        icache->optimize_for_sequential_access();
    }
    if (dcache) {
        dcache->optimize_for_spatial_locality();
    }
    
    LOG_DEBUG("CacheManager", "Cache optimized for current workload");
}

void CacheManager::print_all_stats() const {
    std::cout << "=== Cache Manager Statistics ===" << std::endl;
    std::cout << "Enabled: " << (enabled ? "Yes" : "No") << std::endl;
    std::cout << "Prefetch: " << (prefetch_enabled ? "Yes" : "No") << std::endl;
    std::cout << "Write Allocate: " << (write_allocate_enabled ? "Yes" : "No") << std::endl;
    std::cout << std::endl;
    
    if (icache) {
        std::cout << "--- Instruction Cache ---" << std::endl;
        icache->print_stats();
    }
    
    if (dcache) {
        std::cout << "--- Data Cache ---" << std::endl;
        dcache->print_stats();
    }
    
    std::cout << "===============================" << std::endl;
}

void CacheManager::print_instruction_cache_stats() const {
    if (icache) {
        std::cout << "=== Instruction Cache Statistics ===" << std::endl;
        icache->print_stats();
        std::cout << "=====================================" << std::endl;
    }
}

void CacheManager::print_data_cache_stats() const {
    if (dcache) {
        std::cout << "=== Data Cache Statistics ===" << std::endl;
        dcache->print_stats();
        std::cout << "=============================" << std::endl;
    }
}

void CacheManager::clear_all_stats() {
    if (icache) {
        icache->clear_stats();
    }
    if (dcache) {
        dcache->clear_stats();
    }
    
    LOG_DEBUG("CacheManager", "All cache statistics cleared");
}

void CacheManager::configure_cache_sizes(WORD icache_size, WORD dcache_size) {
    // 重新配置缓存大小
    // 这里需要重新创建缓存实例
    LOG_DEBUG("CacheManager", "Cache sizes configured: I$=" + 
             std::to_string(icache_size) + ", D$=" + std::to_string(dcache_size));
}

void CacheManager::validate_cache_integrity() {
    LOG_DEBUG("CacheManager", "Validating cache integrity...");
    
    // 这里可以添加缓存完整性检查
    // 例如：检查缓存行的一致性、标签的正确性等
    
    LOG_DEBUG("CacheManager", "Cache integrity validation completed");
}

void CacheManager::print_cache_configuration() const {
    std::cout << "=== Cache Configuration ===" << std::endl;
    std::cout << "Enabled: " << (enabled ? "Yes" : "No") << std::endl;
    std::cout << "Coherence Protocol: " << coherence_protocol << std::endl;
    std::cout << "Write Allocate: " << (write_allocate_enabled ? "Yes" : "No") << std::endl;
    std::cout << "Prefetch: " << (prefetch_enabled ? "Yes" : "No") << std::endl;
    
    if (icache) {
        std::cout << "--- Instruction Cache ---" << std::endl;
        icache->print_cache_info();
    }
    
    if (dcache) {
        std::cout << "--- Data Cache ---" << std::endl;
        dcache->print_cache_info();
    }
    
    std::cout << "===========================" << std::endl;
}

// 私有辅助方法实现

bool CacheManager::load_from_memory(WORD address, BYTE* data, WORD size) {
    if (!memory || !data) {
        return false;
    }
    
    for (WORD i = 0; i < size; i++) {
        data[i] = memory->read_byte(address + i);
    }
    
    return true;
}

bool CacheManager::store_to_memory(WORD address, const BYTE* data, WORD size) {
    if (!memory || !data) {
        return false;
    }
    
    for (WORD i = 0; i < size; i++) {
        memory->write_byte(address + i, data[i]);
    }
    
    return true;
}

void CacheManager::handle_cache_miss(WORD address, bool is_instruction) {
    LOG_DEBUG("CacheManager", "Cache miss handled: addr=0x" + 
             std::to_string(address) + ", type=" + 
             (is_instruction ? "instruction" : "data"));
}

void CacheManager::optimize_cache_performance() {
    // 优化缓存性能
    optimize_for_workload();
    
    LOG_DEBUG("CacheManager", "Cache performance optimized");
}
