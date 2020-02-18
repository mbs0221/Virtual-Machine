#include "lexer.h"

#ifndef __INTER_H_
#define __INTER_H_

#include <list>
#include <stack>
#include "builder.h"

class Visitor;

class ASTNode{
	friend class Visitor;
public:
	virtual Value * Codegen() = 0;
};

//���
class Stmt : ASTNode{
public:
	int line;
	int begin, next;
	static int label;
	static int newlabel(){
		return label++;
	}
	virtual Value * Codegen() = 0;
};

int Stmt::label = 0;

//����
class Block : public Stmt{
	vector<Stmt*> block;
public:
	Block(vector<Stmt*> &block) : block(block) {  }
	Value * Codegen();
};

//����ʽ
class ExprAST : public Stmt{
public:
	Type *type;
	int label;
	static int count;
	ExprAST() { label = count++; }
	virtual Value * Codegen() = 0;
};

int ExprAST::count = 0;

class BinaryExprAST : public ExprAST {
	int opt;
	ExprAST *pL, *pR;
public:
	BinaryExprAST(int opt, ExprAST *pL, ExprAST *pR) 
		 : opt(opt), pL(pL), pR(pR) { }
	Value * Codegen();
};

class UnaryExprAST : public ExprAST{
	int opt;
	ExprAST *E1;
public:
	UnaryExprAST(int opt, ExprAST *E1)
		: opt(opt), E1(E1){ }
	Value * Codegen();
};

class VariableExprAST : public ExprAST {
	string name;
public:
	VariableExprAST(const string &name) : name(name){ }
	Value * Codegen();
};

class AssignExprAST : public VariableExprAST {
	ExprAST *expr;
public:
	AssignExprAST(const string &name, ExprAST *expr)
		: VariableExprAST(name), expr(expr) {  }
	Value * Codegen();
};

class ConstantExprAST : public ExprAST {
	Integer *s;
public:
	ConstantExprAST(Integer *s) : s(s) { }
	Value * Codegen();
};

class AccessExprAST : public VariableExprAST {
	ExprAST *loc;
public:
	AccessExprAST(const string &name, ExprAST *loc)
		: VariableExprAST(name), loc(loc) {  }
	Value * Codegen();
};

class MemberExpr : public ExprAST {
	string name;
	ExprAST *index;
public:
	MemberExpr(const string name, ExprAST *index)
		: name(name), index(index) { }
	Value * Codegen();
};

class PointerExpr : public ExprAST {
	string name;
	ExprAST *index;
public:
	PointerExpr(const string name, ExprAST *index)
		: name(name), index(index) { }
};

class CallExprAST : public ExprAST {
	string callee;
	vector<ExprAST*> args;
public:
	CallExprAST() { args.clear(); }
	CallExprAST(const string &callee, vector<ExprAST*> &args)
		 : callee(callee), args(args) { ; }
	Value * Codegen();
};

class Decl : Stmt{
	list<VariableExprAST*> ids;
public:
	Value * Codegen();
};

class IfElse : public Stmt{
	ExprAST *cond;
	Stmt *body_t;
	Stmt *body_f;
public:
	IfElse(ExprAST *cond, Stmt *body_t, Stmt *body_f)
		: cond(cond), body_t(body_t), body_f(body_f) { }
	Value * Codegen();
};

class WhileDo : public Stmt{
	ExprAST *cond;
	Stmt *body;
public:
	WhileDo(ExprAST *cond, Stmt *body) :cond(cond), body(body) {  }
	Value * Codegen();
};

class DoWhile : public Stmt{
	ExprAST *cond;
	Stmt *body;
public:
	DoWhile(ExprAST *cond, Stmt *body) :cond(cond), body(body) {  }
	Value * Codegen();
};

class For : public Stmt{
	ExprAST *init, *cond, *step;
	Stmt *body;
public:
	For(ExprAST *init, ExprAST *cond, ExprAST *step, Stmt *body) 
		: init(init), cond(cond), step(step), body(body) {  }
	Value * Codegen();
};

typedef pair<int, Stmt*> Case;

class Switch : public Stmt {
	ExprAST *expr;
	vector<Case> cases;
public:
	Switch(ExprAST *expr, vector<pair<int, Stmt*>> &cases)
		: expr(expr), cases(cases) {  }
	Value * Codegen();
};

class Break : public Stmt{
	Stmt *stmt;
public:
	Break(Stmt *next) : stmt(next) { }
	Value * Codegen();
};

class Continue : public Stmt{
	Stmt *next;
public:
	Continue(Stmt *next) : next(next) { }
	Value * Codegen();
};

class Throw : public Stmt {
	Type* exception;
public:
	Throw() {
		exception = nullptr;
	}
	Value * Codegen();
};

class TryCatch : public Stmt{
	Stmt *pTry, *pCatch, *pFinally;
public:
	TryCatch(Stmt* pTry, Stmt* pCatch, Stmt* pFinally) 
		 : pTry(pTry), pCatch(pCatch), pFinally(pFinally) { }
	Value * Codegen();
};

typedef map<string, Type*> SymbolTable;

class ParameterAST {
	Type *type;
	string name;
public:
	ParameterAST(Type *type, string name) : type(type), name(name) { }
};

class PrototypeAST {
	Type *type;
	string name;
	vector<ParameterAST*> args;
public:
	PrototypeAST(Type *type, const string &name, vector<ParameterAST*> &args)
		: type(type), name(name), args(args) { }
	Value * Codegen();
};

class FunctionAST {
	PrototypeAST *proto;
	Stmt *body;
public:
	FunctionAST(PrototypeAST *proto, Stmt *body) : proto(proto), body(body) { }
	Value * Codegen();
};

class Global : ASTNode {
	map<string, Type*> symbols;
public:
	Value * putId(string name, Type* type) {
		symbols[name] = type;
	}
	Type* getId(string name) {
		return symbols[name];
	}
	Value * code(){
		ASTNode::Codegen(fp);
	}
};

#endif // !__INTER_H_
