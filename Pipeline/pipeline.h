#ifndef __PIPELINE_H_
#define __PIPELINE_H_

#include <string>
#include "../Asm/asm.h"
#include "../Optimizer/tac.h"
#include "../Optimizer/cfg.h"

// 前向声明，避免包含冲突
class Parser;
struct Stmt;

using namespace std;

// 前向声明
class CodeGenerator;
class VM;

// 包含Optimizer头文件
#include "../Optimizer/optimizer.h"

// 流水线阶段枚举
enum PipelineStage {
	STAGE_PARSER,     // 高级语言解析阶段
	STAGE_OPTIMIZER,  // 代码优化阶段
	STAGE_ASM         // 汇编阶段
};

// 流水线管理器
class Pipeline {
private:
	// 各个阶段的处理器
	Parser* parser;                  // 高级语言解析器
	Optimizer* optimizer;            // 代码优化器
	Asm* asmProcessor;               // 汇编器
	
	// 中间结果（内存中的数据结构）
	Stmt* parserAST;                 // Parser阶段输出：高级语言AST
	string optimizedAsmCode;         // Optimizer阶段输出：优化后汇编代码（字符串）
	string binaryFile;               // Asm阶段输出：二进制文件
	
	// 状态管理
	bool stagesExecuted[3];          // 记录各阶段是否已执行
	string inputFile;
	string outputFile;
	
public:
	Pipeline(const string& input, const string& output);
	~Pipeline();
	
	// 流水线执行
	bool execute();
	bool executeStage(PipelineStage stage);
	
	// 各阶段执行方法
	bool executeParserStage();
	bool executeOptimizerStage();
	bool executeASMStage();
	
	// 获取中间结果
	Stmt* getParserAST() const { return parserAST; }
	string getOptimizedAsmCode() const { return optimizedAsmCode; }
	string getBinaryFile() const { return binaryFile; }
	
	// 状态查询
	bool isStageExecuted(PipelineStage stage) const;
	void printPipelineStatus() const;
	
	// 调试和输出
	void printParserAST() const;
	void printUnoptimizedAsm() const;
	void printOptimizedAsm() const;
	void printBinaryInfo() const;
	
private:
	// 辅助方法
	string getBaseName(const string& filePath) const;
};

#endif
