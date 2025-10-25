#include "parser.h"
#include <iostream>
#include <cstdio>
#include <cstring>

using namespace std;

void printUsage(const char* programName) {
    printf("用法: %s <输入文件> <输出文件>\n", programName);
    printf("参数:\n");
    printf("  输入文件        要解析的文本文件 (必需)\n");
    printf("  输出文件        生成的汇编文件 (必需)\n");
    printf("\n");
    printf("示例:\n");
    printf("  %s input.txt output.asm\n", programName);
    printf("  %s program.txt Examples/Optimizer/Input/program.asm\n", programName);
}

int main(int argc, char* argv[]) {
    // 检查参数数量
    if (argc < 3) {
        printf("错误: 缺少必需参数\n\n");
        printUsage(argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        printUsage(argv[0]);
        return 0;
    }
    
    string inputFile = argv[1];
    string outputFile = argv[2];
    
    printf("输入文件: %s\n", inputFile.c_str());
    printf("输出文件: %s\n", outputFile.c_str());
    
    FILE *fp = NULL;
    
    // 开始语法分析
    printf("开始语法分析\n");
    
    // 重置寄存器分配器，配置超额分配
    // Parser只生成AST，不需要寄存器分配器
    
    Parser p(inputFile);
    Stmt *st = p.parse();
    
    printf(" line  stmt\n");
    
    fp = fopen(outputFile.c_str(), "w");
    if (fp == NULL) {
        printf("错误: 无法打开文件 %s\n", outputFile.c_str());
        return 1;
    }
    
    st->code(fp);
    fprintf(fp, "halt\n");
    fclose(fp);
    
    printf("语法分析完成\n");
    
    // 打印寄存器使用统计
    printf("Parser阶段寄存器分配统计:\n");
    // Parser只生成AST，不需要寄存器分配器状态
    printf("注意: 寄存器分配将在Optimizer阶段进行\n");
    
    return 0;
}