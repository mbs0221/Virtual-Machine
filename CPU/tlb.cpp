#include "tlb.h"
#include "Common/Logger.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

TLB::TLB() : next_index(0), hit_count(0), miss_count(0) {
    // 初始化所有TLB条目
    for (int i = 0; i < TLB_SIZE; i++) {
        entries[i] = TLBEntry();
    }
    
    LOG_DEBUG("TLB", "TLB initialized with " + std::to_string(TLB_SIZE) + " entries");
}

TLBEntry* TLB::lookup(WORD virtual_addr) {
    WORD vpn = virtual_addr & ~PAGE_MASK;  // 虚拟页号
    
    // 遍历TLB查找匹配的条目
    for (int i = 0; i < TLB_SIZE; i++) {
        if (entries[i].matches_virtual_page(virtual_addr)) {
            // 找到匹配的条目，更新访问计数
            entries[i].increment_access();
            hit_count++;
            
            LOG_DEBUG("TLB", "TLB hit: vaddr=0x" + std::to_string(virtual_addr) + 
                     ", paddr=0x" + std::to_string(entries[i].physical_addr) + 
                     ", index=" + std::to_string(i));
            
            return &entries[i];
        }
    }
    
    // 未找到匹配的条目
    miss_count++;
    
    LOG_DEBUG("TLB", "TLB miss: vaddr=0x" + std::to_string(virtual_addr));
    
    return nullptr;
}

void TLB::insert(WORD virtual_addr, WORD physical_addr, BYTE flags) {
    WORD vpn = virtual_addr & ~PAGE_MASK;  // 虚拟页号
    
    // 首先检查是否已存在相同的虚拟页
    for (int i = 0; i < TLB_SIZE; i++) {
        if (entries[i].matches_virtual_page(virtual_addr)) {
            // 更新现有条目
            entries[i].physical_addr = physical_addr;
            entries[i].flags = flags;
            entries[i].valid = true;
            entries[i].increment_access();
            
            LOG_DEBUG("TLB", "TLB entry updated: vaddr=0x" + std::to_string(virtual_addr) + 
                     ", paddr=0x" + std::to_string(physical_addr) + 
                     ", index=" + std::to_string(i));
            
            return;
        }
    }
    
    // 查找空条目
    WORD empty_index = find_empty_entry();
    if (empty_index != TLB_SIZE) {
        // 找到空条目，直接插入
        entries[empty_index] = TLBEntry(vpn, physical_addr, flags);
        entries[empty_index].increment_access();
        
        LOG_DEBUG("TLB", "TLB entry inserted: vaddr=0x" + std::to_string(virtual_addr) + 
                 ", paddr=0x" + std::to_string(physical_addr) + 
                 ", index=" + std::to_string(empty_index));
        
        return;
    }
    
    // 没有空条目，使用LRU替换策略
    WORD lru_index = find_lru_entry();
    entries[lru_index] = TLBEntry(vpn, physical_addr, flags);
    entries[lru_index].increment_access();
    
    LOG_DEBUG("TLB", "TLB entry replaced (LRU): vaddr=0x" + std::to_string(virtual_addr) + 
             ", paddr=0x" + std::to_string(physical_addr) + 
             ", index=" + std::to_string(lru_index));
}

void TLB::invalidate_all() {
    for (int i = 0; i < TLB_SIZE; i++) {
        entries[i].valid = false;
        entries[i].access_count = 0;
    }
    
    LOG_DEBUG("TLB", "All TLB entries invalidated");
}

void TLB::invalidate_entry(WORD virtual_addr) {
    WORD vpn = virtual_addr & ~PAGE_MASK;
    
    for (int i = 0; i < TLB_SIZE; i++) {
        if (entries[i].valid && (entries[i].virtual_addr & ~PAGE_MASK) == vpn) {
            entries[i].valid = false;
            entries[i].access_count = 0;
            
            LOG_DEBUG("TLB", "TLB entry invalidated: vaddr=0x" + std::to_string(virtual_addr) + 
                     ", index=" + std::to_string(i));
            break;
        }
    }
}

void TLB::invalidate_global_entries() {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (entries[i].valid && (entries[i].flags & PTE_GLOBAL)) {
            entries[i].valid = false;
            entries[i].access_count = 0;
        }
    }
    
    LOG_DEBUG("TLB", "Global TLB entries invalidated");
}

