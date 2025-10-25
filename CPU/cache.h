#ifndef __CACHE_H_
#define __CACHE_H_

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

// Toy架构基本类型定义
typedef uint16_t WORD;
typedef uint8_t BYTE;

// 缓存配置常量
#define CACHE_LINE_SIZE 16          // 缓存行大小（字节）
#define CACHE_LINE_SIZE_WORDS 8     // 缓存行大小（字）
#define CACHE_LINE_MASK 0xFFF0      // 缓存行对齐掩码
#define CACHE_LINE_OFFSET_MASK 0xF  // 缓存行内偏移掩码

// 缓存大小定义
#define ICACHE_SIZE 1024            // 指令缓存大小（字节）
#define DCACHE_SIZE 1024            // 数据缓存大小（字节）
#define ICACHE_LINES (ICACHE_SIZE / CACHE_LINE_SIZE)  // 指令缓存行数
#define DCACHE_LINES (DCACHE_SIZE / CACHE_LINE_SIZE)  // 数据缓存行数

// 缓存策略
enum CachePolicy {
    CACHE_WRITE_THROUGH = 0,        // 写通策略
    CACHE_WRITE_BACK = 1,           // 写回策略
    CACHE_WRITE_ALLOCATE = 2,       // 写分配策略
    CACHE_NO_WRITE_ALLOCATE = 3     // 写不分配策略
};

// 缓存替换策略
enum ReplacementPolicy {
    REPLACEMENT_LRU = 0,            // 最近最少使用
    REPLACEMENT_FIFO = 1,           // 先进先出
    REPLACEMENT_RANDOM = 2,         // 随机替换
    REPLACEMENT_LFU = 3             // 最少使用频率
};

// 缓存状态
enum CacheState {
    CACHE_INVALID = 0,              // 无效
    CACHE_VALID = 1,                // 有效
    CACHE_DIRTY = 2,                // 脏（已修改）
    CACHE_SHARED = 3                // 共享
};

// 缓存行结构
struct CacheLine {
    WORD tag;                       // 标签
    BYTE data[CACHE_LINE_SIZE];     // 数据
    CacheState state;               // 状态
    WORD access_count;              // 访问计数
    WORD last_access_time;          // 最后访问时间
    bool valid;                     // 有效标志
    
    CacheLine() : tag(0), state(CACHE_INVALID), access_count(0), 
                  last_access_time(0), valid(false) {
        memset(data, 0, sizeof(data));
    }
    
    // 状态检查方法
    bool is_valid() const { return valid && state != CACHE_INVALID; }
    bool is_dirty() const { return state == CACHE_DIRTY; }
    bool is_shared() const { return state == CACHE_SHARED; }
    
    // 状态设置方法
    void set_valid() { valid = true; state = CACHE_VALID; }
    void set_dirty() { state = CACHE_DIRTY; }
    void set_shared() { state = CACHE_SHARED; }
    void set_invalid() { valid = false; state = CACHE_INVALID; }
    
    // 访问统计
    void increment_access() { access_count++; }
    void update_access_time(WORD time) { last_access_time = time; }
};

// 缓存统计信息
struct CacheStats {
    WORD hits;                      // 命中次数
    WORD misses;                    // 未命中次数
    WORD write_hits;                // 写命中次数
    WORD write_misses;              // 写未命中次数
    WORD read_hits;                 // 读命中次数
    WORD read_misses;               // 读未命中次数
    WORD evictions;                 // 驱逐次数
    WORD writebacks;                // 写回次数
    
    CacheStats() : hits(0), misses(0), write_hits(0), write_misses(0),
                   read_hits(0), read_misses(0), evictions(0), writebacks(0) {}
    
    double get_hit_rate() const {
        WORD total = hits + misses;
        return total > 0 ? static_cast<double>(hits) / total * 100.0 : 0.0;
    }
    
    double get_write_hit_rate() const {
        WORD total = write_hits + write_misses;
        return total > 0 ? static_cast<double>(write_hits) / total * 100.0 : 0.0;
    }
    
