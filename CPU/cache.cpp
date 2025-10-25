#include "cache.h"
#include "Common/Logger.h"
#include <iostream>
#include <iomanip>
#include <random>
#include <cstring>

// 通用缓存基类实现

Cache::Cache(WORD lines, WORD size, CachePolicy wp, ReplacementPolicy rp)
    : num_lines(lines), line_size(size), access_time(0), 
      write_policy(wp), replacement_policy(rp) {
    
    // 初始化缓存行
    this->lines.resize(num_lines);
    for (WORD i = 0; i < num_lines; i++) {
        this->lines[i] = CacheLine();
    }
    
    LOG_DEBUG("Cache", "Cache initialized: " + std::to_string(num_lines) + 
             " lines, " + std::to_string(line_size) + " bytes per line");
}

bool Cache::read(WORD address, BYTE* data, WORD size) {
    if (!data || size == 0) {
        return false;
    }
    
    access_time++;
    
    // 查找缓存行
    CacheLine* line = find_line(address);
    if (line && line->is_valid()) {
        // 缓存命中
        stats.hits++;
        stats.read_hits++;
        line->increment_access();
        line->update_access_time(access_time);
        
        // 复制数据
        WORD offset = get_offset(address);
        WORD copy_size = std::min(size, static_cast<WORD>(line_size - offset));
        memcpy(data, &line->data[offset], copy_size);
        
        LOG_DEBUG("Cache", "Cache read hit: addr=0x" + std::to_string(address) + 
                 ", size=" + std::to_string(size));
        
        return true;
    } else {
        // 缓存未命中
        stats.misses++;
        stats.read_misses++;
        
        LOG_DEBUG("Cache", "Cache read miss: addr=0x" + std::to_string(address) + 
                 ", size=" + std::to_string(size));
        
        return false;
    }
}

bool Cache::write(WORD address, const BYTE* data, WORD size) {
    if (!data || size == 0) {
        return false;
    }
    
    access_time++;
    
    // 查找缓存行
    CacheLine* line = find_line(address);
    if (line && line->is_valid()) {
        // 缓存命中
        stats.hits++;
        stats.write_hits++;
        line->increment_access();
        line->update_access_time(access_time);
        
        // 写入数据
        WORD offset = get_offset(address);
        WORD copy_size = std::min(size, static_cast<WORD>(line_size - offset));
        memcpy(&line->data[offset], data, copy_size);
        
        // 根据写策略处理
        if (write_policy == CACHE_WRITE_BACK) {
            line->set_dirty();
        } else if (write_policy == CACHE_WRITE_THROUGH) {
            // 写通策略需要立即写回内存
            writeback_line(line);
        }
        
        LOG_DEBUG("Cache", "Cache write hit: addr=0x" + std::to_string(address) + 
                 ", size=" + std::to_string(size));
        
        return true;
    } else {
        // 缓存未命中
        stats.misses++;
        stats.write_misses++;
        
        // 根据写分配策略处理
        if (write_policy == CACHE_WRITE_ALLOCATE) {
            // 分配新的缓存行
            line = allocate_line(address);
            if (line) {
                // 写入数据
                WORD offset = get_offset(address);
                WORD copy_size = std::min(size, static_cast<WORD>(line_size - offset));
                memcpy(&line->data[offset], data, copy_size);
                
                if (write_policy == CACHE_WRITE_BACK) {
                    line->set_dirty();
                }
                
                LOG_DEBUG("Cache", "Cache write allocate: addr=0x" + std::to_string(address));
                return true;
            }
        }
        
        LOG_DEBUG("Cache", "Cache write miss: addr=0x" + std::to_string(address) + 
                 ", size=" + std::to_string(size));
        
        return false;
    }
}

void Cache::invalidate(WORD address) {
    CacheLine* line = find_line(address);
    if (line && line->is_valid()) {
        if (line->is_dirty()) {
            writeback_line(line);
        }
        line->set_invalid();
        
        LOG_DEBUG("Cache", "Cache line invalidated: addr=0x" + std::to_string(address));
    }
}

void Cache::invalidate_all() {
    for (WORD i = 0; i < num_lines; i++) {
        if (lines[i].is_valid() && lines[i].is_dirty()) {
            writeback_line(&lines[i]);
        }
        lines[i].set_invalid();
    }
    
    LOG_DEBUG("Cache", "All cache lines invalidated");
}

