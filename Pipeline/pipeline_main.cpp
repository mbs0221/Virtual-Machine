#include "pipeline.h"
#include <iostream>
#include <cstdio>
#include <cstring>

using namespace std;

void printUsage(const char* programName) {
    printf("Toy架构编译流水线\n");
    printf("用法: %s <输入文件> <输出文件>\n", programName);
    printf("参数:\n");
    printf("  输入文件        要编译的高级语言文件 (必需)\n");
    printf("  输出文件        生成的目标文件 (必需)\n");
    printf("\n");
    printf("示例:\n");
    printf("  %s input.txt output.bin\n", programName);
    printf("  %s program.txt Examples/Binaries/program.bin\n", programName);
    printf("\n");
    printf("编译流水线:\n");
    printf("  Parser -> Optimizer -> Asm\n");
    printf("  高级语言解析 -> 代码优化 -> 汇编\n");
    printf("  数据在内存中直接传递，无需中间文件\n");
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
    
    try {
        // 创建并执行编译流水线
        Pipeline pipeline(inputFile, outputFile);
        
        // 执行完整流水线
        bool success = pipeline.execute();
        
        if (success) {
            printf("\n编译成功完成！\n");
            pipeline.printPipelineStatus();
            return 0;
        } else {
            printf("\n编译失败！\n");
            pipeline.printPipelineStatus();
            return 1;
        }
        
    } catch (const exception& e) {
        printf("编译错误: %s\n", e.what());
        return 1;
    }
    
    return 0;
}
