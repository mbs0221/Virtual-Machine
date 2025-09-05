#include "lexer.h"
#include <list>

struct Label {
	Word *w;
	WORD offset;
	Label(Word *w, int offset) :w(w), offset(offset) { }
};

struct Code {
	BYTE opt;
	WORD line = 0; // 当前指令在源文件中的行号
	WORD width = 0; // 当前指令占用的字宽
	WORD offset = 0; // 当前指令的偏移量
	virtual void code(FILE* fp);
};

// 寄存器AST节点
struct Register :Code {
	BYTE reg_num;
	Register(BYTE num) :reg_num(num) {}
	virtual void code(FILE* fp);
};

// 立即数AST节点
struct Immediate :Code {
	WORD value;
	Immediate(WORD val) :value(val) {}
	virtual void code(FILE* fp);
};

// 地址AST节点
struct Address :Code {
	WORD addr;
	Address(WORD address) :addr(address) {}
	virtual void code(FILE* fp);
};

struct Codes :Code {
	list<Code*> codes;
	virtual void code(FILE* fp);
};

struct Data :Code {
	virtual void code(FILE* fp);
};

// 变量定义结构体
struct Variable :Code {
	string name;     // 变量名
	WORD value;      // 变量值
	Variable(string n, WORD v) :name(n), value(v) {}
	virtual void code(FILE* fp);
};

struct Load :Code {
	BYTE reg;
	WORD addr;
	virtual void code(FILE* fp);
};

struct Store :Code {
	BYTE reg;
	WORD addr;
	virtual void code(FILE* fp);
};

struct Halt :Code {
	virtual void code(FILE *fp);
};

struct Push :Code {
	BYTE reg;
	virtual void code(FILE *fp);
};

struct Pop :Code {
	BYTE reg;
	virtual void code(FILE *fp);
};

struct Mov :Code {
	BYTE reg1, reg2;
	virtual void code(FILE *fp);
};

struct Add :Code {
	BYTE reg1, reg2;
	virtual void code(FILE *fp);
};

struct Ret :Code {
	virtual void code(FILE *fp);
};

struct Call :Code {
	Label *addr;
	virtual void code(FILE* fp);
};

struct Jmp :Code {
	Label *addr;
	virtual void code(FILE* fp);
};

struct Arith :Code {
	BYTE reg1, reg2, reg3;
	virtual void code(FILE* fp);
};

struct Unary :Code {
	BYTE reg1, reg2;
	virtual void code(FILE* fp);
};