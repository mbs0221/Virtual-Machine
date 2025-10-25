#include "tac.h"
#include <cstdio>
#include <list>

// 前向声明，避免包含冲突
struct Stmt;
struct Stmts;
struct Decl;
struct Assign;
struct If;
struct While;
struct FuncDef;
struct Return;
struct Print;

// TACOperand 实现
string TACOperand::toString() const {
	switch (type) {
		case TAC_TEMP: return "t" + to_string(value);
		case TAC_REG: return "$" + to_string(reg);
		case TAC_IMM: return "#" + to_string(value);
		case TAC_MEM: return "*" + to_string(value);
		case TAC_LABEL_OP: return name;
		default: return "unknown";
	}
}

// TACInstruction 实现
string TACInstruction::toString() const {
	string result_str = result.toString();
	string arg1_str = arg1.toString();
	string arg2_str = arg2.toString();
	
	switch (op) {
		case TAC_ASSIGN:
			return result_str + " = " + arg1_str;
		case TAC_ADD:
			return result_str + " = " + arg1_str + " + " + arg2_str;
		case TAC_SUB:
			return result_str + " = " + arg1_str + " - " + arg2_str;
		case TAC_MUL:
			return result_str + " = " + arg1_str + " * " + arg2_str;
		case TAC_DIV:
			return result_str + " = " + arg1_str + " / " + arg2_str;
		case TAC_MOD:
			return result_str + " = " + arg1_str + " % " + arg2_str;
		case TAC_LOAD:
			return result_str + " = " + arg1_str;
		case TAC_STORE:
			return result_str + " = " + arg1_str;
		case TAC_CALL:
			return result_str + " = call " + label + "(" + arg1_str + ", " + arg2_str + ")";
		case TAC_RETURN:
			return "return " + arg1_str;
		case TAC_JUMP:
			return "jump " + label;
		case TAC_JUMP_COND:
			return "if " + arg1_str + " jump " + label;
		case TAC_LABEL:
			return label + ":";
		case TAC_HALT:
			return "halt";
		case TAC_PUSH:
			return "push " + arg1_str;
		case TAC_POP:
			return result_str + " = pop";
		case TAC_MOV:
			return result_str + " = " + arg1_str;
		default:
			return "unknown";
	}
}

// TACProgram 实现
TACProgram::TACProgram() {
    // 默认构造函数，创建空的TAC程序
}

TACProgram::TACProgram(Stmt* ast) {
    // 从Parser的AST构造TAC程序
    if (ast) {
        convertStmtToTAC(ast);
    }
}



void TACProgram::addInstruction(const TACInstruction& inst) {
	instructions.push_back(inst);
}

string TACProgram::generateTemp() {
	return "t" + to_string(tempCounter["temp"]++);
}

void TACProgram::print() const {
	printf("=== 三地址码程序 ===\n");
	for (size_t i = 0; i < instructions.size(); i++) {
		printf("[%04zu] %s\n", i, instructions[i].toString().c_str());
	}
	printf("==================\n");
}

const vector<TACInstruction>& TACProgram::getInstructions() const {
	return instructions;
}

size_t TACProgram::size() const {
	return instructions.size();
}

// TACConverter 实现
TACConverter::TACConverter() : tac(nullptr) {}

TACConverter::~TACConverter() {
	if (tac) {
		delete tac;
	}
}


void TACConverter::printTAC() const {
	if (tac) {
		tac->print();
	}
}
