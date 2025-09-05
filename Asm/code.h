#ifndef __CODE_H_
#define __CODE_H_

typedef unsigned char BYTE;
typedef unsigned short int WORD;

// Toy架构指令集 - 与Toy CPU保持一致
enum Tag{
	HALT,
	ADD, SUB, MUL, DIV, MOD, CMP,  // 整数运算
	SHL, SHR, SAL, SAR, SRL, SRR,  // 移位运算
	MOV, IN, OUT,                  // 数据传输
	LOAD, STORE,                   // 内存访问
	PUSH, POP,                     // 栈操作
	JMP, JNE, JG, JE, JB, JGE, JBE, // 跳转指令
	CALL, RET,                     // 函数调用
	NEG,                           // 一元运算
	LOOP,                          // 循环指令
	// 关键字
	ID = 256, INT, END, LABEL, DATA, CODE, STACK, VAR, FS, GS
};

// Toy架构寻址模式
#define MR_A        0x00  // 0000 寄存器寻址
#define MR_B        0x40  // 0100 直接寻址
#define MR_BYTE     0x80  // 字节/字操作

#endif