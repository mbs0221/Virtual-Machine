#include "asm.h"
#include <iostream>
#include <cstdio>
#include <cstring>

using namespace std;

void printUsage(const char* programName) {
    printf("Toy架构汇编器\n");
    printf("用法: %s [输入文件] [输出文件]\n", programName);
    printf("参数:\n");
    printf("  输入文件        要汇编的文件 (默认: data.asm)\n");
    printf("  输出文件        生成的目标文件 (默认: data.bin)\n");
    printf("\n");
    printf("示例:\n");
    printf("  %s input.asm output.bin\n", programName);
    printf("  %s program.asm\n", programName);
    printf("\n");
    printf("Toy架构特点:\n");
    printf("  - 16位字长\n");
    printf("  - 256个寄存器\n");
    printf("  - 64KB内存\n");
    printf("  - 支持字节和字操作\n");
    printf("  - 支持函数调用和栈操作\n");
}

int main(int argc, char* argv[]) {
    string inputFile = "data.asm";
    string outputFile = "data.bin";
    
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
    
    printf("=== Toy架构汇编器 ===\n");
    printf("输入文件: %s\n", inputFile.c_str());
    printf("输出文件: %s\n", outputFile.c_str());
    printf("\n");
    
    FILE *fp = NULL;
    
    try {
        // 汇编器的目标代码
        printf("开始汇编...\n");
        Asm Asm(inputFile);
        Asm.parse();
        printf("汇编解析完成\n");
        printf("line  width offset\n");
        
        fp = fopen(outputFile.c_str(), "wb");
        if (fp == NULL) {
            printf("错误: 无法打开文件 %s\n", outputFile.c_str());
            return 1;
        }
        
        Asm.write(fp);
        fclose(fp);
        
        printf("汇编完成，输出文件: %s\n", outputFile.c_str());
        printf("文件格式: DS(2字节) + CS(2字节) + LENGTH(2字节) + 指令数据\n");
        
    } catch (const exception& e) {
        printf("汇编错误: %s\n", e.what());
        if (fp) fclose(fp);
        return 1;
    }
    
    return 0;
}