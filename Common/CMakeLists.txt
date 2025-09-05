cmake_minimum_required(VERSION 3.10)

# 查找 log4cpp 库
find_package(PkgConfig REQUIRED)
pkg_check_modules(LOG4CPP REQUIRED log4cpp)

# 如果没有通过 pkg-config 找到，尝试使用 find_library
if(NOT LOG4CPP_FOUND)
    find_library(LOG4CPP_LIBRARY log4cpp)
    if(LOG4CPP_LIBRARY)
        set(LOG4CPP_LIBRARIES ${LOG4CPP_LIBRARY})
        set(LOG4CPP_FOUND TRUE)
        message(STATUS "Found log4cpp library: ${LOG4CPP_LIBRARY}")
    endif()
endif()

# 如果仍然没有找到，提供安装指导
if(NOT LOG4CPP_FOUND)
    message(FATAL_ERROR "log4cpp library not found. Please install it using:")
    message(FATAL_ERROR "  Ubuntu/Debian: sudo apt-get install liblog4cpp5-dev")
    message(FATAL_ERROR "  CentOS/RHEL: sudo yum install log4cpp-devel")
    message(FATAL_ERROR "  macOS: brew install log4cpp")
endif()

message(STATUS "log4cpp found: ${LOG4CPP_FOUND}")
if(LOG4CPP_FOUND)
    message(STATUS "log4cpp libraries: ${LOG4CPP_LIBRARIES}")
    message(STATUS "log4cpp include dirs: ${LOG4CPP_INCLUDE_DIRS}")
endif()

# 设置包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${LOG4CPP_INCLUDE_DIRS})

# 源文件
set(COMMON_SOURCES
    src/Logger.cpp
    src/SimpleLogger.cpp
)

# 创建静态库
add_library(Common STATIC ${COMMON_SOURCES})

# 链接 log4cpp 库（如果可用）
if(LOG4CPP_FOUND)
    target_link_libraries(Common ${LOG4CPP_LIBRARIES})
endif()

# 设置编译选项
target_compile_options(Common PRIVATE ${LOG4CPP_CFLAGS_OTHER})

# 设置包含目录
target_include_directories(Common PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 安装配置
install(TARGETS Common
    EXPORT CommonTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)

# 创建日志目录
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/logs)

# 创建测试程序
add_executable(test_logger test_logger.cpp)
target_link_libraries(test_logger Common)
set_target_properties(test_logger PROPERTIES
    OUTPUT_NAME "test_logger"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 创建示例程序
add_executable(example_logger example_usage.cpp)
target_link_libraries(example_logger Common)
set_target_properties(example_logger PROPERTIES
    OUTPUT_NAME "example_logger"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 创建简单测试程序
add_executable(simple_test simple_test.cpp)
target_link_libraries(simple_test Common)
set_target_properties(simple_test PROPERTIES
    OUTPUT_NAME "simple_test"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