void Cache::flush() {
    for (WORD i = 0; i < num_lines; i++) {
        if (lines[i].is_valid() && lines[i].is_dirty()) {
            writeback_line(&lines[i]);
            lines[i].set_shared(); // 清除脏标志
        }
    }
    
    LOG_DEBUG("Cache", "Cache flushed");
}

void Cache::flush_line(WORD address) {
    CacheLine* line = find_line(address);
    if (line && line->is_valid() && line->is_dirty()) {
        writeback_line(line);
        line->set_shared();
        
        LOG_DEBUG("Cache", "Cache line flushed: addr=0x" + std::to_string(address));
    }
}

void Cache::print_stats() const {
    std::cout << "=== Cache Statistics ===" << std::endl;
    std::cout << "Total Hits: " << stats.hits << std::endl;
    std::cout << "Total Misses: " << stats.misses << std::endl;
    std::cout << "Hit Rate: " << std::fixed << std::setprecision(2) << stats.get_hit_rate() << "%" << std::endl;
    std::cout << "Read Hits: " << stats.read_hits << std::endl;
    std::cout << "Read Misses: " << stats.read_misses << std::endl;
    std::cout << "Read Hit Rate: " << std::fixed << std::setprecision(2) << stats.get_read_hit_rate() << "%" << std::endl;
    std::cout << "Write Hits: " << stats.write_hits << std::endl;
    std::cout << "Write Misses: " << stats.write_misses << std::endl;
    std::cout << "Write Hit Rate: " << std::fixed << std::setprecision(2) << stats.get_write_hit_rate() << "%" << std::endl;
    std::cout << "Evictions: " << stats.evictions << std::endl;
    std::cout << "Writebacks: " << stats.writebacks << std::endl;
    std::cout << "=======================" << std::endl;
}

void Cache::print_cache_info() const {
    std::cout << "=== Cache Information ===" << std::endl;
    std::cout << "Lines: " << num_lines << std::endl;
    std::cout << "Line Size: " << line_size << " bytes" << std::endl;
    std::cout << "Total Size: " << (num_lines * line_size) << " bytes" << std::endl;
    std::cout << "Write Policy: " << (write_policy == CACHE_WRITE_BACK ? "Write-Back" : "Write-Through") << std::endl;
    std::cout << "Replacement Policy: ";
    switch (replacement_policy) {
        case REPLACEMENT_LRU: std::cout << "LRU"; break;
        case REPLACEMENT_FIFO: std::cout << "FIFO"; break;
        case REPLACEMENT_RANDOM: std::cout << "Random"; break;
        case REPLACEMENT_LFU: std::cout << "LFU"; break;
    }
    std::cout << std::endl;
    std::cout << "========================" << std::endl;
}

// 内部辅助方法实现

WORD Cache::get_line_index(WORD address) const {
    return (address & CACHE_LINE_MASK) / line_size % num_lines;
}

WORD Cache::get_tag(WORD address) const {
    return address & ~CACHE_LINE_MASK;
}

WORD Cache::get_offset(WORD address) const {
    return address & CACHE_LINE_OFFSET_MASK;
}

CacheLine* Cache::find_line(WORD address) {
    WORD line_index = get_line_index(address);
    WORD tag = get_tag(address);
    
    if (lines[line_index].is_valid() && lines[line_index].tag == tag) {
        return &lines[line_index];
    }
    
    return nullptr;
}

CacheLine* Cache::allocate_line(WORD address) {
    WORD line_index = get_line_index(address);
    WORD tag = get_tag(address);
    
    // 检查是否需要驱逐现有行
    if (lines[line_index].is_valid()) {
        evict_line(line_index);
    }
    
    // 分配新行
    lines[line_index].tag = tag;
    lines[line_index].set_valid();
    lines[line_index].access_count = 0;
    lines[line_index].last_access_time = access_time;
    
    // 从内存加载数据（这里简化处理，实际应该从内存系统加载）
    memset(lines[line_index].data, 0, line_size);
    
    LOG_DEBUG("Cache", "Cache line allocated: addr=0x" + std::to_string(address) + 
             ", line=" + std::to_string(line_index));
    
    return &lines[line_index];
}

void Cache::evict_line(WORD line_index) {
    if (lines[line_index].is_valid()) {
        if (lines[line_index].is_dirty()) {
            writeback_line(&lines[line_index]);
        }
        lines[line_index].set_invalid();
        stats.evictions++;
        
        LOG_DEBUG("Cache", "Cache line evicted: line=" + std::to_string(line_index));
    }
}

