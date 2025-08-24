#ifndef __ASM_H_
#define __ASM_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <list>

#include "inter.h"
#include "vm.h"

using namespace std;

//-------------------------�쳣����------------------------
class MismatchException : public exception {
	int line;
public:
	MismatchException(int line) : exception(), line(line) { ; }
	const char * what() const throw () {
		return "Character mismatch";
	}
};

class UndeclaredException : public exception {
	int line;
public:
	UndeclaredException(int line) : exception(), line(line) { ; }
	const char * what() const throw () {
		return "Variable or function undeclared";
	}
};

class MultipleDeclaredException : exception {
	int line;
public:
	MultipleDeclaredException(int line) : exception(), line(line) { ; }
	const char * what() const throw () {
		return "Multiple labels are declared";
	}
};

class InstructionUnsupportedException : exception {
	int line;
public:
	InstructionUnsupportedException(int line) : exception(), line(line) { ; }
	const char * what() const throw () {
		return "Instruction unsupported";
	}
};

#define MATCH_REG(l, s) \
	match('$'); \
	l->reg = ((Integer*)s)->value; \
	match(NUM); 

#define MATCH_ADDR(l, s) \
	match('#'); \
	l->addr = ((Integer*)s)->value; \
	match(NUM); 

//-------------------------������------------------------

class Asm{
private:
	int DS = 0;
	int CS = 0;
	int SS = 0;
	Token *s;
	Lexer *lexer;
	Codes *cs;
	map<string, Label*> lables;
	map<string, Proc*> funcs;
	// basic opts
	inline BYTE match_reg() {
		match('$');
		BYTE reg = ((Integer*)s)->value;
		match(NUM);
		return reg;
	}
	inline WORD match_addr() {
		match('#');
		WORD addr = ((Integer*)s)->value;
		match(NUM);
		return addr;
	}
	inline Word* match_word() {
		Word *w = (Word*)s;
		match(ID);
		return w;
	}
	// labels
	Code* add_label(string name) {
		if (lables.find(name) == lables.end()) {
			lables[name] = new Label(lexer->line, name);
		}
		return lables[name];
	}
	// parser
	void match(int kind){
		if (s->kind != kind){
			throw MismatchException(lexer->line);
		}
		s = lexer->scan();
	}
	void data(){
		match('.');
		match(DATA);
		//cs->offset = cs->width;
		//cs->width += ((Integer*)s)->value;
		//DS = 0;
		match(NUM);
	}
	void stack(){
		match('.');
		match(STACK);
		//cs->offset = cs->width;
		//cs->width += ((Integer*)s)->value;
		//SS = cs->width;
		match(NUM);
	}
	void code(){
		match('.');
		match(CODE);
		s = lexer->scan();
		Code *c = new Code(0, CODE);
		while (s->kind == PROC){
			c = match_proc();
			if (c){ 
				cs->pushCode(c);
			}
		}
		match('#');
	}
	Code* match_proc(){
		match(PROC);
		Word *w = match_word();
		match(':');
		Code *body = match_codes();
		match(ENDP);
		Proc *proc = new Proc(lexer->line, w->str, body);
		funcs[w->str] = proc;
		return proc;
	}
	Code* match_codes() {
		Codes *cs, *c;
		cs = new Codes(lexer->line);
		while (s->kind != ENDP) {
			Code *c;
			switch (s->kind) {
			case ID:c = match_label(); break;
			case CALL: c = match_call(); break;
			case LOAD:c = match_load(); break;
			case STORE:c = match_store(); break;
			case PUSH:c = match_push(); break;
			case POP:c = match_pop(); break;
			case HALT:c = match_halt(); break;
			case JE: 
			case JNE: 
			case JB: 
			case JG: 
			case JMP: c = match_jmp(); break;
			case ADD:
			case SUB:
			case '+':
			case '-':
			case '*':
			case '/':
			case '%':
			case '<':
			case '>':
			case '=':c = match_arith(); break;
			case '!':
			case '~':c = match_unary(); break;
			default: throw InstructionUnsupportedException(lexer->line); break;
			}
			if (c) { cs->pushCode(c); }
		}
	}
	Code* match_label() {
		Word *w = match_word();
		match(':');
		return add_label(w->str);
	}
	Code* match_call(){
		match(CALL);
		Word *w = match_word();
		if (funcs.find(w->str) != funcs.end()){
			Proc *func = funcs[w->str];
			match(ID);
			return new Call(lexer->line, func);
		}
		throw std::exception();
		return nullptr;
	}
	Code* match_load() {
		BYTE reg = match_reg();
		WORD addr = match_addr();
		return new Load(lexer->line, reg, addr);
	}
	Code* match_store(){
		match(STORE);
		BYTE reg = match_reg();
		WORD addr = match_addr();
		return new Store(lexer->line, reg, addr);
	}
	Code* match_push() {
		match(PUSH);
		BYTE reg = match_reg();
		return new Push(lexer->line, reg);
	}
	Code* match_pop() {
		match(POP);
		BYTE reg = match_reg();
		match(NUM);
		return new Pop(lexer->line, reg);
	}
	Code* match_unary(){
		match('~');
		BYTE reg1 = match_reg();
		BYTE reg2 = match_reg();
		return new Unary(lexer->line, NEG, reg1, reg2);
	}
	Code* match_arith(){
		BYTE opt = s->kind;
		match(s->kind);
		BYTE reg1 = match_reg();
		BYTE reg2 = match_reg();
		BYTE reg3 = match_reg();
		return new Arith(lexer->line, opt, reg1, reg2, reg3);
	}
	Code* match_jmp(){
		match(JMP);
		Word *w = match_word();
		Label *label = (Label*)add_label(w->str);
		match(ID);
		return new Jmp(lexer->line, label);
	}
	Code* match_halt(){
		match(HALT);
		return new Halt(lexer->line);
	}
public:
	Asm(string fp){
		lexer = new Lexer(fp);
	}
	void parse(){
		cs = new Codes(lexer->line);
		data();
		stack();
		code();
	}
	void write(FILE *fp){
		BYTE b = 0x00;
		fwrite(&DS, sizeof(WORD), 1, fp);
		fwrite(&CS, sizeof(WORD), 1, fp);
		fwrite(&SS, sizeof(WORD), 1, fp);
		//fwrite(&cs->width, sizeof(WORD), 1, fp);
		//fwrite(&b, sizeof(BYTE)*CS, 1, fp);
		cs->code(fp);
	}
};


//printf("[%03d]data.\n", lexer->line);
//printf("[%03d]stack.\n", lexer->line);
//printf("[%03d]proc.\n", lexer->line);
//printf("[%03d]call.\n", lexer->line);
//printf("[%03d]pop.\n", lexer->line);
//printf("[%03d]push.\n", lexer->line);
//printf("[%03d]load.\n", lexer->line);
//printf("[%03d]store.\n", lexer->line);
//printf("[%03d]label.\n", lexer->line);
//printf("[%03d]unary.\n", lexer->line);
//printf("[%03d]arith.\n", lexer->line);
//printf("[%03d]jmp.\n", lexer->line);
//printf("[%03d]halt.\n", lexer->line);
#endif