# CPU模块 - 虚拟机
set(CPU_SOURCES
    main.cpp
    architecture.cpp
    toy.cpp
    mmio.cpp
    memory.cpp
    register.cpp
    alu.cpp
    nes_optimizer.cpp
    mmu.cpp
    tlb.cpp
    cache.cpp
    cache_manager.cpp
)

# 创建可执行文件
add_executable(CPU ${CPU_SOURCES})

# 创建统一的可执行文件
add_executable(vm main.cpp architecture.cpp toy.cpp mmio.cpp memory.cpp register.cpp alu.cpp nes_optimizer.cpp mmu.cpp tlb.cpp cache.cpp cache_manager.cpp)

# 创建独立的Toy CPU可执行文件
add_executable(toy_cpu toy_main.cpp toy.cpp architecture.cpp mmio.cpp memory.cpp register.cpp alu.cpp nes_optimizer.cpp mmu.cpp tlb.cpp cache.cpp cache_manager.cpp)

# 设置包含目录
target_include_directories(CPU PRIVATE ${CMAKE_SOURCE_DIR}/Common/include)
target_include_directories(vm PRIVATE ${CMAKE_SOURCE_DIR}/Common/include)
target_include_directories(toy_cpu PRIVATE ${CMAKE_SOURCE_DIR}/Common/include)

# 链接Common库
target_link_libraries(CPU Common)
target_link_libraries(vm Common)
target_link_libraries(toy_cpu Common)

# 设置可执行文件属性
set_target_properties(CPU PROPERTIES
    OUTPUT_NAME "cpu"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

set_target_properties(vm PROPERTIES
    OUTPUT_NAME "vm"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

set_target_properties(toy_cpu PROPERTIES
    OUTPUT_NAME "toy_cpu"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# 添加编译选项
target_compile_options(CPU PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W3>
)

target_compile_options(vm PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W3>
)

target_compile_options(toy_cpu PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W3>
)

# 打印模块信息
message(STATUS "  CPU module configured")
