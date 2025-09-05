#ifndef __ASM_H_
#define __ASM_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <list>
#include <map>
#include <string>

#include "inter.h"
#include "code.h"

using namespace std;

class Asm {
private:
	Token *s;
	Lexer *lexer;
	Codes *cs;
	map<string, Label*> lables;
	
	// 辅助函数
	bool match(int kind);
	
	// 解析函数 - 返回值的版本
	BYTE parse_register_value();
	WORD parse_immediate_value();
	WORD parse_address_value();
	bool parse_addressing_mode(BYTE& opt, WORD& addr);
	
	// 旧版解析函数（将被逐步替换）
	Code* data();
	Code* halt();
	Code* call();
	Code* add();
	Code* ret();
	Code* label();
	Code* unary();
	Code* arith(BYTE b);
	Code* jmp(BYTE b);
	
	// LL(1)递归下降解析函数
	Code* parse_statement();
	Code* parse_data_statement();
	Code* parse_variable_statement();
	Code* parse_fs_statement();
	Code* parse_gs_statement();
	Code* parse_load_statement();
	Code* parse_store_statement();
	Code* parse_halt_statement();
	Code* parse_push_statement();
	Code* parse_pop_statement();
	Code* parse_mov_statement();
	Code* parse_call_statement();
	Code* parse_ret_statement();
	Code* parse_add_statement();
	Code* parse_label_statement();
	Code* parse_jump_statement();
	Code* parse_arithmetic_statement();
	
public:
	int DS = 0;
	int CS = 0;
	
	Asm(string fp);
	void parse();
	void write(FILE *fp);
};

#endif