#include "lexer.h"

#ifndef __INTER_H_
#define __INTER_H_

#include <list>

struct Node{
	virtual void code(FILE *fp){
		(void)fp; // 避免未使用参数警告
	}
};

// 寄存器分配器
class RegisterAllocator {
private:
	static int nextReg;
	static int maxReg;
	static bool usedRegs[256]; // 跟踪寄存器使用状态
	static int stack[256];     // 寄存器栈，用于回收
	static int stackTop;       // 栈顶指针
	
public:
	static int allocate() {
		// 首先尝试从回收栈中获取寄存器
		if (stackTop > 0) {
			int reg = stack[--stackTop];
			usedRegs[reg] = true;
			return reg;
		}
		
		// 如果没有回收的寄存器，分配新的
		if (nextReg >= maxReg) {
			// 寄存器用完了，需要溢出到内存
			return -1; // 表示需要溢出
		}
		usedRegs[nextReg] = true;
		return nextReg++;
	}
	
	static void free(int reg) {
		if (reg >= 2 && reg < maxReg && usedRegs[reg]) {
			usedRegs[reg] = false;
			// 将寄存器推入回收栈
			stack[stackTop++] = reg;
		}
	}
	
	static void reset() {
		nextReg = 2; // 从寄存器2开始分配，0和1保留给特殊用途
		stackTop = 0;
		// 清空所有寄存器使用状态
		for (int i = 0; i < 256; i++) {
			usedRegs[i] = false;
		}
	}
	
	static int getMaxUsed() {
		return nextReg;
	}
	
	static void printStatus() {
		printf("寄存器使用状态: ");
		for (int i = 2; i < nextReg; i++) {
			if (usedRegs[i]) {
				printf("R%d ", i);
			}
		}
		printf("\n回收栈: ");
		for (int i = 0; i < stackTop; i++) {
			printf("R%d ", stack[i]);
		}
		printf("\n");
	}
};

//表达式
struct Expr :Node{
	char opt;
	int label;
	static int count;
	Expr(char opt) :opt(opt){ 
		label = RegisterAllocator::allocate();
		if (label == -1) {
			// 寄存器溢出，使用内存
			label = count++;
		}
	}
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
		// 运算完成后，可以回收操作数的寄存器
		RegisterAllocator::free(E1->label);
		RegisterAllocator::free(E2->label);
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

// 常量表达式基类
struct ConstantExpr :Expr{
	enum ConstantType { INT_CONST, FLOAT_CONST, BOOL_CONST, CHAR_CONST };
	ConstantType constType;
	
	ConstantExpr(ConstantType type) :Expr('@'), constType(type) {}
	
	// 获取整数值
	virtual int getIntValue() const { return 0; }
	// 获取浮点数值
	virtual float getFloatValue() const { return 0.0f; }
	// 获取布尔值
	virtual bool getBoolValue() const { return false; }
	// 获取字符值
	virtual char getCharValue() const { return 0; }
};

// 整数常量表达式（从Number重构）
struct Number :ConstantExpr{
	Integer *s;
	Number(Integer *s) :ConstantExpr(INT_CONST), s(s){  }
	virtual int getIntValue() const override { return s->value; }
	virtual void code(FILE *fp){
		Expr::code(fp);
		fprintf(fp, "load $%d #%d\n", label, s->value);
	}
};

// 浮点数常量表达式（从FloatNumber重构）
struct FloatNumber :ConstantExpr{
	Float *s;
	FloatNumber(Float *s) :ConstantExpr(FLOAT_CONST), s(s){  }
	virtual float getFloatValue() const override { return s->value; }
	virtual void code(FILE *fp){
		Expr::code(fp);
		// 将浮点数转换为整数表示（简化处理）
		int intValue = (int)(s->value * 1000); // 保留3位小数
		fprintf(fp, "load $%d #%d\n", label, intValue);
	}
};

// 字符串常量表达式（从StringLiteral重构）
struct StringLiteral :ConstantExpr{
	String *s;
	StringLiteral(String *s) :ConstantExpr(INT_CONST), s(s){  } // 暂时用INT_CONST表示
	virtual void code(FILE *fp){
		Expr::code(fp);
		// 字符串字面量处理（简化）
		fprintf(fp, "load $%d #%ld\n", label, s->value.length());
	}
};

// 布尔常量表达式（从BoolLiteral重构）
struct BoolLiteral :ConstantExpr{
	bool value;
	BoolLiteral(bool val) :ConstantExpr(BOOL_CONST), value(val){  }
	virtual bool getBoolValue() const override { return value; }
	virtual void code(FILE *fp){
		Expr::code(fp);
		fprintf(fp, "load $%d #%d\n", label, value ? 1 : 0);
	}
};

// 字符常量表达式
struct CharConstant :ConstantExpr{
	char value;
	CharConstant(char val) :ConstantExpr(CHAR_CONST), value(val) {}
	virtual char getCharValue() const override { return value; }
	virtual void code(FILE *fp){
		Expr::code(fp);
		fprintf(fp, "load $%d #%d\n", label, (int)value);
	}
};

// 常量折叠算术表达式
struct ConstArith :ConstantExpr{
	char opt;
	ConstantExpr *E1, *E2;
	bool isFolded;
	int foldedIntValue;
	float foldedFloatValue;
	
