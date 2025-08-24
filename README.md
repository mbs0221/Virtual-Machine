# Parser-Asm-VM
一个编译程序，一个汇编程序和一个简单的16位虚拟机，一条龙实现从源代码到汇编程序再到虚拟机机器指令程序的转换过程，并可以在该虚拟机上运行该虚拟机机器指令程序

## 项目结构
- **Parser**: 语法分析器，将源代码转换为汇编代码
- **Asm**: 汇编器，将汇编代码转换为机器码
- **CPU**: 虚拟机，执行机器码

## Linux构建说明

### 系统要求
- Linux操作系统
- GCC编译器 (g++)
- Make工具

### 安装依赖
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential

# CentOS/RHEL
sudo yum groupinstall "Development Tools"
```

### 构建项目
```bash
# 使用构建脚本（推荐）
./build.sh

# 或者使用Make
make all
```

### 运行测试
```bash
make test
```

### 清理构建文件
```bash
make clean
```

### 安装到系统
```bash
make install
```

## 使用说明

1. **编译源代码**：
   ```bash
   ./bin/parser
   ```

2. **汇编代码**：
   ```bash
   ./bin/asm
   ```

3. **执行虚拟机**：
   ```bash
   ./bin/cpu
   ```

## 分支说明
- `VM-1.2`: 原始Windows版本
- `linux-support`: Linux支持版本，包含Makefile构建系统

## 注意事项
- 本项目已从Visual Studio项目转换为Linux兼容的Makefile构建系统
- 修复了Windows特定的代码（如`fopen_s`、`void main()`等）
- 支持在Linux环境下编译和运行