    double get_read_hit_rate() const {
        WORD total = read_hits + read_misses;
        return total > 0 ? static_cast<double>(read_hits) / total * 100.0 : 0.0;
    }
};

// 前向声明
class Memory;

/**
 * 通用缓存基类
 * 提供缓存的基本功能和接口
 */
class Cache {
protected:
    std::vector<CacheLine> lines;   // 缓存行数组
    WORD num_lines;                 // 缓存行数
    WORD line_size;                 // 缓存行大小
    WORD access_time;               // 访问时间计数器
    CachePolicy write_policy;       // 写策略
    ReplacementPolicy replacement_policy; // 替换策略
    CacheStats stats;               // 统计信息
    
public:
    Cache(WORD lines, WORD size, CachePolicy wp = CACHE_WRITE_BACK, 
          ReplacementPolicy rp = REPLACEMENT_LRU);
    virtual ~Cache() = default;
    
    // 基本缓存操作
    virtual bool read(WORD address, BYTE* data, WORD size);
    virtual bool write(WORD address, const BYTE* data, WORD size);
    virtual void invalidate(WORD address);
    virtual void invalidate_all();
    
    // 缓存管理
    virtual void flush();
    virtual void flush_line(WORD address);
    
    // 统计和调试
    const CacheStats& get_stats() const { return stats; }
    void clear_stats() { stats = CacheStats(); }
    void print_stats() const;
    void print_cache_info() const;
    
    // 配置
    void set_write_policy(CachePolicy policy) { write_policy = policy; }
    void set_replacement_policy(ReplacementPolicy policy) { replacement_policy = policy; }
    
protected:
    // 内部辅助方法
    virtual WORD get_line_index(WORD address) const;
    virtual WORD get_tag(WORD address) const;
    virtual WORD get_offset(WORD address) const;
    virtual CacheLine* find_line(WORD address);
    virtual CacheLine* allocate_line(WORD address);
    virtual void evict_line(WORD line_index);
    virtual void writeback_line(CacheLine* line);
    
    // 替换策略实现
    virtual WORD find_victim_line(WORD set_index) const;
    virtual WORD find_lru_line(WORD set_index) const;
    virtual WORD find_fifo_line(WORD set_index) const;
    virtual WORD find_lfu_line(WORD set_index) const;
    virtual WORD find_random_line(WORD set_index) const;
};

/**
 * 指令缓存类
 * 专门用于缓存指令数据
 */
class InstructionCache : public Cache {
public:
    InstructionCache() : Cache(ICACHE_LINES, CACHE_LINE_SIZE, 
                              CACHE_WRITE_THROUGH, REPLACEMENT_LRU) {}
    virtual ~InstructionCache() = default;
    
    // 指令缓存特定操作
    bool fetch_instruction(WORD address, WORD& instruction);
    bool prefetch_instructions(WORD start_address, WORD count);
    
    // 重写基类方法以优化指令访问
    bool read(WORD address, BYTE* data, WORD size) override;
    
    // 指令缓存特定的优化
    void optimize_for_sequential_access();
    void predict_next_instruction(WORD current_address);
    
private:
};

/**
 * 数据缓存类
 * 专门用于缓存数据
 */
class DataCache : public Cache {
public:
    DataCache() : Cache(DCACHE_LINES, CACHE_LINE_SIZE, 
                       CACHE_WRITE_BACK, REPLACEMENT_LRU) {}
    virtual ~DataCache() = default;
    
    // 数据缓存特定操作
    bool read_word(WORD address, WORD& value);
    bool write_word(WORD address, WORD value);
    bool read_byte(WORD address, BYTE& value);
    bool write_byte(WORD address, BYTE value);
    
    // 重写基类方法以优化数据访问
    bool read(WORD address, BYTE* data, WORD size) override;
    bool write(WORD address, const BYTE* data, WORD size) override;
    
    // 数据缓存特定的优化
    void optimize_for_spatial_locality();
    void handle_write_through(WORD address, const BYTE* data, WORD size);
    void handle_write_back(WORD address, const BYTE* data, WORD size);
    
private:
};

#endif // __CACHE_H_
