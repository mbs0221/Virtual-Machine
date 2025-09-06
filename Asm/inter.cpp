#include "inter.h"
#include "Common/Logger.h"
#include "code.h"

// 指令宽度查找表 - 定义每个指令占用的字节数
const WORD InstructionFactory::instructionWidths[256] = {
	// 基本指令 (1字节操作码)
	/* HALT */ 1,                    // HALT
	/* ADD */ 4, /* SUB */ 4, /* MUL */ 4, /* DIV */ 4, /* MOD */ 4, /* CMP */ 4,  // 算术运算: 1字节操作码 + 3字节寄存器
	/* SHL */ 4, /* SHR */ 4, /* SAL */ 4, /* SAR */ 4, /* SRL */ 4, /* SRR */ 4,  // 移位运算: 1字节操作码 + 3字节寄存器
	/* MOV */ 3, /* IN */ 3, /* OUT */ 3,                  // 数据传输: MOV(1+2), IN/OUT(1+1+1)
	/* LOAD */ 4, /* STORE */ 4, /* LEA */ 4,      // 内存访问: 1字节操作码 + 1字节寄存器 + 2字节地址
	/* PUSH */ 2, /* POP */ 2,                     // 栈操作: 1字节操作码 + 1字节寄存器
	/* JMP */ 3, /* JNE */ 3, /* JG */ 3, /* JE */ 3, /* JB */ 3, /* JGE */ 3, /* JBE */ 3, // 跳转指令: 1字节操作码 + 2字节地址
	/* CALL */ 3, /* RET */ 1,                     // 函数调用: CALL(1+2), RET(1)
	/* NEG */ 3,                           // 一元运算: 1字节操作码 + 2字节寄存器
	/* LOOP */ 3,                          // 循环指令: 1字节操作码 + 2字节地址
	// 数据段指令
	/* VAR */ 2,                           // 变量: 2字节值
	/* DATA */ 0,                          // 数据段标记: 宽度由DATA指令的参数决定
	// 其他指令默认为0
};

// Code类实现
void Code::code(FILE* fp) {
	LOG_DEBUG("Asm.Code", "[" + std::to_string(line) + "][" + std::to_string(width) + "][0x" + std::to_string(offset) + "]");
	(void)fp; // 避免未使用参数警告
}

// Register类实现
void Register::code(FILE* fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "register\t$" + std::to_string(reg_num));
	if (fp) {
		fwrite(&reg_num, sizeof(BYTE), 1, fp);
	}
}

// Immediate类实现
void Immediate::code(FILE* fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "immediate\t#0x" + std::to_string(value));
	fwrite(&value, sizeof(WORD), 1, fp);
}

// Address类实现
void Address::code(FILE* fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "address\t*0x" + std::to_string(addr));
	fwrite(&addr, sizeof(WORD), 1, fp);
}

// Codes类实现
void Codes::code(FILE* fp) {
	list<Code*>::iterator iter;
	for (iter = codes.begin(); iter != codes.end(); iter++) {
		(*iter)->code(fp);
	}
}

// Data类实现
void Data::code(FILE* fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "data:" + std::to_string(width));
	// data语句不写入任何数据，只设置段地址
}

// Variable类实现
void Variable::code(FILE* fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "variable\t" + name + " = 0x" + std::to_string(value));
	fwrite(&value, sizeof(WORD), 1, fp);
}

// StringConstant类实现
void StringConstant::code(FILE* fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "string\t\"" + value + "\" at 0x" + std::to_string(addr));
	// 写入字符串数据（以null结尾）
	for (size_t i = 0; i < value.length(); i++) {
		fputc(value[i], fp);
	}
	fputc(0, fp); // null终止符
}

// Load类实现
void Load::code(FILE* fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "load\t$" + std::to_string(opt) + " $" + std::to_string(reg) + " $0x" + std::to_string(addr));
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&reg, sizeof(BYTE), 1, fp);
	fwrite(&addr, sizeof(WORD), 1, fp);
}

// Store类实现
void Store::code(FILE* fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "store\t$" + std::to_string(opt) + " $" + std::to_string(reg) + " $0x" + std::to_string(addr));
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&reg, sizeof(BYTE), 1, fp);
	fwrite(&addr, sizeof(WORD), 1, fp);
}

// Lea类实现
void Lea::code(FILE* fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "lea\t$" + std::to_string(opt) + " $" + std::to_string(reg) + " $0x" + std::to_string(addr));
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&reg, sizeof(BYTE), 1, fp);
	fwrite(&addr, sizeof(WORD), 1, fp);
}

// Halt类实现
void Halt::code(FILE *fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "halt\t$" + std::to_string(opt));
	fwrite(&opt, sizeof(BYTE), 1, fp);
}

// Push类实现
void Push::code(FILE *fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "push\t$" + std::to_string(opt) + " $" + std::to_string(reg));
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&reg, sizeof(BYTE), 1, fp);
}

// Pop类实现
void Pop::code(FILE *fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "pop\t$" + std::to_string(opt) + " $" + std::to_string(reg));
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&reg, sizeof(BYTE), 1, fp);
}

// Mov类实现
void Mov::code(FILE *fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "mov\t$" + std::to_string(opt) + " $" + std::to_string(reg1) + " $" + std::to_string(reg2));
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&reg1, sizeof(BYTE), 1, fp);
	fwrite(&reg2, sizeof(BYTE), 1, fp);
}

// In类实现
void In::code(FILE *fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "in\t$" + std::to_string(reg) + " $" + std::to_string(port));
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&reg, sizeof(BYTE), 1, fp);
	fwrite(&port, sizeof(BYTE), 1, fp);
}

