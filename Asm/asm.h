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

class Asm{
private:
	Token *s;
	Lexer *lexer;
	Codes *cs;
	map<string, Label*> lables;
	
	bool match(int kind){
		if (s->kind == kind){
			s = lexer->scan();
			return true;
		}
		s = lexer->scan();
		return false;
	}
	
	Code* data(){
		Data *d = new Data;
		d->opt = NULL;
		d->line = lexer->line;
		match(DATA);
		d->width = ((Integer*)s)->value;
		DS = 0;
		CS = d->width;
		match(INT);
		match(DATA);
		return d;
	}
	
	Code* store(){
		Store *l = new Store;
		l->line = lexer->line;
		l->opt = STORE;
		match(STORE);
		match('$');
		l->reg = ((Integer*)s)->value;
		match(INT);
		switch (s->kind){
		case '#':l->opt |= MR_A; match('#'); break;
		case '*':l->opt |= MR_B; match('*'); break;
		default:break;
		}
		l->addr = ((Integer*)s)->value;
		l->width = 4;
		match(INT);
		return l;
	}
	
	Code* halt(){
		Halt *h = new Halt;
		h->line = lexer->line;
		h->opt = HALT;
		h->width = 1;
		match(HALT);
		return h;
	}
	
	Code* call(){
		Call *c = new Call;
		c->line = lexer->line;
		c->opt = CALL;
		match(CALL);
		if (lables.find(((Word*)s)->word) == lables.end()){
			c->addr = new Label((Word*)s, cs->width);
			lables[((Word*)s)->word] = c->addr;
		}else{
			c->addr = lables[((Word*)s)->word];
		}
		match(ID);
		c->width = 3;
		return c;
	}
	
	Code* push(){
		Push *p = new Push;
		p->line = lexer->line;
		p->opt = PUSH;
		match(PUSH);
		match('$');
		p->reg = ((Integer*)s)->value;
		match(INT);
		p->width = 2;
		return p;
	}
	
	Code* pop(){
		Pop *p = new Pop;
		p->line = lexer->line;
		p->opt = POP;
		match(POP);
		match('$');
		p->reg = ((Integer*)s)->value;
		match(INT);
		p->width = 2;
		return p;
	}
	
	Code* mov(){
		Mov *m = new Mov;
		m->line = lexer->line;
		m->opt = MOV;
		match(MOV);
		match('$');
		m->reg1 = ((Integer*)s)->value;
		match(INT);
		match('$');
		m->reg2 = ((Integer*)s)->value;
		match(INT);
		m->width = 3;
		return m;
	}
	
	Code* add(){
		Add *a = new Add;
		a->line = lexer->line;
		a->opt = ADD;
		match(ADD);
		match('$');
		a->reg1 = ((Integer*)s)->value;
		match(INT);
		match('#');
		a->reg2 = ((Integer*)s)->value;
		match(INT);
		a->width = 4;
		return a;
	}
	
	Code* ret(){
		Ret *r = new Ret;
		r->line = lexer->line;
		r->opt = RET;
		r->width = 1;
		match(RET);
		return r;
	}
	
	Code* load(){
		Load *l = new Load;
		l->line = lexer->line;
		l->opt = LOAD;
		match(LOAD);
		match('$');
		l->reg = ((Integer*)s)->value;
		match(INT);
		switch (s->kind){
		case '#':l->opt |= MR_A; match('#'); break;// 寄存器
		case '*':l->opt |= MR_B; match('*'); break;// 直接寻址
		default:break;
		}
		l->addr = ((Integer*)s)->value;
		match(INT);
		l->width = 4;
		return l;
	}
	
	Code* label(){
		match(LABEL);
		if (lables.find(((Word*)s)->word) == lables.end()){
			lables[((Word*)s)->word] = new Label((Word*)s, cs->width);
		}else{
			lables[((Word*)s)->word]->offset = cs->width;
		}
		match(ID);
		match(':');
		return nullptr;
	}
	
	Code* unary(){
		Unary *u = new Unary;
		u->line = lexer->line;
		match('~');
		u->opt = NEG;
		match('$');
		u->reg1 = ((Integer*)s)->value;
		match(INT);
		match('$');
		u->reg2 = ((Integer*)s)->value;
		match(INT);
		u->width = 3;
		return u;
	}
	
	Code* arith(BYTE b){
		Arith *a = new Arith;
		a->line = lexer->line;
		a->opt = b;
		match(s->kind);
		match('$');
		a->reg1 = ((Integer*)s)->value;
		match(INT);
		match('$');
		a->reg2 = ((Integer*)s)->value;
		match(INT);
		match('$');
		a->reg3 = ((Integer*)s)->value;
		match(INT);
		a->width = 4;
		return a;
	}
	
	Code* jmp(BYTE b){
		Jmp *j = new Jmp;
		j->line = lexer->line;
		j->opt = b;
		match(s->kind);
		if (lables.find(((Word*)s)->word) == lables.end()){
			j->addr = new Label((Word*)s, cs->width);
			lables[((Word*)s)->word] = j->addr;
		}else{
			j->addr = lables[((Word*)s)->word];
		}
		match(ID);
		j->width = 3;
		return j;
	}
	
public:
	int DS = 0;
	int CS = 0;
	
	Asm(string fp){
		lexer = new Lexer(fp);
	}
	
	void parse(){
		Code *c = new Code;
		cs = new Codes;
		s = lexer->scan();
		while (s->kind != END){
			switch (s->kind){
			case ID:printf("[%3d]find id:%s\n", lexer->line, ((Word*)s)->word.c_str()); break;
			case DATA:c = data(); break;
			case LOAD:c = load(); break;
			case STORE:c = store(); break;
			case HALT:c = halt(); break;
			case CALL:c = call(); break;
			case RET:c = ret(); break;
			case PUSH:c = push(); break;
			case POP:c = pop(); break;
			case MOV:c = mov(); break;
			case ADD:c = add(); break;
			case LABEL:c = label(); break;
			case JE: c = jmp(JE); break;
			case JNE: c = jmp(JNE); break;
			case JB: c = jmp(JB); break;
			case JG: c = jmp(JG); break;
			case JMP: c = jmp(JMP); break;
			case '~':c = unary(); break;
			case '+':c = arith(ADD); break;
			case '-':c = arith(SUB); break;
			case '*':c = arith(MUL); break;
			case '/':c = arith(DIV); break;
			case '%':c = arith(MOD); break;
			case '<':c = arith(CMP); break;
			case '>':c = arith(CMP); break;
			case '=':c = arith(CMP); break;
			case '!':c = arith(CMP); break;
			default:printf("[%3d]find unsupported instuction '%d'\n", lexer->line, s->kind); break;
			}
			if (c){ cs->codes.push_back(c); c->offset = cs->width; cs->width += c->width; }
			s = lexer->scan(); // 读取下一个token
		}
	}
	
	void write(FILE *fp){
		fwrite(&DS, sizeof(WORD), 1, fp);
		fwrite(&CS, sizeof(WORD), 1, fp);
		fwrite(&cs->width, sizeof(WORD), 1, fp);
		cs->code(fp);
	}
};

#endif