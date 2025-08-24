#include "toy.h"
#include <iostream>
#include <cstring>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Toy CPU 虚拟机" << std::endl;
        std::cout << "用法: " << argv[0] << " <程序文件>" << std::endl;
        std::cout << "示例: " << argv[0] << " program.bin" << std::endl;
        std::cout << std::endl;
        std::cout << "Toy架构特点:" << std::endl;
        std::cout << "- 16位字长" << std::endl;
        std::cout << "- 256个寄存器" << std::endl;
        std::cout << "- 64KB内存" << std::endl;
        std::cout << "- 支持字节和字操作" << std::endl;
        std::cout << "- 支持函数调用和栈操作" << std::endl;
        return 1;
    }
    
    ToyCPU cpu;
    
    // 加载程序
    cpu.load_program(argv[1]);
    
    // 执行程序
    cpu.execute();
    
    // 显示最终状态
    cpu.dump_registers();
    cpu.dump_memory(0x0000, 0x0020); // 显示程序区域
    
    return 0;
}
