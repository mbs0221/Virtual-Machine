# Virtual Machine项目 - CMake构建指南

## 项目结构

```
Virtual-Machine/
├── CMakeLists.txt          # 根目录CMake配置
├── Parser/
│   ├── CMakeLists.txt      # Parser模块配置
│   ├── main.cpp
│   ├── parser.h
│   ├── lexer.h
│   ├── inter.h
│   └── ...
├── Asm/
│   ├── CMakeLists.txt      # Asm模块配置
│   ├── main.cpp
│   ├── vm.cpp
│   ├── asm.h
│   └── ...
├── CPU/
│   ├── CMakeLists.txt      # CPU模块配置
│   ├── main.cpp
│   ├── vm.cpp
│   └── ...
└── build.sh                # 快速构建脚本
```

## 构建方法

### 方法1: 使用构建脚本（推荐）

```bash
# 在项目根目录执行
./build.sh
```

### 方法2: 手动构建

```bash
# 1. 创建构建目录
mkdir build
cd build

# 2. 配置项目
cmake ..

# 3. 构建项目
make -j$(nproc)

# 4. 查看生成的可执行文件
ls bin/
```

## 生成的可执行文件

构建成功后，在 `build/bin/` 目录下会生成以下可执行文件：

- `parser` - 语法分析器
- `asm` - 汇编器  
- `cpu` - 虚拟机

## 命令行参数使用

### Parser模块 (语法分析器)

```bash
# 显示帮助信息
./parser --help

# 使用默认文件
./parser

# 指定输入文件
./parser input.txt

# 指定输入和输出文件
./parser input.txt output.asm
```

**参数说明:**
- 输入文件：要解析的文本文件 (默认: Text.txt)
- 输出文件：生成的汇编文件 (默认: data.asm)

### Asm模块 (汇编器)

```bash
# 显示帮助信息
./asm --help

# 使用默认文件
./asm

# 指定输入文件
./asm input.asm

# 指定输入和输出文件
./asm input.asm output.bin
```

**参数说明:**
- 输入文件：要汇编的文件 (默认: data.asm)
- 输出文件：生成的目标文件 (默认: data.bin)

### CPU模块 (虚拟机)

```bash
# 显示帮助信息
./cpu --help

# 使用默认文件
./cpu

# 指定输入文件
./cpu program.obj
```

**参数说明:**
- 输入文件：要执行的目标文件 (默认: data.obj)

## 完整工作流程示例

```bash
# 1. 语法分析：将文本文件转换为汇编代码
./parser Text.txt data.asm

# 2. 汇编：将汇编代码转换为目标代码
./asm data.asm data.bin

# 3. 执行：虚拟机执行目标代码
./cpu data.bin
```

## 运行示例

```bash
# 进入构建目录
cd build/bin

# 运行语法分析器
./parser

# 运行汇编器
./asm

# 运行虚拟机
./cpu
```

## 清理构建

```bash
# 删除构建目录
rm -rf build

# 或者进入build目录后清理
cd build
make clean
```

## 系统要求

- CMake 3.10 或更高版本
- C++11 兼容的编译器（GCC、Clang、MSVC等）
- Make 工具

## 故障排除

1. **CMake版本过低**: 升级CMake到3.10或更高版本
2. **编译器不支持C++11**: 升级编译器或安装支持C++11的编译器
3. **权限问题**: 确保构建脚本有执行权限 `chmod +x build.sh`

## 模块说明

### Parser模块
- 功能：语法分析器，将文本文件解析为汇编代码
- 输入：Text.txt
- 输出：data.asm

### Asm模块  
- 功能：汇编器，将汇编代码编译为目标代码
- 输入：data.asm
- 输出：data.bin

### CPU模块
- 功能：虚拟机，执行目标代码
- 输入：data.obj
- 输出：执行结果
