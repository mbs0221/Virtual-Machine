#include "lexer.h"

#ifndef __INTER_H_
#define __INTER_H_

#include <list>

struct Node{
	virtual void code(FILE *fp){
	}
};

//表达式
struct Expr :Node{
	char opt;
	int label;
	static int count;
	Expr(char opt) :opt(opt){ label = count++; }
	virtual void code(FILE *fp){
		Node::code(fp);
	}
};

int Expr::count = 0;

// ��������ʽ
struct Cond :Expr{
	int True, False;
	Expr *E1, *E2;
	Cond(char opt, Expr *E1, Expr *E2) :Expr(opt), E1(E1), E2(E2){}
	virtual void code(FILE *fp){
		Expr::code(fp);
		E1->code(fp);
		E2->code(fp);
		fprintf(fp, "%c $%d $%d $%d\n", opt, E1->label, E2->label, label);
		switch (opt){
		case '>':fprintf(fp, "jg L%d\n", True); break;
		case '=':fprintf(fp, "je L%d\n", True); break;
		case '<':fprintf(fp, "jb L%d\n", True); break;
		case '!':fprintf(fp, "jne L%d\n", True); break;
		default:fprintf(fp, "jmp L%d\n", True); break;
		}
		fprintf(fp, "jmp L%d\n", False);
	}
};

// ��������ʽ
struct Arith :Expr{
	Expr *E1, *E2;
	Arith(char opt, Expr *E1, Expr *E2) :Expr(opt), E1(E1), E2(E2){}
	virtual void code(FILE *fp){
		Expr::code(fp);
		E1->code(fp);
		E2->code(fp);
		fprintf(fp, "%c $%d $%d $%d\n", opt, E1->label, E2->label, label);
	}
};

struct Unary :Expr{
	Expr *E1;
	Unary(char opt, Expr *E1) :Expr(opt), E1(E1){  }
	virtual void code(FILE *fp){
		Expr::code(fp);
		E1->code(fp);
		fprintf(fp, "%c $%d $%d\n", opt, E1->label, label);
	}
};

// ID
struct Id :Expr{
	Type *t;
	Word *s;
	int offset;
	Id(Type *t, Word *s, int offset) :Expr('@'), t(t), s(s), offset(offset){  }
	virtual void code(FILE *fp){
		Expr::code(fp);
		fprintf(fp, "load $%d *%d\n", label, offset);
	}
};

struct Number :Expr{
	Integer *s;
	Number(Integer *s) :Expr('@'), s(s){  }
	virtual void code(FILE *fp){
		Expr::code(fp);
		fprintf(fp, "load $%d #%d\n", label, s->value);
	}
};

//���
struct Stmt :Node{
	int line;
	int begin, next;
	static int label;
	static int newlabel(){
		return label++;
	}
	virtual void code(FILE *fp){
		Node::code(fp);
		printf("[%04d]", line);
	}
};

//����
struct Stmts :Stmt{
	list<Stmt*> Ss;
	virtual void code(FILE *fp){
		Stmt::code(fp);
		printf("stmts\n");
		list<Stmt*>::iterator iter;
		for (iter = Ss.begin(); iter != Ss.end(); iter++){
			(*iter)->code(fp);
		}
	}
};

int Stmt::label = 0;

struct Decl :Stmt{
	list<Id*> ids;
	virtual void code(FILE *fp){
		Stmt::code(fp);
		printf("decl\n");
		int width = 0;
		list<Id*>::iterator iter;
		for (iter = ids.begin(); iter != ids.end(); iter++){
			width += (*iter)->t->width;
		}
		fprintf(fp, "data %d data\n", width);
	}
};

struct Assign :Stmt{
	Id *E1;
	Expr *E2;
	virtual void code(FILE *fp){
		Stmt::code(fp);
		printf("assign\n");
		E2->code(fp);
		fprintf(fp, "store $%d *%d\n", E2->label, E1->offset);
	}
};

struct If :Stmt{
	Cond *C;
	Stmt *S1;
	virtual void code(FILE *fp){
		Stmt::code(fp);
		printf("if\n");
		next = newlabel();
		C->True = newlabel();
		C->False = next;
		S1->next = next;
		C->code(fp);
		fprintf(fp, "label L%d:\n", C->True);
		S1->code(fp);
		fprintf(fp, "label L%d:\n", next);
	}
};

struct Else :Stmt{
	Cond *C;
	Stmt *S1;
	Stmt *S2;
	virtual void code(FILE *fp){
		Stmt::code(fp);
		printf("if-else\n");
		next = newlabel();
		C->True = newlabel();
		C->False = newlabel();
		S1->next = next;
		C->code(fp);
		fprintf(fp, "label L%d:\n", C->True);
		S1->code(fp);
		fprintf(fp, "jmp L%d\n", next);
		fprintf(fp, "label L%d:\n", C->False);
		S2->code(fp);
		fprintf(fp, "label L%d:\n", next);
	}
};

