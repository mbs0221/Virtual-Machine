#ifndef __MMU_H_
#define __MMU_H_

#include <cstdint>
#include <string>

// Toy架构基本类型定义
typedef uint16_t WORD;
typedef uint8_t BYTE;

// 虚拟内存和MMU定义
#define PAGE_SIZE 4096            // 4KB页面大小
#define PAGE_SHIFT 12             // 页面大小位移
#define PAGE_MASK 0xFFF           // 页面掩码

// 三级页表定义
#define PGD_SHIFT 10              // 页全局目录位移
#define PUD_SHIFT 8               // 页上级目录位移
#define PMD_SHIFT 6               // 页中间目录位移
#define PTE_SHIFT 4               // 页表项位移

#define PGD_SIZE (1 << PGD_SHIFT) // 页全局目录大小
#define PUD_SIZE (1 << PUD_SHIFT) // 页上级目录大小
#define PMD_SIZE (1 << PMD_SHIFT) // 页中间目录大小
#define PTE_SIZE (1 << PTE_SHIFT) // 页表项大小

// 页表项标志位
#define PTE_PRESENT   0x01        // 页面存在
#define PTE_WRITE     0x02        // 可写
#define PTE_USER      0x04        // 用户可访问
#define PTE_EXEC      0x08        // 可执行
#define PTE_DIRTY     0x10        // 脏页
#define PTE_ACCESSED  0x20        // 已访问
#define PTE_GLOBAL    0x40        // 全局页
#define PTE_NX        0x80        // 不可执行

// 特权级别定义
#define PRIVILEGE_KERNEL 0        // 内核特权级别
#define PRIVILEGE_USER   1        // 用户特权级别

// 异常类型定义
enum ExceptionType {
    EXCEPTION_NONE = 0,
    EXCEPTION_DIVISION_BY_ZERO,   // 除零异常
    EXCEPTION_INVALID_OPCODE,     // 无效操作码
    EXCEPTION_MEMORY_ACCESS,      // 内存访问异常
    EXCEPTION_PRIVILEGE_VIOLATION, // 特权违规
    EXCEPTION_STACK_OVERFLOW,     // 栈溢出
    EXCEPTION_STACK_UNDERFLOW,    // 栈下溢
    EXCEPTION_SYSCALL,            // 系统调用
    EXCEPTION_PAGE_FAULT,         // 页面错误
    EXCEPTION_TLB_MISS,           // TLB缺失
    EXCEPTION_MAX
};

// 页表项结构
struct PageTableEntry {
    WORD physical_addr;           // 物理地址
    BYTE flags;                   // 页表项标志
    BYTE reserved;                // 保留字段
    
    PageTableEntry() : physical_addr(0), flags(0), reserved(0) {}
    PageTableEntry(WORD addr, BYTE f) : physical_addr(addr), flags(f), reserved(0) {}
    
    bool is_present() const { return flags & PTE_PRESENT; }
    bool is_writable() const { return flags & PTE_WRITE; }
    bool is_user() const { return flags & PTE_USER; }
    bool is_executable() const { return flags & PTE_EXEC; }
    bool is_dirty() const { return flags & PTE_DIRTY; }
    bool is_accessed() const { return flags & PTE_ACCESSED; }
    
    void set_dirty() { flags |= PTE_DIRTY; }
    void set_accessed() { flags |= PTE_ACCESSED; }
};

// 前向声明
class Memory;
class TLB;

/**
 * MMU (Memory Management Unit) 类
 * 负责虚拟地址到物理地址的转换
 * 实现三级页表遍历和内存保护
 */
class MMU {
private:
    Memory* memory;               // 内存接口
    TLB* tlb;                    // TLB缓存
    bool enabled;                 // MMU使能标志
    WORD pgd_base;               // 页全局目录基地址
    BYTE current_privilege_level; // 当前特权级别
    WORD page_fault_addr;        // 页面错误地址
    BYTE page_fault_flags;       // 页面错误标志
    
public:
    MMU(Memory* mem, TLB* tlb_cache);
    ~MMU() = default;
    
    // MMU控制
    void enable() { enabled = true; }
    void disable() { enabled = false; }
    bool is_enabled() const { return enabled; }
    
    // 页表管理
    void set_pgd_base(WORD base) { pgd_base = base; }
    WORD get_pgd_base() const { return pgd_base; }
    
    // 地址转换
    WORD translate_address(WORD virtual_addr, bool is_write = false, bool is_execute = false);
    
    // 页表遍历
    WORD walk_page_table(WORD virtual_addr);
    PageTableEntry* get_page_table_entry(WORD virtual_addr);
    
    // 权限检查
    bool check_page_permissions(const PageTableEntry& pte, bool is_write, bool is_execute);
    
    // 页面错误处理
    void handle_page_fault(WORD virtual_addr, BYTE fault_flags);
    WORD get_page_fault_addr() const { return page_fault_addr; }
    BYTE get_page_fault_flags() const { return page_fault_flags; }
    
    // 特权级别管理
    void set_privilege_level(BYTE level) { current_privilege_level = level; }
    BYTE get_privilege_level() const { return current_privilege_level; }
    
    // 调试和诊断
    void print_mmu_info();
    void validate_page_table();
    
private:
    // 内部辅助方法
    WORD extract_pgd_index(WORD virtual_addr) const;
    WORD extract_pud_index(WORD virtual_addr) const;
    WORD extract_pmd_index(WORD virtual_addr) const;
    WORD extract_pte_index(WORD virtual_addr) const;
};

#endif // __MMU_H_
