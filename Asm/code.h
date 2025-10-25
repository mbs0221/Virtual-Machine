#ifndef __CODE_H_
#define __CODE_H_

typedef unsigned char BYTE;
typedef unsigned short int WORD;

// Toy架构指令集 - 与Toy CPU保持一致
// 使用100以上的数值范围避免与ASCII码冲突
enum Tag{
	// 基础指令 (100-109)
	HALT = 100,
	ADD = 101, SUB = 102, MUL = 103, DIV = 104, MOD = 105, CMP = 106,  // 整数运算
	NEG = 107,                           // 一元运算
	LOOP = 108,                          // 循环指令
	
	// 位运算指令 (110-119)
	AND = 110, OR = 111, XOR = 112, NOT = 113,  // 位运算
	SHL = 114, SHR = 115, SAL = 116, SAR = 117, SRL = 118, SRR = 119,  // 移位运算
	
	// 数据传输指令 (120-129)
	MOV = 120, IN = 121, OUT = 122,                  // 数据传输
	LOAD = 123, STORE = 124, LEA = 125,              // 内存访问
	PUSH = 127, POP = 128,                     // 栈操作
	
	// 跳转指令 (130-139)
	JMP = 130, JNE = 131, JG = 132, JE = 133, JB = 134, JGE = 135, JBE = 136, // 跳转指令
	CALL = 137, RET = 138,                     // 函数调用
	
	// 数学运算指令 (140-149)
	INC = 140, DEC = 141, ABS = 142,  // 数学运算
	
	// 条件设置指令 (150-159)
	SETZ = 150, SETNZ = 151, SETG = 152, SETL = 153, SETGE = 154, SETLE = 155,  // 条件设置指令
	
	// 字符串操作指令 (160-169)
	STRLEN = 160, STRCPY = 161, STRCMP = 162, STRCHR = 163,  // 字符串操作
	
	// 中断和系统调用指令 (170-179)
	INT_INST = 170, IRET = 171, CLI_INST = 172, STI_INST = 173,     // 中断指令
	SYSCALL = 174, HLT = 175,                                     // 系统调用和停机指令
	// 关键字
	ID = 256, INT, STRING, COMMENT, END, LABEL, DATA, CODE, STACK, VAR, FS, GS
};

// Toy架构寻址模式
#define MR_A        0x00  // 0000 寄存器寻址
#define MR_B        0x40  // 0100 直接寻址
#define MR_BYTE     0x80  // 字节/字操作
#define MR_INDIRECT 0x20  // 0010 寄存器间接寻址 [$reg]
#define MR_PC_REL   0x60  // 0110 PC相对寻址 [PC+offset]

#endif