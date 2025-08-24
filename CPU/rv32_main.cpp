#include "rv32.h"
#include <iostream>
#include <cstring>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "RV32 CPU 虚拟机" << std::endl;
        std::cout << "用法: " << argv[0] << " <程序文件>" << std::endl;
        std::cout << "示例: " << argv[0] << " program.bin" << std::endl;
        return 1;
    }
    
    RV32CPU cpu;
    
    // 加载程序
    cpu.load_program(argv[1]);
    
    // 执行程序
    cpu.execute();
    
    // 显示最终状态
    cpu.dump_registers();
    cpu.dump_memory(0x1000, 0x1020); // 显示程序区域
    
    return 0;
}