struct While :Stmt{
	Cond *C;
	Stmt *S1;
	virtual void code(FILE *fp){
		Stmt::code(fp);
		printf("while\n");
		begin = newlabel();
		next = newlabel();
		C->True = newlabel();
		C->False = next;
		S1->next = begin;
		fprintf(fp, "label L%d:\n", begin);
		C->code(fp);
		fprintf(fp, "label L%d:\n", C->True);
		S1->code(fp);
		fprintf(fp, "jmp L%d\n", begin);
		fprintf(fp, "label L%d:\n", next);
	}
};


struct Do :Stmt{
	Cond *C;
	Stmt *S1;
	virtual void code(FILE *fp){
		Stmt::code(fp);
		printf("do-while\n");
		begin = newlabel();
		next = newlabel();
		C->True = begin;
		C->False = next;
		S1->next = begin;
		fprintf(fp, "label L%d:\n", begin);
		S1->code(fp);
		C->code(fp);
		fprintf(fp, "label L%d:\n", next);
	}
};

struct For :Stmt{
	Stmt *S1;
	Cond *C;
	Stmt *S2;
	Stmt *S3;
	virtual void code(FILE *fp){
		Stmt::code(fp);
		printf("for\n");
		begin = newlabel();
		next = newlabel();
		C->True = newlabel();
		C->False = next;
		S3->next = begin;
		S1->code(fp);
		fprintf(fp, "label L%d:\n", begin);
		C->code(fp);
		fprintf(fp, "label L%d:\n", C->True);
		S2->code(fp);
		S3->code(fp);
		fprintf(fp, "jmp L%d\n", begin);
		fprintf(fp, "label L%d:\n", next);
	}
};

struct Case :Stmt{
	Expr *E;
	map<int, Stmt*> Ss;
	virtual void code(FILE *fp){
		Stmt::code(fp);
		printf("case\n");
		//fprintf(fp, "jmp L%d\n", begin);
		//map<int, Stmt*>::iterator iter;
		//for (iter = Ss.begin(); iter != Ss.end(); iter++){
		//	fprintf(fp, "label L%d:\n", iter->second->begin);
		//	iter->second->code(fp);
		//}
		//fprintf(fp, "label L%d:\n", begin);
		//E->code(fp);
		//for (iter = Ss.begin(); iter != Ss.end(); iter++){
		//	fprintf(fp, "label L%d:\n", iter->second->begin);
		//	fprintf(fp, "= $%d $%d $%d", E->label, );
		//	iter->second->code(fp);
		//}
	}
};

// 函数定义
struct FuncDef :Stmt{
	string name;
	list<Id*> params;
	Stmt *body;
	virtual void code(FILE *fp){
		Stmt::code(fp);
		printf("func %s\n", name.c_str());
		fprintf(fp, "label func_%s:\n", name.c_str());
		// 设置栈帧：保存BP，设置新的BP
		fprintf(fp, "push $%d\n", 0); // 保存BP（假设BP在寄存器0）
		fprintf(fp, "mov $%d $%d\n", 0, 1); // BP = SP
		// 从栈上获取参数并加载到寄存器
		int paramOffset = 4; // 返回地址(2字节) + 旧BP(2字节)
		list<Id*>::iterator iter;
		for (iter = params.begin(); iter != params.end(); iter++){
			fprintf(fp, "load $%d *%d\n", (*iter)->offset, paramOffset);
			paramOffset += 2; // 每个参数2字节
		}
		body->code(fp);
		// 恢复栈帧
		fprintf(fp, "mov $%d $%d\n", 1, 0); // SP = BP
		fprintf(fp, "pop $%d\n", 0); // 恢复BP
		// 如果函数体末尾没有return语句，才添加ret指令
		// 这里简化处理，不自动添加ret
	}
};

// 函数调用
struct FuncCall :Expr{
	string name;
	list<Expr*> args;
	FuncCall(string n) :Expr('@'), name(n) {}
	virtual void code(FILE *fp){
		Expr::code(fp);
		// 将参数压栈（从右到左）
		list<Expr*>::reverse_iterator iter;
		for (iter = args.rbegin(); iter != args.rend(); iter++){
			(*iter)->code(fp);
			fprintf(fp, "push $%d\n", (*iter)->label);
		}
		// 使用CALL指令调用函数
		fprintf(fp, "call func_%s\n", name.c_str());
		// 清理栈上的参数
		fprintf(fp, "add $%d #%d\n", 0, args.size() * 2); // 调整栈指针
	}
};

// 返回语句
struct Return :Stmt{
	Expr *value;
	virtual void code(FILE *fp){
		Stmt::code(fp);
		printf("return\n");
		if (value) {
			value->code(fp);
			fprintf(fp, "store $%d *ret_val\n", value->label);
		}
		fprintf(fp, "ret\n");
	}
};

#endif