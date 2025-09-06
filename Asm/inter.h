#include "lexer.h"
#include <list>
#include <vector>
#include <string>
// TAC 相关定义已移动到 Optimizer 模块

struct Label {
	Word *w;
	WORD offset;
	Label(Word *w, int offset) :w(w), offset(offset) { }
};

struct Code {
	BYTE opt;
	WORD line = 0; // 当前指令在源文件中的行号
	WORD width = 0; // 当前指令占用的字宽
	WORD offset = 0; // 当前指令的偏移量
	
	// 构造函数
	Code() : opt(0) {}
	Code(BYTE op) : opt(op) {}
	
	virtual void code(FILE* fp);
};

// 标签代码类
class LabelCode : public Code {
public:
	string labelName;
	LabelCode(const string& name) : Code(), labelName(name) {}
	virtual void code(FILE* fp) {
		// printf("[%04d][%04d][%04x]label      %s:\n", line, width, offset, labelName.c_str());
		(void)fp; // 避免未使用参数警告
	}
};

// 寄存器AST节点
struct Register :Code {
	BYTE reg_num;
	Register(BYTE num) :Code(0), reg_num(num) {}
	virtual void code(FILE* fp);
};

// 立即数AST节点
struct Immediate :Code {
	WORD value;
	Immediate(WORD val) :Code(0), value(val) {}
	virtual void code(FILE* fp);
};

// 地址AST节点
struct Address :Code {
	WORD addr;
	Address(WORD address) :Code(0), addr(address) {}
	virtual void code(FILE* fp);
};

struct Codes :Code {
	list<Code*> codes;
	Codes() :Code(0) {}
	virtual void code(FILE* fp);
};

struct Data :Code {
	Data() :Code(DATA) {}
	virtual void code(FILE* fp);
};

// 变量定义结构体
struct Variable :Code {
	string name;     // 变量名
	WORD value;      // 变量值
	Variable(string n, WORD v) :Code(VAR), name(n), value(v) {}
	virtual void code(FILE* fp);
};

struct StringConstant :Code {
	string value;    // 字符串值
	WORD addr;       // 字符串在内存中的地址
	StringConstant(string v, WORD a) :Code(VAR), value(v), addr(a) {}
	virtual void code(FILE* fp);
};

struct Load :Code {
	BYTE reg;
	WORD addr;
	Load(BYTE r, WORD a) :Code(LOAD), reg(r), addr(a) {}
	Load(BYTE op, BYTE r, WORD a) :Code(op), reg(r), addr(a) {}
	virtual void code(FILE* fp);
};

struct Store :Code {
	BYTE reg;
	WORD addr;
	Store(BYTE r, WORD a) :Code(STORE), reg(r), addr(a) {}
	Store(BYTE op, BYTE r, WORD a) :Code(op), reg(r), addr(a) {}
	virtual void code(FILE* fp);
};

struct Lea :Code {
	BYTE reg;
	WORD addr;
	Lea(BYTE r, WORD a) :Code(LEA), reg(r), addr(a) {}
	Lea(BYTE op, BYTE r, WORD a) :Code(op), reg(r), addr(a) {}
	virtual void code(FILE* fp);
};

struct Halt :Code {
	Halt() :Code(HALT) {}
	virtual void code(FILE *fp);
};

struct Push :Code {
	BYTE reg;
	Push(BYTE r) :Code(PUSH), reg(r) {}
	virtual void code(FILE *fp);
};

struct Pop :Code {
	BYTE reg;
	Pop(BYTE r) :Code(POP), reg(r) {}
	virtual void code(FILE *fp);
};

struct Mov :Code {
	BYTE reg1, reg2;
	Mov(BYTE r1, BYTE r2) :Code(MOV), reg1(r1), reg2(r2) {}
	virtual void code(FILE *fp);
};

struct In :Code {
	BYTE reg;
	BYTE port;
	In(BYTE r, BYTE p) :Code(IN), reg(r), port(p) {}
	virtual void code(FILE *fp);
};

