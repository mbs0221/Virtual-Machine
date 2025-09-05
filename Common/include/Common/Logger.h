#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <memory>

// 定义 USE_SIMPLE_LOGGER 来使用简化版本，避免 log4cpp 的析构问题
#define USE_SIMPLE_LOGGER

#ifdef USE_SIMPLE_LOGGER
#include "Common/SimpleLogger.h"
#else
#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/PropertyConfigurator.hh>
#endif

namespace Common {

/**
 * @brief 日志管理器类，基于 log4cpp 提供统一的日志记录功能
 */
class Logger {
public:
    /**
     * @brief 获取日志管理器单例实例
     * @return Logger& 日志管理器引用
     */
    static Logger& getInstance();

    /**
     * @brief 初始化日志系统
     * @param configFile 配置文件路径，如果为空则使用默认配置
     * @return bool 初始化是否成功
     */
    bool initialize(const std::string& configFile = "");

    /**
     * @brief 使用默认配置初始化日志系统
     * @param logLevel 日志级别 (DEBUG, INFO, WARN, ERROR, FATAL)
     * @param logFile 日志文件路径，如果为空则只输出到控制台
     * @return bool 初始化是否成功
     */
    bool initializeDefault(const std::string& logLevel = "INFO", 
                          const std::string& logFile = "");

    /**
     * @brief 获取指定模块的日志记录器
     * @param module 模块名称
     * @return 日志记录器引用
     */
#ifdef USE_SIMPLE_LOGGER
    SimpleLogger& getLogger(const std::string& module);
#else
    log4cpp::Category& getLogger(const std::string& module);
#endif

    /**
     * @brief 设置全局日志级别
     * @param level 日志级别
     */
    void setLogLevel(const std::string& level);

    /**
     * @brief 关闭日志系统
     */
    void shutdown();

    /**
     * @brief 检查是否已初始化
     * @return bool 是否已初始化
     */
    bool isInitialized() const { return initialized_; }

private:
    Logger() = default;
    ~Logger() {
        // 在析构函数中不调用 shutdown，避免重复清理
        initialized_ = false;
    }
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    bool initialized_;
    std::string configFile_;
#ifdef USE_SIMPLE_LOGGER
    // 简化版本不需要 parseLogLevel 方法
#else
    log4cpp::Priority::PriorityLevel parseLogLevel(const std::string& level);
#endif
};

/**
 * @brief 便捷的日志宏定义
 */
#ifdef USE_SIMPLE_LOGGER
#define LOG_DEBUG(module, message) \
    Common::Logger::getInstance().getLogger(module).debug(module, message)

#define LOG_INFO(module, message) \
    Common::Logger::getInstance().getLogger(module).info(module, message)

#define LOG_WARN(module, message) \
    Common::Logger::getInstance().getLogger(module).warn(module, message)

#define LOG_ERROR(module, message) \
    Common::Logger::getInstance().getLogger(module).error(module, message)

#define LOG_FATAL(module, message) \
    Common::Logger::getInstance().getLogger(module).fatal(module, message)

/**
 * @brief 带格式化的日志宏定义（简化版本不支持格式化，直接输出）
 */
#define LOG_DEBUG_FMT(module, format, ...) \
    Common::Logger::getInstance().getLogger(module).debug(module, format)

#define LOG_INFO_FMT(module, format, ...) \
    Common::Logger::getInstance().getLogger(module).info(module, format)

#define LOG_WARN_FMT(module, format, ...) \
    Common::Logger::getInstance().getLogger(module).warn(module, format)

#define LOG_ERROR_FMT(module, format, ...) \
    Common::Logger::getInstance().getLogger(module).error(module, format)

#define LOG_FATAL_FMT(module, format, ...) \
    Common::Logger::getInstance().getLogger(module).fatal(module, format)
#else
#define LOG_DEBUG(module, message) \
    Common::Logger::getInstance().getLogger(module).debug(message)

#define LOG_INFO(module, message) \
    Common::Logger::getInstance().getLogger(module).info(message)

#define LOG_WARN(module, message) \
    Common::Logger::getInstance().getLogger(module).warn(message)

#define LOG_ERROR(module, message) \
    Common::Logger::getInstance().getLogger(module).error(message)

#define LOG_FATAL(module, message) \
    Common::Logger::getInstance().getLogger(module).fatal(message)

/**
 * @brief 带格式化的日志宏定义
 */
#define LOG_DEBUG_FMT(module, format, ...) \
    Common::Logger::getInstance().getLogger(module).debug(format, ##__VA_ARGS__)

#define LOG_INFO_FMT(module, format, ...) \
    Common::Logger::getInstance().getLogger(module).info(format, ##__VA_ARGS__)

#define LOG_WARN_FMT(module, format, ...) \
    Common::Logger::getInstance().getLogger(module).warn(format, ##__VA_ARGS__)

#define LOG_ERROR_FMT(module, format, ...) \
    Common::Logger::getInstance().getLogger(module).error(format, ##__VA_ARGS__)

#define LOG_FATAL_FMT(module, format, ...) \
    Common::Logger::getInstance().getLogger(module).fatal(format, ##__VA_ARGS__)
#endif

} // namespace Common

#endif // LOGGER_H
