# Asm模块 - 汇编器
set(ASM_SOURCES
    main.cpp
    asm.cpp
    lexer.cpp
    inter.cpp
)

# 创建静态库
add_library(Asm STATIC ${ASM_SOURCES})
target_include_directories(Asm PRIVATE ${CMAKE_SOURCE_DIR}/Common/include)
target_link_libraries(Asm Common)

# 创建可执行文件（用于独立运行）
add_executable(AsmMain main.cpp)
target_include_directories(AsmMain PRIVATE ${CMAKE_SOURCE_DIR}/Common/include)
target_link_libraries(AsmMain Asm)

# 设置库属性
set_target_properties(Asm PROPERTIES
    OUTPUT_NAME "asm"
)

# 设置可执行文件属性
set_target_properties(AsmMain PROPERTIES
    OUTPUT_NAME "asm"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# 添加编译选项
target_compile_options(Asm PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W3>
)

target_compile_options(AsmMain PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W3>
)

# 打印模块信息
message(STATUS "  Asm module configured")
