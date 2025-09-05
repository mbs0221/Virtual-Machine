#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
架构测试脚本
用于测试Toy架构
"""

import os
import sys
import subprocess
import time
import signal
from pathlib import Path

class ArchitectureTester:
    def __init__(self):
        self.project_dir = Path(__file__).parent
        self.build_dir = self.project_dir / "build"
        self.bin_dir = self.build_dir / "bin"
        
        # 超时设置
        self.timeout = 5  # 5秒超时
        
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
        executables = ["vm", "toy_cpu"]
        missing = []
        
        for exe in executables:
            exe_path = self.bin_dir / exe
            if exe_path.exists():
                print(f"[SUCCESS] 可执行文件存在: {exe}")
            else:
                print(f"[ERROR] 可执行文件不存在: {exe}")
                missing.append(exe)
        
        return len(missing) == 0
    
    def test_architecture_info(self):
        """测试架构信息显示"""
        print("\n=== 测试架构信息显示 ===")
        
        # 测试统一VM的帮助信息
        print("测试统一VM的帮助信息...")
        result = self.run_command_with_timeout(["./build/bin/vm"])
        
        if result.returncode == 1 and "多架构CPU模拟器" in result.stdout:
            print("[SUCCESS] 统一VM帮助信息正常")
            return True
        else:
            print(f"[ERROR] 统一VM帮助信息异常: {result.stdout}")
            return False
    
    def create_test_programs(self):
        """创建测试程序"""
        test_programs = {
            "toy": {
                "name": "Toy测试程序",
                "description": "简单的Toy架构测试程序",
                "instructions": [
                    0x00,  # HALT
                ]
            },
        }
        
        created_files = {}
        
        for arch, program in test_programs.items():
            test_file = self.project_dir / f"test_{arch}.bin"
            
            with open(test_file, 'wb') as f:
                if arch == "toy":
                    # Toy格式：DS, CS, LENGTH, 指令...
                    f.write((0x0000).to_bytes(2, byteorder='little'))  # DS
                    f.write((0x0000).to_bytes(2, byteorder='little'))  # CS
                    f.write((0x0001).to_bytes(2, byteorder='little'))  # LENGTH
                    f.write(program["instructions"][0].to_bytes(1, byteorder='little'))
                else:
                    # 其他格式：直接指令
                    for instruction in program["instructions"]:
                        f.write(instruction.to_bytes(4, byteorder='little'))
            
            created_files[arch] = test_file
            print(f"创建测试程序: {test_file}")
        
        return created_files
    
    def test_architecture_execution(self, test_files):
        """测试架构执行"""
        print("\n=== 测试架构执行 ===")
        
        results = {}
        
        for arch, test_file in test_files.items():
            print(f"\n测试 {arch.upper()} 架构...")
            
            # 使用统一VM测试
            result = self.run_command_with_timeout(["./build/bin/vm", arch, str(test_file)])
            
            if result.returncode == 0:
                print(f"[SUCCESS] {arch.upper()} 架构执行成功")
                results[arch] = {
                    "status": "SUCCESS",
                    "output": result.stdout,
                    "error": result.stderr
                }
            else:
                print(f"[ERROR] {arch.upper()} 架构执行失败: {result.stderr}")
                results[arch] = {
                    "status": "FAILURE",
                    "output": result.stdout,
                    "error": result.stderr
                }
        
        return results
    
    def compare_architectures(self):
        """比较两种架构"""
        print("\n=== 架构比较 ===")
        
        comparison = {
            "Toy": {
                "字长": "16位",
                "寄存器": "256个",
                "内存": "64KB",
                "指令格式": "变长",
                "设计理念": "教学用简单架构",
                "特点": "支持字节和字操作，函数调用"
            },
        }
        
        print("架构特性:")
        print(f"{'特性':<12} {'Toy':<20}")
        print("-" * 32)
        
        for feature in ["字长", "寄存器", "内存", "指令格式", "设计理念", "特点"]:
            toy_val = comparison["Toy"][feature]
            print(f"{feature:<12} {toy_val:<20}")
        
        return comparison
    
    def run_all_tests(self):
        """运行所有测试"""
        print("=== 架构比较测试开始 ===")
        print(f"项目目录: {self.project_dir}")
        print(f"构建目录: {self.build_dir}")
        print(f"超时设置: {self.timeout}秒")
        
        # 检查构建目录
        if not self.build_dir.exists():
            print("[ERROR] 构建目录不存在，请先运行 build.sh")
            return False
        
        # 检查可执行文件
        if not self.check_executables():
            print("[ERROR] 缺少必要的可执行文件")
            return False
        
        # 测试架构信息
        if not self.test_architecture_info():
            print("[ERROR] 架构信息测试失败")
            return False
        
        # 创建测试程序
        test_files = self.create_test_programs()
        
        # 测试架构执行
        execution_results = self.test_architecture_execution(test_files)
        
        # 比较架构
        comparison = self.compare_architectures()
        
        # 显示测试结果
        self.print_results(execution_results)
        
        # 清理测试文件
        for test_file in test_files.values():
            test_file.unlink(missing_ok=True)
        
        return all(result["status"] == "SUCCESS" for result in execution_results.values())
    
    def print_results(self, results):
        """打印测试结果"""
        print("\n=== 测试结果 ===")
        
        success_count = 0
        for arch, result in results.items():
            status = "✓" if result["status"] == "SUCCESS" else "✗"
            print(f"  {status} {arch.upper()}: {result['status']}")
            if result["status"] == "SUCCESS":
                success_count += 1
        
        print(f"\n成功: {success_count}/{len(results)}")
        
        if success_count == len(results):
            print("\n🎉 所有架构测试通过！")
        else:
            print(f"\n❌ {len(results) - success_count} 个架构测试失败")
        
        # 显示详细输出
        print("\n=== 详细输出 ===")
        for arch, result in results.items():
            print(f"\n{arch.upper()} 架构输出:")
            if result["output"]:
                print("标准输出:")
                print(result["output"])
            if result["error"]:
                print("错误输出:")
                print(result["error"])

def main():
    """主函数"""
    tester = ArchitectureTester()
    success = tester.run_all_tests()
    
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
