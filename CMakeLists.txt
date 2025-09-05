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
add_subdirectory(Parser)
add_subdirectory(Asm)
add_subdirectory(CPU)

# 架构模块配置
message(STATUS "  Toy Architecture module configured")

# 创建统一的可执行文件
add_executable(vm CPU/main.cpp CPU/architecture.cpp CPU/toy.cpp)
target_include_directories(vm PRIVATE CPU)

# 设置VM的属性
set_target_properties(vm PROPERTIES
    OUTPUT_NAME "vm"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# 编译选项
target_compile_options(vm PRIVATE -Wall -Wextra)

# 创建独立的Toy CPU可执行文件
add_executable(toy_cpu CPU/toy_main.cpp CPU/toy.cpp CPU/architecture.cpp)
target_include_directories(toy_cpu PRIVATE CPU)
set_target_properties(toy_cpu PROPERTIES
    OUTPUT_NAME "toy_cpu"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
target_compile_options(toy_cpu PRIVATE -Wall -Wextra)


message(STATUS "Configuring done")
message(STATUS "Generating done")
message(STATUS "Build files have been written to: ${CMAKE_BINARY_DIR}")