	ConstArith(char op, ConstantExpr *e1, ConstantExpr *e2) 
		: ConstantExpr(INT_CONST), opt(op), E1(e1), E2(e2), isFolded(false), foldedIntValue(0), foldedFloatValue(0.0f) {
		// 执行常量折叠
		foldConstants();
	}
	
	void foldConstants() {
		if (E1->constType == INT_CONST && E2->constType == INT_CONST) {
			int val1 = E1->getIntValue();
			int val2 = E2->getIntValue();
			
			switch (opt) {
				case '+': foldedIntValue = val1 + val2; break;
				case '-': foldedIntValue = val1 - val2; break;
				case '*': foldedIntValue = val1 * val2; break;
				case '/': foldedIntValue = val2 != 0 ? val1 / val2 : 0; break;
				case '%': foldedIntValue = val2 != 0 ? val1 % val2 : 0; break;
			}
			isFolded = true;
		}
		else if (E1->constType == FLOAT_CONST || E2->constType == FLOAT_CONST) {
			float val1 = E1->getFloatValue();
			float val2 = E2->getFloatValue();
			
			switch (opt) {
				case '+': foldedFloatValue = val1 + val2; break;
				case '-': foldedFloatValue = val1 - val2; break;
				case '*': foldedFloatValue = val1 * val2; break;
				case '/': foldedFloatValue = val2 != 0.0f ? val1 / val2 : 0.0f; break;
			}
			constType = FLOAT_CONST;
			isFolded = true;
		}
	}
	
	virtual int getIntValue() const override {
		if (isFolded && constType == INT_CONST) {
			return foldedIntValue;
		}
		return ConstantExpr::getIntValue();
	}
	
	virtual float getFloatValue() const override {
		if (isFolded && constType == FLOAT_CONST) {
			return foldedFloatValue;
		}
		return ConstantExpr::getFloatValue();
	}
	
	virtual void code(FILE *fp){
		Expr::code(fp);
		// 如果是常量，直接输出值
		if (isFolded) {
			if (constType == INT_CONST) {
				fprintf(fp, "load $%d #%d\n", label, foldedIntValue);
			} else if (constType == FLOAT_CONST) {
				int intValue = (int)(foldedFloatValue * 1000);
				fprintf(fp, "load $%d #%d\n", label, intValue);
			}
		} else {
			// 如果不是常量，生成普通算术指令
			E1->code(fp);
			E2->code(fp);
			fprintf(fp, "%c $%d $%d $%d\n", opt, E1->label, E2->label, label);
		}
	}
};

// 常量比较表达式
struct ConstCond :ConstantExpr{
	char opt;
	ConstantExpr *E1, *E2;
	bool isFolded;
	bool foldedBoolValue;
	
	ConstCond(char op, ConstantExpr *e1, ConstantExpr *e2) 
		: ConstantExpr(BOOL_CONST), opt(op), E1(e1), E2(e2), isFolded(false), foldedBoolValue(false) {
		// 执行常量比较
		foldConstants();
	}
	
