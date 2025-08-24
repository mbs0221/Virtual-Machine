#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Toy架构测试脚本
用于测试Toy架构的汇编器和CPU
"""

import os
import sys
import subprocess
import time
import signal
from pathlib import Path

class ToyTester:
    def __init__(self):
        self.project_dir = Path(__file__).parent
        self.build_dir = self.project_dir / "build"
        self.bin_dir = self.build_dir / "bin"
        self.tests_dir = self.project_dir / "tests"
        
        # 超时设置
        self.timeout = 10  # 10秒超时
        
        # 测试结果
        self.results = {}
    
    def run_command_with_timeout(self, command, timeout=None):
        """运行命令并设置超时"""
        if timeout is None:
            timeout = self.timeout
            
        try:
            result = subprocess.run(
                command,
                timeout=timeout,
                capture_output=True,
                text=True,
                cwd=self.project_dir
            )
            return result
        except subprocess.TimeoutExpired:
            return subprocess.CompletedProcess(
                command, -1, "", f"命令超时 ({timeout}秒)"
            )
        except Exception as e:
            return subprocess.CompletedProcess(
                command, -1, "", f"执行错误: {str(e)}"
            )
    
    def check_executables(self):
        """检查可执行文件是否存在"""
        executables = ["asm", "toy_cpu", "vm"]
        missing = []
        
        for exe in executables:
            exe_path = self.bin_dir / exe
            if exe_path.exists():
                print(f"[SUCCESS] 可执行文件存在: {exe}")
            else:
                print(f"[ERROR] 可执行文件不存在: {exe}")
                missing.append(exe)
        
        return len(missing) == 0
    
    def test_asm_help(self):
        """测试汇编器帮助信息"""
        print("\n=== 测试汇编器帮助信息 ===")
        
        result = self.run_command_with_timeout(["./build/bin/asm", "-h"])
        
        if result.returncode == 0 and "Toy架构汇编器" in result.stdout:
            print("[SUCCESS] 汇编器帮助信息正常")
            return True
        else:
            print(f"[ERROR] 汇编器帮助信息异常: {result.stdout}")
            return False
    
    def test_toy_cpu_help(self):
        """测试Toy CPU帮助信息"""
        print("\n=== 测试Toy CPU帮助信息 ===")
        
        result = self.run_command_with_timeout(["./build/bin/toy_cpu"])
        
        if result.returncode == 1 and "Toy CPU 虚拟机" in result.stdout:
            print("[SUCCESS] Toy CPU帮助信息正常")
            return True
        else:
            print(f"[ERROR] Toy CPU帮助信息异常: {result.stdout}")
            return False
    
    def test_unified_vm_help(self):
        """测试统一VM帮助信息"""
        print("\n=== 测试统一VM帮助信息 ===")
        
        result = self.run_command_with_timeout(["./build/bin/vm"])
        
        if result.returncode == 1 and "多架构CPU模拟器" in result.stdout:
            print("[SUCCESS] 统一VM帮助信息正常")
            return True
        else:
            print(f"[ERROR] 统一VM帮助信息异常: {result.stdout}")
            return False
    
    def test_assembly_pipeline(self):
        """测试汇编流水线"""
        print("\n=== 测试汇编流水线 ===")
        
        # 使用示例程序
        asm_file = self.tests_dir / "toy_example.asm"
        if not asm_file.exists():
            print(f"[ERROR] 示例程序不存在: {asm_file}")
            return False
        
        # 汇编
        print("步骤1: 汇编...")
        result = self.run_command_with_timeout([
            "./build/bin/asm", 
            str(asm_file), 
            "toy_example.bin"
        ])
        
        if result.returncode != 0:
            print(f"[ERROR] 汇编失败: {result.stderr}")
            return False
        
        print("[SUCCESS] 汇编成功")
        
        # 检查输出文件
        bin_file = self.project_dir / "toy_example.bin"
        if not bin_file.exists():
            print(f"[ERROR] 输出文件不存在: {bin_file}")
            return False
        
        print(f"[SUCCESS] 输出文件生成: {bin_file}")
        
        # 使用Toy CPU执行
        print("步骤2: 使用Toy CPU执行...")
        result = self.run_command_with_timeout([
            "./build/bin/toy_cpu", 
            "toy_example.bin"
        ])
        
        if result.returncode == 0:
            print("[SUCCESS] Toy CPU执行成功")
            print("输出:")
            print(result.stdout)
        else:
            print(f"[ERROR] Toy CPU执行失败: {result.stderr}")
            return False
        
        # 使用统一VM执行
        print("步骤3: 使用统一VM执行...")
        result = self.run_command_with_timeout([
            "./build/bin/vm", 
            "toy", 
            "toy_example.bin"
        ])
        
        if result.returncode == 0:
            print("[SUCCESS] 统一VM执行成功")
            print("输出:")
            print(result.stdout)
        else:
            print(f"[ERROR] 统一VM执行失败: {result.stderr}")
            return False
        
        # 清理临时文件
        bin_file.unlink(missing_ok=True)
        
        return True
    
    def test_simple_program(self):
        """测试简单程序"""
        print("\n=== 测试简单程序 ===")
        
        # 创建简单的测试程序
        simple_asm = """# 简单测试程序
