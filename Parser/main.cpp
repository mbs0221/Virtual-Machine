#include "parser.h"
#include <iostream>
#include <cstdio>
#include <cstring>

using namespace std;

void printUsage(const char* programName) {
    printf("用法: %s [输入文件] [输出文件]\n", programName);
    printf("参数:\n");
    printf("  输入文件        要解析的文本文件 (默认: Text.txt)\n");
    printf("  输出文件        生成的汇编文件 (默认: data.asm)\n");
    printf("\n");
    printf("示例:\n");
    printf("  %s input.txt output.asm\n", programName);
    printf("  %s program.txt\n", programName);
}

int main(int argc, char* argv[]) {
    string inputFile = "Text.txt";
    string outputFile = "data.asm";
    
    // 解析命令行参数
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        }
        inputFile = argv[1];
    }
    
    if (argc > 2) {
        outputFile = argv[2];
    }
    
    printf("输入文件: %s\n", inputFile.c_str());
    printf("输出文件: %s\n", outputFile.c_str());
    
    FILE *fp = NULL;
    
    // 开始语法分析
    printf("开始语法分析\n");
    
    // 重置寄存器分配器
    RegisterAllocator::reset();
    
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
    printf("寄存器分配统计:\n");
    RegisterAllocator::printStatus();
    printf("最大使用寄存器数: %d\n", RegisterAllocator::getMaxUsed());
    
    return 0;
}