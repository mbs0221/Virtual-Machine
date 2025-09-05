#include "inter.h"

// Code类实现
void Code::code(FILE* fp) {
	printf("[%04d][%04d][%04x]", line, width, offset);
}

// Register类实现
void Register::code(FILE* fp) {
	Code::code(fp);
	printf("register\t$%02x\n", reg_num);
	fwrite(&reg_num, sizeof(BYTE), 1, fp);
}

// Immediate类实现
void Immediate::code(FILE* fp) {
	Code::code(fp);
	printf("immediate\t#%04x\n", value);
	fwrite(&value, sizeof(WORD), 1, fp);
}

// Address类实现
void Address::code(FILE* fp) {
	Code::code(fp);
	printf("address\t*%04x\n", addr);
	fwrite(&addr, sizeof(WORD), 1, fp);
}

// Codes类实现
void Codes::code(FILE* fp) {
	list<Code*>::iterator iter;
	for (iter = codes.begin(); iter != codes.end(); iter++) {
		(*iter)->code(fp);
	}
}

// Data类实现
void Data::code(FILE* fp) {
	Code::code(fp);
	printf("data:%2d\n", width);
	for (WORD i = 0; i < width; i++) {
		fwrite(&opt, sizeof(BYTE), 1, fp);
	}
}

// Variable类实现
void Variable::code(FILE* fp) {
	Code::code(fp);
	printf("variable\t%s = %04x\n", name.c_str(), value);
	fwrite(&value, sizeof(WORD), 1, fp);
}

// Load类实现
void Load::code(FILE* fp) {
	Code::code(fp);
	printf("load\t$%02x $%02x $%04x\n", opt, reg, addr);
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&reg, sizeof(BYTE), 1, fp);
	fwrite(&addr, sizeof(WORD), 1, fp);
}

// Store类实现
void Store::code(FILE* fp) {
	Code::code(fp);
	printf("store\t$%02x $%02x $%04x\n", opt, reg, addr);
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&reg, sizeof(BYTE), 1, fp);
	fwrite(&addr, sizeof(WORD), 1, fp);
}

// Halt类实现
void Halt::code(FILE *fp) {
	Code::code(fp);
	opt = HALT;
	printf("halt\t$%02x\n", opt);
	fwrite(&opt, sizeof(BYTE), 1, fp);
}

// Push类实现
void Push::code(FILE *fp) {
	Code::code(fp);
	printf("push\t$%02x $%02x\n", opt, reg);
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&reg, sizeof(BYTE), 1, fp);
}

// Pop类实现
void Pop::code(FILE *fp) {
	Code::code(fp);
	printf("pop\t$%02x $%02x\n", opt, reg);
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&reg, sizeof(BYTE), 1, fp);
}

// Mov类实现
void Mov::code(FILE *fp) {
	Code::code(fp);
	printf("mov\t$%02x $%02x $%02x\n", opt, reg1, reg2);
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&reg1, sizeof(BYTE), 1, fp);
	fwrite(&reg2, sizeof(BYTE), 1, fp);
}

// Add类实现
void Add::code(FILE *fp) {
	Code::code(fp);
	printf("add\t$%02x $%02x $%02x\n", opt, reg1, reg2);
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&reg1, sizeof(BYTE), 1, fp);
	fwrite(&reg2, sizeof(BYTE), 1, fp);
}

// Ret类实现
void Ret::code(FILE *fp) {
	Code::code(fp);
	opt = RET;
	printf("ret\t$%02x\n", opt);
	fwrite(&opt, sizeof(BYTE), 1, fp);
}

// Call类实现
void Call::code(FILE* fp) {
	Code::code(fp);
	printf("call\t$%02x $%04x\n", opt, addr->offset);
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&addr->offset, sizeof(WORD), 1, fp);
}

// Jmp类实现
void Jmp::code(FILE* fp) {
	Code::code(fp);
	printf("jmp \t$%02x $%04x\n", opt, addr->offset);
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&addr->offset, sizeof(WORD), 1, fp);
}

// Arith类实现
void Arith::code(FILE* fp) {
	Code::code(fp);
	printf("bino\t$%02x $%02x $%02x $%02x\n", opt, reg1, reg2, reg3);
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&reg1, sizeof(BYTE), 1, fp);
	fwrite(&reg2, sizeof(BYTE), 1, fp);
	fwrite(&reg3, sizeof(BYTE), 1, fp);
}

// Unary类实现
void Unary::code(FILE* fp) {
	Code::code(fp);
	printf("unary\t$%02x $%02x $%02x\n", opt, reg1, reg2);
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&reg1, sizeof(BYTE), 1, fp);
	fwrite(&reg2, sizeof(BYTE), 1, fp);
}
