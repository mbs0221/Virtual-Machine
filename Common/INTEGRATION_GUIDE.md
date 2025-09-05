# Common 库集成指南

## 概述

Common 库为虚拟机项目提供了统一的日志管理功能。由于 log4cpp 在某些环境下存在析构问题，我们实现了一个基于标准库的 SimpleLogger 作为默认选择，同时保留了 log4cpp 的支持。

## 功能特性

- **统一日志管理**: 提供一致的日志记录接口
- **模块化日志**: 支持不同模块的独立日志配置
- **多种输出方式**: 支持控制台和文件输出
- **线程安全**: 使用互斥锁保证多线程安全
- **灵活配置**: 支持运行时日志级别调整
- **便捷宏定义**: 提供简化的日志记录宏

## 使用方法

### 1. 基本集成

在任何需要日志的源文件中包含头文件：

```cpp
#include "Common/Logger.h"
```

### 2. 初始化日志系统

在程序开始时初始化日志系统：

```cpp
// 使用默认配置初始化
Common::Logger::getInstance().initializeDefault("INFO", "logs/app.log");

// 或者使用配置文件初始化（仅 log4cpp 版本）
Common::Logger::getInstance().initialize("log4cpp.conf");
```

### 3. 记录日志

使用便捷宏记录日志：

```cpp
// 基本日志记录
LOG_INFO("CPU", "CPU module initialized");
LOG_DEBUG("Asm", "Parsing instruction: LOAD");
LOG_WARN("MMIO", "Device not responding");
LOG_ERROR("CPU", "Memory access violation at 0x%04X", address);

// 格式化日志记录（注意：SimpleLogger 版本不支持格式化参数）
LOG_INFO_FMT("CPU", "Register R%d = 0x%04X", regNum, value);
```

### 4. 运行时配置

```cpp
// 动态调整日志级别
Common::Logger::getInstance().setLogLevel("DEBUG");

// 检查初始化状态
if (Common::Logger::getInstance().isInitialized()) {
    LOG_INFO("App", "Logger is ready");
}
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

## 配置选项

### 日志级别

支持的日志级别（从低到高）：
- `DEBUG`: 调试信息
- `INFO`: 一般信息
- `WARN`: 警告信息
- `ERROR`: 错误信息
- `FATAL`: 致命错误

### 输出格式

日志输出格式：
```
2025-09-06 06:08:03 [INFO ] CPU: ToyCPU constructor called
```

格式说明：
- 时间戳：`YYYY-MM-DD HH:MM:SS`
- 日志级别：`[LEVEL]`
- 模块名称：`MODULE`
- 消息内容：`message`

## 版本选择

### SimpleLogger（默认）

- 基于标准库实现
- 避免 log4cpp 的析构问题
- 支持基本的日志功能
- 线程安全
- 不支持格式化参数

### log4cpp 版本

- 功能更强大
- 支持配置文件
- 支持格式化参数
- 可能存在析构问题

要切换到 log4cpp 版本，在 `Common/include/Common/Logger.h` 中注释掉：
```cpp
// #define USE_SIMPLE_LOGGER
```

## 集成示例

### CPU 模块集成

```cpp
#include "Common/Logger.h"

ToyCPU::ToyCPU() {
    // 初始化日志系统
    Common::Logger::getInstance().initializeDefault("INFO", "logs/cpu.log");
    LOG_INFO("CPU", "ToyCPU constructor called");
    
    // 其他初始化代码...
}

void ToyCPU::execute() {
    LOG_DEBUG("CPU", "Executing instruction at PC=0x%04X", PC);
    // 执行指令...
}
```

### 汇编器模块集成

```cpp
#include "Common/Logger.h"

Asm::Asm() {
    Common::Logger::getInstance().initializeDefault("DEBUG", "logs/asm.log");
    LOG_INFO("Asm", "Assembler initialized");
}

void Asm::parse() {
    LOG_DEBUG("Asm", "Starting parse of assembly file");
    // 解析代码...
    if (error_count > 0) {
        LOG_ERROR("Asm", "Parse completed with %d errors", error_count);
    } else {
        LOG_INFO("Asm", "Parse completed successfully");
    }
}
```

## 编译配置

确保在 CMakeLists.txt 中正确链接 Common 库：

```cmake
target_link_libraries(your_target Common)
```

## 注意事项

1. **线程安全**: SimpleLogger 使用互斥锁保证线程安全
2. **性能**: 日志记录会有一定的性能开销，生产环境建议使用 INFO 或更高级别
3. **文件权限**: 确保程序有权限在指定目录创建日志文件
4. **磁盘空间**: 定期清理日志文件，避免占用过多磁盘空间

## 故障排除

### 常见问题

1. **编译错误**: 确保包含了正确的头文件路径
2. **链接错误**: 确保链接了 Common 库
3. **日志文件无法创建**: 检查目录权限和磁盘空间
4. **段错误**: 如果使用 log4cpp 版本，尝试切换到 SimpleLogger

### 调试技巧

1. 使用 `DEBUG` 级别获取详细信息
2. 检查日志文件内容确认日志记录正常
3. 使用 `isInitialized()` 检查日志系统状态