double TLB::get_hit_rate() const {
    WORD total_accesses = hit_count + miss_count;
    if (total_accesses == 0) {
        return 0.0;
    }
    return static_cast<double>(hit_count) / total_accesses * 100.0;
}

void TLB::print_tlb_info() {
    std::cout << "=== TLB Information ===" << std::endl;
    std::cout << "Size: " << TLB_SIZE << " entries" << std::endl;
    std::cout << "Hit Count: " << hit_count << std::endl;
    std::cout << "Miss Count: " << miss_count << std::endl;
    std::cout << "Hit Rate: " << std::fixed << std::setprecision(2) << get_hit_rate() << "%" << std::endl;
    std::cout << "Next Index: " << next_index << std::endl;
    std::cout << "======================" << std::endl;
}

void TLB::print_tlb_entries() {
    std::cout << "=== TLB Entries ===" << std::endl;
    std::cout << "Index | Valid | Virtual Addr | Physical Addr | Flags | Access Count" << std::endl;
    std::cout << "------|-------|--------------|---------------|-------|-------------" << std::endl;
    
    for (int i = 0; i < TLB_SIZE; i++) {
        std::cout << std::setw(5) << i << " | ";
        std::cout << std::setw(5) << (entries[i].valid ? "Yes" : "No") << " | ";
        std::cout << "0x" << std::setw(10) << std::hex << entries[i].virtual_addr << std::dec << " | ";
        std::cout << "0x" << std::setw(11) << std::hex << entries[i].physical_addr << std::dec << " | ";
        std::cout << "0x" << std::setw(4) << std::hex << (int)entries[i].flags << std::dec << " | ";
        std::cout << std::setw(11) << entries[i].access_count << std::endl;
    }
    
    std::cout << "===================" << std::endl;
}

void TLB::validate_tlb_integrity() {
    LOG_DEBUG("TLB", "Validating TLB integrity...");
    
    bool integrity_ok = true;
    
    // 检查是否有重复的虚拟地址
    for (int i = 0; i < TLB_SIZE; i++) {
        if (!entries[i].valid) continue;
        
        for (int j = i + 1; j < TLB_SIZE; j++) {
            if (entries[j].valid && 
                (entries[i].virtual_addr & ~PAGE_MASK) == (entries[j].virtual_addr & ~PAGE_MASK)) {
                LOG_ERROR("TLB", "Duplicate virtual address found: index " + std::to_string(i) + 
                         " and " + std::to_string(j));
                integrity_ok = false;
            }
        }
    }
    
    if (integrity_ok) {
        LOG_DEBUG("TLB", "TLB integrity check passed");
    } else {
        LOG_ERROR("TLB", "TLB integrity check failed");
    }
}

void TLB::optimize_tlb() {
    LOG_DEBUG("TLB", "Optimizing TLB...");
    
    // 按访问计数排序，将最常用的条目放在前面
    std::sort(entries, entries + TLB_SIZE, [](const TLBEntry& a, const TLBEntry& b) {
        if (!a.valid && !b.valid) return false;
        if (!a.valid) return false;
        if (!b.valid) return true;
        return a.access_count > b.access_count;
    });
    
    LOG_DEBUG("TLB", "TLB optimization completed");
}

void TLB::clear_statistics() {
    hit_count = 0;
    miss_count = 0;
    
    LOG_DEBUG("TLB", "TLB statistics cleared");
}

// 私有辅助方法实现
WORD TLB::find_lru_entry() const {
    WORD lru_index = 0;
    WORD min_access_count = entries[0].access_count;
    
    for (int i = 1; i < TLB_SIZE; i++) {
        if (entries[i].access_count < min_access_count) {
            min_access_count = entries[i].access_count;
            lru_index = i;
        }
    }
    
    return lru_index;
}

WORD TLB::find_empty_entry() const {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (!entries[i].valid) {
            return i;
        }
    }
    return TLB_SIZE;  // 没有空条目
}

void TLB::update_access_count(WORD index) {
    if (index < TLB_SIZE) {
        entries[index].increment_access();
    }
}

bool TLB::is_entry_valid(WORD index) const {
    return index < TLB_SIZE && entries[index].valid;
}
