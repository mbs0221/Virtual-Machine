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
		case INT:
		case CHAR:
		case FLOAT:
		case BOOL:
		case VOID:
			st = stmt_decl(); break;
		case ID:st = stmt_assign(); break;
		case IF:st = stmt_if(); break;
		case WHILE:st = stmt_while(); break;
		case DO:st = stmt_do(); break;
		case FOR:st = stmt_for(); break;
		case SWITCH:st = stmt_switch(); break;
		case CASE:st = stmt_case(); break;
		case FUNC:st = stmt_func(); break;
		case RETURN:st = stmt_return(); break;
		case PRINT:st = stmt_print(); break;
		case SCAN:st = stmt_scan(); break;
		case BREAK:st = stmt_break(); break;
		case CONTINUE:st = stmt_continue(); break;
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
		
		// 获取类型
		Type *type = nullptr;
		switch (s->kind){
			case INT: type = Type::Int; match(INT); break;
			case CHAR: type = Type::Char; match(CHAR); break;
			case FLOAT: type = Type::Float; match(FLOAT); break;
			case BOOL: type = Type::Bool; match(BOOL); break;
			case VOID: type = Type::Void; match(VOID); break;
			default: printf("未知类型\n"); return nullptr;
		}
		
		// 保存变量名
		string varName = ((Word*)s)->word;
		putId(new Id(type, (Word*)s, width));
		d->ids.push_back(getId());
		width += type->width;
		match(ID);
		
		// 检查是否有初始化
		if (s->kind == '='){
			match('=');
			// 创建赋值语句
			Assign *a = new Assign;
			a->line = lexer->line;
			a->E1 = getId(varName);
			a->E2 = expr_expr();
			match(';');
			return a;
		}
		
		while (s->kind == ','){
			match(',');
			putId(new Id(type, (Word*)s, width));
			d->ids.push_back(getId());
			width += type->width;
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
		case FLOAT: e = new FloatNumber((Float*)s); match(FLOAT); break;
		case STRING: e = new StringLiteral((String*)s); match(STRING); break;
		case TRUE: e = new BoolLiteral(true); match(TRUE); break;
		case FALSE: e = new BoolLiteral(false); match(FALSE); break;
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
			// 跳过类型关键字
			if (s->kind == INT) match(INT);
			else if (s->kind == CHAR) match(CHAR);
			else if (s->kind == FLOAT) match(FLOAT);
			else if (s->kind == BOOL) match(BOOL);
			else if (s->kind == VOID) match(VOID);
			
			// 现在s指向参数名
			putId(new Id(Type::Int, (Word*)s, width));
			f->params.push_back(getId());
			width += Type::Int->width;
			match(ID);
			while (s->kind == ','){
				match(',');
				// 跳过类型关键字
				if (s->kind == INT) match(INT);
				else if (s->kind == CHAR) match(CHAR);
				else if (s->kind == FLOAT) match(FLOAT);
				else if (s->kind == BOOL) match(BOOL);
				else if (s->kind == VOID) match(VOID);
				
				// 现在s指向参数名
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
	
	// 打印语句解析
	Stmt* stmt_print(){
		Print *p = new Print;
		p->line = lexer->line;
		match(PRINT);
		match('(');
		if (s->kind != ')'){
			p->args.push_back(expr_expr());
			while (s->kind == ','){
				match(',');
				p->args.push_back(expr_expr());
			}
		}
		match(')');
		match(';');
		return p;
	}
	
	// 输入语句解析
	Stmt* stmt_scan(){
		Scan *sc = new Scan;
		sc->line = lexer->line;
		match(SCAN);
		match('(');
		sc->var = getId();
		match(ID);
		match(')');
		match(';');
		return sc;
	}
	
	// 中断语句解析
	Stmt* stmt_break(){
		Break *b = new Break;
		b->line = lexer->line;
		match(BREAK);
		match(';');
		return b;
	}
	
	// 继续语句解析
	Stmt* stmt_continue(){
		Continue *c = new Continue;
		c->line = lexer->line;
		match(CONTINUE);
		match(';');
		return c;
	}
	
	// 开关语句解析
	Stmt* stmt_switch(){
		Switch *sw = new Switch;
		sw->line = lexer->line;
		match(SWITCH);
		match('(');
		sw->expr = expr_expr();
		match(')');
		match('{');
		while (s->kind == CASE){
			match(CASE);
			Integer *caseValue = (Integer*)s;
			match(INT);
			match(':');
			sw->cases[caseValue->value] = stmt();
		}
		if (s->kind == DEFAULT){
			match(DEFAULT);
			match(':');
			sw->defaultCase = stmt();
		}
		match('}');
		return sw;
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