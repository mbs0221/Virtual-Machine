#include "Common/SimpleLogger.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <cctype>
#include <sstream>

namespace Common {

SimpleLogger& SimpleLogger::getInstance() {
    static SimpleLogger instance;
    return instance;
}

bool SimpleLogger::initialize(const std::string& logLevel, const std::string& logFile) {
    if (initialized_) {
        return true;
    }

    currentLevel_ = parseLogLevel(logLevel);
    
    if (!logFile.empty()) {
        logFile_.reset(new std::ofstream(logFile, std::ios::app));
        if (!logFile_->is_open()) {
            std::cerr << "Failed to open log file: " << logFile << std::endl;
            return false;
        }
    }
    
    initialized_ = true;
    return true;
}

void SimpleLogger::log(LogLevel level, const std::string& module, const std::string& message) {
    if (!initialized_ || level < currentLevel_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(logMutex_);
    
    // 获取当前时间
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    // 格式化时间戳
    std::ostringstream timestamp;
    timestamp << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    // 格式化日志消息
    std::ostringstream logMessage;
    logMessage << timestamp.str() << " [" << getLevelString(level) << "] " 
               << module << ": " << message << std::endl;
    
    // 输出到控制台
    if (level >= WARN) {
        std::cerr << logMessage.str();
    } else {
        std::cout << logMessage.str();
    }
    
    // 输出到文件
    if (logFile_ && logFile_->is_open()) {
        *logFile_ << logMessage.str();
        logFile_->flush();
    }
}

void SimpleLogger::setLogLevel(const std::string& level) {
    if (initialized_) {
        currentLevel_ = parseLogLevel(level);
    }
}

void SimpleLogger::shutdown() {
    std::lock_guard<std::mutex> lock(logMutex_);
    if (logFile_ && logFile_->is_open()) {
        logFile_->close();
    }
    logFile_.reset();
    initialized_ = false;
}

std::string SimpleLogger::getLevelString(LogLevel level) {
    switch (level) {
        case DEBUG: return "DEBUG";
        case INFO:  return "INFO ";
        case WARN:  return "WARN ";
        case ERROR: return "ERROR";
        case FATAL: return "FATAL";
        default:    return "UNKNOWN";
    }
}

SimpleLogger::LogLevel SimpleLogger::parseLogLevel(const std::string& level) {
    std::string upperLevel = level;
    std::transform(upperLevel.begin(), upperLevel.end(), upperLevel.begin(), ::toupper);
    
    if (upperLevel == "DEBUG") {
        return DEBUG;
    } else if (upperLevel == "INFO") {
        return INFO;
    } else if (upperLevel == "WARN" || upperLevel == "WARNING") {
        return WARN;
    } else if (upperLevel == "ERROR") {
        return ERROR;
    } else if (upperLevel == "FATAL") {
        return FATAL;
    } else {
        return INFO; // 默认级别
    }
}

} // namespace Common