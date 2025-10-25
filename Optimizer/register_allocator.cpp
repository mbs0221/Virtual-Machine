#include "register_allocator.h"
#include <cstdio>
#include <algorithm>
#include <queue>

// RegisterAllocator 实现
RegisterAllocator::RegisterAllocator() : nextRegister(RESERVED_REGISTERS) {
    reset();
}

RegisterAllocator::~RegisterAllocator() {
    // 清理资源
}

void RegisterAllocator::reset() {
    nextRegister = RESERVED_REGISTERS;
    for (int i = 0; i < MAX_REGISTERS; i++) {
        usedRegisters[i] = false;
    }
    varToRegister.clear();
    registerToVar.clear();
    liveIn.clear();
    liveOut.clear();
    liveBefore.clear();
    liveAfter.clear();
    conflictGraph.clear();
    spilledVars.clear();
}

bool RegisterAllocator::allocateRegisters(ControlFlowGraph* cfg) {
    if (!cfg) {
        printf("错误: CFG为空\n");
        return false;
    }
    
    printf("开始寄存器分配...\n");
    
    // 步骤1: 活跃变量分析
    printf("步骤1: 进行活跃变量分析\n");
    performLivenessAnalysis(cfg);
    
    // 步骤2: 构建冲突图
    printf("步骤2: 构建冲突图\n");
    buildConflictGraph(cfg);
    
    // 步骤3: 寄存器分配
    printf("步骤3: 执行寄存器分配\n");
    bool success = allocateWithGraphColoring();
    
    if (success) {
        printf("寄存器分配成功\n");
        printAllocationInfo();
    } else {
        printf("寄存器分配失败，使用线性扫描算法\n");
        success = allocateWithLinearScan();
        if (success) {
            printf("线性扫描寄存器分配成功\n");
            printAllocationInfo();
        }
    }
    
    return success;
}

bool RegisterAllocator::compressVirtualRegisters(ControlFlowGraph* cfg) {
    if (!cfg) {
        printf("错误: CFG为空，无法进行虚拟寄存器压缩\n");
        return false;
    }
    
    printf("开始虚拟寄存器压缩...\n");
    
    // 收集所有使用的虚拟寄存器
    set<string> virtualRegs;
    const vector<BasicBlock*>& blocks = cfg->getBlocks();
    
    for (size_t i = 0; i < blocks.size(); i++) {
        BasicBlock* block = blocks[i];
        const vector<TACInstruction>& instructions = block->getInstructions();
        
        for (size_t j = 0; j < instructions.size(); j++) {
            const TACInstruction& inst = instructions[j];
            
            // 收集所有虚拟寄存器
            if (inst.result.type == TAC_REG) {
                virtualRegs.insert(inst.result.name);
            }
            if (inst.arg1.type == TAC_REG) {
                virtualRegs.insert(inst.arg1.name);
            }
            if (inst.arg2.type == TAC_REG) {
                virtualRegs.insert(inst.arg2.name);
            }
        }
    }
    
    printf("发现 %zu 个虚拟寄存器需要压缩\n", virtualRegs.size());
    
    // 如果虚拟寄存器数量不超过物理寄存器限制，直接映射
    if (virtualRegs.size() <= MAX_REGISTERS - RESERVED_REGISTERS) {
        printf("虚拟寄存器数量在物理限制内，直接映射\n");
        
        int physicalReg = RESERVED_REGISTERS;
        for (const string& virtualReg : virtualRegs) {
            virtualToPhysical[virtualReg] = physicalReg;
            physicalToVirtual[physicalReg] = virtualReg;
            varToRegister[virtualReg] = physicalReg;
            registerToVar[physicalReg] = virtualReg;
            usedRegisters[physicalReg] = true;
            physicalReg++;
        }
        
        printf("虚拟寄存器压缩完成，使用 %d 个物理寄存器\n", physicalReg - RESERVED_REGISTERS);
        return true;
    }
    
    // 虚拟寄存器数量超过物理限制，需要优化分配
    printf("虚拟寄存器数量超过物理限制，执行优化分配\n");
    
    // 先进行活跃变量分析
    performLivenessAnalysis(cfg);
    
    // 构建冲突图
    buildConflictGraph(cfg);
    
    // 执行寄存器分配
    bool success = allocateWithGraphColoring();
    
    if (!success) {
        printf("图着色失败，尝试线性扫描\n");
        success = allocateWithLinearScan();
    }
    
    if (success) {
        printf("虚拟寄存器压缩成功完成\n");
        printAllocationInfo();
    } else {
        printf("虚拟寄存器压缩失败\n");
    }
    
    return success;
}

