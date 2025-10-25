#include "mmu.h"
#include "tlb.h"
#include "memory.h"
#include "Common/Logger.h"
#include <iostream>
#include <iomanip>

MMU::MMU(Memory* mem, TLB* tlb_cache) 
    : memory(mem), tlb(tlb_cache), enabled(false), pgd_base(0), 
      current_privilege_level(PRIVILEGE_KERNEL), page_fault_addr(0), page_fault_flags(0) {
    LOG_DEBUG("MMU", "MMU initialized");
}

WORD MMU::translate_address(WORD virtual_addr, bool is_write, bool is_execute) {
    if (!enabled) {
        return virtual_addr;  // MMU禁用时直接返回虚拟地址
    }
    
    // 首先检查TLB
    TLBEntry* tlb_entry = tlb->lookup(virtual_addr);
    if (tlb_entry && tlb_entry->valid) {
        // TLB命中，检查权限
        if (check_page_permissions(PageTableEntry(tlb_entry->physical_addr, tlb_entry->flags), is_write, is_execute)) {
            // 更新访问和脏页标志
            if (is_write) {
                tlb_entry->set_dirty();
            }
            tlb_entry->set_accessed();
            tlb_entry->increment_access();
            return tlb_entry->physical_addr + (virtual_addr & PAGE_MASK);
        } else {
            // 权限检查失败
            handle_page_fault(virtual_addr, is_write ? PTE_WRITE : 0);
            return 0;
        }
    }
    
    // TLB未命中，遍历页表
    WORD physical_addr = walk_page_table(virtual_addr);
    if (physical_addr == 0) {
        // 页表遍历失败
        handle_page_fault(virtual_addr, is_write ? PTE_WRITE : 0);
        return 0;
    }
    
    // 获取页表项
    PageTableEntry* pte = get_page_table_entry(virtual_addr);
    if (!pte || !check_page_permissions(*pte, is_write, is_execute)) {
        handle_page_fault(virtual_addr, is_write ? PTE_WRITE : 0);
        return 0;
    }
    
    // 更新TLB
    tlb->insert(virtual_addr & ~PAGE_MASK, physical_addr, pte->flags);
    
    // 更新页表项标志
    if (is_write) {
        pte->set_dirty();
    }
    pte->set_accessed();
    
    return physical_addr + (virtual_addr & PAGE_MASK);
}

WORD MMU::walk_page_table(WORD virtual_addr) {
    // 三级页表遍历
    WORD pgd_index = extract_pgd_index(virtual_addr);
    WORD pud_index = extract_pud_index(virtual_addr);
    WORD pmd_index = extract_pmd_index(virtual_addr);
    WORD pte_index = extract_pte_index(virtual_addr);
    
    // 页全局目录
    WORD pgd_addr = pgd_base + pgd_index * sizeof(PageTableEntry);
    PageTableEntry pgd_entry = memory->read_page_table_entry(pgd_addr);
    if (!pgd_entry.is_present()) {
        LOG_DEBUG("MMU", "PGD entry not present: index=" + std::to_string(pgd_index));
        return 0;  // 页全局目录项不存在
    }
    
    // 页上级目录
    WORD pud_addr = pgd_entry.physical_addr + pud_index * sizeof(PageTableEntry);
    PageTableEntry pud_entry = memory->read_page_table_entry(pud_addr);
    if (!pud_entry.is_present()) {
        LOG_DEBUG("MMU", "PUD entry not present: index=" + std::to_string(pud_index));
        return 0;  // 页上级目录项不存在
    }
    
    // 页中间目录
    WORD pmd_addr = pud_entry.physical_addr + pmd_index * sizeof(PageTableEntry);
    PageTableEntry pmd_entry = memory->read_page_table_entry(pmd_addr);
    if (!pmd_entry.is_present()) {
        LOG_DEBUG("MMU", "PMD entry not present: index=" + std::to_string(pmd_index));
        return 0;  // 页中间目录项不存在
    }
    
    // 页表项
    WORD pte_addr = pmd_entry.physical_addr + pte_index * sizeof(PageTableEntry);
    PageTableEntry pte_entry = memory->read_page_table_entry(pte_addr);
    if (!pte_entry.is_present()) {
        LOG_DEBUG("MMU", "PTE entry not present: index=" + std::to_string(pte_index));
        return 0;  // 页表项不存在
    }
    
    LOG_DEBUG("MMU", "Page table walk successful: vaddr=0x" + std::to_string(virtual_addr) + 
             ", paddr=0x" + std::to_string(pte_entry.physical_addr));
    
    return pte_entry.physical_addr;
}

