# Pipeline模块 - 编译器流水线统筹协调器
set(PIPELINE_SOURCES
    pipeline_main.cpp
    pipeline.cpp
)

# 创建可执行文件
add_executable(Pipeline ${PIPELINE_SOURCES})

# 设置可执行文件属性
set_target_properties(Pipeline PROPERTIES
    OUTPUT_NAME "pipeline"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# 链接依赖库
target_link_libraries(Pipeline 
    Asm
    Optimizer
)

# 添加编译选项
target_compile_options(Pipeline PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W3>
)

# 打印模块信息
message(STATUS "  Pipeline module configured")