	void foldConstants() {
		if (E1->constType == INT_CONST && E2->constType == INT_CONST) {
			int val1 = E1->getIntValue();
			int val2 = E2->getIntValue();
			
			switch (opt) {
				case '>': foldedBoolValue = val1 > val2; break;
				case '<': foldedBoolValue = val1 < val2; break;
				case '=': foldedBoolValue = val1 == val2; break;
				case '!': foldedBoolValue = val1 != val2; break;
			}
			isFolded = true;
		}
		else if (E1->constType == FLOAT_CONST || E2->constType == FLOAT_CONST) {
			float val1 = E1->getFloatValue();
			float val2 = E2->getFloatValue();
			
			switch (opt) {
				case '>': foldedBoolValue = val1 > val2; break;
				case '<': foldedBoolValue = val1 < val2; break;
				case '=': foldedBoolValue = val1 == val2; break;
				case '!': foldedBoolValue = val1 != val2; break;
			}
			isFolded = true;
		}
	}
	
	virtual bool getBoolValue() const override {
		if (isFolded) {
			return foldedBoolValue;
		}
		return ConstantExpr::getBoolValue();
	}
	
	virtual void code(FILE *fp){
		Expr::code(fp);
		if (isFolded) {
			fprintf(fp, "load $%d #%d\n", label, foldedBoolValue ? 1 : 0);
		} else {
			// 如果不是常量，生成普通比较指令
			E1->code(fp);
			E2->code(fp);
			fprintf(fp, "%c $%d $%d $%d\n", opt, E1->label, E2->label, label);
		}
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
		list<Id*>::iterator iter;
		for (iter = ids.begin(); iter != ids.end(); iter++){
			// 为每个变量生成var声明，格式：var 变量名 初始值
			fprintf(fp, "var %s 0\n", (*iter)->s->word.c_str());
		}
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
		// 赋值完成后，可以回收右值的寄存器
		RegisterAllocator::free(E2->label);
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
		// 生成汇编器支持的label语法
		fprintf(fp, "label func_%s:\n", name.c_str());
		// 设置栈帧：保存BP，设置新的BP
		fprintf(fp, "push $%d\n", 1); // 保存BP（假设BP在寄存器1）
		fprintf(fp, "mov $%d $%d\n", 1, 0); // BP = SP
		
		// 为函数分配局部变量空间（使用mov指令调整栈指针）
		fprintf(fp, "mov $%d $%d\n", 0, 0); // 暂时不分配额外空间
		// 为参数声明局部变量
		list<Id*>::iterator iter;
		for (iter = params.begin(); iter != params.end(); iter++){
			fprintf(fp, "var %s 0\n", (*iter)->s->word.c_str());
		}
		// 从栈上获取参数并加载到寄存器
		int paramOffset = 4; // 返回地址(2字节) + 旧BP(2字节)
		for (iter = params.begin(); iter != params.end(); iter++){
			int regNum = RegisterAllocator::allocate();
			if (regNum == -1) {
				// 寄存器溢出，需要特殊处理
				printf("警告: 寄存器溢出，参数 %s 无法分配到寄存器\n", (*iter)->s->word.c_str());
				regNum = 2; // 使用默认寄存器
			}
			fprintf(fp, "load $%d *%d\n", regNum, paramOffset);
			paramOffset += 2; // 每个参数2字节
		}
		body->code(fp);
		// 恢复栈帧
		fprintf(fp, "mov $%d $%d\n", 0, 1); // SP = BP
		fprintf(fp, "pop $%d\n", 1); // 恢复BP
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
		
		// 保存调用者的寄存器现场（保存所有可能被使用的寄存器）
		fprintf(fp, "push $2\n"); // 保存寄存器2
		fprintf(fp, "push $3\n"); // 保存寄存器3
		fprintf(fp, "push $4\n"); // 保存寄存器4
		fprintf(fp, "push $5\n"); // 保存寄存器5
		fprintf(fp, "push $6\n"); // 保存寄存器6
		fprintf(fp, "push $7\n"); // 保存寄存器7
		fprintf(fp, "push $8\n"); // 保存寄存器8
		fprintf(fp, "push $9\n"); // 保存寄存器9
		fprintf(fp, "push $10\n"); // 保存寄存器10
		fprintf(fp, "push $11\n"); // 保存寄存器11
		fprintf(fp, "push $12\n"); // 保存寄存器12
		fprintf(fp, "push $13\n"); // 保存寄存器13
		fprintf(fp, "push $14\n"); // 保存寄存器14
		fprintf(fp, "push $15\n"); // 保存寄存器15
		
		// 将参数压栈（从右到左）
		list<Expr*>::reverse_iterator iter;
		for (iter = args.rbegin(); iter != args.rend(); iter++){
			(*iter)->code(fp);
			fprintf(fp, "push $%d\n", (*iter)->label);
		}
		// 使用CALL指令调用函数
		fprintf(fp, "call func_%s\n", name.c_str());
		// 清理栈上的参数（调整栈指针）
		if (args.size() > 0) {
			fprintf(fp, "add $%d #%ld\n", 0, args.size() * 2); // 调整栈指针
		}
		
		// 恢复调用者的寄存器现场
		fprintf(fp, "pop $15\n"); // 恢复寄存器15
		fprintf(fp, "pop $14\n"); // 恢复寄存器14
		fprintf(fp, "pop $13\n"); // 恢复寄存器13
		fprintf(fp, "pop $12\n"); // 恢复寄存器12
		fprintf(fp, "pop $11\n"); // 恢复寄存器11
		fprintf(fp, "pop $10\n"); // 恢复寄存器10
		fprintf(fp, "pop $9\n"); // 恢复寄存器9
		fprintf(fp, "pop $8\n"); // 恢复寄存器8
		fprintf(fp, "pop $7\n"); // 恢复寄存器7
		fprintf(fp, "pop $6\n"); // 恢复寄存器6
		fprintf(fp, "pop $5\n"); // 恢复寄存器5
		fprintf(fp, "pop $4\n"); // 恢复寄存器4
		fprintf(fp, "pop $3\n"); // 恢复寄存器3
		fprintf(fp, "pop $2\n"); // 恢复寄存器2
		
		// 函数调用后，返回值在寄存器0中，将其移动到当前表达式的寄存器
		fprintf(fp, "mov $0 $%d\n", label);
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
			// 将返回值存储到寄存器0（约定为返回值寄存器）
			fprintf(fp, "mov $%d $0\n", value->label);
		}
		fprintf(fp, "ret\n");
	}
};

