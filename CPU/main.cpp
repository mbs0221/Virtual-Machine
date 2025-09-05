#include "architecture.h"
#include <iostream>
#include <cstring>

void print_usage(const char* program_name) {
    std::cout << "Virtual Machine - 多架构CPU模拟器" << std::endl;
    std::cout << "用法: " << program_name << " <架构> <程序文件>" << std::endl;
    std::cout << std::endl;
    std::cout << "支持的架构:" << std::endl;
    std::cout << "  toy    - Toy架构 (16位，256寄存器)" << std::endl;
    std::cout << std::endl;
    std::cout << "示例:" << std::endl;
    std::cout << "  " << program_name << " toy program.bin" << std::endl;
    std::cout << std::endl;
    std::cout << "架构信息:" << std::endl;
    std::cout << "  Toy: " << ArchitectureFactory::get_architecture_description(ArchitectureFactory::TOY) << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }
    
    // 解析架构类型
    ArchitectureFactory::ArchitectureType arch_type;
    if (strcmp(argv[1], "toy") == 0) {
        arch_type = ArchitectureFactory::TOY;
    } else {
        std::cerr << "错误: 不支持的架构 '" << argv[1] << "'" << std::endl;
        print_usage(argv[0]);
        return 1;
    }
    
    // 创建架构实例
    Architecture* cpu = ArchitectureFactory::create_architecture(arch_type);
    if (!cpu) {
        std::cerr << "错误: 无法创建架构实例" << std::endl;
        return 1;
    }
    
    // 显示架构信息
    std::cout << "=== " << cpu->get_name() << " 架构 ===" << std::endl;
    std::cout << "描述: " << cpu->get_description() << std::endl;
    std::cout << "字长: " << cpu->get_word_size() << " 位" << std::endl;
    std::cout << "寄存器: " << cpu->get_register_count() << " 个" << std::endl;
    std::cout << "内存: " << cpu->get_memory_size() << " 字节" << std::endl;
    std::cout << std::endl;
    
    try {
        // 加载程序
        cpu->load_program(argv[2]);
        
        // 执行程序
        cpu->execute();
        
        // 显示最终状态
        cpu->dump_registers();
        cpu->dump_memory(0x0000, 0x0020);
        
        // 显示执行统计
        std::cout << "\n=== 执行统计 ===" << std::endl;
        std::cout << "运行状态: " << (cpu->is_running() ? "运行中" : "已停止") << std::endl;
        std::cout << "程序计数器: 0x" << std::hex << cpu->get_pc() << std::endl;
        std::cout << "执行指令数: " << std::dec << cpu->get_instruction_count() << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "执行错误: " << e.what() << std::endl;
        ArchitectureFactory::destroy_architecture(cpu);
        return 1;
    }
    
    // 清理资源
    ArchitectureFactory::destroy_architecture(cpu);
    
    return 0;
}