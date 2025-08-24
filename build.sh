#!/bin/bash

echo "================================================="
echo "            Virtual-Machine Linux 构建            "
echo "================================================="

# 检查编译器
if ! command -v g++ &> /dev/null; then
    echo "错误: 未找到 g++ 编译器"
    echo "请安装 g++: sudo apt-get install g++"
    exit 1
fi

# 创建必要的目录
mkdir -p bin

# 清理之前的构建
echo "清理之前的构建..."
make clean

# 编译项目
echo "开始编译项目..."
make all

# 检查编译结果
if [ $? -eq 0 ]; then
    echo "编译成功！"
    echo ""
    echo "生成的可执行文件:"
    ls -la bin/
    echo ""
    echo "运行测试..."
    make test
else
    echo "编译失败！"
    exit 1
fi
