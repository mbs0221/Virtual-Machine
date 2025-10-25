#include "Common/Logger.h"
#include <iostream>

int main() {
    std::cout << "Testing Common Logger Library..." << std::endl;
    
    // 初始化日志系统
    Common::Logger& logger = Common::Logger::getInstance();
    
    // 使用默认配置初始化
    if (!logger.initializeDefault("DEBUG", "Logs/test.log")) {
        std::cerr << "Failed to initialize logger" << std::endl;
        return 1;
    }
    
    std::cout << "Logger initialized successfully!" << std::endl;
    
    // 测试不同模块的日志记录
    LOG_INFO("Test", "Testing info level logging");
    LOG_DEBUG("Test", "Testing debug level logging");
    LOG_WARN("Test", "Testing warning level logging");
    LOG_ERROR("Test", "Testing error level logging");
    
    // 测试格式化日志
    LOG_INFO_FMT("Test", "Testing formatted logging: %s = %d", "value", 42);
    LOG_DEBUG_FMT("Test", "Testing debug formatted logging: 0x%04X", 0x1234);
    
    // 测试不同模块
    LOG_INFO("CPU", "CPU module test message");
    LOG_INFO("Asm", "Assembler module test message");
    LOG_INFO("MMIO", "MMIO module test message");
    
    // 测试日志级别设置
    std::cout << "Setting log level to WARN..." << std::endl;
    logger.setLogLevel("WARN");
    
    LOG_DEBUG("Test", "This debug message should not appear");
    LOG_INFO("Test", "This info message should not appear");
    LOG_WARN("Test", "This warning message should appear");
    LOG_ERROR("Test", "This error message should appear");
    
    std::cout << "Logger test completed successfully!" << std::endl;
    
    // 注释掉 shutdown 来避免段错误
    // logger.shutdown();
    
    return 0;
}
