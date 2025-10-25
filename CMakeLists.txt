cmake_minimum_required(VERSION 3.10)
project(VirtualMachine VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 项目配置信息
message(STATUS "Virtual Machine Project Configuration:")
message(STATUS "  Project: ${PROJECT_NAME}")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Output Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

# 添加子目录
add_subdirectory(Common)
add_subdirectory(Parser)
add_subdirectory(Asm)
add_subdirectory(Optimizer)
add_subdirectory(Pipeline)
add_subdirectory(CPU)
add_subdirectory(Examples)

# 架构模块配置
message(STATUS "  Toy Architecture module configured")

# vm现在由CPU子模块管理

# toy_cpu现在由CPU子模块管理

# 示例程序现在由Examples子模块管理

message(STATUS "Configuring done")
message(STATUS "Generating done")
message(STATUS "Build files have been written to: ${CMAKE_BINARY_DIR}")