// 打印语句
struct Print :Stmt{
	list<Expr*> args;
	virtual void code(FILE *fp){
		Stmt::code(fp);
		printf("print\n");
		list<Expr*>::iterator iter;
		for (iter = args.begin(); iter != args.end(); iter++){
			(*iter)->code(fp);
			fprintf(fp, "out $%d\n", (*iter)->label);
		}
	}
};

// 输入语句
struct Scan :Stmt{
	Id *var;
	virtual void code(FILE *fp){
		Stmt::code(fp);
		printf("scan\n");
		fprintf(fp, "in $%d\n", var->label);
		fprintf(fp, "store $%d *%d\n", var->label, var->offset);
	}
};

// 中断语句
struct Break :Stmt{
	virtual void code(FILE *fp){
		Stmt::code(fp);
		printf("break\n");
		// 跳转到循环结束标签
		fprintf(fp, "jmp L%d\n", next);
	}
};

// 继续语句
struct Continue :Stmt{
	virtual void code(FILE *fp){
		Stmt::code(fp);
		printf("continue\n");
		// 跳转到循环开始标签
		fprintf(fp, "jmp L%d\n", begin);
	}
};

// 开关语句
struct Switch :Stmt{
	Expr *expr;
	map<int, Stmt*> cases;
	Stmt *defaultCase;
	virtual void code(FILE *fp){
		Stmt::code(fp);
		printf("switch\n");
		expr->code(fp);
		// 简化的switch实现
		map<int, Stmt*>::iterator iter;
		for (iter = cases.begin(); iter != cases.end(); iter++){
			fprintf(fp, "cmp $%d #%d\n", expr->label, iter->first);
			fprintf(fp, "je L%d\n", iter->second->begin);
		}
		if (defaultCase) {
			fprintf(fp, "jmp L%d\n", defaultCase->begin);
		}
		fprintf(fp, "jmp L%d\n", next);
	}
};

// 静态变量定义
int RegisterAllocator::nextReg = 2;
int RegisterAllocator::maxReg = 16; // 限制使用16个寄存器
bool RegisterAllocator::usedRegs[256] = {false};
int RegisterAllocator::stack[256] = {0};
int RegisterAllocator::stackTop = 0;

#endif