void RegisterAllocator::performLivenessAnalysis(ControlFlowGraph* cfg) {
    LivenessAnalyzer analyzer(cfg);
    analyzer.analyze();
    
    // 复制分析结果
    const vector<BasicBlock*>& blocks = cfg->getBlocks();
    for (size_t i = 0; i < blocks.size(); i++) {
        BasicBlock* block = blocks[i];
        liveIn[block] = analyzer.getLiveIn(block);
        liveOut[block] = analyzer.getLiveOut(block);
        
        const vector<TACInstruction>& instructions = block->getInstructions();
        for (size_t j = 0; j < instructions.size(); j++) {
            TACInstruction* inst = const_cast<TACInstruction*>(&instructions[j]);
            liveBefore[inst] = analyzer.getLiveBefore(inst);
            liveAfter[inst] = analyzer.getLiveAfter(inst);
        }
    }
}

void RegisterAllocator::buildConflictGraph(ControlFlowGraph* cfg) {
    conflictGraph.clear();
    
    const vector<BasicBlock*>& blocks = cfg->getBlocks();
    for (size_t i = 0; i < blocks.size(); i++) {
        BasicBlock* block = blocks[i];
        const vector<TACInstruction>& instructions = block->getInstructions();
        
        for (size_t j = 0; j < instructions.size(); j++) {
            const TACInstruction& inst = instructions[j];
            TACInstruction* instPtr = const_cast<TACInstruction*>(&inst);
            
            // 获取当前指令后的活跃变量
            set<string> liveVars = liveAfter[instPtr];
            
            // 如果当前指令定义了变量，则与所有活跃变量冲突
            set<string> definedVars;
            if (inst.result.type == TAC_TEMP || inst.result.type == TAC_REG) {
                definedVars.insert(inst.result.name);
            }
            
            // 构建冲突边
            for (const string& defVar : definedVars) {
                for (const string& liveVar : liveVars) {
                    if (defVar != liveVar) {
                        conflictGraph[defVar].insert(liveVar);
                        conflictGraph[liveVar].insert(defVar);
                    }
                }
            }
        }
    }
}

bool RegisterAllocator::allocateWithGraphColoring() {
    GraphColoringAllocator allocator(MAX_REGISTERS - RESERVED_REGISTERS);
    return allocator.allocate(conflictGraph);
}

bool RegisterAllocator::allocateWithLinearScan() {
    // 简化的线性扫描实现
    // 这里需要先构建变量的活跃区间
    map<string, pair<int, int>> varIntervals; // var -> (start, end)
    
    // 为每个变量计算活跃区间
    for (auto& pair : liveBefore) {
        TACInstruction* inst = pair.first;
        set<string> liveVars = pair.second;
        
        for (const string& var : liveVars) {
            if (varIntervals.find(var) == varIntervals.end()) {
                varIntervals[var] = make_pair(0, 0);
            }
            // 简化：使用指令在基本块中的位置
            int pos = 0; // 简化实现，实际应该计算指令位置
            varIntervals[var].second = max(varIntervals[var].second, pos);
        }
    }
    
    // 按开始位置排序
    vector<pair<string, pair<int, int>>> sortedVars(varIntervals.begin(), varIntervals.end());
    sort(sortedVars.begin(), sortedVars.end(), 
         [](const pair<string, pair<int, int>>& a, const pair<string, pair<int, int>>& b) {
             return a.second.first < b.second.first;
         });
    
    // 线性扫描分配
    for (const auto& varPair : sortedVars) {
        const string& var = varPair.first;
        if (nextRegister < MAX_REGISTERS) {
            varToRegister[var] = nextRegister;
            registerToVar[nextRegister] = var;
            usedRegisters[nextRegister] = true;
            nextRegister++;
        } else {
            // 寄存器溢出
            spillVariable(var);
        }
    }
    
    return true;
}

