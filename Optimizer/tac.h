#ifndef __TAC_H_
#define __TAC_H_

#include <vector>
#include <string>
#include <map>
#include <list>
#include "../Asm/code.h"

using namespace std;

// 三地址码操作类型
enum TACOpType {
	TAC_ASSIGN,     // 赋值: t1 = t2
	TAC_ADD,        // 加法: t1 = t2 + t3
	TAC_SUB,        // 减法: t1 = t2 - t3
	TAC_MUL,        // 乘法: t1 = t2 * t3
	TAC_DIV,        // 除法: t1 = t2 / t3
	TAC_MOD,        // 取模: t1 = t2 % t3
	TAC_LOAD,       // 加载: t1 = *t2
	TAC_STORE,      // 存储: *t1 = t2
	TAC_CALL,       // 函数调用: t1 = call func(t2, t3, ...)
	TAC_RETURN,     // 返回: return t1
	TAC_JUMP,       // 无条件跳转: jump label
	TAC_JUMP_COND,  // 条件跳转: if t1 op t2 jump label
	TAC_LABEL,      // 标签: label:
	TAC_HALT,       // 停机: halt
	TAC_PUSH,       // 压栈: push t1
	TAC_POP,        // 弹栈: t1 = pop
	TAC_MOV         // 移动: t1 = t2
};

// 三地址码操作数类型
enum TACOperandType {
	TAC_TEMP,       // 临时变量
	TAC_REG,        // 寄存器
	TAC_IMM,        // 立即数
	TAC_MEM,        // 内存地址
	TAC_LABEL_OP    // 标签操作数
};

// 三地址码操作数
struct TACOperand {
	TACOperandType type;
	string name;    // 变量名、标签名等
	WORD value;     // 立即数值
	BYTE reg;       // 寄存器号
	
	TACOperand() : type(TAC_TEMP), value(0), reg(0) {}
	TACOperand(TACOperandType t, const string& n) : type(t), name(n), value(0), reg(0) {}
	TACOperand(TACOperandType t, WORD v) : type(t), value(v), reg(0) {}
	TACOperand(TACOperandType t, BYTE r) : type(t), reg(r), value(0) {}
	
	string toString() const;
};

// 三地址码指令
struct TACInstruction {
	TACOpType op;
	TACOperand result;    // 结果操作数
	TACOperand arg1;      // 第一个操作数
	TACOperand arg2;      // 第二个操作数
	string label;         // 标签名（用于跳转）
	WORD line;            // 源程序行号
	
	TACInstruction(TACOpType o, WORD l = 0) : op(o), line(l) {}
	
	string toString() const;
};

// 前向声明
struct Stmt;
class Code;

// 三地址码程序
class TACProgram {
private:
	vector<TACInstruction> instructions;
	map<string, int> tempCounter;
	
	// 将Parser的Stmt转换为TAC指令的辅助方法
	bool convertStmtToTAC(Stmt* stmt);
	
public:
	// 默认构造函数
	TACProgram();
	
	// 从Parser的AST构造TAC程序
	TACProgram(Stmt* ast);
	
	// 从Asm的Code列表构造TAC程序
	TACProgram(const list<Code*>& codes);
	
	void addInstruction(const TACInstruction& inst);
	string generateTemp();
	void print() const;
	const vector<TACInstruction>& getInstructions() const;
	size_t size() const;
};

// 前向声明
class Asm;

// 三地址码转换器
class TACConverter {
private:
	TACProgram* tac;
	
public:
	TACConverter();
	~TACConverter();
	
	// 转换AST到三地址码
	TACProgram* convertFromAST(const list<Code*>& codes);
	
	// 打印三地址码
	void printTAC() const;
	
	// 获取三地址码程序
	TACProgram* getTACProgram() const { return tac; }
};

#endif
