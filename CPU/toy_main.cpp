#include "toy.h"
#include "Common/Logger.h"
#include <iostream>
#include <cstring>

int main(int argc, char* argv[]) {
    // 确保stdout不被缓冲
    setvbuf(stdout, NULL, _IONBF, 0);
    
    // 初始化日志系统
    Common::Logger::getInstance().initializeDefault("DEBUG", "Logs/toy.log");
    LOG_INFO("ToyMain", "Toy CPU 虚拟机启动");
    
    if (argc < 2) {
        LOG_INFO("ToyMain", "显示使用帮助");
        LOG_INFO("ToyMain", "Toy CPU 虚拟机");
        LOG_INFO("ToyMain", "用法: " + std::string(argv[0]) + " <程序文件>");
        LOG_INFO("ToyMain", "示例: " + std::string(argv[0]) + " program.bin");
        LOG_INFO("ToyMain", "Toy架构特点:");
        LOG_INFO("ToyMain", "- 16位字长");
        LOG_INFO("ToyMain", "- 256个寄存器");
        LOG_INFO("ToyMain", "- 64KB内存");
        LOG_INFO("ToyMain", "- 支持字节和字操作");
        LOG_INFO("ToyMain", "- 支持函数调用和栈操作");
        return 1;
    }
    
    ToyCPU cpu;
    
    // 加载程序
    LOG_INFO("ToyMain", "加载程序: " + std::string(argv[1]));
    cpu.load_program(argv[1]);
    
    // 执行程序
    LOG_INFO("ToyMain", "开始执行程序");
    cpu.execute();
    
    // 显示最终状态
    LOG_INFO("ToyMain", "显示最终状态");
    cpu.dump_registers();
    cpu.dump_memory(0x0000, 0x0020); // 显示程序区域
    
    LOG_INFO("ToyMain", "程序执行完成");
    return 0;
}
