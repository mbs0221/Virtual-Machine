#include "vm.h"
#include <iostream>
#include <cstdio>

using namespace std;

int main()
{
	char a;
	FILE *fp = NULL;
	// 虚拟机执行
	printf("虚拟机执行\n");
	CPU CPU;
	CPU.init();
	CPU.load("data.obj");
	CPU.execute();
	CPU.store();
	cout << "exit." << endl;
	cin >> a;
	return 0;
}