PageTableEntry* MMU::get_page_table_entry(WORD virtual_addr) {
    WORD pgd_index = extract_pgd_index(virtual_addr);
    WORD pud_index = extract_pud_index(virtual_addr);
    WORD pmd_index = extract_pmd_index(virtual_addr);
    WORD pte_index = extract_pte_index(virtual_addr);
    
    // 遍历页表获取页表项
    WORD pgd_addr = pgd_base + pgd_index * sizeof(PageTableEntry);
    PageTableEntry pgd_entry = memory->read_page_table_entry(pgd_addr);
    if (!pgd_entry.is_present()) return nullptr;
    
    WORD pud_addr = pgd_entry.physical_addr + pud_index * sizeof(PageTableEntry);
    PageTableEntry pud_entry = memory->read_page_table_entry(pud_addr);
    if (!pud_entry.is_present()) return nullptr;
    
    WORD pmd_addr = pud_entry.physical_addr + pmd_index * sizeof(PageTableEntry);
    PageTableEntry pmd_entry = memory->read_page_table_entry(pmd_addr);
    if (!pmd_entry.is_present()) return nullptr;
    
    WORD pte_addr = pmd_entry.physical_addr + pte_index * sizeof(PageTableEntry);
    return memory->get_page_table_entry_ptr(pte_addr);
}

bool MMU::check_page_permissions(const PageTableEntry& pte, bool is_write, bool is_execute) {
    // 检查页面是否存在
    if (!pte.is_present()) {
        LOG_DEBUG("MMU", "Page not present");
        return false;
    }
    
    // 检查写权限
    if (is_write && !pte.is_writable()) {
        LOG_DEBUG("MMU", "Write permission denied");
        return false;
    }
    
    // 检查执行权限
    if (is_execute && !pte.is_executable()) {
        LOG_DEBUG("MMU", "Execute permission denied");
        return false;
    }
    
    // 检查用户访问权限
    if (current_privilege_level == PRIVILEGE_USER && !pte.is_user()) {
        LOG_DEBUG("MMU", "User access denied");
        return false;
    }
    
    return true;
}

void MMU::handle_page_fault(WORD virtual_addr, BYTE fault_flags) {
    page_fault_addr = virtual_addr;
    page_fault_flags = fault_flags;
    
    LOG_ERROR("MMU", "Page fault: virtual_addr=0x" + std::to_string(virtual_addr) + 
              ", flags=0x" + std::to_string(fault_flags) + 
              ", privilege=" + std::to_string(current_privilege_level));
}

void MMU::print_mmu_info() {
    std::cout << "=== MMU Information ===" << std::endl;
    std::cout << "Enabled: " << (enabled ? "Yes" : "No") << std::endl;
    std::cout << "PGD Base: 0x" << std::hex << pgd_base << std::dec << std::endl;
    std::cout << "Current Privilege Level: " << (int)current_privilege_level << std::endl;
    std::cout << "Page Fault Address: 0x" << std::hex << page_fault_addr << std::dec << std::endl;
    std::cout << "Page Fault Flags: 0x" << std::hex << (int)page_fault_flags << std::dec << std::endl;
    std::cout << "=======================" << std::endl;
}

void MMU::validate_page_table() {
    LOG_DEBUG("MMU", "Validating page table structure...");
    
    // 这里可以添加页表完整性检查
    // 例如：检查页表项的一致性、循环引用等
    
    LOG_DEBUG("MMU", "Page table validation completed");
}

// 私有辅助方法实现
WORD MMU::extract_pgd_index(WORD virtual_addr) const {
    return (virtual_addr >> PGD_SHIFT) & 0x3F;  // 6位
}

WORD MMU::extract_pud_index(WORD virtual_addr) const {
    return (virtual_addr >> PUD_SHIFT) & 0x3;   // 2位
}

WORD MMU::extract_pmd_index(WORD virtual_addr) const {
    return (virtual_addr >> PMD_SHIFT) & 0x3;   // 2位
}

WORD MMU::extract_pte_index(WORD virtual_addr) const {
    return (virtual_addr >> PTE_SHIFT) & 0x3;   // 2位
}