void Cache::writeback_line(CacheLine* line) {
    if (line && line->is_dirty()) {
        // 这里应该将数据写回内存系统
        // 简化实现，实际应该调用内存系统的写方法
        line->set_shared(); // 清除脏标志
        stats.writebacks++;
        
        LOG_DEBUG("Cache", "Cache line writeback: tag=0x" + std::to_string(line->tag));
    }
}

WORD Cache::find_victim_line(WORD set_index) const {
    switch (replacement_policy) {
        case REPLACEMENT_LRU: return find_lru_line(set_index);
        case REPLACEMENT_FIFO: return find_fifo_line(set_index);
        case REPLACEMENT_RANDOM: return find_random_line(set_index);
        case REPLACEMENT_LFU: return find_lfu_line(set_index);
        default: return find_lru_line(set_index);
    }
}

WORD Cache::find_lru_line(WORD set_index) const {
    // 简化实现，假设直接映射缓存
    (void)set_index; // 避免未使用参数警告
    return set_index;
}

WORD Cache::find_fifo_line(WORD set_index) const {
    // 简化实现，假设直接映射缓存
    return set_index;
}

WORD Cache::find_lfu_line(WORD set_index) const {
    // 简化实现，假设直接映射缓存
    (void)set_index; // 避免未使用参数警告
    return set_index;
}

WORD Cache::find_random_line(WORD set_index) const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, num_lines - 1);
    
    (void)set_index; // 避免未使用参数警告
    return dis(gen);
}

// 指令缓存实现

bool InstructionCache::fetch_instruction(WORD address, WORD& instruction) {
    BYTE data[2];
    if (read(address, data, 2)) {
        instruction = data[0] | (data[1] << 8);
        return true;
    }
    return false;
}

bool InstructionCache::prefetch_instructions(WORD start_address, WORD count) {
    // 预取指令到缓存
    WORD current_addr = start_address;
    for (WORD i = 0; i < count; i++) {
        BYTE dummy[2];
        if (!read(current_addr, dummy, 2)) {
            // 如果缓存未命中，分配缓存行
            allocate_line(current_addr);
        }
        current_addr += 2;
    }
    
    LOG_DEBUG("ICache", "Prefetched " + std::to_string(count) + " instructions from 0x" + 
             std::to_string(start_address));
    
    return true;
}

bool InstructionCache::read(WORD address, BYTE* data, WORD size) {
    // 指令缓存通常只读，优化读取性能
    return Cache::read(address, data, size);
}

void InstructionCache::optimize_for_sequential_access() {
    // 优化顺序访问模式
    LOG_DEBUG("ICache", "Optimizing for sequential access");
}

void InstructionCache::predict_next_instruction(WORD current_address) {
    // 预测下一条指令并预取
    WORD next_addr = current_address + 2; // 假设指令长度为2字节
    prefetch_instructions(next_addr, 4); // 预取4条指令
}

// 数据缓存实现

bool DataCache::read_word(WORD address, WORD& value) {
    BYTE data[2];
    if (read(address, data, 2)) {
        value = data[0] | (data[1] << 8);
        return true;
    }
    return false;
}

bool DataCache::write_word(WORD address, WORD value) {
    BYTE data[2];
    data[0] = value & 0xFF;
    data[1] = (value >> 8) & 0xFF;
    return write(address, data, 2);
}

bool DataCache::read_byte(WORD address, BYTE& value) {
    return read(address, &value, 1);
}

bool DataCache::write_byte(WORD address, BYTE value) {
    return write(address, &value, 1);
}

bool DataCache::read(WORD address, BYTE* data, WORD size) {
    // 数据缓存优化读取
    return Cache::read(address, data, size);
}

bool DataCache::write(WORD address, const BYTE* data, WORD size) {
    // 数据缓存优化写入
    return Cache::write(address, data, size);
}

void DataCache::optimize_for_spatial_locality() {
    // 优化空间局部性
    LOG_DEBUG("DCache", "Optimizing for spatial locality");
}

void DataCache::handle_write_through(WORD address, const BYTE* data, WORD size) {
    // 处理写通策略
    write(address, data, size);
    // 立即写回内存
    writeback_line(find_line(address));
}

void DataCache::handle_write_back(WORD address, const BYTE* data, WORD size) {
    // 处理写回策略
    write(address, data, size);
    // 标记为脏，延迟写回
}
