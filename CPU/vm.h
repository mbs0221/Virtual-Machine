#ifndef __VM_H_
#define __VM_H_

#include <string>
#include <iostream>
#include <fstream>

using namespace std;

enum VM_CODE{
	HALT,
	ADD, SUB, MUL, DIV, MOD, CMP,// integer operator
	SHL, SHR, SAL, SAR, SRL, SRR,// 
	MOV, IN, OUT,
	LOAD, STORE,
	PUSH, POP,
	JMP, JNE, JG, JE, JB, JGE, JBE,
	LOOP
};

typedef unsigned char BYTE;
typedef unsigned short int WORD;

#define BIT_MASK	0x0000
#define BIT_ZERO	0x1000
#define BIT_EQ		0x0800
#define BIT_GT		0x0400
#define BIT_NEG		0x0200
#define BIT_ERR		0x0001

// �ڴ�Ѱַ��ʽ
#define MR_A		0x00// 0000������
#define MR_B		0x40// 0100ֱ��Ѱַ
// ��/�ֽڲ���
#define MR_BYTE		0x80
// [111][111][0][0]
#define REG_SRC_MASK	0xE0
#define REG_DST_MASK	0x1C

class ALU{
public:
	BYTE OP;
	WORD RA, RB;
	WORD R;
	WORD FR;
	ALU(){
		OP = 0;
		RA = RB = R = 0;
		FR = 0;
	}
	void execute(){
		switch (OP){
		case ADD:R = RA + RB; break;
		case SUB:R = RA - RB; break;
		case MUL:R = RA * RB; break;
		case DIV:R = RA / RB; break;
		case MOD:R = RA % RB; break;
		case CMP:R = RA == RB; break;
		default:FR |= BIT_ERR; break;
		}
		FR &= ~BIT_ZERO;
		FR |= (R == 0) ? BIT_ZERO : BIT_MASK;
		FR &= ~BIT_NEG;
		FR |= (R < 0) ? BIT_NEG : BIT_MASK;
		FR &= ~BIT_EQ;
		FR |= (RA == RB) ? BIT_EQ : BIT_MASK;
	}
};

class CPU{
private:
	
	BYTE REG[0x100];
	WORD SP, BP, SI, DI;				// ͨ�üĴ���
	WORD CS, DS, ES, SS;				// �μĴ���
	WORD IN[0x100], OUT[0x100];			// I/O�˿�
	WORD IP;							// ����ָ��
	WORD IBUS, DBUS, ABUS;				// �ڲ�����
	BYTE RAM[0x10000];					// �ڴ�
	ALU alu;							// ALU
	BYTE ReadB(){
		return RAM[IP++];
	}
	WORD ReadB(WORD ADDR){
		return RAM[ADDR];
	}
	WORD ReadW(){
		WORD W;
		W ^= W;
		W |= (WORD)RAM[IP++] << 8;
		W |= (WORD)RAM[IP++];
		return W;
	}
	WORD ReadW(WORD ADDR){
		WORD A, W;
		A = ADDR << 1;
		W ^= W;
		W |= (WORD)RAM[A] << 8;
		W |= (WORD)RAM[A + 1];
		return W;
	}
	void WriteW(WORD ADDR, WORD DATA){
		WORD A;
		A = ADDR << 1;
		RAM[A] = DATA >> 8;
		RAM[A + 1] = DATA;
	}
public:
	void init();
	void load(string fp);
	void store();
	void execute();
	void trace();
};

#endif