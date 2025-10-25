# Optimizer模块 - 代码优化器
set(OPTIMIZER_SOURCES
    tac.cpp
    ast_to_tac.cpp
    ast_to_tac_impl.cpp
    tac_from_asm.cpp
    cfg.cpp
    generator.cpp
    register_allocator.cpp
    optimizer.cpp
)

# 创建静态库
add_library(Optimizer STATIC ${OPTIMIZER_SOURCES})

# 设置库属性
set_target_properties(Optimizer PROPERTIES
    OUTPUT_NAME "optimizer"
)

# 添加编译选项
target_compile_options(Optimizer PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W3>
)

# 打印模块信息
message(STATUS "  Optimizer module configured")
