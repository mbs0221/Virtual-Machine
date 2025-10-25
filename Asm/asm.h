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
	InstructionFactory *factory;
	int error_count;
	
	// 辅助函数
	bool match(int kind);
	
	// 解析函数 - 返回值的版本
	BYTE parse_register_value();
	WORD parse_immediate_value();
	WORD parse_address_value();
	WORD parse_direct_address_value();
	WORD parse_indirect_register_value();
	WORD parse_pc_relative_value();
	bool parse_addressing_mode(BYTE& opt, WORD& addr);
	
	// 模板方法：匹配并返回指定类型指针
	template<typename T>
	T* match_and_get(int kind) {
		if (!match(kind)) {
			// 使用简单的错误处理，避免模板实例化问题
			fprintf(stderr, "match_and_get: 类型不匹配，期望kind=%d, 实际kind=%d\n", kind, s->kind);
			exit(EXIT_FAILURE);
		}
		T* ptr = dynamic_cast<T*>(s);
		if (!ptr) {
			fprintf(stderr, "match_and_get: 类型转换失败，期望类型不符\n");
			exit(EXIT_FAILURE);
		}
		return ptr;
	}
	
	// 辅助函数：解析多个寄存器（使用现有的parse_register_value函数）
	bool parse_multiple_registers(BYTE& reg1, BYTE& reg2);
	bool parse_multiple_registers(BYTE& reg1, BYTE& reg2, BYTE& reg3);
	
	// 辅助函数：创建操作码对象
	Integer* create_operation_object(BYTE op);
	
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
	Code* parse_lea_statement();
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
	Code* parse_interrupt_statement();
	Code* parse_io_statement();
	Code* parse_neg_statement();
	Code* parse_loop_statement();
	
public:
	int DS = 0;
	int CS = 0;
	int codeOffset = 0;  // 代码段偏移计数器
	
	Asm(string fp);
	~Asm();
	void parse();
	void write(FILE *fp);
	
	// 指令列表管理方法
	list<Code*>& getInstructions() { return cs->codes; }
	void printInstructions();
	void clearInstructions();
	std::string getInstructionDescription(Code* code);
	int getInstructionCount() const { return cs->codes.size(); }
	int getErrorCount() const { return error_count; }
};

#endif