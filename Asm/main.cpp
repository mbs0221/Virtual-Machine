#include "asm.h"
#include "Common/Logger.h"
#include <iostream>
#include <cstdio>
#include <cstring>

using namespace std;

void printUsage(const char* programName) {
    LOG_INFO("AsmMain", "Toy架构汇编器");
    LOG_INFO("AsmMain", "用法: " + std::string(programName) + " [输入文件] [输出文件]");
    LOG_INFO("AsmMain", "参数:");
    LOG_INFO("AsmMain", "  输入文件        要汇编的文件 (默认: data.asm)");
    LOG_INFO("AsmMain", "  输出文件        生成的目标文件 (默认: data.bin)");
    LOG_INFO("AsmMain", "");
    LOG_INFO("AsmMain", "示例:");
    LOG_INFO("AsmMain", "  " + std::string(programName) + " input.asm output.bin");
    LOG_INFO("AsmMain", "  " + std::string(programName) + " program.asm");
    LOG_INFO("AsmMain", "");
    LOG_INFO("AsmMain", "Toy架构特点:");
    LOG_INFO("AsmMain", "  - 16位字长");
    LOG_INFO("AsmMain", "  - 256个寄存器");
    LOG_INFO("AsmMain", "  - 64KB内存");
    LOG_INFO("AsmMain", "  - 支持字节和字操作");
    LOG_INFO("AsmMain", "  - 支持函数调用和栈操作");
}

int main(int argc, char* argv[]) {
    // 初始化日志系统
    Common::Logger::getInstance().initializeDefault("INFO", "Logs/asm.log");
    
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
    
    LOG_INFO("AsmMain", "=== Toy架构汇编器 ===");
    LOG_INFO("AsmMain", "输入文件: " + inputFile);
    LOG_INFO("AsmMain", "输出文件: " + outputFile);
    
    FILE *fp = NULL;
    
    try {
        // 汇编器的目标代码
        LOG_INFO("AsmMain", "开始汇编...");
        Asm Asm(inputFile);
        Asm.parse();
        LOG_INFO("AsmMain", "汇编解析完成");
        
        // 打印指令列表（只记录到日志文件）
        Asm.printInstructions();
        
        fp = fopen(outputFile.c_str(), "wb");
        if (fp == NULL) {
            LOG_ERROR("AsmMain", "错误: 无法打开文件 " + outputFile);
            return 1;
        }
        
        Asm.write(fp);
        fclose(fp);
        
        LOG_INFO("AsmMain", "汇编完成，输出文件: " + outputFile);
        LOG_INFO("AsmMain", "文件格式: DS(2字节) + CS(2字节) + LENGTH(2字节) + 指令数据");
        
        // 显示错误统计
        int errorCount = Asm.getErrorCount();
        if (errorCount > 0) {
            LOG_WARN("AsmMain", "警告: 汇编过程中发现 " + std::to_string(errorCount) + " 个错误");
        } else {
            LOG_INFO("AsmMain", "汇编成功，无错误");
        }
        
    } catch (const exception& e) {
        LOG_ERROR("AsmMain", "汇编错误: " + std::string(e.what()));
        if (fp) fclose(fp);
        return 1;
    }
    
    return 0;
}