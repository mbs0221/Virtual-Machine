#ifndef __REGISTER_ALLOCATOR_H_
#define __REGISTER_ALLOCATOR_H_

#include <map>
#include <set>
#include <vector>
#include <string>
#include "cfg.h"

using namespace std;

// 寄存器分配器类
class RegisterAllocator {
private:
    // 寄存器管理
    static const int MAX_REGISTERS = 16;  // 最大寄存器数
    static const int RESERVED_REGISTERS = 2;  // 保留寄存器数（0,1）
    bool usedRegisters[MAX_REGISTERS];
    int nextRegister;
    
    // 变量到寄存器的映射
    map<string, int> varToRegister;        // 变量名 -> 物理寄存器
    map<int, string> registerToVar;        // 物理寄存器 -> 变量名
    map<string, int> virtualToPhysical;    // 虚拟寄存器 -> 物理寄存器映射
    map<int, string> physicalToVirtual;    // 物理寄存器 -> 虚拟寄存器映射
    
    // 活跃变量分析结果
    map<BasicBlock*, set<string>> liveIn;
    map<BasicBlock*, set<string>> liveOut;
    map<TACInstruction*, set<string>> liveBefore;
    map<TACInstruction*, set<string>> liveAfter;
    
    // 冲突图（用于图着色算法）
    map<string, set<string>> conflictGraph;
    
    // 溢出变量
    set<string> spilledVars;
    
public:
    RegisterAllocator();
    ~RegisterAllocator();
    
    // 主要接口
    bool allocateRegisters(ControlFlowGraph* cfg);
    
    // 虚拟寄存器压缩
    bool compressVirtualRegisters(ControlFlowGraph* cfg);
    
    // 活跃变量分析
    void performLivenessAnalysis(ControlFlowGraph* cfg);
    void computeLiveInOut(BasicBlock* block);
    void computeLiveBeforeAfter(TACInstruction* inst);
    
    // 冲突图构建
    void buildConflictGraph(ControlFlowGraph* cfg);
    
    // 寄存器分配算法
    bool allocateWithGraphColoring();
    bool allocateWithLinearScan();
    
    // 寄存器管理
    int allocateRegister(const string& var);
    void freeRegister(int reg);
    void freeRegister(const string& var);
    bool isRegisterAvailable(int reg);
    int getRegisterForVar(const string& var);
    
    // 溢出处理
    void spillVariable(const string& var);
    bool isSpilled(const string& var);
    
    // 查询接口
    int getMaxUsedRegisters() const;
    set<string> getSpilledVariables() const;
    
    // 调试和输出
    void printAllocationInfo() const;
    void printLivenessInfo() const;
    void printConflictGraph() const;
    
    // 重置
    void reset();
};

// 活跃变量分析器
class LivenessAnalyzer {
private:
    ControlFlowGraph* cfg;
    map<BasicBlock*, set<string>> liveIn;
    map<BasicBlock*, set<string>> liveOut;
    map<TACInstruction*, set<string>> liveBefore;
    map<TACInstruction*, set<string>> liveAfter;
    
public:
    LivenessAnalyzer(ControlFlowGraph* cfg);
    
    // 分析接口
    void analyze();
    void computeBlockLiveness(BasicBlock* block);
    void computeInstructionLiveness(TACInstruction* inst);
    
    // 查询接口
    set<string> getLiveIn(BasicBlock* block) const;
    set<string> getLiveOut(BasicBlock* block) const;
    set<string> getLiveBefore(TACInstruction* inst) const;
    set<string> getLiveAfter(TACInstruction* inst) const;
    
    // 辅助函数
    set<string> getDefinedVars(TACInstruction* inst) const;
    set<string> getUsedVars(TACInstruction* inst) const;
    set<string> getDefinedVars(BasicBlock* block) const;
    set<string> getUsedVars(BasicBlock* block) const;
};

// 图着色寄存器分配器
class GraphColoringAllocator {
private:
    map<string, set<string>> conflictGraph;
    map<string, int> varToRegister;
    set<string> spilledVars;
    int maxRegisters;
    
public:
    GraphColoringAllocator(int maxRegs = 16);
    
    // 分配接口
    bool allocate(map<string, set<string>>& conflicts);
    
    // 图着色算法
    bool colorGraph();
    bool tryColorVariable(const string& var, int color);
    bool isColorValid(const string& var, int color);
    
    // 查询接口
    int getRegisterForVar(const string& var) const;
    set<string> getSpilledVariables() const;
    
    // 调试
    void printColoring() const;
};

// 线性扫描寄存器分配器
class LinearScanAllocator {
private:
    struct Interval {
        string var;
        int start;
        int end;
        int reg;
        bool spilled;
        
        Interval(const string& v, int s, int e) 
            : var(v), start(s), end(e), reg(-1), spilled(false) {}
    };
    
    vector<Interval> intervals;
    map<string, int> varToRegister;
    set<string> spilledVars;
    int maxRegisters;
    
public:
    LinearScanAllocator(int maxRegs = 16);
    
    // 分配接口
    bool allocate(vector<Interval>& varIntervals);
    
    // 线性扫描算法
    void scanIntervals();
    void allocateInterval(Interval& interval);
    void spillInterval(Interval& interval);
    
    // 查询接口
    int getRegisterForVar(const string& var) const;
    set<string> getSpilledVariables() const;
    
    // 调试
    void printAllocation() const;
};

#endif
