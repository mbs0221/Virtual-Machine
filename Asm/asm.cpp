#include "asm.h"

// 辅助函数实现
bool Asm::match(int kind) {
	if (s->kind == kind) {
		s = lexer->scan();
		return true;
	}
	s = lexer->scan();
	return false;
}

// 解析函数 - 返回值的版本
BYTE Asm::parse_register_value() {
	if (s->kind != '$') {
		printf("[%3d]语法错误: 期望寄存器符号 '$'，但得到 '%c'\n", lexer->line, s->kind);
		return 0xFF; // 错误标记
	}
	match('$');
	if (s->kind != INT) {
		printf("[%3d]语法错误: 期望寄存器编号，但得到 '%c'\n", lexer->line, s->kind);
		return 0xFF; // 错误标记
	}
	BYTE reg = ((Integer*)s)->value;
	match(INT);
	return reg;
}

WORD Asm::parse_immediate_value() {
	if (s->kind != '#') {
		printf("[%3d]语法错误: 期望立即数符号 '#'，但得到 '%c'\n", lexer->line, s->kind);
		return 0xFFFF; // 错误标记
	}
	match('#');
	if (s->kind != INT) {
		printf("[%3d]语法错误: 期望立即数值，但得到 '%c'\n", lexer->line, s->kind);
		return 0xFFFF; // 错误标记
	}
	WORD value = ((Integer*)s)->value;
	match(INT);
	return value;
}

WORD Asm::parse_address_value() {
	if (s->kind != '*') {
		printf("[%3d]语法错误: 期望地址符号 '*'，但得到 '%c'\n", lexer->line, s->kind);
		return 0xFFFF; // 错误标记
	}
	match('*');
	if (s->kind != INT) {
		printf("[%3d]语法错误: 期望地址值，但得到 '%c'\n", lexer->line, s->kind);
		return 0xFFFF; // 错误标记
	}
	WORD value = ((Integer*)s)->value;
	match(INT);
	return value;
}

bool Asm::parse_addressing_mode(BYTE& opt, WORD& addr) {
	if (s->kind == '#') {
		opt |= MR_A; // 立即数寻址
		addr = parse_immediate_value();
		return (addr != 0xFFFF); // 检查是否解析成功
	} else if (s->kind == '*') {
		opt |= MR_B; // 直接寻址
		addr = parse_address_value();
		return (addr != 0xFFFF); // 检查是否解析成功
	} else {
		printf("[%3d]语法错误: 期望立即数符号 '#' 或地址符号 '*'，但得到 '%c'\n", lexer->line, s->kind);
		return false;
	}
}

// 旧版解析函数（将被逐步替换）
Code* Asm::data() {
	Data *d = new Data;
	d->opt = 0; // 修复NULL赋值警告
	d->line = lexer->line;
	match(DATA);
	d->width = ((Integer*)s)->value;
	DS = 0;
	CS = d->width;
	match(INT);
	match(DATA);
	return d;
}

Code* Asm::halt() {
	Halt *h = new Halt;
	h->line = lexer->line;
	h->opt = HALT;
	h->width = 1;
	match(HALT);
	return h;
}

Code* Asm::call() {
	Call *c = new Call;
	c->line = lexer->line;
	c->opt = CALL;
	match(CALL);
	if (lables.find(((Word*)s)->word) == lables.end()) {
		c->addr = new Label((Word*)s, cs->width);
		lables[((Word*)s)->word] = c->addr;
	} else {
		c->addr = lables[((Word*)s)->word];
	}
	match(ID);
	c->width = 3;
	return c;
}

