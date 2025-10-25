#include "cfg.h"
#include "register_allocator.h"
#include <cstdio>
#include <algorithm>

// BasicBlock 实现
void BasicBlock::addInstruction(const TACInstruction& inst) {
	instructions.push_back(inst);
}

void BasicBlock::addPredecessor(BasicBlock* pred) {
	// 避免重复添加
	if (find(predecessors.begin(), predecessors.end(), pred) == predecessors.end()) {
		predecessors.push_back(pred);
	}
}

void BasicBlock::addSuccessor(BasicBlock* succ) {
	// 避免重复添加
	if (find(successors.begin(), successors.end(), succ) == successors.end()) {
		successors.push_back(succ);
	}
}

void BasicBlock::print() const {
	printf("=== 基本块 %d ===\n", id);
	if (isEntry) printf("(入口块)\n");
	if (isExit) printf("(出口块)\n");
	
	printf("前驱块: ");
	for (size_t i = 0; i < predecessors.size(); i++) {
		printf("%d", predecessors[i]->getId());
		if (i < predecessors.size() - 1) printf(", ");
	}
	printf("\n");
	
	printf("后继块: ");
	for (size_t i = 0; i < successors.size(); i++) {
		printf("%d", successors[i]->getId());
		if (i < successors.size() - 1) printf(", ");
	}
	printf("\n");
	
	printf("指令:\n");
	for (size_t i = 0; i < instructions.size(); i++) {
		printf("  [%04zu] %s\n", i, instructions[i].toString().c_str());
	}
	printf("================\n");
}

// ControlFlowGraph 实现
ControlFlowGraph::~ControlFlowGraph() {
	for (size_t i = 0; i < blocks.size(); i++) {
		delete blocks[i];
	}
}

BasicBlock* ControlFlowGraph::createBlock() {
	BasicBlock* block = new BasicBlock(nextBlockId++);
	addBlock(block);
	return block;
}

BasicBlock* ControlFlowGraph::getBlock(int id) const {
	auto it = blockMap.find(id);
	return (it != blockMap.end()) ? it->second : nullptr;
}

void ControlFlowGraph::addBlock(BasicBlock* block) {
	blocks.push_back(block);
	blockMap[block->getId()] = block;
}

void ControlFlowGraph::setEntryBlock(BasicBlock* block) {
	entryBlock = block;
	if (block) {
		block->setEntry(true);
	}
}

void ControlFlowGraph::addExitBlock(BasicBlock* block) {
	if (find(exitBlocks.begin(), exitBlocks.end(), block) == exitBlocks.end()) {
		exitBlocks.push_back(block);
		block->setExit(true);
	}
}

void ControlFlowGraph::print() const {
	printf("=== 控制流图 ===\n");
	printf("总基本块数: %zu\n", blocks.size());
	printf("入口块: %s\n", entryBlock ? to_string(entryBlock->getId()).c_str() : "无");
	
	printf("出口块: ");
	for (size_t i = 0; i < exitBlocks.size(); i++) {
		printf("%d", exitBlocks[i]->getId());
		if (i < exitBlocks.size() - 1) printf(", ");
	}
	printf("\n\n");
	
	for (size_t i = 0; i < blocks.size(); i++) {
		blocks[i]->print();
	}
	printf("================\n");
}

void ControlFlowGraph::printDot() const {
	printf("digraph CFG {\n");
	printf("  rankdir=TB;\n");
	printf("  node [shape=box, style=filled];\n\n");
	
	// 定义节点
	for (size_t i = 0; i < blocks.size(); i++) {
		BasicBlock* block = blocks[i];
		printf("  block_%d [label=\"基本块 %d", block->getId(), block->getId());
		
		if (block->getIsEntry()) printf(" (入口)");
		if (block->getIsExit()) printf(" (出口)");
		printf("\"];\n");
	}
	
	printf("\n");
	
	// 定义边
	for (size_t i = 0; i < blocks.size(); i++) {
		BasicBlock* block = blocks[i];
		const vector<BasicBlock*>& successors = block->getSuccessors();
		
		for (size_t j = 0; j < successors.size(); j++) {
			printf("  block_%d -> block_%d;\n", block->getId(), successors[j]->getId());
		}
	}
	
	printf("}\n");
}

// CFGBuilder 实现
CFGBuilder::CFGBuilder() : cfg(nullptr), registerAllocator(nullptr) {}

CFGBuilder::~CFGBuilder() {
	if (cfg) {
		delete cfg;
	}
	if (registerAllocator) {
		delete registerAllocator;
	}
}