data 0
data 0
load $0 #42
halt
"""
        
        simple_file = self.project_dir / "simple_test.asm"
        with open(simple_file, 'w') as f:
            f.write(simple_asm)
        
        # 汇编
        result = self.run_command_with_timeout([
            "./build/bin/asm", 
            "simple_test.asm", 
            "simple_test.bin"
        ])
        
        if result.returncode != 0:
            print(f"[ERROR] 简单程序汇编失败: {result.stderr}")
            simple_file.unlink(missing_ok=True)
            return False
        
        # 执行
        result = self.run_command_with_timeout([
            "./build/bin/toy_cpu", 
            "simple_test.bin"
        ])
        
        if result.returncode == 0:
            print("[SUCCESS] 简单程序执行成功")
        else:
            print(f"[ERROR] 简单程序执行失败: {result.stderr}")
            simple_file.unlink(missing_ok=True)
            (self.project_dir / "simple_test.bin").unlink(missing_ok=True)
            return False
        
        # 清理
        simple_file.unlink(missing_ok=True)
        (self.project_dir / "simple_test.bin").unlink(missing_ok=True)
        
        return True
    
    def run_all_tests(self):
        """运行所有测试"""
        print("=== Toy架构测试开始 ===")
        print(f"项目目录: {self.project_dir}")
        print(f"构建目录: {self.build_dir}")
        print(f"测试目录: {self.tests_dir}")
        print(f"超时设置: {self.timeout}秒")
        
        # 检查构建目录
        if not self.build_dir.exists():
            print("[ERROR] 构建目录不存在，请先运行 build.sh")
            return False
        
        # 检查可执行文件
        if not self.check_executables():
            print("[ERROR] 缺少必要的可执行文件")
            return False
        
        # 测试帮助信息
        tests = [
            ("汇编器帮助", self.test_asm_help),
            ("Toy CPU帮助", self.test_toy_cpu_help),
            ("统一VM帮助", self.test_unified_vm_help),
        ]
        
        for test_name, test_func in tests:
            if not test_func():
                print(f"[ERROR] {test_name}测试失败")
                return False
        
        # 测试汇编流水线
        if not self.test_assembly_pipeline():
            print("[ERROR] 汇编流水线测试失败")
            return False
        
        # 测试简单程序
        if not self.test_simple_program():
            print("[ERROR] 简单程序测试失败")
            return False
        
        print("\n🎉 所有Toy架构测试通过！")
        return True

def main():
    """主函数"""
    tester = ToyTester()
    success = tester.run_all_tests()
    
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
