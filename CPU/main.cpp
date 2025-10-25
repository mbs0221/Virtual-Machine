#include "architecture.h"
#include "Common/Logger.h"
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
    // 初始化日志系统
    Common::Logger::getInstance().initializeDefault("INFO", "Logs/vm.log");
    LOG_INFO("Main", "Virtual Machine started");
    
    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }
    
    // 解析架构类型
    ArchitectureFactory::ArchitectureType arch_type;
    if (strcmp(argv[1], "toy") == 0) {
        arch_type = ArchitectureFactory::TOY;
    } else {
        LOG_ERROR("Main", "不支持的架构: " + std::string(argv[1]));
        std::cerr << "错误: 不支持的架构 '" << argv[1] << "'" << std::endl;
        print_usage(argv[0]);
        return 1;
    }
    
    // 创建架构实例
    Architecture* cpu = ArchitectureFactory::create_architecture(arch_type);
    if (!cpu) {
        LOG_ERROR("Main", "无法创建架构实例");
        std::cerr << "错误: 无法创建架构实例" << std::endl;
        return 1;
    }
    
    // 显示架构信息（只记录到日志，不显示在屏幕上）
    LOG_INFO("Main", "=== " + std::string(cpu->get_name()) + " 架构 ===");
    LOG_INFO("Main", "描述: " + std::string(cpu->get_description()));
    LOG_INFO("Main", "字长: " + std::to_string(cpu->get_word_size()) + " 位");
    LOG_INFO("Main", "寄存器: " + std::to_string(cpu->get_register_count()) + " 个");
    LOG_INFO("Main", "内存: " + std::to_string(cpu->get_memory_size()) + " 字节");
    
    try {
        // 加载程序
        LOG_INFO("Main", "加载程序: " + std::string(argv[2]));
        cpu->load_program(argv[2]);
        
        // 执行程序
        LOG_INFO("Main", "开始执行程序");
        cpu->execute();
        
        // 显示最终状态
        cpu->dump_registers();
        cpu->dump_memory(0x0000, 0x0020);
        
        // 显示执行统计（只记录到日志，不显示在屏幕上）
        LOG_INFO("Main", "=== 执行统计 ===");
        LOG_INFO("Main", "运行状态: " + std::string(cpu->is_running() ? "运行中" : "已停止"));
        LOG_INFO("Main", "程序计数器: 0x" + std::to_string(cpu->get_pc()));
        LOG_INFO("Main", "执行指令数: " + std::to_string(cpu->get_instruction_count()));
        
    } catch (const std::exception& e) {
        LOG_ERROR("Main", "执行错误: " + std::string(e.what()));
        std::cerr << "执行错误: " << e.what() << std::endl;
        ArchitectureFactory::destroy_architecture(cpu);
        return 1;
    }
    
    // 清理资源
    LOG_INFO("Main", "程序执行完成，清理资源");
    ArchitectureFactory::destroy_architecture(cpu);
    
    return 0;
}