ControlFlowGraph* CFGBuilder::buildFromTAC(const TACProgram& tac) {
	if (cfg) {
		delete cfg;
	}
	cfg = new ControlFlowGraph();
	
	const vector<TACInstruction>& instructions = tac.getInstructions();
	if (instructions.empty()) {
		return cfg;
	}
	
	// 创建第一个基本块作为入口块
	BasicBlock* currentBlock = cfg->createBlock();
	cfg->setEntryBlock(currentBlock);
	
	map<string, BasicBlock*> labelMap;  // 标签到基本块的映射
	vector<pair<BasicBlock*, string>> pendingJumps;  // 待处理的跳转
	
	// 第一遍：识别标签和跳转指令，创建基本块
	for (size_t i = 0; i < instructions.size(); i++) {
		const TACInstruction& inst = instructions[i];
		
		switch (inst.op) {
			case TAC_LABEL: {
				// 遇到标签，创建新基本块
				if (currentBlock->size() > 0) {
					currentBlock = cfg->createBlock();
				}
				labelMap[inst.label] = currentBlock;
				break;
			}
			case TAC_JUMP:
			case TAC_JUMP_COND:
			case TAC_CALL:
			case TAC_RETURN:
			case TAC_HALT: {
				// 控制流指令，添加到当前块
				currentBlock->addInstruction(inst);
				
				// 记录跳转信息
				if (inst.op == TAC_JUMP || inst.op == TAC_JUMP_COND) {
					pendingJumps.push_back(make_pair(currentBlock, inst.label));
				}
				
				// 创建新基本块（除非是halt或return）
				if (inst.op != TAC_HALT && inst.op != TAC_RETURN) {
					currentBlock = cfg->createBlock();
				}
				break;
			}
			default: {
				// 普通指令，添加到当前块
				currentBlock->addInstruction(inst);
				break;
			}
		}
	}
	
	// 第二遍：建立基本块之间的连接
	for (size_t i = 0; i < pendingJumps.size(); i++) {
		BasicBlock* fromBlock = pendingJumps[i].first;
		string targetLabel = pendingJumps[i].second;
		
		auto it = labelMap.find(targetLabel);
		if (it != labelMap.end()) {
			BasicBlock* toBlock = it->second;
			fromBlock->addSuccessor(toBlock);
			toBlock->addPredecessor(fromBlock);
		}
	}
	
	// 第三遍：建立顺序执行的基本块连接
	for (size_t i = 0; i < cfg->getBlocks().size() - 1; i++) {
		BasicBlock* current = cfg->getBlocks()[i];
		BasicBlock* next = cfg->getBlocks()[i + 1];
		
		// 如果当前块没有后继且不是控制流指令结尾，则连接到下一个块
		if (current->getSuccessors().empty() && current->size() > 0) {
			const TACInstruction& lastInst = current->getInstructions().back();
			if (lastInst.op != TAC_JUMP && lastInst.op != TAC_JUMP_COND && 
			    lastInst.op != TAC_CALL && lastInst.op != TAC_RETURN && 
			    lastInst.op != TAC_HALT) {
				current->addSuccessor(next);
				next->addPredecessor(current);
			}
		}
	}
	
	// 识别出口块（没有后继的基本块）
	for (size_t i = 0; i < cfg->getBlocks().size(); i++) {
		BasicBlock* block = cfg->getBlocks()[i];
		if (block->getSuccessors().empty() && !block->getInstructions().empty()) {
			cfg->addExitBlock(block);
		}
	}
	
	return cfg;
}

void CFGBuilder::printCFG() const {
	if (cfg) {
		cfg->print();
	}
}

bool CFGBuilder::performRegisterAllocation() {
	if (!cfg) {
		printf("错误: CFG为空，无法进行寄存器分配\n");
		return false;
	}
	
	// 创建寄存器分配器
	if (registerAllocator) {
		delete registerAllocator;
	}
	registerAllocator = new RegisterAllocator();
	
	// 执行寄存器分配
	bool success = registerAllocator->allocateRegisters(cfg);
	
	if (success) {
		printf("寄存器分配成功完成\n");
		registerAllocator->printAllocationInfo();
	} else {
		printf("寄存器分配失败\n");
	}
	
	return success;
}

bool CFGBuilder::performVirtualRegisterCompression() {
	if (!cfg) {
		printf("错误: CFG为空，无法进行虚拟寄存器压缩\n");
		return false;
	}
	
	// 创建寄存器分配器
	if (registerAllocator) {
		delete registerAllocator;
	}
	registerAllocator = new RegisterAllocator();
	
	// 执行虚拟寄存器压缩
	bool success = registerAllocator->compressVirtualRegisters(cfg);
	
	if (success) {
		printf("虚拟寄存器压缩成功完成\n");
		registerAllocator->printAllocationInfo();
	} else {
		printf("虚拟寄存器压缩失败\n");
	}
	
	return success;
}
