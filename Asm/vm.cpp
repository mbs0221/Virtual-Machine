#include "vm.h"

void CPU::init(){
	DS = CS = BP = IP = 0;
	SS = 0xffff;// ջ��ַ
	SP = SS;// ջָ��
}
void CPU::load(FILE *fp){
	fread(&DS, sizeof(WORD), 1, fp);
	fread(&CS, sizeof(WORD), 1, fp);
	fread(&SS, sizeof(WORD), 1, fp);
	fread(&LENGTH, sizeof(WORD), 1, fp);
	fread(&RAM, sizeof(BYTE)* LENGTH, 1, fp);
	printf("DS:%04d,CS:%04d,LENGTH:%04d\n", DS, CS, LENGTH);
	printf("START\t[CYCLE:%04d DS:%04d CS:%04d IP:%04x]", CYCLE, DS, CS, IP);
	printf("[%4d", RAM[DS]);
	for (int i = DS + 1; i < CS; i++){
		printf(" %4d", RAM[i]);
	}
	printf("]\n");
	IP = CS;
}
void CPU::store(){
	printf("END\t[CYCLE:%04d DS:%04d CS:%04d IP:%04x]", CYCLE, DS, CS, IP);
	printf("[%4d", RAM[DS]);
	for (int i = DS + 1; i < CS; i++){
		printf(" %4d", RAM[i]);
	}
	printf("]\n");
}
void CPU::execute(){
	WORD ABUS, DBUS;
	BYTE OP, TYPE, MR;
	do{ 
		OP = RAM[IP++];
		CYCLE++;
		TYPE = OP & MR_BYTE;
		MR = OP & MR_B;
		OP &= (~MR_BYTE);
		OP &= (~MR_B);
		switch (OP){
		case ADD:
		case SUB:
		case MUL:
		case DIV:
		case MOD:
		case CMP:
			ALU.OP = OP;
			if (TYPE == MR_BYTE){
				ABUS = ReadB();
				ALU.RA ^= ALU.RA;
				ALU.RA |= REG[ABUS];
				ABUS = ReadB();
				ALU.RB ^= ALU.RB;
				ALU.RB |= REG[ABUS];
				ALU.execute();
				ABUS = ReadB();
				REG[ABUS] = ALU.R;
			}else{
				ABUS = ReadB();
				ALU.RA ^= ALU.RA;
				ALU.RA |= REG[ABUS];
				ALU.RA |= REG[ABUS + 1] << 8;
				ABUS = ReadB();
				ALU.RB ^= ALU.RB;
				ALU.RB |= REG[ABUS];
				ALU.RB |= REG[ABUS + 1] << 8;
				ALU.execute();
				ABUS = ReadB();
				REG[ABUS] = ALU.R;
				REG[ABUS + 1] = ALU.R >> 8;
			}
			break;
		case NEG:
			ALU.OP = OP;
			if (TYPE == MR_BYTE){
				ABUS = ReadB();
				ALU.RA ^= ALU.RA;
				ALU.RA |= REG[ABUS];
				ALU.execute();
				ABUS = ReadB();
				REG[ABUS] = ALU.R;
			}
			else{
				ABUS = ReadB();
				ALU.RA ^= ALU.RA;
				ALU.RA |= REG[ABUS];
				ALU.RA |= REG[ABUS + 1] << 8;
				ALU.execute();
				ABUS = ReadB();
				REG[ABUS] = ALU.R;
				REG[ABUS + 1] = ALU.R >> 8;
			}
			break;
		case JB:
			if (ALU.FR&BIT_LT){
				IP = ReadW();
			}else{
				IP++; IP++;
			}
			break;
		case JG:
			if (ALU.FR&BIT_GT){
				IP = ReadW();
			}
			else{
				IP++; IP++;
			}
			break;
		case JE:
			if (ALU.FR & (BIT_GT | BIT_LT)){
				IP++; IP++;
			}else{
				IP = ReadW();
			}
			break;
		case JNE:
			if (ALU.FR & (BIT_GT | BIT_LT)){
				IP = ReadW();
			}else{
				IP++; IP++;
			}
			break;
		case JMP:
			IP = ReadW();
			break;
		case PUSH:
			ABUS = ReadB();
			if (TYPE == MR_BYTE){
				RAM[SP--] = REG[ABUS];
			}else{
				RAM[SP--] = REG[ABUS + 1];
				RAM[SP--] = REG[ABUS];
			}
			break;
		case POP:
			ABUS = ReadB();
			if (TYPE == MR_BYTE){
				REG[ABUS] = RAM[SP++];
			}else{
				REG[ABUS] = RAM[SP++];
				REG[ABUS + 1] = RAM[SP++];
			}
			break;
		case LOAD:
			ABUS = ReadB();
			if (TYPE == MR_BYTE){
				switch (MR){// 01100000
				case MR_A:DBUS = ReadB(); break;
				case MR_B:DBUS = ReadB(ReadB());  break;
				default:printf("ERROR %d\n", OP); break;
				}
				REG[ABUS] = DBUS;
			}else{
				switch (MR){
				case MR_A:DBUS = ReadW(); break;
				case MR_B:DBUS = ReadW(ReadW()); break;
				default:printf("ERROR %d\n", OP); break;
				}
				REG[ABUS] = DBUS;
				REG[ABUS + 1] = DBUS >> 8;
			}
			break;
		case STORE:
			ABUS = ReadB();
			if (TYPE == MR_BYTE){
				DBUS = REG[ABUS];
				switch (MR){
				case MR_B:ABUS = ReadW(); break;
				default: printf("error storew"); break;
				}
				RAM[ABUS] = DBUS;
			}else{
				DBUS ^= DBUS;
				DBUS |= REG[ABUS];
				DBUS |= REG[ABUS + 1] << 8;
				switch (MR){
				case MR_B:ABUS = ReadW(); break;
				default: printf("error storew"); break;
				}
				RAM[ABUS] = DBUS;
				RAM[ABUS + 1] = DBUS >> 8;
			}
			break;
		case IN:
			break;
		case OUT:
			break;
		case HALT:
			break;
		default:
			printf("invalid OP=%d at IP=%04x\n", OP, IP);
			break;
		}
		trace();
	}while (IP < LENGTH && OP != HALT);
}
void CPU::trace(){
#ifdef _DEBUG
	// char c;
	// cin >> c;  // 注释掉等待用户输入
	printf("[CYCLE:%04d DS:%04d CS:%04d IP:%04x]", CYCLE, DS, CS, IP);
	printf("[%4d", RAM[DS]);
	for (int i = DS + 1; i < CS; i++){
		printf(" %4d", RAM[i]);
	}
	printf("]\n");
#endif
}