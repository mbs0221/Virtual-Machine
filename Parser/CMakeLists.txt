# Parser模块 - 语法分析器
# 拆分为库和程序两部分

# =============================================================================
# Parser库部分
# =============================================================================

# Parser库的源文件（不包含main.cpp）
# 注意：Parser模块目前只包含头文件，没有实现文件
# set(PARSER_LIB_SOURCES
#     lexer.cpp
#     inter.cpp
# )

# 创建静态库（暂时注释掉，因为没有源文件）
# add_library(ParserLib STATIC ${PARSER_LIB_SOURCES})

# 设置库属性（暂时注释掉）
# set_target_properties(ParserLib PROPERTIES
#     OUTPUT_NAME "parser"
# )

# 添加编译选项（暂时注释掉）
# target_compile_options(ParserLib PRIVATE
#     $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
#     $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
#     $<$<CXX_COMPILER_ID:MSVC>:/W3>
# )

# 设置包含目录（暂时注释掉）
# target_include_directories(ParserLib PUBLIC
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
#     $<INSTALL_INTERFACE:include>
# )

# =============================================================================
# Parser程序部分
# =============================================================================

# Parser程序的源文件
set(PARSER_EXE_SOURCES
    main.cpp
)

# 创建可执行文件
add_executable(Parser ${PARSER_EXE_SOURCES})

# 链接Parser库（暂时注释掉）
# target_link_libraries(Parser ParserLib)

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

# =============================================================================
# 安装配置
# =============================================================================

# 安装库（暂时注释掉）
# install(TARGETS ParserLib
#     EXPORT ParserLibTargets
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     RUNTIME DESTINATION bin
#     INCLUDES DESTINATION include
# )

# 安装头文件
install(FILES 
    parser.h
    lexer.h
    inter.h
    DESTINATION include
)

# 安装可执行文件
install(TARGETS Parser
    RUNTIME DESTINATION bin
)

# 打印模块信息
message(STATUS "  Parser module configured")
message(STATUS "    - ParserLib: Static library")
message(STATUS "    - Parser: Executable")