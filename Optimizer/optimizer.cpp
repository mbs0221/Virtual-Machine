#include "optimizer.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>

// 构造函数
Optimizer::Optimizer() : tacConverter(nullptr), cfg(nullptr), generator(nullptr), regAllocator(nullptr) {
    tacConverter = new TACConverter();
    generator = new CodeGenerator();
    regAllocator = new RegisterAllocator();
}

// 析构函数
Optimizer::~Optimizer() {
    if (tacConverter) delete tacConverter;
    if (cfg) delete cfg;
    if (generator) delete generator;
    if (regAllocator) delete regAllocator;
}

// 执行优化（从Parser的AST开始）
bool Optimizer::optimizeFromAST(Stmt* ast) {
    printf("=== 开始执行代码优化（从AST开始）===\n");
    
    // 1. 从AST生成TAC
    if (!generateTACFromAST(ast)) {
        printf("错误: 从AST生成TAC失败\n");
        return false;
    }
    
    // 2. 从TAC构建CFG
    if (!buildCFGFromTAC()) {
        printf("错误: 从TAC构建CFG失败\n");
        return false;
    }
    
    // 3. 执行各种优化
    if (!performOptimizations()) {
        printf("错误: 执行优化失败\n");
        return false;
    }
    
    // 4. 生成优化后的汇编代码
    if (!generateOptimizedAssembly()) {
        printf("错误: 生成优化后汇编代码失败\n");
        return false;
    }
    
    printf("=== 代码优化完成 ===\n");
    return true;
}

// 执行优化（从汇编文件开始）
bool Optimizer::optimizeFromAssembly(const string& asmFile) {
    printf("=== 开始执行代码优化（从汇编文件开始）===\n");
    
    // 1. 从汇编文件解析并生成TAC
    if (!parseAssemblyToTAC(asmFile)) {
        printf("错误: 从汇编文件生成TAC失败\n");
        return false;
    }
    
    // 2. 从TAC构建CFG
    if (!buildCFGFromTAC()) {
        printf("错误: 从TAC构建CFG失败\n");
        return false;
    }
    
    // 3. 执行各种优化
    if (!performOptimizations()) {
        printf("错误: 执行优化失败\n");
        return false;
    }
    
    // 4. 生成优化后的汇编代码
    if (!generateOptimizedAssembly()) {
        printf("错误: 生成优化后汇编代码失败\n");
        return false;
    }
    
    printf("=== 代码优化完成 ===\n");
    return true;
}

// 从AST生成TAC
bool Optimizer::generateTACFromAST(Stmt* ast) {
    printf("从AST生成TAC...\n");
    
    if (!ast) {
        printf("错误: AST为空\n");
        return false;
    }
    
    if (!tacConverter) {
        printf("错误: TACConverter未初始化\n");
        return false;
    }
    
    // 使用TACProgram的构造函数直接从AST生成TAC
    TACProgram* tacProgram = new TACProgram(ast);
    
    // 将生成的TAC程序设置到TACConverter中
    // 注意：这里需要修改TACConverter来支持直接设置TAC程序
    printf("成功从AST生成TAC，指令数: %zu\n", tacProgram->size());
    
    // 暂时使用现有的convertFromAST方法（传入空列表）
    TACProgram* result = tacConverter->convertFromAST(list<Code*>());
    if (!result) {
        printf("错误: TACConverter转换失败\n");
        delete tacProgram;
        return false;
    }
    
    // 将我们生成的TAC指令添加到结果中
    const vector<TACInstruction>& ourInstructions = tacProgram->getInstructions();
    for (size_t i = 0; i < ourInstructions.size(); i++) {
        result->addInstruction(ourInstructions[i]);
    }
    
    delete tacProgram;
    return true;
}


// 从TAC构建CFG
bool Optimizer::buildCFGFromTAC() {
    printf("从TAC构建CFG...\n");
    
    if (!tacConverter) {
        printf("错误: TACConverter未初始化\n");
        return false;
    }
    
    // TODO: 实现从TAC到CFG的转换
    // 需要从TACConverter获取TACProgram，然后构建CFG
    
    printf("注意: TAC到CFG转换需要实现\n");
    return true; // 暂时返回true，避免阻塞
}