int RegisterAllocator::allocateRegister(const string& var) {
    if (varToRegister.find(var) != varToRegister.end()) {
        return varToRegister[var];
    }
    
    if (nextRegister < MAX_REGISTERS) {
        int reg = nextRegister++;
        varToRegister[var] = reg;
        registerToVar[reg] = var;
        usedRegisters[reg] = true;
        return reg;
    }
    
    return -1; // 寄存器溢出
}

void RegisterAllocator::freeRegister(int reg) {
    if (reg >= 0 && reg < MAX_REGISTERS && usedRegisters[reg]) {
        usedRegisters[reg] = false;
        string var = registerToVar[reg];
        varToRegister.erase(var);
        registerToVar.erase(reg);
    }
}

void RegisterAllocator::freeRegister(const string& var) {
    if (varToRegister.find(var) != varToRegister.end()) {
        int reg = varToRegister[var];
        freeRegister(reg);
    }
}

bool RegisterAllocator::isRegisterAvailable(int reg) {
    return reg >= 0 && reg < MAX_REGISTERS && !usedRegisters[reg];
}

int RegisterAllocator::getRegisterForVar(const string& var) {
    auto it = varToRegister.find(var);
    return (it != varToRegister.end()) ? it->second : -1;
}

void RegisterAllocator::spillVariable(const string& var) {
    spilledVars.insert(var);
    printf("变量 %s 溢出到内存\n", var.c_str());
}

bool RegisterAllocator::isSpilled(const string& var) {
    return spilledVars.find(var) != spilledVars.end();
}

int RegisterAllocator::getMaxUsedRegisters() const {
    return nextRegister;
}

set<string> RegisterAllocator::getSpilledVariables() const {
    return spilledVars;
}

void RegisterAllocator::printAllocationInfo() const {
    printf("=== 寄存器分配信息 ===\n");
    printf("最大使用寄存器数: %d\n", nextRegister);
    printf("溢出变量数: %zu\n", spilledVars.size());
    
    printf("变量到寄存器映射:\n");
    for (const auto& pair : varToRegister) {
        printf("  %s -> $%d\n", pair.first.c_str(), pair.second);
    }
    
    if (!spilledVars.empty()) {
        printf("溢出变量:\n");
        for (const string& var : spilledVars) {
            printf("  %s\n", var.c_str());
        }
    }
    printf("====================\n");
}

void RegisterAllocator::printLivenessInfo() const {
    printf("=== 活跃变量信息 ===\n");
    for (const auto& pair : liveIn) {
        BasicBlock* block = pair.first;
        set<string> liveVars = pair.second;
        printf("基本块 %d LiveIn: ", block->getId());
        for (const string& var : liveVars) {
            printf("%s ", var.c_str());
        }
        printf("\n");
    }
    printf("==================\n");
}

void RegisterAllocator::printConflictGraph() const {
    printf("=== 冲突图 ===\n");
    for (const auto& pair : conflictGraph) {
        const string& var = pair.first;
        const set<string>& conflicts = pair.second;
        printf("%s 与: ", var.c_str());
        for (const string& conflict : conflicts) {
            printf("%s ", conflict.c_str());
        }
        printf("\n");
    }
    printf("==============\n");
}

// LivenessAnalyzer 实现
LivenessAnalyzer::LivenessAnalyzer(ControlFlowGraph* cfg) : cfg(cfg) {
}

void LivenessAnalyzer::analyze() {
    // 初始化
    const vector<BasicBlock*>& blocks = cfg->getBlocks();
    for (size_t i = 0; i < blocks.size(); i++) {
        BasicBlock* block = blocks[i];
        liveIn[block] = set<string>();
        liveOut[block] = set<string>();
    }
    
    // 迭代计算直到收敛
    bool changed = true;
    int iteration = 0;
    
    while (changed && iteration < 100) { // 防止无限循环
        changed = false;
        iteration++;
        
        // 逆序遍历基本块
        for (int i = blocks.size() - 1; i >= 0; i--) {
            BasicBlock* block = blocks[i];
            set<string> oldLiveIn = liveIn[block];
            set<string> oldLiveOut = liveOut[block];
            
            computeBlockLiveness(block);
            
            // 检查是否发生变化
            if (oldLiveIn != liveIn[block] || oldLiveOut != liveOut[block]) {
                changed = true;
            }
        }
    }
    
    printf("活跃变量分析完成，迭代次数: %d\n", iteration);
}

