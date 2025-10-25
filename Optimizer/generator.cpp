#include "generator.h"
#include <cstdio>
#include <fstream>

// 从CFG生成目标代码
bool CodeGenerator::generateFromCFG(ControlFlowGraph* controlFlowGraph, const string& output) {
	cfg = controlFlowGraph;
	outputFile = output;
	
	printf("开始从CFG生成目标代码...\n");
	
	// 这里可以实现不同的代码生成策略
	// 目前先实现简单的汇编代码生成
	return generateAssembly();
}

// 生成汇编代码
bool CodeGenerator::generateAssembly() {
	if (!cfg) {
		printf("错误: CFG为空\n");
		return false;
	}
	
	// 创建输出文件
	string asmFile = outputFile + ".asm";
	FILE* fp = fopen(asmFile.c_str(), "w");
	if (!fp) {
		printf("错误: 无法创建输出文件 %s\n", asmFile.c_str());
		return false;
	}
	
	printf("生成汇编代码到: %s\n", asmFile.c_str());
	
	// 写入文件头
	fprintf(fp, "; 由CFG生成的汇编代码\n");
	fprintf(fp, "; 基本块数: %zu\n", cfg->size());
	fprintf(fp, "\n");
	
	// 遍历所有基本块
	const vector<BasicBlock*>& blocks = cfg->getBlocks();
	for (size_t i = 0; i < blocks.size(); i++) {
		BasicBlock* block = blocks[i];
		
		// 写入基本块信息
		fprintf(fp, "; === 基本块 %d ===\n", block->getId());
		if (block->getIsEntry()) {
			fprintf(fp, "; (入口块)\n");
		}
		if (block->getIsExit()) {
			fprintf(fp, "; (出口块)\n");
		}
		
		// 写入前驱和后继信息
		const vector<BasicBlock*>& predecessors = block->getPredecessors();
		const vector<BasicBlock*>& successors = block->getSuccessors();
		
		if (!predecessors.empty()) {
			fprintf(fp, "; 前驱块: ");
			for (size_t j = 0; j < predecessors.size(); j++) {
				fprintf(fp, "%d", predecessors[j]->getId());
				if (j < predecessors.size() - 1) fprintf(fp, ", ");
			}
			fprintf(fp, "\n");
		}
		
		if (!successors.empty()) {
			fprintf(fp, "; 后继块: ");
			for (size_t j = 0; j < successors.size(); j++) {
				fprintf(fp, "%d", successors[j]->getId());
				if (j < successors.size() - 1) fprintf(fp, ", ");
			}
			fprintf(fp, "\n");
		}
		
		// 写入指令
		const vector<TACInstruction>& instructions = block->getInstructions();
		for (size_t j = 0; j < instructions.size(); j++) {
			fprintf(fp, "%s\n", instructions[j].toString().c_str());
		}
		
		fprintf(fp, "\n");
	}
	
	fclose(fp);
	return true;
}

// 生成二进制代码
bool CodeGenerator::generateBinary() {
	if (!cfg) {
		printf("错误: CFG为空\n");
		return false;
	}
	
	printf("生成二进制代码到: %s\n", outputFile.c_str());
	
	// 这里可以实现从CFG直接生成二进制代码的逻辑
	// 目前先创建一个简单的二进制文件
	FILE* fp = fopen(outputFile.c_str(), "wb");
	if (!fp) {
		printf("错误: 无法创建输出文件 %s\n", outputFile.c_str());
		return false;
	}
	
	// 写入简单的文件头
	WORD magic = 0x1234;  // 魔数
	WORD version = 0x0001; // 版本号
	WORD blockCount = (WORD)cfg->size();
	
	fwrite(&magic, sizeof(WORD), 1, fp);
	fwrite(&version, sizeof(WORD), 1, fp);
	fwrite(&blockCount, sizeof(WORD), 1, fp);
	
	// 写入基本块信息
	const vector<BasicBlock*>& blocks = cfg->getBlocks();
	for (size_t i = 0; i < blocks.size(); i++) {
		BasicBlock* block = blocks[i];
		
		// 写入基本块ID
		WORD blockId = (WORD)block->getId();
		fwrite(&blockId, sizeof(WORD), 1, fp);
		
		// 写入指令数量
		WORD instructionCount = (WORD)block->size();
		fwrite(&instructionCount, sizeof(WORD), 1, fp);
		
		// 写入标志位
		BYTE flags = 0;
		if (block->getIsEntry()) flags |= 0x01;
		if (block->getIsExit()) flags |= 0x02;
		fwrite(&flags, sizeof(BYTE), 1, fp);
	}
	
	fclose(fp);
	return true;
}

// 打印生成信息
void CodeGenerator::printGenerationInfo() const {
	if (!cfg) {
		printf("错误: CFG为空\n");
		return;
	}
	
	printf("=== 代码生成信息 ===\n");
	printf("输出文件: %s\n", outputFile.c_str());
	printf("基本块数: %zu\n", cfg->size());
	printf("入口块: %s\n", cfg->getEntryBlock() ? 
		to_string(cfg->getEntryBlock()->getId()).c_str() : "无");
	
	const vector<BasicBlock*>& exitBlocks = cfg->getExitBlocks();
	printf("出口块数: %zu\n", exitBlocks.size());
	
	// 统计指令总数
	size_t totalInstructions = 0;
	const vector<BasicBlock*>& blocks = cfg->getBlocks();
	for (size_t i = 0; i < blocks.size(); i++) {
		totalInstructions += blocks[i]->size();
	}
	printf("总指令数: %zu\n", totalInstructions);
	printf("==================\n");
}
