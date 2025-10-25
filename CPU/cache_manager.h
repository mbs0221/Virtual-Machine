#ifndef __CACHE_MANAGER_H_
#define __CACHE_MANAGER_H_

#include "cache.h"
#include "memory.h"
#include <memory>

// Toy架构基本类型定义
typedef uint16_t WORD;
typedef uint8_t BYTE;

/**
 * 缓存管理器类
 * 统一管理指令缓存和数据缓存
 * 提供缓存一致性保证和性能优化
 */
class CacheManager {
private:
    InstructionCache* icache;  // 指令缓存
    DataCache* dcache;         // 数据缓存
    Memory* memory;                            // 内存系统接口
    bool enabled;                              // 缓存使能标志
    
    // 缓存一致性管理
    WORD coherence_protocol;                   // 一致性协议
    bool write_allocate_enabled;               // 写分配使能
    bool prefetch_enabled;                     // 预取使能
    
public:
    CacheManager(Memory* mem);
    ~CacheManager();
    
    // 缓存控制
    void enable() { enabled = true; }
    void disable() { enabled = false; }
    bool is_enabled() const { return enabled; }
    
    // 指令缓存操作
    bool fetch_instruction(WORD address, WORD& instruction);
    bool prefetch_instructions(WORD start_address, WORD count);
    void invalidate_instruction_cache();
    
    // 数据缓存操作
    bool read_word(WORD address, WORD& value);
    bool write_word(WORD address, WORD value);
    bool read_byte(WORD address, BYTE& value);
    bool write_byte(WORD address, BYTE value);
    
    // 缓存管理
    void flush_all();
    void flush_data_cache();
    void flush_instruction_cache();
    void invalidate_all();
    
    // 缓存一致性
    void maintain_coherence(WORD address);
    void handle_write_invalidate(WORD address);
    void handle_write_update(WORD address, WORD value);
    
    // 性能优化
    void optimize_for_workload();
    void enable_prefetch(bool enable) { prefetch_enabled = enable; }
    void enable_write_allocate(bool enable) { write_allocate_enabled = enable; }
    
    // 统计和调试
    void print_all_stats() const;
    void print_instruction_cache_stats() const;
    void print_data_cache_stats() const;
    void clear_all_stats();
    
    // 配置
    void set_coherence_protocol(WORD protocol) { coherence_protocol = protocol; }
    void configure_cache_sizes(WORD icache_size, WORD dcache_size);
    
    // 调试和诊断
    void validate_cache_integrity();
    void print_cache_configuration() const;
    
private:
    // 内部辅助方法
    bool load_from_memory(WORD address, BYTE* data, WORD size);
    bool store_to_memory(WORD address, const BYTE* data, WORD size);
    void handle_cache_miss(WORD address, bool is_instruction);
    void optimize_cache_performance();
};

#endif // __CACHE_MANAGER_H_
