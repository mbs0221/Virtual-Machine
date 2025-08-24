#include "asm.h"
#include <iostream>

using namespace std;

int main(){
	char a;
	FILE *fp = NULL;
	// 汇编器的目标代码
	Asm Asm("data.s");
	printf("语法分析开始\n");
	Asm.parse();
	printf("语法分析完成\n");
	printf("汇编开始\n");
	printf("line  width offset\n");
	fp = fopen("data.bin", "w");
	if (fp == NULL) {
		printf("错误: 无法打开文件 data.bin\n");
		return 1;
	}
	Asm.write(fp);
	fclose(fp);
	printf("汇编完成\n");
	// 虚拟机执行
	printf("虚拟机执行\n");
	//CPU CPU;
	//CPU.init();
	//fp = fopen("data.bin", "r");
	//CPU.load(fp);
	//fclose(fp);
	//CPU.execute();
	//CPU.store();
	printf("执行结束\n");
	// std::cin >> a;
	return 0;
}