Code* Asm::add() {
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

Code* Asm::ret() {
	Ret *r = new Ret;
	r->line = lexer->line;
	r->opt = RET;
	r->width = 1;
	match(RET);
	return r;
}

Code* Asm::label() {
	match(LABEL);
	if (lables.find(((Word*)s)->word) == lables.end()) {
		lables[((Word*)s)->word] = new Label((Word*)s, cs->width);
	} else {
		lables[((Word*)s)->word]->offset = cs->width;
	}
	match(ID);
	match(':');
	return nullptr;
}

Code* Asm::unary() {
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

Code* Asm::arith(BYTE b) {
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

Code* Asm::jmp(BYTE b) {
	Jmp *j = new Jmp;
	j->line = lexer->line;
	j->opt = b;
	match(s->kind);
	if (lables.find(((Word*)s)->word) == lables.end()) {
		j->addr = new Label((Word*)s, cs->width);
		lables[((Word*)s)->word] = j->addr;
	} else {
		j->addr = lables[((Word*)s)->word];
	}
	match(ID);
	j->width = 3;
	return j;
}

// 构造函数
Asm::Asm(string fp) {
	lexer = new Lexer(fp);
}

// 主解析函数
void Asm::parse() {
	cs = new Codes;
	s = lexer->scan();
	
	while (s->kind != END) {
		Code *c = parse_statement();
		if (c) {
			cs->codes.push_back(c);
			c->offset = cs->width;
			cs->width += c->width;
		} else {
			// 解析失败，停止汇编
			printf("[%3d]汇编失败，停止解析\n", lexer->line);
			break;
		}
	}
}

// LL(1)递归下降解析函数
Code* Asm::parse_statement() {
			switch (s->kind) {
		case DATA:
			return parse_data_statement();
		case VAR:
			return parse_variable_statement();
		case FS:
			return parse_fs_statement();
		case GS:
			return parse_gs_statement();
		case LOAD:
			return parse_load_statement();
		case STORE:
			return parse_store_statement();
		case HALT:
			return parse_halt_statement();
		case CALL:
			return parse_call_statement();
		case RET:
			return parse_ret_statement();
		case PUSH:
			return parse_push_statement();
		case POP:
			return parse_pop_statement();
		case MOV:
			return parse_mov_statement();
		case ADD:
			return parse_add_statement();
		case LABEL:
			return parse_label_statement();
		case JE:
		case JNE:
		case JB:
		case JG:
		case JMP:
			return parse_jump_statement();
		case '~':
		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
		case '<':
		case '>':
		case '=':
		case '!':
			return parse_arithmetic_statement();
		case ID:
			printf("[%3d]语法错误: 意外的标识符 '%s'\n", lexer->line, ((Word*)s)->word.c_str());
			return nullptr;
		default:
			printf("[%3d]语法错误: 不支持的指令或符号 '%c' (代码: %d)\n", lexer->line, s->kind, s->kind);
			return nullptr;
		}
}

// 解析data语句: data <value>
Code* Asm::parse_data_statement() {
	match(DATA);
	if (s->kind != INT) {
		printf("[%3d]语法错误: 期望数值，但得到 '%c'\n", lexer->line, s->kind);
		return nullptr;
	}
	Data *d = new Data;
	d->line = lexer->line;
	d->width = ((Integer*)s)->value;
	DS = 0;
	CS = d->width;
	match(INT);
	return d;
}

// 解析变量定义语句: var <name> <value>
Code* Asm::parse_variable_statement() {
	match(VAR);
	
	// 解析变量名
	if (s->kind != ID) {
		printf("[%3d]语法错误: 期望变量名，但得到 '%c'\n", lexer->line, s->kind);
		return nullptr;
	}
	string varName = ((Word*)s)->word;
	match(ID);
	
	// 解析变量值
	if (s->kind != INT) {
		printf("[%3d]语法错误: 期望变量值，但得到 '%c'\n", lexer->line, s->kind);
		return nullptr;
	}
	WORD varValue = ((Integer*)s)->value;
	match(INT);
	
	// 创建变量定义
	Variable *v = new Variable(varName, varValue);
	v->line = lexer->line;
	v->width = 2; // 一个WORD的大小
	return v;
}

// 解析FS段寄存器设置语句: fs <value>
Code* Asm::parse_fs_statement() {
	match(FS);
	if (s->kind != INT) {
		printf("[%3d]语法错误: 期望数值，但得到 '%c'\n", lexer->line, s->kind);
		return nullptr;
	}
	Data *d = new Data;
	d->line = lexer->line;
	d->width = ((Integer*)s)->value;
	// 这里可以设置FS段寄存器，但需要扩展Data结构体或创建新的结构体
	match(INT);
	return d;
}

// 解析GS段寄存器设置语句: gs <value>
Code* Asm::parse_gs_statement() {
	match(GS);
	if (s->kind != INT) {
		printf("[%3d]语法错误: 期望数值，但得到 '%c'\n", lexer->line, s->kind);
		return nullptr;
	}
	Data *d = new Data;
	d->line = lexer->line;
	d->width = ((Integer*)s)->value;
	// 这里可以设置GS段寄存器，但需要扩展Data结构体或创建新的结构体
	match(INT);
	return d;
}

// 解析load语句: load <register> <immediate_or_address>
Code* Asm::parse_load_statement() {
	match(LOAD);
	
	// 解析寄存器
	BYTE reg = parse_register_value();
	if (reg == 0xFF) return nullptr; // 解析失败
	
	// 解析寻址模式
	BYTE opt = LOAD;
	WORD addr = 0;
	if (!parse_addressing_mode(opt, addr)) {
		return nullptr; // 解析失败
	}
	
	Load *l = new Load;
	l->line = lexer->line;
	l->opt = opt;
	l->reg = reg;
	l->addr = addr;
	l->width = 4;
	return l;
}

// 解析store语句: store <register> <immediate_or_address>
Code* Asm::parse_store_statement() {
	match(STORE);
	
	// 解析寄存器
	BYTE reg = parse_register_value();
	if (reg == 0xFF) return nullptr; // 解析失败
	
	// 解析寻址模式
	BYTE opt = STORE;
	WORD addr = 0;
	if (!parse_addressing_mode(opt, addr)) {
		return nullptr; // 解析失败
	}
	
	Store *st = new Store;
	st->line = lexer->line;
	st->opt = opt;
	st->reg = reg;
	st->addr = addr;
	st->width = 4;
	return st;
}

// 解析halt语句: halt
Code* Asm::parse_halt_statement() {
	match(HALT);
	Halt *h = new Halt;
	h->line = lexer->line;
	h->opt = HALT;
	h->width = 1;
	return h;
}

// 解析push语句: push <register>
Code* Asm::parse_push_statement() {
	match(PUSH);
	
	// 解析寄存器
	BYTE reg = parse_register_value();
	if (reg == 0xFF) return nullptr; // 解析失败
	
	Push *p = new Push;
	p->line = lexer->line;
	p->opt = PUSH;
	p->reg = reg;
	p->width = 2;
	return p;
}

// 解析pop语句: pop <register>
Code* Asm::parse_pop_statement() {
	match(POP);
	
	// 解析寄存器
	BYTE reg = parse_register_value();
	if (reg == 0xFF) return nullptr; // 解析失败
	
	Pop *p = new Pop;
	p->line = lexer->line;
	p->opt = POP;
	p->reg = reg;
	p->width = 2;
	return p;
}

// 解析mov语句: mov <register> <register>
Code* Asm::parse_mov_statement() {
	match(MOV);
	
	// 解析第一个寄存器
	BYTE reg1 = parse_register_value();
	if (reg1 == 0xFF) return nullptr; // 解析失败
	
	// 解析第二个寄存器
	BYTE reg2 = parse_register_value();
	if (reg2 == 0xFF) return nullptr; // 解析失败
	
	Mov *m = new Mov;
	m->line = lexer->line;
	m->opt = MOV;
	m->reg1 = reg1;
	m->reg2 = reg2;
	m->width = 3;
	return m;
}

// 解析其他语句的占位符
Code* Asm::parse_call_statement() { return call(); }
Code* Asm::parse_ret_statement() { return ret(); }
Code* Asm::parse_add_statement() { return add(); }
Code* Asm::parse_label_statement() { return label(); }
Code* Asm::parse_jump_statement() { return jmp(s->kind); }
Code* Asm::parse_arithmetic_statement() { return arith(s->kind); }

// 输出函数
void Asm::write(FILE *fp) {
	fwrite(&DS, sizeof(WORD), 1, fp);
	fwrite(&CS, sizeof(WORD), 1, fp);
	fwrite(&cs->width, sizeof(WORD), 1, fp);
	cs->code(fp);
}
