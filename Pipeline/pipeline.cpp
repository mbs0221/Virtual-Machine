#include "pipeline.h"
#include <cstdio>
#include <fstream>
#include <iostream>

// 暂时注释掉Parser包含，避免冲突
// #include "../Parser/parser.h"

// 流水线构造函数
Pipeline::Pipeline(const string& input, const string& output) 
	: inputFile(input), outputFile(output), parserAST(nullptr) {
	
	// 初始化各阶段处理器
	parser = nullptr;  // 暂时设为nullptr，避免Parser冲突
	optimizer = new Optimizer();  // 创建真正的Optimizer实例
	asmProcessor = nullptr;  // 延迟初始化
	
	// 初始化中间结果
	optimizedAsmCode = "";
	binaryFile = output;
	
	// 初始化状态
	for (int i = 0; i < 3; i++) {
		stagesExecuted[i] = false;
	}
}

// 流水线析构函数
Pipeline::~Pipeline() {
	// 注意: parser, optimizer 暂时为 nullptr，不需要删除
	if (asmProcessor) delete asmProcessor;
}

// 获取文件名（不含扩展名）
string Pipeline::getBaseName(const string& filePath) const {
	size_t lastSlash = filePath.find_last_of("/\\");
	size_t lastDot = filePath.find_last_of(".");
	
	if (lastDot != string::npos && lastDot > lastSlash) {
		return filePath.substr(lastSlash + 1, lastDot - lastSlash - 1);
	} else {
		return filePath.substr(lastSlash + 1);
	}
}

// 执行完整流水线
bool Pipeline::execute() {
	printf("=== 开始执行编译流水线 ===\n");
	printf("输入文件: %s\n", inputFile.c_str());
	printf("输出文件: %s\n", outputFile.c_str());
	printf("优化后ASM代码: 内存中\n");
	printf("二进制文件: %s\n", binaryFile.c_str());
	printf("\n");
	
	// 按顺序执行各阶段
	if (!executeParserStage()) {
		printf("错误: Parser阶段执行失败\n");
		return false;
	}
	
	if (!executeOptimizerStage()) {
		printf("错误: Optimizer阶段执行失败\n");
		return false;
	}
	
	if (!executeASMStage()) {
		printf("错误: ASM阶段执行失败\n");
		return false;
	}
	
	printf("=== 编译流水线执行完成 ===\n");
	return true;
}

// 执行指定阶段
bool Pipeline::executeStage(PipelineStage stage) {
	switch (stage) {
		case STAGE_PARSER: return executeParserStage();
		case STAGE_OPTIMIZER: return executeOptimizerStage();
		case STAGE_ASM: return executeASMStage();
		default: return false;
	}
}

// 执行Parser阶段
bool Pipeline::executeParserStage() {
	printf("=== 执行Parser阶段：高级语言解析 ===\n");
	
	// 检查输入文件是否存在
	ifstream inputStream(inputFile);
	if (!inputStream.good()) {
		printf("错误: 输入文件不存在: %s\n", inputFile.c_str());
		return false;
	}
	inputStream.close();
	
	// TODO: 直接使用Parser库解析文件生成AST
	// 目前暂时使用外部程序调用，但应该改为直接调用Parser库
	printf("调用Parser解析文件生成AST...\n");
	
	// 暂时设置parserAST为nullptr，表示需要实现直接调用Parser库
	parserAST = nullptr;
	
	printf("注意: 需要实现直接调用Parser库来生成AST\n");
	
	stagesExecuted[STAGE_PARSER] = true;
	printf("Parser阶段完成\n\n");
	return true;
}

// 执行Optimizer阶段
bool Pipeline::executeOptimizerStage() {
	printf("=== 执行Optimizer阶段：代码优化 ===\n");
	
	if (!stagesExecuted[STAGE_PARSER]) {
		printf("错误: Parser阶段未执行\n");
		return false;
	}
	
	if (!optimizer) {
		printf("错误: Optimizer未初始化\n");
		return false;
	}
	
	// 直接传递AST给Optimizer进行优化
	bool success = optimizer->optimizeFromAST(parserAST);
	if (!success) {
		printf("错误: Optimizer执行失败\n");
		return false;
	}
	
	// 获取优化后的汇编代码（字符串形式）
	optimizedAsmCode = optimizer->getOptimizedAssemblyCode();
	
	printf("优化完成，生成优化后汇编代码\n");
	
	stagesExecuted[STAGE_OPTIMIZER] = true;
	printf("Optimizer阶段完成\n\n");
	return true;
}

