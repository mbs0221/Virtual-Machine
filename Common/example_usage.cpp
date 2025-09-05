#include "Common/Logger.h"
#include <iostream>

int main() {
    // 初始化日志系统
    Common::Logger& logger = Common::Logger::getInstance();
    
    // 使用默认配置初始化
    if (!logger.initializeDefault("DEBUG", "logs/example.log")) {
        std::cerr << "Failed to initialize logger" << std::endl;
        return 1;
    }
    
    // 获取不同模块的日志记录器
    log4cpp::Category& cpuLogger = logger.getLogger("CPU");
    log4cpp::Category& asmLogger = logger.getLogger("Asm");
    log4cpp::Category& mmioLogger = logger.getLogger("MMIO");
    
    // 使用日志记录器
    cpuLogger.info("CPU module initialized");
    cpuLogger.debug("Register R0 = 0x%04X", 0x1234);
    cpuLogger.warn("Memory access at invalid address: 0x%04X", 0xFFFF);
    
    asmLogger.info("Assembler started");
    asmLogger.error("Syntax error at line %d: %s", 42, "unexpected token");
    
    mmioLogger.debug("MMIO write: address=0x%04X, value=0x%04X", 0xF011, 0x0003);
    mmioLogger.info("Timer device configured");
    
    // 使用便捷宏
    LOG_INFO("CPU", "Instruction executed: MOV R0, R1");
    LOG_DEBUG_FMT("Asm", "Parsing instruction: %s at line %d", "LOAD", 10);
    LOG_ERROR_FMT("MMIO", "Device not found at address 0x%04X", 0xF020);
    
    // 设置日志级别
    logger.setLogLevel("WARN");
    
    // 这些消息不会显示（级别太低）
    cpuLogger.debug("This debug message will not be shown");
    cpuLogger.info("This info message will not be shown");
    
    // 这些消息会显示
    cpuLogger.warn("This warning will be shown");
    cpuLogger.error("This error will be shown");
    
    // 关闭日志系统
    logger.shutdown();
    
    return 0;
}
