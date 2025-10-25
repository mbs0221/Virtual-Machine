#ifndef __NES_OPTIMIZER_H_
#define __NES_OPTIMIZER_H_

#include <cstdint>
#include <unordered_map>
#include <vector>

// 前向声明
class ToyCPU;

// 基本类型定义
typedef uint16_t WORD;
typedef uint8_t BYTE;

/**
 * NES游戏优化器 - 简化版本
 * 专注于sequential CPU的NES游戏优化
 * 核心功能：帧率控制、内存优化、性能监控
 */
class NESOptimizer {
private:
    ToyCPU* cpu;
    
    // 性能统计
    struct PerformanceStats {
        uint32_t total_cycles;
        uint32_t instruction_count;
        uint32_t cache_hits;
        uint32_t cache_misses;
        uint32_t memory_accesses;
        uint32_t branch_predictions;
        uint32_t branch_mispredictions;
    } stats;
    
    // 指令缓存
    std::unordered_map<WORD, BYTE> instruction_cache;
    
    // 分支预测器
    std::unordered_map<WORD, bool> branch_predictions;
    
public:
    NESOptimizer(ToyCPU* cpu);
    ~NESOptimizer() = default;
    
    // 初始化NES优化
    void initialize_nes_mode();
    
    // 性能监控
    void reset_performance_stats();
    PerformanceStats get_performance_stats() const;
    void print_performance_report() const;
    
    // NES特定优化
    void optimize_for_nes_game();
    void setup_nes_memory_layout();
    void configure_nes_interrupts();
    
    // 帧率控制
    void sync_to_frame_rate();
    bool is_frame_complete() const;
    void wait_for_vblank();
    
    // 缓存管理
    void update_instruction_cache(WORD address, BYTE instruction);
    BYTE get_cached_instruction(WORD address);
    void clear_cache();
    
    // 分支预测
    bool predict_branch(WORD address, BYTE opcode);
    void update_branch_prediction(WORD address, bool taken, bool correct);
    
    // 性能统计更新
    void increment_instruction_count();
    void increment_memory_access();
    void increment_cache_hit();
    void increment_cache_miss();
    void increment_branch_prediction();
    void increment_branch_misprediction();
    
private:
    // 内部优化方法
    bool is_nes_memory_region(WORD address) const;
    bool is_hot_instruction(BYTE opcode) const;
    void optimize_memory_access_pattern();
    void optimize_instruction_sequence();
};

#endif // __NES_OPTIMIZER_H_