void LivenessAnalyzer::computeBlockLiveness(BasicBlock* block) {
    // LiveOut[block] = Union of LiveIn[successor]
    set<string> newLiveOut;
    const vector<BasicBlock*>& successors = block->getSuccessors();
    for (size_t i = 0; i < successors.size(); i++) {
        BasicBlock* succ = successors[i];
        set<string> succLiveIn = liveIn[succ];
        newLiveOut.insert(succLiveIn.begin(), succLiveIn.end());
    }
    liveOut[block] = newLiveOut;
    
    // LiveIn[block] = Use[block] Union (LiveOut[block] - Def[block])
    set<string> useVars = getUsedVars(block);
    set<string> defVars = getDefinedVars(block);
    
    set<string> newLiveIn = useVars;
    for (const string& var : liveOut[block]) {
        if (defVars.find(var) == defVars.end()) {
            newLiveIn.insert(var);
        }
    }
    liveIn[block] = newLiveIn;
}

set<string> LivenessAnalyzer::getDefinedVars(TACInstruction* inst) const {
    set<string> defVars;
    if (inst->result.type == TAC_TEMP || inst->result.type == TAC_REG) {
        defVars.insert(inst->result.name);
    }
    return defVars;
}

set<string> LivenessAnalyzer::getUsedVars(TACInstruction* inst) const {
    set<string> useVars;
    if (inst->arg1.type == TAC_TEMP || inst->arg1.type == TAC_REG) {
        useVars.insert(inst->arg1.name);
    }
    if (inst->arg2.type == TAC_TEMP || inst->arg2.type == TAC_REG) {
        useVars.insert(inst->arg2.name);
    }
    return useVars;
}

set<string> LivenessAnalyzer::getDefinedVars(BasicBlock* block) const {
    set<string> defVars;
    const vector<TACInstruction>& instructions = block->getInstructions();
    for (size_t i = 0; i < instructions.size(); i++) {
        TACInstruction* inst = const_cast<TACInstruction*>(&instructions[i]);
        set<string> instDefs = getDefinedVars(inst);
        defVars.insert(instDefs.begin(), instDefs.end());
    }
    return defVars;
}

set<string> LivenessAnalyzer::getUsedVars(BasicBlock* block) const {
    set<string> useVars;
    const vector<TACInstruction>& instructions = block->getInstructions();
    for (size_t i = 0; i < instructions.size(); i++) {
        TACInstruction* inst = const_cast<TACInstruction*>(&instructions[i]);
        set<string> instUses = getUsedVars(inst);
        useVars.insert(instUses.begin(), instUses.end());
    }
    return useVars;
}

set<string> LivenessAnalyzer::getLiveIn(BasicBlock* block) const {
    auto it = liveIn.find(block);
    return (it != liveIn.end()) ? it->second : set<string>();
}

set<string> LivenessAnalyzer::getLiveOut(BasicBlock* block) const {
    auto it = liveOut.find(block);
    return (it != liveOut.end()) ? it->second : set<string>();
}

set<string> LivenessAnalyzer::getLiveBefore(TACInstruction* inst) const {
    auto it = liveBefore.find(inst);
    return (it != liveBefore.end()) ? it->second : set<string>();
}

set<string> LivenessAnalyzer::getLiveAfter(TACInstruction* inst) const {
    auto it = liveAfter.find(inst);
    return (it != liveAfter.end()) ? it->second : set<string>();
}

// GraphColoringAllocator 实现
GraphColoringAllocator::GraphColoringAllocator(int maxRegs) : maxRegisters(maxRegs) {
}

bool GraphColoringAllocator::allocate(map<string, set<string>>& conflicts) {
    conflictGraph = conflicts;
    return colorGraph();
}