struct Out :Code {
	BYTE reg;
	BYTE port;
	Out(BYTE r, BYTE p) :Code(OUT), reg(r), port(p) {}
	virtual void code(FILE *fp);
};

struct Add :Code {
	BYTE reg1, reg2;
	Add(BYTE r1, BYTE r2) :Code(ADD), reg1(r1), reg2(r2) {}
	virtual void code(FILE *fp);
};

struct Ret :Code {
	Ret() :Code(RET) {}
	virtual void code(FILE *fp);
};

struct Int :Code {
	BYTE vector;
	Int(BYTE v) :Code(INT_INST), vector(v) {}
	virtual void code(FILE *fp);
};

struct Iret :Code {
	Iret() :Code(IRET) {}
	virtual void code(FILE *fp);
};

struct Cli :Code {
	Cli() :Code(CLI_INST) {}
	virtual void code(FILE *fp);
};

struct Sti :Code {
	Sti() :Code(STI_INST) {}
	virtual void code(FILE *fp);
};

struct Call :Code {
	Label *addr;
	Call(Label *a) :Code(CALL), addr(a) {}
	virtual void code(FILE* fp);
};

struct Jmp :Code {
	Label *addr;
	Jmp(BYTE op, Label *a) :Code(op), addr(a) {}
	virtual void code(FILE* fp);
};

struct Arith :Code {
	BYTE reg1, reg2, reg3;
	Arith(BYTE op, BYTE r1, BYTE r2, BYTE r3) :Code(op), reg1(r1), reg2(r2), reg3(r3) {}
	virtual void code(FILE* fp);
};

struct Unary :Code {
	BYTE reg1, reg2;
	Unary(BYTE op, BYTE r1, BYTE r2) :Code(op), reg1(r1), reg2(r2) {}
	virtual void code(FILE* fp);
};

// 前向声明
class Lexer;

// 指令工厂类
class InstructionFactory {
private:
	Lexer* lexer;
	
	// 指令宽度查找表
	static const WORD instructionWidths[256];
	
	// 内存布局管理
	WORD dataSize;      // 数据段大小
	WORD codeSize;      // 代码段大小
	WORD dataOffset;    // 数据段偏移
	WORD codeOffset;    // 代码段偏移
	
	// 标签管理
	map<string, Label*> labels;
	
	// 三地址码存储
	vector<Code*> instructions;
	
	// 初始化指令宽度
	void initializeWidth(Code* code);
	
	// 更新内存布局
	void updateMemoryLayout(Code* code);
	
public:
	// 构造函数
	InstructionFactory(Lexer* l) : lexer(l), dataSize(0), codeSize(0), dataOffset(0), codeOffset(0) {}
	
	// 获取当前行号
	WORD getCurrentLine() const;
	
	// 内存布局管理方法
	WORD getDataSize() const { return dataSize; }
	WORD getCodeSize() const { return codeSize; }
	WORD getDataOffset() const { return dataOffset; }
	WORD getCodeOffset() const { return codeOffset; }
	WORD getCS() const { return dataSize; }  // CS = 数据段结束地址
	
	// 标签管理方法
	Label* getLabel(const string& name);
	Label* createLabel(const string& name);
	void updateLabelOffset(const string& name, WORD offset);
	map<string, Label*>& getLabels() { return labels; }
	
	// 创建各种指令的工厂方法
	Load* createLoad(Integer* regObj, Integer* addrObj) {
		if (!regObj || !addrObj) return nullptr;
		Load* l = new Load(regObj->value, addrObj->value);
		l->line = getCurrentLine();
		initializeWidth(l);
		updateMemoryLayout(l);
		return l;
	}

	Load* createLoad(Integer* optObj, Integer* regObj, Integer* addrObj) {
		if (!optObj || !regObj || !addrObj) return nullptr;
		Load* l = new Load(optObj->value, regObj->value, addrObj->value);
		l->line = getCurrentLine();
		initializeWidth(l);
		updateMemoryLayout(l);
		return l;
	}

