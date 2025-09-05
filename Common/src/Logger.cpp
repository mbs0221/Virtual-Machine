#include "Common/Logger.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace Common {

#ifdef USE_SIMPLE_LOGGER
// 简化版本实现

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

bool Logger::initialize(const std::string& configFile) {
    // 简化版本忽略配置文件，使用默认配置
    return initializeDefault();
}

bool Logger::initializeDefault(const std::string& logLevel, const std::string& logFile) {
    if (initialized_) {
        return true;
    }

    SimpleLogger& simpleLogger = SimpleLogger::getInstance();
    if (simpleLogger.initialize(logLevel, logFile)) {
        initialized_ = true;
        return true;
    }
    
    return false;
}

SimpleLogger& Logger::getLogger(const std::string& module) {
    if (!initialized_) {
        // 如果未初始化，使用默认配置初始化
        initializeDefault();
    }
    
    return SimpleLogger::getInstance();
}

void Logger::setLogLevel(const std::string& level) {
    if (initialized_) {
        SimpleLogger::getInstance().setLogLevel(level);
    }
}

void Logger::shutdown() {
    if (initialized_) {
        SimpleLogger::getInstance().shutdown();
        initialized_ = false;
    }
}

#else
// log4cpp 版本实现

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

bool Logger::initialize(const std::string& configFile) {
    if (initialized_) {
        return true;
    }

    try {
        if (!configFile.empty()) {
            // 使用配置文件初始化
            log4cpp::PropertyConfigurator::configure(configFile);
            configFile_ = configFile;
        } else {
            // 使用默认配置
            return initializeDefault();
        }
        
        initialized_ = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize logger with config file: " << e.what() << std::endl;
        return false;
    }
}

bool Logger::initializeDefault(const std::string& logLevel, const std::string& logFile) {
    if (initialized_) {
        return true;
    }

    try {
        // 创建根日志记录器
        log4cpp::Category& root = log4cpp::Category::getRoot();
        
        // 设置日志级别
        root.setPriority(parseLogLevel(logLevel));
        
        // 创建控制台输出器布局
        log4cpp::PatternLayout* consoleLayout = new log4cpp::PatternLayout();
        consoleLayout->setConversionPattern("%d{%Y-%m-%d %H:%M:%S.%l} [%p] %c: %m%n");
        
        // 创建控制台输出器
        log4cpp::OstreamAppender* consoleAppender = new log4cpp::OstreamAppender("console", &std::cout);
        consoleAppender->setLayout(consoleLayout);
        root.addAppender(consoleAppender);
        
        // 如果指定了日志文件，创建文件输出器
        if (!logFile.empty()) {
            // 创建文件输出器布局
            log4cpp::PatternLayout* fileLayout = new log4cpp::PatternLayout();
            fileLayout->setConversionPattern("%d{%Y-%m-%d %H:%M:%S.%l} [%p] %c: %m%n");
            
            log4cpp::FileAppender* fileAppender = new log4cpp::FileAppender("file", logFile);
            fileAppender->setLayout(fileLayout);
            root.addAppender(fileAppender);
        }
        
        initialized_ = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize logger with default config: " << e.what() << std::endl;
        return false;
    }
}

log4cpp::Category& Logger::getLogger(const std::string& module) {
    if (!initialized_) {
        // 如果未初始化，使用默认配置初始化
        initializeDefault();
    }
    
    return log4cpp::Category::getInstance(module);
}

void Logger::setLogLevel(const std::string& level) {
    if (initialized_) {
        log4cpp::Category::getRoot().setPriority(parseLogLevel(level));
    }
}

void Logger::shutdown() {
    if (initialized_) {
        // 不调用 log4cpp::Category::shutdown()，让系统自动清理
        // 这样可以避免在程序退出时的析构顺序问题
        initialized_ = false;
    }
}

log4cpp::Priority::PriorityLevel Logger::parseLogLevel(const std::string& level) {
    std::string upperLevel = level;
    std::transform(upperLevel.begin(), upperLevel.end(), upperLevel.begin(), ::toupper);
    
    if (upperLevel == "DEBUG") {
        return log4cpp::Priority::DEBUG;
    } else if (upperLevel == "INFO") {
        return log4cpp::Priority::INFO;
    } else if (upperLevel == "WARN" || upperLevel == "WARNING") {
        return log4cpp::Priority::WARN;
    } else if (upperLevel == "ERROR") {
        return log4cpp::Priority::ERROR;
    } else if (upperLevel == "FATAL") {
        return log4cpp::Priority::FATAL;
    } else {
        return log4cpp::Priority::INFO; // 默认级别
    }
}

#endif // USE_SIMPLE_LOGGER

} // namespace Common