// 执行ASM阶段
bool Pipeline::executeASMStage() {
	printf("=== 执行ASM阶段：汇编 ===\n");
	
	if (!stagesExecuted[STAGE_OPTIMIZER]) {
		printf("错误: Optimizer阶段未执行\n");
		return false;
	}
	
	// TODO: 修改Asm类来支持从字符串解析汇编代码
	// 目前Asm类需要文件路径，需要改为支持字符串输入
	
	printf("注意: 需要修改Asm类来支持从字符串解析汇编代码\n");
	
	// 暂时创建临时文件来测试
	string tempAsmFile = "/tmp/temp_asm.asm";
	FILE* tempFp = fopen(tempAsmFile.c_str(), "w");
	if (tempFp) {
		fprintf(tempFp, "%s", optimizedAsmCode.c_str());
		fclose(tempFp);
		
		// 创建Asm处理器
		if (!asmProcessor) {
			asmProcessor = new Asm(tempAsmFile);
		}
		
		// 解析汇编文件
		asmProcessor->parse();
		
		// 生成二进制文件
		FILE* fp = fopen(binaryFile.c_str(), "wb");
		if (!fp) {
			printf("错误: 无法创建二进制文件: %s\n", binaryFile.c_str());
			return false;
		}
		asmProcessor->write(fp);
		fclose(fp);
		
		// 清理临时文件
		remove(tempAsmFile.c_str());
	}
	
	printf("汇编完成，生成二进制文件: %s\n", binaryFile.c_str());
	
	stagesExecuted[STAGE_ASM] = true;
	printf("ASM阶段完成\n\n");
	return true;
}


// 检查阶段是否已执行
bool Pipeline::isStageExecuted(PipelineStage stage) const {
	if (stage >= 0 && stage < 3) {
		return stagesExecuted[stage];
	}
	return false;
}

// 打印流水线状态
void Pipeline::printPipelineStatus() const {
	printf("\n=== 流水线执行状态 ===\n");
	printf("Parser阶段: %s\n", stagesExecuted[STAGE_PARSER] ? "已完成" : "未执行");
	printf("Optimizer阶段: %s\n", stagesExecuted[STAGE_OPTIMIZER] ? "已完成" : "未执行");
	printf("ASM阶段: %s\n", stagesExecuted[STAGE_ASM] ? "已完成" : "未执行");
	printf("\n=== 中间文件 ===\n");
	printf("优化后ASM代码: 内存中\n");
	printf("二进制文件: %s\n", binaryFile.c_str());
}

// 打印Parser AST
void Pipeline::printParserAST() const {
	printf("Parser AST: 暂时跳过\n");
}

// 打印未优化汇编
void Pipeline::printUnoptimizedAsm() const {
	printf("=== 未优化汇编代码 ===\n");
	printf("注意: 未优化汇编代码现在在内存中，不再写入文件\n");
	printf("====================\n");
}

// 打印优化后汇编
void Pipeline::printOptimizedAsm() const {
	printf("=== 优化后汇编代码 ===\n");
	printf("%s", optimizedAsmCode.c_str());
	printf("====================\n");
}

// 打印二进制文件信息
void Pipeline::printBinaryInfo() const {
	printf("=== 二进制文件信息 ===\n");
	printf("文件路径: %s\n", binaryFile.c_str());
	
	ifstream file(binaryFile, ios::binary);
	if (file.is_open()) {
		file.seekg(0, ios::end);
		size_t size = file.tellg();
		file.close();
		printf("文件大小: %zu 字节\n", size);
	} else {
		printf("无法打开文件: %s\n", binaryFile.c_str());
	}
}
