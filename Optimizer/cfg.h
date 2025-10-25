#ifndef __CFG_H_
#define __CFG_H_

#include <vector>
#include <map>
#include <string>
#include "tac.h"

// 前向声明
class RegisterAllocator;

using namespace std;

// 基本块类
class BasicBlock {
private:
	int id;                                    // 基本块ID
	vector<TACInstruction> instructions;       // 基本块中的指令
	vector<BasicBlock*> predecessors;          // 前驱基本块
	vector<BasicBlock*> successors;            // 后继基本块
	bool isEntry;                              // 是否为入口基本块
	bool isExit;                               // 是否为出口基本块
	
public:
	BasicBlock(int blockId) : id(blockId), isEntry(false), isExit(false) {}
	
	// 基本块管理
	void addInstruction(const TACInstruction& inst);
	void addPredecessor(BasicBlock* pred);
	void addSuccessor(BasicBlock* succ);
	
	// 属性设置
	void setEntry(bool entry) { isEntry = entry; }
	void setExit(bool exit) { isExit = exit; }
	
	// 属性获取
	int getId() const { return id; }
	const vector<TACInstruction>& getInstructions() const { return instructions; }
	const vector<BasicBlock*>& getPredecessors() const { return predecessors; }
	const vector<BasicBlock*>& getSuccessors() const { return successors; }
	bool getIsEntry() const { return isEntry; }
	bool getIsExit() const { return isExit; }
	size_t size() const { return instructions.size(); }
	
	// 打印基本块
	void print() const;
};

// 控制流图类
class ControlFlowGraph {
private:
	vector<BasicBlock*> blocks;                // 所有基本块
	map<int, BasicBlock*> blockMap;            // ID到基本块的映射
	BasicBlock* entryBlock;                    // 入口基本块
	vector<BasicBlock*> exitBlocks;            // 出口基本块
	int nextBlockId;                           // 下一个基本块ID
	
public:
	ControlFlowGraph() : entryBlock(nullptr), nextBlockId(0) {}
	~ControlFlowGraph();
	
	// 基本块管理
	BasicBlock* createBlock();
	BasicBlock* getBlock(int id) const;
	void addBlock(BasicBlock* block);
	void setEntryBlock(BasicBlock* block);
	void addExitBlock(BasicBlock* block);
	
	// 属性获取
	const vector<BasicBlock*>& getBlocks() const { return blocks; }
	BasicBlock* getEntryBlock() const { return entryBlock; }
	const vector<BasicBlock*>& getExitBlocks() const { return exitBlocks; }
	size_t size() const { return blocks.size(); }
	
	// 打印控制流图
	void print() const;
	void printDot() const;  // 输出DOT格式，用于可视化
};

// CFG构建器
class CFGBuilder {
private:
	ControlFlowGraph* cfg;
	RegisterAllocator* registerAllocator;
	
public:
	CFGBuilder();
	~CFGBuilder();
	
	// 从三地址码构建CFG
	ControlFlowGraph* buildFromTAC(const TACProgram& tac);
	
	// 寄存器分配
	bool performRegisterAllocation();
	
	// 虚拟寄存器压缩
	bool performVirtualRegisterCompression();
	
	// 获取CFG
	ControlFlowGraph* getCFG() const { return cfg; }
	
	// 获取寄存器分配器
	RegisterAllocator* getRegisterAllocator() const { return registerAllocator; }
	
	// 打印CFG
	void printCFG() const;
};

#endif
