#!/bin/bash

# Virtual Machine项目构建脚本

echo "=== Virtual Machine项目构建脚本 ==="

# 创建构建目录
if [ ! -d "build" ]; then
    echo "创建构建目录..."
    mkdir build
fi

# 进入构建目录
cd build

# 配置项目
echo "配置项目..."
cmake ..

# 检查配置是否成功
if [ $? -eq 0 ]; then
    echo "配置成功！"
    
    # 构建项目
    echo "构建项目..."
    make -j$(nproc)
    
    # 检查构建是否成功
    if [ $? -eq 0 ]; then
        echo "构建成功！"
        echo "可执行文件位置: $(pwd)/bin/"
        echo "可用的可执行文件:"
        ls -la bin/
    else
        echo "构建失败！"
        exit 1
    fi
else
    echo "配置失败！"
    exit 1
fi
