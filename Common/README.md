# Common 库

Common 库为虚拟机项目提供通用的功能，包括基于 log4cpp 的日志管理系统。

## 功能特性

- **统一日志管理**: 基于 log4cpp 提供统一的日志记录功能
- **模块化日志**: 支持不同模块的独立日志配置
- **多种输出方式**: 支持控制台和文件输出
- **灵活配置**: 支持配置文件和代码配置两种方式
- **便捷宏定义**: 提供简化的日志记录宏

## 依赖

- log4cpp 库
- C++11 或更高版本

### 安装 log4cpp

**Ubuntu/Debian:**
```bash
sudo apt-get install liblog4cpp5-dev
```

**CentOS/RHEL:**
```bash
sudo yum install log4cpp-devel
```

**macOS:**
```bash
brew install log4cpp
```

## 使用方法

### 1. 基本使用

```cpp
#include "Common/Logger.h"

int main() {
    // 初始化日志系统
    Common::Logger& logger = Common::Logger::getInstance();
    logger.initializeDefault("INFO", "logs/app.log");
    
    // 获取模块日志记录器
    log4cpp::Category& cpuLogger = logger.getLogger("CPU");
    
    // 记录日志
    cpuLogger.info("CPU module initialized");
    cpuLogger.debug("Register R0 = 0x%04X", 0x1234);
    cpuLogger.error("Memory access error at 0x%04X", 0xFFFF);
    
    return 0;
}
```

### 2. 使用便捷宏

```cpp
#include "Common/Logger.h"

// 初始化日志系统
Common::Logger::getInstance().initializeDefault("DEBUG");

// 使用宏记录日志
LOG_INFO("CPU", "Instruction executed: MOV R0, R1");
LOG_DEBUG_FMT("Asm", "Parsing instruction: %s at line %d", "LOAD", 10);
LOG_ERROR("MMIO", "Device not found at address 0x%04X", 0xF020);
```

### 3. 使用配置文件

```cpp
#include "Common/Logger.h"

// 使用配置文件初始化
Common::Logger::getInstance().initialize("log4cpp.conf");
```

## 日志级别

支持的日志级别（从低到高）：
- `DEBUG`: 调试信息
- `INFO`: 一般信息
- `WARN`: 警告信息
- `ERROR`: 错误信息
- `FATAL`: 致命错误

## 配置文件格式

配置文件使用 log4cpp 的标准格式：

```
# 根日志记录器配置
log4cpp.rootCategory=INFO, console, file

# 控制台输出器配置
log4cpp.appender.console=ConsoleAppender
log4cpp.appender.console.layout=PatternLayout
log4cpp.appender.console.layout.ConversionPattern=%d{%Y-%m-%d %H:%M:%S.%l} [%p] %c: %m%n

# 文件输出器配置
log4cpp.appender.file=FileAppender
log4cpp.appender.file.fileName=logs/vm.log
log4cpp.appender.file.layout=PatternLayout
log4cpp.appender.file.layout.ConversionPattern=%d{%Y-%m-%d %H:%M:%S.%l} [%p] %c: %m%n

# 模块特定配置
log4cpp.category.CPU=INFO
log4cpp.category.Asm=DEBUG
log4cpp.category.MMIO=DEBUG
```

## 模块命名约定

建议使用以下模块名称：
- `CPU`: CPU 相关日志
- `Asm`: 汇编器相关日志
- `Parser`: 解析器相关日志
- `Optimizer`: 优化器相关日志
- `Pipeline`: 流水线相关日志
- `MMIO`: MMIO 设备相关日志
- `Interrupt`: 中断处理相关日志

## 编译

Common 库会自动被主项目的 CMakeLists.txt 包含和编译。确保已安装 log4cpp 库后，直接编译主项目即可。

```bash
mkdir build
cd build
cmake ..
make
```

## 示例

查看 `example_usage.cpp` 文件了解完整的使用示例。
