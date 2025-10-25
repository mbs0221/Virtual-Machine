#ifndef __OPTIMIZER_H_
#define __OPTIMIZER_H_

#include <string>
#include "cfg.h"
#include "tac.h"
#include "generator.h"
#include "register_allocator.h"

// 前向声明，避免包含冲突
struct Stmt;
struct Stmts;
struct Decl;
struct Assign;
struct If;
struct While;
struct FuncDef;
struct Return;
struct Print;

using namespace std;

// 优化器主类
class Optimizer {
private:
    TACConverter* tacConverter;      // TAC转换器
    ControlFlowGraph* cfg;           // 控制流图
    CodeGenerator* generator;        // 代码生成器
    RegisterAllocator* regAllocator; // 寄存器分配器
    
    string inputFile;                // 输入文件（未优化汇编）
    string outputFile;               // 输出文件（优化后汇编）
    string optimizedAsmCode;         // 优化后的汇编代码（字符串形式）
    
public:
    Optimizer();
    ~Optimizer();
    
    // 设置输入输出文件
    void setInputFile(const string& file) { inputFile = file; }
    void setOutputFile(const string& file) { outputFile = file; }
    
    // 执行优化（从Parser的AST开始）
    bool optimizeFromAST(Stmt* ast);
    
    // 执行优化（从汇编文件开始）
    bool optimizeFromAssembly(const string& asmFile);
    
    // 从AST生成TAC
    bool generateTACFromAST(Stmt* ast);
    
    // 从TAC构建CFG
    bool buildCFGFromTAC();
    
    // 从汇编文件解析并生成TAC
    bool parseAssemblyToTAC(const string& asmFile);
    
    // 将汇编指令行转换为TAC指令
    bool parseAssemblyLineToTAC(const string& line, TACProgram* tacProgram, int lineNum);
    
    // 执行各种优化
    bool performOptimizations();
    
    // 生成优化后的汇编代码
    bool generateOptimizedAssembly();
    
    // 获取优化后的汇编代码（字符串形式）
    string getOptimizedAssemblyCode() const;
    
    // 打印优化信息
    void printOptimizationInfo() const;
};

#endif
