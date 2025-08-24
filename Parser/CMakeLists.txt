# Parser模块 - 语法分析器
set(PARSER_SOURCES
    main.cpp
)

# 创建可执行文件
add_executable(Parser ${PARSER_SOURCES})

# 设置可执行文件属性
set_target_properties(Parser PROPERTIES
    OUTPUT_NAME "parser"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# 添加编译选项
target_compile_options(Parser PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W3>
)

# 打印模块信息
message(STATUS "  Parser module configured")