bool GraphColoringAllocator::colorGraph() {
    // 简化的图着色算法
    // 按度数排序变量（度数高的先着色）
    vector<pair<string, int>> sortedVars;
    for (const auto& pair : conflictGraph) {
        sortedVars.push_back(make_pair(pair.first, pair.second.size()));
    }
    sort(sortedVars.begin(), sortedVars.end(), 
         [](const pair<string, int>& a, const pair<string, int>& b) {
             return a.second > b.second;
         });
    
    // 为每个变量分配颜色
    for (const auto& varPair : sortedVars) {
        const string& var = varPair.first;
        bool colored = false;
        
        // 尝试分配颜色
        for (int color = 0; color < maxRegisters; color++) {
            if (tryColorVariable(var, color)) {
                varToRegister[var] = color;
                colored = true;
                break;
            }
        }
        
        if (!colored) {
            // 无法着色，溢出
            spilledVars.insert(var);
        }
    }
    
    return spilledVars.empty() || spilledVars.size() < conflictGraph.size() / 2;
}

bool GraphColoringAllocator::tryColorVariable(const string& var, int color) {
    // 检查是否与已着色的邻居冲突
    auto it = conflictGraph.find(var);
    if (it == conflictGraph.end()) {
        return true; // 没有冲突
    }
    
    const set<string>& neighbors = it->second;
    for (const string& neighbor : neighbors) {
        auto regIt = varToRegister.find(neighbor);
        if (regIt != varToRegister.end() && regIt->second == color) {
            return false; // 冲突
        }
    }
    
    return true;
}

int GraphColoringAllocator::getRegisterForVar(const string& var) const {
    auto it = varToRegister.find(var);
    return (it != varToRegister.end()) ? it->second : -1;
}

set<string> GraphColoringAllocator::getSpilledVariables() const {
    return spilledVars;
}

void GraphColoringAllocator::printColoring() const {
    printf("=== 图着色结果 ===\n");
    for (const auto& pair : varToRegister) {
        printf("%s -> 寄存器 %d\n", pair.first.c_str(), pair.second);
    }
    if (!spilledVars.empty()) {
        printf("溢出变量: ");
        for (const string& var : spilledVars) {
            printf("%s ", var.c_str());
        }
        printf("\n");
    }
    printf("================\n");
}

// LinearScanAllocator 实现
LinearScanAllocator::LinearScanAllocator(int maxRegs) : maxRegisters(maxRegs) {
}

bool LinearScanAllocator::allocate(vector<Interval>& varIntervals) {
    intervals = varIntervals;
    scanIntervals();
    return true;
}

void LinearScanAllocator::scanIntervals() {
    // 按开始位置排序
    sort(intervals.begin(), intervals.end(), 
         [](const Interval& a, const Interval& b) {
             return a.start < b.start;
         });
    
    // 线性扫描
    for (Interval& interval : intervals) {
        allocateInterval(interval);
    }
}

void LinearScanAllocator::allocateInterval(Interval& interval) {
    // 简化的线性扫描实现
    // 寻找可用的寄存器
    for (int reg = 0; reg < maxRegisters; reg++) {
        bool available = true;
        
        // 检查是否与已分配的区间冲突
        for (const Interval& other : intervals) {
            if (other.reg == reg && 
                !(interval.end <= other.start || interval.start >= other.end)) {
                available = false;
                break;
            }
        }
        
        if (available) {
            interval.reg = reg;
            varToRegister[interval.var] = reg;
            return;
        }
    }
    
    // 没有可用寄存器，溢出
    spillInterval(interval);
}

void LinearScanAllocator::spillInterval(Interval& interval) {
    interval.spilled = true;
    spilledVars.insert(interval.var);
}

int LinearScanAllocator::getRegisterForVar(const string& var) const {
    auto it = varToRegister.find(var);
    return (it != varToRegister.end()) ? it->second : -1;
}

set<string> LinearScanAllocator::getSpilledVariables() const {
    return spilledVars;
}

void LinearScanAllocator::printAllocation() const {
    printf("=== 线性扫描分配结果 ===\n");
    for (const auto& pair : varToRegister) {
        printf("%s -> 寄存器 %d\n", pair.first.c_str(), pair.second);
    }
    if (!spilledVars.empty()) {
        printf("溢出变量: ");
        for (const string& var : spilledVars) {
            printf("%s ", var.c_str());
        }
        printf("\n");
    }
    printf("======================\n");
}
