#ifndef __CODE_H_
#define __CODE_H_

typedef unsigned char BYTE;
typedef unsigned short int WORD;

enum Tag{
	// �����ָ�
	HALT,
	ADD, SUB, MUL, DIV, MOD, CMP,
	JMP, JNE, JG, JE, JB, JGE, JBE,
	LOAD, STORE,
	PUSH, POP,
	NEG,
	MOV, IN, OUT,
	SHL, SHR, SAL, SAR, SRL, SRR,// 
	LOOP,
	// �ؼ���
	ID = 256, INT, END, LABEL, DATA, CODE, STACK
};

#endif