	Store* createStore(Integer* regObj, Integer* addrObj) {
		if (!regObj || !addrObj) return nullptr;
		Store* s = new Store(regObj->value, addrObj->value);
		s->line = getCurrentLine();
		initializeWidth(s);
		updateMemoryLayout(s);
		return s;
	}

	Store* createStore(Integer* optObj, Integer* regObj, Integer* addrObj) {
		if (!optObj || !regObj || !addrObj) return nullptr;
		Store* s = new Store(optObj->value, regObj->value, addrObj->value);
		s->line = getCurrentLine();
		initializeWidth(s);
		updateMemoryLayout(s);
		return s;
	}

	Lea* createLea(Integer* regObj, Integer* addrObj) {
		if (!regObj || !addrObj) return nullptr;
		Lea* l = new Lea(regObj->value, addrObj->value);
		l->line = getCurrentLine();
		initializeWidth(l);
		updateMemoryLayout(l);
		return l;
	}

	Lea* createLea(Integer* optObj, Integer* regObj, Integer* addrObj) {
		if (!optObj || !regObj || !addrObj) return nullptr;
		Lea* l = new Lea(optObj->value, regObj->value, addrObj->value);
		l->line = getCurrentLine();
		initializeWidth(l);
		updateMemoryLayout(l);
		return l;
	}

	Push* createPush(Integer* regObj) {
		if (!regObj) return nullptr;
		Push* p = new Push(regObj->value);
		p->line = getCurrentLine();
		initializeWidth(p);
		updateMemoryLayout(p);
		return p;
	}

	Pop* createPop(Integer* regObj) {
		if (!regObj) return nullptr;
		Pop* p = new Pop(regObj->value);
		p->line = getCurrentLine();
		initializeWidth(p);
		updateMemoryLayout(p);
		return p;
	}

	Mov* createMov(Integer* reg1Obj, Integer* reg2Obj) {
		if (!reg1Obj || !reg2Obj) return nullptr;
		Mov* m = new Mov(reg1Obj->value, reg2Obj->value);
		m->line = getCurrentLine();
		initializeWidth(m);
		updateMemoryLayout(m);
		return m;
	}

	In* createIn(Integer* regObj, Integer* portObj) {
		if (!regObj || !portObj) return nullptr;
		In* i = new In(regObj->value, portObj->value);
		i->line = getCurrentLine();
		initializeWidth(i);
		updateMemoryLayout(i);
		return i;
	}

	Add* createAdd(Integer* reg1Obj, Integer* reg2Obj) {
		if (!reg1Obj || !reg2Obj) return nullptr;
		Add* a = new Add(reg1Obj->value, reg2Obj->value);
		a->line = getCurrentLine();
		initializeWidth(a);
		updateMemoryLayout(a);
		return a;
	}

	Ret* createRet() {
		Ret* r = new Ret();
		r->line = getCurrentLine();
		initializeWidth(r);
		updateMemoryLayout(r);
		return r;
	}

	Arith* createArith(Integer* opObj, Integer* reg1Obj, Integer* reg2Obj, Integer* reg3Obj) {
		if (!opObj || !reg1Obj || !reg2Obj || !reg3Obj) return nullptr;
		Arith* a = new Arith(opObj->value, reg1Obj->value, reg2Obj->value, reg3Obj->value);
		a->line = getCurrentLine();
		initializeWidth(a);
		updateMemoryLayout(a);
		return a;
	}

	Unary* createUnary(Integer* opObj, Integer* reg1Obj, Integer* reg2Obj) {
		if (!opObj || !reg1Obj || !reg2Obj) return nullptr;
		Unary* u = new Unary(opObj->value, reg1Obj->value, reg2Obj->value);
		u->line = getCurrentLine();
		initializeWidth(u);
		updateMemoryLayout(u);
		return u;
	}
	
	
	Int* createInt(Integer* intObj) {
		if (!intObj) return nullptr;
		Int* i = new Int(intObj->value);
		i->line = getCurrentLine();
		initializeWidth(i);
		updateMemoryLayout(i);
		return i;
	}
	
