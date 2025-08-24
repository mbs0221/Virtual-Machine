# CPU模块 - 虚拟机
set(CPU_SOURCES
    main.cpp
    architecture.cpp
    toy.cpp
    rv32.cpp
)

# 创建可执行文件
add_executable(CPU ${CPU_SOURCES})

# 设置可执行文件属性
set_target_properties(CPU PROPERTIES
    OUTPUT_NAME "cpu"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# 添加编译选项
target_compile_options(CPU PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W3>
)

# 打印模块信息
message(STATUS "  CPU module configured")