// Out类实现
void Out::code(FILE *fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "out\t$" + std::to_string(reg) + " $" + std::to_string(port));
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&reg, sizeof(BYTE), 1, fp);
	fwrite(&port, sizeof(BYTE), 1, fp);
}

// Add类实现
void Add::code(FILE *fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "add\t$" + std::to_string(opt) + " $" + std::to_string(reg1) + " $" + std::to_string(reg2));
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&reg1, sizeof(BYTE), 1, fp);
	fwrite(&reg2, sizeof(BYTE), 1, fp);
}

// Ret类实现
void Ret::code(FILE *fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "ret\t$" + std::to_string(opt));
	fwrite(&opt, sizeof(BYTE), 1, fp);
}

// Int类实现
void Int::code(FILE *fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "int\t$" + std::to_string(opt) + " $" + std::to_string(vector));
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&vector, sizeof(BYTE), 1, fp);
}

// Iret类实现
void Iret::code(FILE *fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "iret\t$" + std::to_string(opt));
	fwrite(&opt, sizeof(BYTE), 1, fp);
}

// Cli类实现
void Cli::code(FILE *fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "cli\t$" + std::to_string(opt));
	fwrite(&opt, sizeof(BYTE), 1, fp);
}

// Sti类实现
void Sti::code(FILE *fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "sti\t$" + std::to_string(opt));
	fwrite(&opt, sizeof(BYTE), 1, fp);
}

// Call类实现
void Call::code(FILE* fp) {
	Code::code(fp);
	// 计算相对于代码段起始地址的偏移量
	WORD relativeAddr = addr->offset;
	LOG_DEBUG("Asm.Code", "call\t$" + std::to_string(opt) + " $0x" + std::to_string(relativeAddr));
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&relativeAddr, sizeof(WORD), 1, fp);
}

// Jmp类实现
void Jmp::code(FILE* fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "jmp \t$" + std::to_string(opt) + " $0x" + std::to_string(addr->offset));
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&addr->offset, sizeof(WORD), 1, fp);
}

// Arith类实现
void Arith::code(FILE* fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "bino\t$" + std::to_string(opt) + " $" + std::to_string(reg1) + " $" + std::to_string(reg2) + " $" + std::to_string(reg3));
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&reg1, sizeof(BYTE), 1, fp);
	fwrite(&reg2, sizeof(BYTE), 1, fp);
	fwrite(&reg3, sizeof(BYTE), 1, fp);
}

// Unary类实现
void Unary::code(FILE* fp) {
	Code::code(fp);
	LOG_DEBUG("Asm.Code", "unary\t$" + std::to_string(opt) + " $" + std::to_string(reg1) + " $" + std::to_string(reg2));
	fwrite(&opt, sizeof(BYTE), 1, fp);
	fwrite(&reg1, sizeof(BYTE), 1, fp);
	fwrite(&reg2, sizeof(BYTE), 1, fp);
}

// 指令工厂方法实现
WORD InstructionFactory::getCurrentLine() const {
	// 通过lexer获取当前行号
	// 这里需要访问lexer的line成员
	return lexer->line;
}

// 初始化指令宽度
void InstructionFactory::initializeWidth(Code* code) {
	if (code) {
		if (code->opt == (BYTE)VAR) {
			// 检查是否是StringConstant
			StringConstant* strConst = dynamic_cast<StringConstant*>(code);
			if (strConst) {
				// 字符串常量占用字符串长度+1字节（null终止符）
				code->width = strConst->value.length() + 1;
			} else {
				// 普通变量占用2字节（WORD大小）
				code->width = 2;
			}
		} else if (code->opt < 256) {
			code->width = instructionWidths[code->opt];
		} else {
			// 其他指令默认为0
			code->width = 0;
		}
	}
}

// 更新内存布局
void InstructionFactory::updateMemoryLayout(Code* code) {
	if (!code) return;
	
	if (code->opt == (BYTE)VAR) {  // 强制转换为BYTE类型
		// 变量定义属于数据段
		code->offset = dataOffset;
		dataOffset += code->width;
		dataSize += code->width;
	} else {
		// 其他指令属于代码段
		code->offset = codeOffset;
		codeOffset += code->width;
		codeSize += code->width;
	}
}

// 标签管理方法
Label* InstructionFactory::getLabel(const string& name) {
	if (labels.find(name) == labels.end()) {
		// 如果标签不存在，创建一个新的
		labels[name] = new Label(new Word(ID, name), 0);
	}
	return labels[name];
}

Label* InstructionFactory::createLabel(const string& name) {
	// 如果标签已存在（前向引用），更新其offset
	if (labels.find(name) != labels.end()) {
		labels[name]->offset = codeOffset;
		LOG_DEBUG("Asm.Factory", "更新前向引用标签 '" + name + "' 的 offset 为 " + std::to_string(codeOffset));
		return labels[name];
	} else {
		// 如果标签不存在，创建新标签
		Label* label = new Label(new Word(ID, name), codeOffset);
		labels[name] = label;
		LOG_DEBUG("Asm.Factory", "创建新标签 '" + name + "'，offset 为 " + std::to_string(codeOffset));
		return label;
	}
}

void InstructionFactory::updateLabelOffset(const string& name, WORD offset) {
	LOG_DEBUG("Asm.Factory", "更新标签 '" + name + "' 的 offset 为 " + std::to_string(offset));
	if (labels.find(name) != labels.end()) {
		labels[name]->offset = offset;
		LOG_DEBUG("Asm.Factory", "标签 '" + name + "' 已存在，更新 offset 为 " + std::to_string(offset));
	} else {
		labels[name] = new Label(new Word(ID, name), offset);
		LOG_DEBUG("Asm.Factory", "创建新标签 '" + name + "'，offset 为 " + std::to_string(offset));
	}
}
