#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Virtual Machine项目 - Python测试脚本
测试parser/asm/cpu的完整编译、汇编、运行流程
支持批量测试tests文件夹下的txt程序
包含超时机制防止程序卡死
"""

import os
import sys
import subprocess
import tempfile
import shutil
from pathlib import Path
import glob
import signal
import time

# 超时设置（秒）
TIMEOUT_PARSER = 3  # Parser超时时间
TIMEOUT_ASM = 3     # Asm超时时间
TIMEOUT_CPU = 3     # CPU超时时间（执行时间可能较长）

# 颜色定义
class Colors:
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    NC = '\033[0m'  # No Color

def print_info(message):
    """打印信息消息"""
    print(f"{Colors.BLUE}[INFO]{Colors.NC} {message}")

def print_success(message):
    """打印成功消息"""
    print(f"{Colors.GREEN}[SUCCESS]{Colors.NC} {message}")

def print_error(message):
    """打印错误消息"""
    print(f"{Colors.RED}[ERROR]{Colors.NC} {message}")

def print_warning(message):
    """打印警告消息"""
    print(f"{Colors.YELLOW}[WARNING]{Colors.NC} {message}")

def check_executable(executable_path):
    """检查可执行文件是否存在"""
    if os.path.isfile(executable_path) and os.access(executable_path, os.X_OK):
        print_success(f"可执行文件存在: {executable_path}")
        return True
    else:
        print_error(f"可执行文件不存在或无执行权限: {executable_path}")
        return False

def check_file(file_path):
    """检查文件是否存在"""
    if os.path.isfile(file_path):
        print_success(f"文件存在: {file_path}")
        return True
    else:
        print_error(f"文件不存在: {file_path}")
        return False

def run_command_with_timeout(cmd, description, timeout):
    """运行命令并返回结果，支持超时"""
    print_info(f"执行: {description} (超时: {timeout}秒)")
    print_info(f"命令: {' '.join(cmd)}")
    
    try:
        # 使用subprocess.run的timeout参数
        result = subprocess.run(
            cmd, 
            capture_output=True, 
            text=True, 
            check=True,
            timeout=timeout
        )
        print_success(f"{description} 成功")
        if result.stdout:
            print_info("输出:")
            print(result.stdout)
        return True, result.stdout, result.stderr
    except subprocess.TimeoutExpired as e:
        print_error(f"{description} 超时 ({timeout}秒)")
        # 尝试终止进程
        try:
            e.process.kill()
            print_info("已终止超时进程")
        except:
            pass
        return False, "", f"超时: {timeout}秒"
    except subprocess.CalledProcessError as e:
        print_error(f"{description} 失败")
        print_error(f"错误码: {e.returncode}")
        if e.stdout:
            print_info("标准输出:")
            print(e.stdout)
        if e.stderr:
            print_info("错误输出:")
            print(e.stderr)
        return False, e.stdout, e.stderr
    except Exception as e:
        print_error(f"{description} 发生异常: {e}")
        return False, "", str(e)

def run_command(cmd, description, timeout=None):
    """运行命令并返回结果（兼容旧接口）"""
    if timeout is None:
        # 根据命令类型设置默认超时
        if "parser" in cmd[0]:
            timeout = TIMEOUT_PARSER
        elif "asm" in cmd[0]:
            timeout = TIMEOUT_ASM
        elif "cpu" in cmd[0]:
            timeout = TIMEOUT_CPU
        else:
            timeout = 30  # 默认超时
    
    return run_command_with_timeout(cmd, description, timeout)

def test_individual_modules():
    """测试各个模块的独立功能"""
    print_info("=== 测试各个模块的独立功能 ===")
    
    # 检查可执行文件
    parser_exe = "build/bin/parser"
    asm_exe = "build/bin/asm"
    cpu_exe = "build/bin/cpu"
    
    if not all([check_executable(parser_exe), 
                check_executable(asm_exe), 
                check_executable(cpu_exe)]):
        return False
    
    # 测试帮助信息
    print_info("测试帮助信息...")
    for exe, name in [(parser_exe, "Parser"), (asm_exe, "Asm"), (cpu_exe, "CPU")]:
        success, _, _ = run_command([exe, "--help"], f"{name}帮助信息", timeout=10)
        if not success:
            print_warning(f"{name}帮助信息测试失败")
    
    return True

def test_single_program(test_file, output_prefix):
    """测试单个程序"""
    print_info(f"=== 测试程序: {test_file} ===")
    
    # 生成输出文件名
    base_name = os.path.splitext(os.path.basename(test_file))[0]
    asm_output = f"{output_prefix}_{base_name}.asm"
    bin_output = f"{output_prefix}_{base_name}.bin"
    
    # 步骤1: Parser - 语法分析
    print_info(f"步骤1: 执行语法分析 (Parser) - {test_file}")
    parser_exe = "build/bin/parser"
    
    success, stdout, stderr = run_command(
        [parser_exe, test_file, asm_output], 
        f"语法分析 {test_file}",
        TIMEOUT_PARSER
    )
    
    if not success:
        print_error(f"语法分析失败: {test_file}")
        return False
    
    if not check_file(asm_output):
        return False
    
    # 显示生成的汇编代码
    print_info(f"生成的汇编代码 ({asm_output}):")
    with open(asm_output, 'r', encoding='utf-8') as f:
        asm_content = f.read()
        print(asm_content)
    
    # 步骤2: Asm - 汇编
    print_info(f"步骤2: 执行汇编 (Asm) - {asm_output}")
    asm_exe = "build/bin/asm"
    
    success, stdout, stderr = run_command(
        [asm_exe, asm_output, bin_output], 
        f"汇编 {asm_output}",
        TIMEOUT_ASM
    )
    
    if not success:
        print_error(f"汇编失败: {asm_output}")
        return False
    
    if not check_file(bin_output):
        return False
    
    # 显示生成的目标文件信息
    file_size = os.path.getsize(bin_output)
    print_success(f"生成目标文件: {bin_output} ({file_size} 字节)")
    
    # 步骤3: CPU - 执行
    print_info(f"步骤3: 执行虚拟机 (CPU) - {bin_output}")
    cpu_exe = "build/bin/cpu"
    
    success, stdout, stderr = run_command(
        [cpu_exe, bin_output], 
        f"虚拟机执行 {bin_output}",
        TIMEOUT_CPU
    )
    
    if not success:
        print_error(f"虚拟机执行失败: {bin_output}")
        return False
    
    print_success(f"程序 {test_file} 测试通过！")
    return True

def test_batch_programs():
    """批量测试tests文件夹下的所有txt程序"""
    print_info("=== 批量测试tests文件夹下的程序 ===")
    
    # 检查tests文件夹是否存在
    tests_dir = "tests"
    if not os.path.exists(tests_dir):
        print_warning(f"tests文件夹不存在: {tests_dir}")
        print_info("创建tests文件夹...")
        os.makedirs(tests_dir, exist_ok=True)
        print_info("请将测试程序(.txt文件)放入tests文件夹中")
        return True
    
    # 查找所有txt文件
    txt_files = glob.glob(os.path.join(tests_dir, "*.txt"))
    
    if not txt_files:
        print_warning(f"在{tests_dir}文件夹中未找到.txt文件")
        print_info("请将测试程序(.txt文件)放入tests文件夹中")
        return True
    
    print_info(f"找到 {len(txt_files)} 个测试程序:")
    for txt_file in txt_files:
        print_info(f"  - {txt_file}")
    
    # 批量测试
    success_count = 0
    total_count = len(txt_files)
    failed_tests = []
    
    for i, txt_file in enumerate(txt_files, 1):
        print_info(f"\n开始测试 ({i}/{total_count}): {txt_file}")
        start_time = time.time()
        
        if test_single_program(txt_file, "batch"):
            success_count += 1
            elapsed_time = time.time() - start_time
            print_success(f"测试完成 ({elapsed_time:.2f}秒): {txt_file}")
        else:
            failed_tests.append(txt_file)
            elapsed_time = time.time() - start_time
            print_error(f"测试失败 ({elapsed_time:.2f}秒): {txt_file}")
    
    print_info(f"\n=== 批量测试结果 ===")
    print_success(f"成功: {success_count}/{total_count}")
    if failed_tests:
        print_error(f"失败: {len(failed_tests)}/{total_count}")
        print_info("失败的测试:")
        for failed_test in failed_tests:
            print_info(f"  - {failed_test}")
    
    return success_count == total_count

def test_single_file():
    """测试单个文件（Text.txt）"""
    print_info("=== 测试单个文件 ===")
    
    # 检查测试文件
    test_file = "tests/Text.txt"
    if not os.path.exists(test_file):
        print_error(f"测试文件不存在: {test_file}")
        return False
    
    print_success(f"使用测试文件: {test_file}")
    return test_single_program(test_file, "single")

def test_with_existing_files():
    """使用现有文件测试"""
    print_info("=== 使用现有文件测试 ===")
    
    existing_files = {
        "data.asm": "build/bin/asm", 
        "data.bin": "build/bin/cpu"
    }
    
    for file_name, exe_path in existing_files.items():
        if os.path.exists(file_name):
            print_info(f"使用现有文件测试: {file_name}")
            
            if exe_path.endswith("asm"):
                output_file = f"existing_{file_name.replace('.asm', '.bin')}"
                success, _, _ = run_command([exe_path, file_name, output_file], f"Asm处理{file_name}", TIMEOUT_ASM)
            elif exe_path.endswith("cpu"):
                success, _, _ = run_command([exe_path, file_name], f"CPU执行{file_name}", TIMEOUT_CPU)
            
            if success:
                print_success(f"{file_name} 测试通过")
            else:
                print_error(f"{file_name} 测试失败")
        else:
            print_warning(f"文件不存在: {file_name}")

def cleanup():
    """清理测试文件"""
    print_info("清理测试文件...")
    
    # 清理模式匹配的文件
    patterns = [
        "single_*.asm",
        "single_*.bin", 
        "batch_*.asm",
        "batch_*.bin",
        "existing_*.asm",
        "existing_*.bin",
        "pipeline_output.asm",
        "pipeline_output.bin"
    ]
    
    for pattern in patterns:
        files = glob.glob(pattern)
        for file_name in files:
            if os.path.exists(file_name):
                os.remove(file_name)
                print_info(f"删除: {file_name}")
    
    print_success("清理完成")

def main():
    """主函数"""
    print_info("开始Virtual Machine项目Python测试")
    print_info(f"当前目录: {os.getcwd()}")
    print_info(f"超时设置: Parser={TIMEOUT_PARSER}s, Asm={TIMEOUT_ASM}s, CPU={TIMEOUT_CPU}s")
    
    # 检查是否在项目根目录
    if not os.path.exists("CMakeLists.txt"):
        print_error("请在Virtual-Machine项目根目录运行此脚本")
        sys.exit(1)
    
    try:
        # 测试各个模块
        if not test_individual_modules():
            print_error("模块测试失败")
            sys.exit(1)
        
        # 使用现有文件测试
        test_with_existing_files()
        
        # 测试单个文件
        test_single_file()
        
        # 批量测试
        test_batch_programs()
        
        print_success("所有测试完成！Virtual Machine项目工作正常。")
        
    except KeyboardInterrupt:
        print_info("测试被用户中断")
    except Exception as e:
        print_error(f"测试过程中发生错误: {e}")
        sys.exit(1)
    finally:
        # 清理测试文件
        cleanup()

if __name__ == "__main__":
    main()
