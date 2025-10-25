#include "Common/Logger.h"
#include <iostream>

int main() {
    std::cout << "Simple Logger Test..." << std::endl;
    
    // 初始化日志系统
    Common::Logger& logger = Common::Logger::getInstance();
    
    // 使用默认配置初始化
    if (!logger.initializeDefault("INFO", "Logs/simple.log")) {
        std::cerr << "Failed to initialize logger" << std::endl;
        return 1;
    }
    
    std::cout << "Logger initialized successfully!" << std::endl;
    
    // 测试基本日志功能
    LOG_INFO("Test", "This is a test message");
    LOG_WARN("Test", "This is a warning message");
    LOG_ERROR("Test", "This is an error message");
    
    std::cout << "Simple test completed!" << std::endl;
    
    return 0;
}