	Iret* createIret() {
		Iret* ir = new Iret();
		ir->line = getCurrentLine();
		initializeWidth(ir);
		updateMemoryLayout(ir);
		return ir;
	}
	
	Cli* createCli() {
		Cli* c = new Cli();
		c->line = getCurrentLine();
		initializeWidth(c);
		updateMemoryLayout(c);
		return c;
	}
	
	Sti* createSti() {
		Sti* s = new Sti();
		s->line = getCurrentLine();
		initializeWidth(s);
		updateMemoryLayout(s);
		return s;
	}
	
	Call* createCall(const string& labelName) {
		Label* addr = getLabel(labelName);
		Call* c = new Call(addr);
		c->line = getCurrentLine();
		initializeWidth(c);
		updateMemoryLayout(c);
		return c;
	}
	
	Jmp* createJmp(BYTE op, const string& labelName) {
		Label* addr = getLabel(labelName);
		Jmp* j = new Jmp(op, addr);
		j->line = getCurrentLine();
		initializeWidth(j);
		updateMemoryLayout(j);
		return j;
	}
	
	// 创建标签代码
	Code* createLabelCode(const string& name) {
		// 使用createLabel来确保标签的offset被正确设置
		createLabel(name);
		LabelCode* c = new LabelCode(name);
		c->line = getCurrentLine();
		c->width = 0; // 标签不占用机器代码空间
		return c;
	}
	
	// 创建数据段指令
	Data* createData(WORD size) {
		Data* d = new Data();
		d->line = getCurrentLine();
		d->width = size;
		updateMemoryLayout(d);
		return d;
	}
	
	// 创建HALT指令
	Halt* createHalt() {
		Halt* h = new Halt();
		h->line = getCurrentLine();
		initializeWidth(h);
		updateMemoryLayout(h);
		return h;
	}
	
	// 创建变量
	Variable* createVariable(const string& name, Integer* valueObj) {
		if (!valueObj) return nullptr;
		Variable* v = new Variable(name, valueObj->value);
		v->line = getCurrentLine();
		initializeWidth(v);
		updateMemoryLayout(v);
		return v;
	}
	
	// 创建字符串常量
	StringConstant* createStringConstant(const string& value, WORD addr) {
		StringConstant* sc = new StringConstant(value, addr);
		sc->line = getCurrentLine();
		initializeWidth(sc);
		updateMemoryLayout(sc);
		return sc;
	}
	
	// 创建Out指令
	Out* createOut(Integer* regObj, Integer* portObj) {
		if (!regObj || !portObj) return nullptr;
		Out* o = new Out(regObj->value, portObj->value);
		o->line = getCurrentLine();
		initializeWidth(o);
		updateMemoryLayout(o);
		return o;
	}
	
	// 创建Load指令（带操作码版本）
	Load* createLoad(BYTE opt, BYTE reg, WORD addr) {
		Load* l = new Load(opt, reg, addr);
		l->line = getCurrentLine();
		initializeWidth(l);
		updateMemoryLayout(l);
		return l;
	}
	
	// 创建Store指令（带操作码版本）
	Store* createStore(BYTE opt, BYTE reg, WORD addr) {
		Store* s = new Store(opt, reg, addr);
		s->line = getCurrentLine();
		initializeWidth(s);
		updateMemoryLayout(s);
		return s;
	}
	
	// 创建Lea指令（带操作码版本）
	Lea* createLea(BYTE opt, BYTE reg, WORD addr) {
		Lea* l = new Lea(opt, reg, addr);
		l->line = getCurrentLine();
		initializeWidth(l);
		updateMemoryLayout(l);
		return l;
	}
	
	// 创建Arith指令（带操作码版本）
	Arith* createArith(BYTE op, BYTE reg1, BYTE reg2, BYTE reg3) {
		Arith* a = new Arith(op, reg1, reg2, reg3);
		a->line = getCurrentLine();
		initializeWidth(a);
		updateMemoryLayout(a);
		return a;
	}
	
};
