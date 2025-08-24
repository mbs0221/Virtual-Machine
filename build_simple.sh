#!/bin/bash

echo "================================================="
echo "            Virtual-Machine 简化构建              "
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
find . -name "*.o" -delete
find . -name "parser" -delete
find . -name "asm" -delete
find . -name "cpu" -delete

# 编译CPU项目（最简单的）
echo "编译CPU项目..."
cd CPU
g++ -std=c++11 -Wall -Wextra -O2 -c vm.cpp -o vm.o
g++ -std=c++11 -Wall -Wextra -O2 -c main.cpp -o main.o
g++ vm.o main.o -o cpu
cd ..

# 检查编译结果
if [ -f "CPU/cpu" ]; then
    echo "CPU编译成功！"
    cp CPU/cpu bin/
else
    echo "CPU编译失败！"
fi

echo ""
echo "构建完成！"
echo "生成的可执行文件:"
ls -la bin/ 2>/dev/null || echo "没有生成可执行文件"

echo ""
echo "注意：Parser和Asm项目需要进一步修复编译错误"
echo "当前只编译了CPU虚拟机部分"
