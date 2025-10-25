#ifndef __TLB_H_
#define __TLB_H_

#include <cstdint>
#include <string>

// Toy架构基本类型定义
typedef uint16_t WORD;
typedef uint8_t BYTE;

// TLB配置
#define TLB_SIZE 16               // TLB条目数量
#define TLB_ENTRY_SIZE 4         // 每个TLB条目大小（字节）

// 页表项标志位（与MMU保持一致）
#define PTE_PRESENT   0x01        // 页面存在
#define PTE_WRITE     0x02        // 可写
#define PTE_USER      0x04        // 用户可访问
#define PTE_EXEC      0x08        // 可执行
#define PTE_DIRTY     0x10        // 脏页
#define PTE_ACCESSED  0x20        // 已访问
#define PTE_GLOBAL    0x40        // 全局页
#define PTE_NX        0x80        // 不可执行

// 页面大小定义
#define PAGE_SIZE 4096            // 4KB页面大小
#define PAGE_MASK 0xFFF           // 页面掩码

// TLB条目结构
struct TLBEntry {
    WORD virtual_addr;            // 虚拟地址（页对齐）
    WORD physical_addr;           // 物理地址（页对齐）
    BYTE flags;                   // 页表项标志
    bool valid;                   // 有效标志
    WORD access_count;            // 访问计数（用于LRU替换）
    
    TLBEntry() : virtual_addr(0), physical_addr(0), flags(0), valid(false), access_count(0) {}
    TLBEntry(WORD vaddr, WORD paddr, BYTE f) 
        : virtual_addr(vaddr), physical_addr(paddr), flags(f), valid(true), access_count(0) {}
    
    // 标志位检查方法
    bool is_present() const { return flags & PTE_PRESENT; }
    bool is_writable() const { return flags & PTE_WRITE; }
    bool is_user() const { return flags & PTE_USER; }
    bool is_executable() const { return flags & PTE_EXEC; }
    bool is_dirty() const { return flags & PTE_DIRTY; }
    bool is_accessed() const { return flags & PTE_ACCESSED; }
    
    // 标志位设置方法
    void set_dirty() { flags |= PTE_DIRTY; }
    void set_accessed() { flags |= PTE_ACCESSED; }
    void increment_access() { access_count++; }
    
    // 比较方法
    bool matches_virtual_page(WORD vaddr) const {
        return valid && (virtual_addr & ~PAGE_MASK) == (vaddr & ~PAGE_MASK);
    }
};

/**
 * TLB (Translation Lookaside Buffer) 类
 * 负责缓存虚拟地址到物理地址的转换
 * 提供快速地址转换和缓存管理
 */
class TLB {
private:
    TLBEntry entries[TLB_SIZE];   // TLB条目数组
    WORD next_index;              // 下一个要替换的索引
    WORD hit_count;               // 命中计数
    WORD miss_count;              // 未命中计数
    
public:
    TLB();
    ~TLB() = default;
    
    // TLB操作
    TLBEntry* lookup(WORD virtual_addr);
    void insert(WORD virtual_addr, WORD physical_addr, BYTE flags);
    void invalidate_all();
    void invalidate_entry(WORD virtual_addr);
    void invalidate_global_entries();
    
    // 统计信息
    WORD get_hit_count() const { return hit_count; }
    WORD get_miss_count() const { return miss_count; }
    double get_hit_rate() const;
    
    // 调试和诊断
    void print_tlb_info();
    void print_tlb_entries();
    void validate_tlb_integrity();
    
    // 性能优化
    void optimize_tlb();
    void clear_statistics();
    
private:
    // 内部辅助方法
    WORD find_lru_entry() const;
    WORD find_empty_entry() const;
    void update_access_count(WORD index);
    bool is_entry_valid(WORD index) const;
};

#endif // __TLB_H_
