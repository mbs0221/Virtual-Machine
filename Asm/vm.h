#ifndef __VM_H_
#define __VM_H_

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include "code.h"

using namespace std;

#define BIT_MASK	0x0000
#define BIT_ZERO	0x1000
#define BIT_GT		0x0400
#define BIT_LT		0x0200
#define BIT_NEG		0x0100
#define BIT_ERR		0x0001

// �ڴ�Ѱַ��ʽ
#define MR_A		0x00// 0000������
#define MR_B		0x40// 0100ֱ��Ѱַ
// ��/�ֽڲ���
#define MR_BYTE		0x80
// [111][111][0][0]
#define REG_SRC_MASK	0xE0	// Դ�Ĵ���
#define REG_DST_MASK	0x1C	// Ŀ�ļĴ���

class ALU{
public:
	BYTE OP;		// ����ָ��
	WORD RA, RB;	// ������
	WORD R;			// ����Ĵ���
	WORD FR;		// ��׼�Ĵ���
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
		case NEG:R = 0 - RA; break;
		default:FR |= BIT_ERR; break;
		}
		FR &= ~BIT_ZERO;
		FR &= ~BIT_GT;
		FR &= ~BIT_LT;
		FR &= ~BIT_NEG;
		FR |= (R == 0) ? BIT_ZERO : BIT_MASK;
		FR |= (R < 0) ? BIT_NEG : BIT_MASK;
		FR |= (RA > RB) ? BIT_GT : BIT_MASK;
		FR |= (RA < RB) ? BIT_LT : BIT_MASK;
	}
};

class CPU{
private:
	WORD LENGTH = 0;
	BYTE REG[0x100];
	WORD SP, BP, SI, DI;				// ͨ�üĴ���
	WORD CS, DS, ES, SS;				// �μĴ���
	WORD IN[0x100], OUT[0x100];			// I/O�˿�
	WORD IP;							// ����ָ��
	WORD IBUS, DBUS, ABUS;				// �ڲ�����
	BYTE RAM[0x10000];					// �ڴ�
	WORD CYCLE = 0;						// ִ������
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
		W |= (WORD)RAM[IP++];
		W |= (WORD)RAM[IP++] << 8;// ���ֽ�
		return W;
	}
	WORD ReadW(WORD ADDR){
		WORD W;
		W ^= W;
		W |= (WORD)RAM[ADDR];
		W |= (WORD)RAM[ADDR + 1] << 8;// ���ֽ�
		return W;
	}
	void WriteW(WORD ADDR, WORD DATA){
		RAM[ADDR] = DATA;
		RAM[ADDR + 1] = DATA >> 8;// ���ֽ�
	}
public:
	void init();
	void load(FILE *fp);
	void store();
	void execute();
	void trace();
};

#endif