#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
RV32指令集测试脚本
用于测试RV32 CPU的功能
"""

import os
import sys
import subprocess
import time
import signal
from pathlib import Path

class RV32Tester:
    def __init__(self):
        self.project_dir = Path(__file__).parent
        self.build_dir = self.project_dir / "build"
        self.bin_dir = self.build_dir / "bin"
        self.tests_dir = self.project_dir / "tests"
        
        # 超时设置
        self.timeout = 5  # 5秒超时
        
        # 测试结果
        self.success_count = 0
        self.failure_count = 0
        self.test_results = []
    
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
    
    def check_executable(self, name):
        """检查可执行文件是否存在"""
        exe_path = self.bin_dir / name
        if exe_path.exists():
            print(f"[SUCCESS] 可执行文件存在: {name}")
            return True
        else:
            print(f"[ERROR] 可执行文件不存在: {name}")
            return False
    
    def test_rv32_cpu_basic(self):
        """测试RV32 CPU基本功能"""
        print("\n=== 测试RV32 CPU基本功能 ===")
        
        # 检查RV32 CPU可执行文件
        if not self.check_executable("rv32_cpu"):
            return False
        
        # 测试帮助信息
        print("测试RV32 CPU帮助信息...")
        result = self.run_command_with_timeout(["./build/bin/rv32_cpu"])
        
        if result.returncode == 1 and "用法" in result.stdout:
            print("[SUCCESS] RV32 CPU帮助信息正常")
            return True
        else:
            print(f"[ERROR] RV32 CPU帮助信息异常: {result.stdout}")
            return False
    
    def create_test_program(self):
        """创建测试程序"""
        test_program = [
            0x00A00093,  # li x1, 10
            0x01400113,  # li x2, 20
            0x002081B3,  # add x3, x1, x2
            0x00518213,  # addi x4, x3, 5
            0x0020A2B3,  # slt x5, x1, x2
            0x00000073,  # ecall
        ]
        
        test_file = self.project_dir / "test_program.bin"
        with open(test_file, 'wb') as f:
            for instruction in test_program:
                f.write(instruction.to_bytes(4, byteorder='little'))
        
        return test_file
    
    def test_rv32_execution(self):
        """测试RV32程序执行"""
        print("\n=== 测试RV32程序执行 ===")
        
        # 创建测试程序
        test_file = self.create_test_program()
        
        # 执行测试程序
        print(f"执行测试程序: {test_file}")
        result = self.run_command_with_timeout(["./build/bin/rv32_cpu", str(test_file)])
        
        if result.returncode == 0:
            print("[SUCCESS] RV32程序执行成功")
            print("输出:")
            print(result.stdout)
            return True
        else:
            print(f"[ERROR] RV32程序执行失败: {result.stderr}")
            return False
    
    def test_rv32_instructions(self):
        """测试RV32指令集"""
        print("\n=== 测试RV32指令集 ===")
        
        # 测试各种指令类型
        test_cases = [
            {
                "name": "算术指令",
                "instructions": [
                    0x00A00093,  # li x1, 10
                    0x01400113,  # li x2, 20
                    0x002081B3,  # add x3, x1, x2
                ]
            },
            {
                "name": "逻辑指令",
                "instructions": [
                    0x00A00093,  # li x1, 10
                    0x01400113,  # li x2, 20
                    0x0020C1B3,  # xor x3, x1, x2
                ]
            },
            {
                "name": "分支指令",
                "instructions": [
                    0x00A00093,  # li x1, 10
                    0x01400113,  # li x2, 20
                    0x0020A2B3,  # slt x5, x1, x2
                ]
            }
        ]
        
        success_count = 0
        for test_case in test_cases:
            print(f"测试: {test_case['name']}")
            
            # 创建测试程序
            test_file = self.project_dir / f"test_{test_case['name']}.bin"
            with open(test_file, 'wb') as f:
                for instruction in test_case['instructions']:
                    f.write(instruction.to_bytes(4, byteorder='little'))
                f.write(0x00000073.to_bytes(4, byteorder='little'))  # ecall
            
            # 执行测试
            result = self.run_command_with_timeout(["./build/bin/rv32_cpu", str(test_file)])
            
            if result.returncode == 0:
                print(f"[SUCCESS] {test_case['name']} 测试通过")
                success_count += 1
            else:
                print(f"[ERROR] {test_case['name']} 测试失败: {result.stderr}")
            
            # 清理测试文件
            test_file.unlink(missing_ok=True)
        
        return success_count == len(test_cases)
    
    def run_all_tests(self):
        """运行所有测试"""
        print("=== RV32指令集测试开始 ===")
        print(f"项目目录: {self.project_dir}")
        print(f"构建目录: {self.build_dir}")
        print(f"超时设置: {self.timeout}秒")
        
        # 检查构建目录
        if not self.build_dir.exists():
            print("[ERROR] 构建目录不存在，请先运行 build.sh")
            return False
        
        # 运行测试
        tests = [
            ("RV32 CPU基本功能", self.test_rv32_cpu_basic),
            ("RV32程序执行", self.test_rv32_execution),
            ("RV32指令集", self.test_rv32_instructions),
        ]
        
        for test_name, test_func in tests:
            print(f"\n开始测试: {test_name}")
            try:
                if test_func():
                    self.success_count += 1
                    self.test_results.append((test_name, "SUCCESS"))
                else:
                    self.failure_count += 1
                    self.test_results.append((test_name, "FAILURE"))
            except Exception as e:
                self.failure_count += 1
                self.test_results.append((test_name, f"ERROR: {str(e)}"))
                print(f"[ERROR] 测试异常: {str(e)}")
        
        # 显示测试结果
        self.print_results()
        
        return self.failure_count == 0
    
    def print_results(self):
        """打印测试结果"""
        print("\n=== 测试结果 ===")
        print(f"成功: {self.success_count}")
        print(f"失败: {self.failure_count}")
        print(f"总计: {self.success_count + self.failure_count}")
        
        print("\n详细结果:")
        for test_name, result in self.test_results:
            status = "✓" if result == "SUCCESS" else "✗"
            print(f"  {status} {test_name}: {result}")
        
        if self.failure_count == 0:
            print("\n🎉 所有测试通过！RV32指令集工作正常。")
        else:
            print(f"\n❌ {self.failure_count} 个测试失败，需要修复。")

def main():
    """主函数"""
    tester = RV32Tester()
    success = tester.run_all_tests()
    
    # 清理测试文件
    for test_file in tester.project_dir.glob("test_*.bin"):
        test_file.unlink(missing_ok=True)
    
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