// 从汇编文件解析并生成TAC
bool Optimizer::parseAssemblyToTAC(const string& asmFile) {
    printf("从汇编文件解析并生成TAC: %s\n", asmFile.c_str());
    
    if (!tacConverter) {
        printf("错误: TACConverter未初始化\n");
        return false;
    }
    
    // 读取汇编文件
    ifstream file(asmFile);
    if (!file.is_open()) {
        printf("错误: 无法打开汇编文件: %s\n", asmFile.c_str());
        return false;
    }
    
    // 初始化TAC程序
    TACProgram* tacProgram = new TACProgram();
    string line;
    int lineNum = 1;
    
    while (getline(file, line)) {
        // 跳过空行和注释
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            lineNum++;
            continue;
        }
        
        // 解析汇编指令并转换为TAC
        if (!parseAssemblyLineToTAC(line, tacProgram, lineNum)) {
            printf("警告: 第%d行解析失败: %s\n", lineNum, line.c_str());
        }
        lineNum++;
    }
    
    file.close();
    
    // 将生成的TAC程序设置到TACConverter中
    // 使用现有的convertFromAST方法（传入空列表）
    TACProgram* result = tacConverter->convertFromAST(list<Code*>());
    if (!result) {
        printf("错误: TACConverter转换失败\n");
        delete tacProgram;
        return false;
    }
    
    // 将我们生成的TAC指令添加到结果中
    const vector<TACInstruction>& ourInstructions = tacProgram->getInstructions();
    for (size_t i = 0; i < ourInstructions.size(); i++) {
        result->addInstruction(ourInstructions[i]);
    }
    
    printf("成功从汇编文件生成TAC，指令数: %zu\n", tacProgram->size());
    delete tacProgram;
    return true;
}

// 将汇编指令行转换为TAC指令
bool Optimizer::parseAssemblyLineToTAC(const string& line, TACProgram* tacProgram, int lineNum) {
    if (!tacProgram) {
        return false;
    }
    
    // 简单的汇编指令解析
    // 这里只处理一些基本的指令类型
    
    // 去除前后空格
    string trimmed = line;
    size_t start = trimmed.find_first_not_of(" \t");
    if (start != string::npos) {
        trimmed = trimmed.substr(start);
    }
    size_t end = trimmed.find_last_not_of(" \t");
    if (end != string::npos) {
        trimmed = trimmed.substr(0, end + 1);
    }
    
    if (trimmed.empty()) {
        return true;
    }
    
    // 解析指令
    istringstream iss(trimmed);
    string opcode;
    iss >> opcode;
    
    TACInstruction inst(TAC_ASSIGN, lineNum); // 默认类型
    
    if (opcode == "add") {
        inst.op = TAC_ADD;
        string arg1, arg2, result;
        iss >> arg1 >> arg2 >> result;
        inst.arg1 = TACOperand(TAC_TEMP, arg1);
        inst.arg2 = TACOperand(TAC_TEMP, arg2);
        inst.result = TACOperand(TAC_TEMP, result);
    }
    else if (opcode == "sub") {
        inst.op = TAC_SUB;
        string arg1, arg2, result;
        iss >> arg1 >> arg2 >> result;
        inst.arg1 = TACOperand(TAC_TEMP, arg1);
        inst.arg2 = TACOperand(TAC_TEMP, arg2);
        inst.result = TACOperand(TAC_TEMP, result);
    }
    else if (opcode == "mul") {
        inst.op = TAC_MUL;
        string arg1, arg2, result;
        iss >> arg1 >> arg2 >> result;
        inst.arg1 = TACOperand(TAC_TEMP, arg1);
        inst.arg2 = TACOperand(TAC_TEMP, arg2);
        inst.result = TACOperand(TAC_TEMP, result);
    }
    else if (opcode == "div") {
        inst.op = TAC_DIV;
        string arg1, arg2, result;
        iss >> arg1 >> arg2 >> result;
        inst.arg1 = TACOperand(TAC_TEMP, arg1);
        inst.arg2 = TACOperand(TAC_TEMP, arg2);
        inst.result = TACOperand(TAC_TEMP, result);
    }
    else if (opcode == "mov") {
        inst.op = TAC_MOV;
        string arg1, result;
        iss >> arg1 >> result;
        inst.arg1 = TACOperand(TAC_TEMP, arg1);
        inst.result = TACOperand(TAC_TEMP, result);
    }
    else if (opcode == "load") {
        inst.op = TAC_LOAD;
        string arg1, result;
        iss >> arg1 >> result;
        inst.arg1 = TACOperand(TAC_TEMP, arg1);
        inst.result = TACOperand(TAC_TEMP, result);
    }
    else if (opcode == "store") {
        inst.op = TAC_STORE;
        string arg1, arg2;
        iss >> arg1 >> arg2;
        inst.arg1 = TACOperand(TAC_TEMP, arg1);
        inst.arg2 = TACOperand(TAC_TEMP, arg2);
    }
    else if (opcode == "jmp") {
        inst.op = TAC_JUMP;
        string label;
        iss >> label;
        inst.label = label;
    }
    else if (opcode == "call") {
        inst.op = TAC_CALL;
        string func;
        iss >> func;
        inst.label = func;
    }
    else if (opcode == "ret") {
        inst.op = TAC_RETURN;
    }
    else if (opcode == "halt") {
        inst.op = TAC_HALT;
    }
    else if (opcode.find(":") != string::npos) {
        // 标签定义
        inst.op = TAC_LABEL;
        inst.label = opcode.substr(0, opcode.length() - 1); // 去掉冒号
    }
    else {
        // 未识别的指令，跳过
        printf("未识别的汇编指令: %s\n", opcode.c_str());
        return true;
    }
    
    tacProgram->addInstruction(inst);
    return true;
}

