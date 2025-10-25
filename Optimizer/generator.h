#ifndef __GENERATOR_H_
#define __GENERATOR_H_

#include <string>
#include <vector>
#include "cfg.h"

using namespace std;

// 代码生成器类
class CodeGenerator {
private:
	ControlFlowGraph* cfg;
	string outputFile;
	
public:
	CodeGenerator() : cfg(nullptr) {}
	~CodeGenerator() {}
	
	// 从CFG生成目标代码
	bool generateFromCFG(ControlFlowGraph* controlFlowGraph, const string& output);
	
	// 生成汇编代码
	bool generateAssembly();
	
	// 生成二进制代码
	bool generateBinary();
	
	// 设置输出文件
	void setOutputFile(const string& file) { outputFile = file; }
	
	// 获取输出文件
	string getOutputFile() const { return outputFile; }
	
	// 打印生成信息
	void printGenerationInfo() const;
};

#endif
