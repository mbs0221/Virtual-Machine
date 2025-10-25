#ifndef SIMPLE_LOGGER_H
#define SIMPLE_LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <mutex>

namespace Common {

/**
 * @brief 简化版日志管理器，避免 log4cpp 的析构问题
 */
class SimpleLogger {
public:
    enum LogLevel {
        DEBUG = 0,
        INFO = 1,
        WARN = 2,
        ERROR = 3,
        FATAL = 4
    };

    static SimpleLogger& getInstance();
    
    bool initialize(const std::string& logLevel = "INFO", const std::string& logFile = "");
    void log(LogLevel level, const std::string& module, const std::string& message);
    void setLogLevel(const std::string& level);
    void shutdown();
    bool isInitialized() const { return initialized_; }

    // 便捷方法
    void debug(const std::string& module, const std::string& message) {
        log(DEBUG, module, message);
    }
    
    void info(const std::string& module, const std::string& message) {
        log(INFO, module, message);
    }
    
    void warn(const std::string& module, const std::string& message) {
        log(WARN, module, message);
    }
    
    void error(const std::string& module, const std::string& message) {
        log(ERROR, module, message);
    }
    
    void fatal(const std::string& module, const std::string& message) {
        log(FATAL, module, message);
    }

private:
    SimpleLogger() : initialized_(false), currentLevel_(INFO) {}
    ~SimpleLogger() = default;
    SimpleLogger(const SimpleLogger&) = delete;
    SimpleLogger& operator=(const SimpleLogger&) = delete;

    bool initialized_;
    LogLevel currentLevel_;
    std::unique_ptr<std::ofstream> logFile_;
    std::mutex logMutex_;
    std::string getLevelString(LogLevel level);
    LogLevel parseLogLevel(const std::string& level);
};

} // namespace Common

#endif // SIMPLE_LOGGER_H