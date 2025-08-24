#ifndef __PARSER_H_
#define __PARSER_H_

#include "inter.h"

using namespace std;

class Parser{
private:
	Token *s;
	Lexer *lexer;
	int width = 0;
	map<string, Id*> m;
	bool match(int kind){
		if (s->kind == kind){
			s = lexer->scan();
			return true;
		}
		s = lexer->scan();
		printf("%d not matched.\n", kind);
		return false;
	}
	void putId(Id *id){
		m[id->s->word] = id;
	}
	Id* getId(){
		string str = ((Word*)s)->word;
		if (m.find(str) == m.end()){
			return nullptr;
		}
		return m[str];
	}
	Id* getId(string str){
		if (m.find(str) == m.end()){
			return nullptr;
		}
		return m[str];
	}
	Stmt* stmt()
	{
		Stmt *st = nullptr;
		switch (s->kind){
		case INT:st = stmt_decl(); break;
		case ID:st = stmt_assign(); break;
		case IF:st = stmt_if(); break;
		case WHILE:st = stmt_while(); break;
		case DO:st = stmt_do(); break;
		case FOR:st = stmt_for(); break;
		case CASE:st = stmt_case(); break;
		case FUNC:st = stmt_func(); break;
		case RETURN:st = stmt_return(); break;
		case ';':match(';'); break;
		case '{':st = stmts(); break;
		default:match(s->kind); break;
		}
		return st;
	}
	Stmt* stmts(){
		Stmts *sts = new Stmts;
		sts->line = lexer->line;
		match('{');
		while (s->kind != '}'){
			Stmt *st = stmt();
			if (st)sts->Ss.push_back(st);
		}
		match('}');
		return sts;
	}
	Stmt* stmt_decl(){
		Decl *d = new Decl;
		d->line = lexer->line;
		match(INT);
		putId(new Id(Type::Int, (Word*)s, width));
		d->ids.push_back(getId());
		width += Type::Int->width;
		match(ID);
		while (s->kind == ','){
			match(',');
			putId(new Id(Type::Int, (Word*)s, width));
			d->ids.push_back(getId());
			width += Type::Int->width;
			match(ID);
		}
		match(';');
		return d;
	}
	Stmt* stmt_assign(){
		Assign *a = new Assign;
		a->line = lexer->line;
		a->E1 = getId();
		match(ID);
		match('=');
		a->E2 = expr_expr();
		return a;
	}
	Stmt* stmt_if(){
		If *i = new If;
		i->line = lexer->line;
		match(IF);
		match('(');
		i->C = expr_cond();
		match(')');
		i->S1 = stmt();
		if (s->kind == ELSE){
			Else *e = new Else;
			match(ELSE);
			e->line = i->line;
			e->C = i->C;
			e->S1 = i->S1;
			e->S2 = stmt();
			return e;
		}
		return i;
	}
	Stmt* stmt_while(){
		While *w = new While;
		w->line = lexer->line;
		match(WHILE);
		match('(');
		w->C = expr_cond();
		match(')');
		w->S1 = stmt();
		return w;
	}
	Stmt* stmt_do(){
		Do *d = new Do;
		d->line = lexer->line;
		match(DO);
		d->S1 = stmt();
		match(WHILE); 
		match('(');
		d->C = expr_cond(); 
		match(')');
		match(';');
		return d;
	}
	Stmt* stmt_for(){
		For *f = new For;
		f->line = lexer->line;
		match(FOR);
		match('(');
		f->S1 = stmt_assign();
		match(';');
		f->C = expr_cond();
		match(';');
		f->S2 = stmt_assign();
		match(')');
		f->S3 = stmt();
		return f;
	}
	Stmt* stmt_case(){
		Case *c = new Case;
		c->line = lexer->line;
		match(CASE);
		c->E = expr_expr();
		while (s->kind != END){
			Integer *i = (Integer*)s;
			match(INT);
			match(':');
			c->Ss[i->value] = stmt();
		}
		match(END);
		return c;
	}
	Cond* expr_cond()
	{
		Expr* e = expr_expr();
		if (s->kind == '<' || s->kind == '>' || s->kind == '=' || s->kind == '!'){
			char opt = s->kind;
			match(s->kind);
			Expr *r = expr_expr();
			return new Cond(opt, e, r);
		}
		return nullptr;
	}
	Expr* expr_expr()
	{
		Expr* e = expr_term();
		while (s->kind == '+' || s->kind == '-'){
			char opt = s->kind;
			match(s->kind);
			Expr *r = expr_term();
			e = new Arith(opt, e, r);
		}
		return e;
	}
	Expr* expr_term()
	{
		Expr* e = expr_unary();
		while (s->kind == '*' || s->kind == '/' || s->kind == '%')
		{
			char opt = s->kind;
			match(s->kind);
			Expr *r = expr_unary();
			e = new Arith(opt, e, r);
		}
		return e;
	}
	Expr* expr_unary(){
		Expr *u;
		if (s->kind == '~'){
			match('~');
			u = new Unary('~', expr_unary());
		}else{
			u = expr_factor();
		}
		return u;
	}
	Expr* expr_factor()
	{
		Expr* e = nullptr;
		switch (s->kind){
		case '(': match('('); e = expr_expr(); match(')'); break;
		case ID: {
			// 保存当前ID，然后检查下一个token
			string idName = ((Word*)s)->word;
			match(ID);
			// 检查是否是函数调用
			if (s->kind == '('){
				// 这是一个函数调用
				FuncCall *fc = new FuncCall(idName);
				match('(');
				// 解析参数列表
				if (s->kind != ')'){
					fc->args.push_back(expr_expr());
					while (s->kind == ','){
						match(',');
						fc->args.push_back(expr_expr());
					}
				}
				match(')');
				e = fc;
			} else {
				// 这是一个变量引用
				e = getId(idName);
			}
			break;
		}
		case INT:  e = new Number((Integer*)s); match(INT); break;
		default: printf("F->('%c')\n", s->kind); match(s->kind); break;
		}
		return e;
	}
	
	// 函数定义解析
	Stmt* stmt_func(){
		FuncDef *f = new FuncDef;
		f->line = lexer->line;
		match(FUNC);
		f->name = ((Word*)s)->word;
		match(ID);
		match('(');
		// 解析参数列表
		if (s->kind != ')'){
			putId(new Id(Type::Int, (Word*)s, width));
			f->params.push_back(getId());
			width += Type::Int->width;
			match(ID);
			while (s->kind == ','){
				match(',');
				putId(new Id(Type::Int, (Word*)s, width));
				f->params.push_back(getId());
				width += Type::Int->width;
				match(ID);
			}
		}
		match(')');
		f->body = stmt();
		return f;
	}
	
	// 返回语句解析
	Stmt* stmt_return(){
		Return *r = new Return;
		r->line = lexer->line;
		match(RETURN);
		if (s->kind != ';'){
			r->value = expr_expr();
		}
		match(';');
		return r;
	}
	

public:
	Parser(string fp){
		lexer = new Lexer(fp);
	}
	~Parser(){
		printf("~Parser");
		delete lexer;
	}
	Stmt* parse(){
		Stmts *sts = new Stmts;
		sts->line = lexer->line;
		s = lexer->scan();// 预读一个符号
		while (s->kind != '#'){
			Stmt *st = stmt();
			if (st)sts->Ss.push_back(st);
		}
		return sts;
	}
};

#endif