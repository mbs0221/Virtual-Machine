#include "parser.h"

int main(){
	char a;
	FILE *fp = NULL;
	Parser *p = new Parser();
	printf("开始语法分析\n");
	AST *st = p->parse("Text.txt");
	printf("语法分析完成\n");
	printf("代码开始\n");
	printf(" line  stmt\n");
	fp = fopen("data.s", "w");
	if (fp == NULL) {
		printf("错误: 无法打开文件 data.s\n");
		return 1;
	}
	st->Codegen();
	fclose(fp);
	printf("代码完成\n");
	delete p;
	//fp = fopen("G.txt", "r");
	//printf("开始SLR(1)语法分析\n");
	//parse(fp);
	//printf("SLR(1)语法分析完成\n");
	//fclose(fp);
	//cin >> a;
	return 0;
}