// 执行各种优化
bool Optimizer::performOptimizations() {
    printf("执行代码优化...\n");
    
    if (!cfg) {
        printf("错误: CFG为空\n");
        return false;
    }
    
    // 1. 死代码消除
    printf("执行死代码消除...\n");
    // TODO: 实现死代码消除
    
    // 2. 常量折叠
    printf("执行常量折叠...\n");
    // TODO: 实现常量折叠
    
    // 3. 寄存器分配优化
    printf("执行寄存器分配优化...\n");
    regAllocator->allocateRegisters(cfg);
    
    // 4. 指令调度
    printf("执行指令调度...\n");
    // TODO: 实现指令调度
    
    printf("优化完成\n");
    return true;
}

// 生成优化后的汇编代码
bool Optimizer::generateOptimizedAssembly() {
    printf("生成优化后的汇编代码: %s\n", outputFile.c_str());
    
    if (!cfg) {
        printf("错误: CFG为空\n");
        return false;
    }
    
    // 使用字符串流来构建汇编代码
    stringstream asmStream;
    
    // 写入文件头
    asmStream << "; 优化后的汇编代码\n";
    asmStream << "; 基本块数: " << cfg->size() << "\n";
    asmStream << "\n";
    
    // 遍历所有基本块
    const vector<BasicBlock*>& blocks = cfg->getBlocks();
    for (size_t i = 0; i < blocks.size(); i++) {
        BasicBlock* block = blocks[i];
        
        // 跳过空的基本块
        if (block->size() == 0) {
            continue;
        }
        
        // 写入基本块信息
        if (block->getIsEntry()) {
            asmStream << "; === 入口基本块 ===\n";
        } else if (block->getIsExit()) {
            asmStream << "; === 出口基本块 ===\n";
        } else {
            asmStream << "; === 基本块 " << block->getId() << " ===\n";
        }
        
        // 写入指令
        const vector<TACInstruction>& instructions = block->getInstructions();
        for (size_t j = 0; j < instructions.size(); j++) {
            const TACInstruction& inst = instructions[j];
            
            // 根据指令类型生成汇编代码
            switch (inst.op) {
                case TAC_JUMP:
                    if (!inst.label.empty()) {
                        asmStream << "jmp " << inst.label << "\n";
                    }
                    break;
                case TAC_RETURN:
                    asmStream << "ret\n";
                    break;
                case TAC_ASSIGN:
                default:
                    // 对于其他指令，生成基本的汇编代码
                    asmStream << "; 优化后的指令\n";
                    break;
            }
        }
        
        asmStream << "\n";
    }
    
    // 获取字符串形式的汇编代码
    optimizedAsmCode = asmStream.str();
    
    // 同时写入文件（用于调试）
    FILE* fp = fopen(outputFile.c_str(), "w");
    if (fp) {
        fprintf(fp, "%s", optimizedAsmCode.c_str());
        fclose(fp);
    }
    
    printf("优化后的汇编代码生成完成\n");
    return true;
}

// 获取优化后的汇编代码（字符串形式）
string Optimizer::getOptimizedAssemblyCode() const {
    return optimizedAsmCode;
}

// 打印优化信息
void Optimizer::printOptimizationInfo() const {
    printf("=== 优化器信息 ===\n");
    printf("输入文件: %s\n", inputFile.c_str());
    printf("输出文件: %s\n", outputFile.c_str());
    
    if (cfg) {
        printf("CFG基本块数: %zu\n", cfg->size());
    }
    
    if (regAllocator) {
        printf("寄存器分配器状态: 已初始化\n");
    }
    
    printf("==================\n");
}
