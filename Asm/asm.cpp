#include "asm.h"
#include "Common/Logger.h"
#include <string>
#include <sstream>



// 辅助函数实现
bool Asm::parse_multiple_registers(BYTE& reg1, BYTE& reg2) {
	reg1 = parse_register_value();
	if (reg1 == 0xFF) return false;
	reg2 = parse_register_value();
	if (reg2 == 0xFF) return false;
	return true;
}

bool Asm::parse_multiple_registers(BYTE& reg1, BYTE& reg2, BYTE& reg3) {
	reg1 = parse_register_value();
	if (reg1 == 0xFF) return false;
	reg2 = parse_register_value();
	if (reg2 == 0xFF) return false;
	reg3 = parse_register_value();
	if (reg3 == 0xFF) return false;
	return true;
}

Integer* Asm::create_operation_object(BYTE op) {
	return new Integer(op, op);
}

bool Asm::match(int kind) {
	if (s->kind == kind) {
		s = lexer->scan();
		return true;
	}
	LOG_ERROR("Asm.Parser", "Syntax error at line " + std::to_string(lexer->line) + ": expected token " + std::to_string(kind) + ", but got token " + std::to_string(s->kind));
	error_count++;
	exit(EXIT_FAILURE);
}

// 解析函数 - 返回值的版本
BYTE Asm::parse_register_value() {
	match('$');
	Integer* intObj = match_and_get<Integer>(INT);
	return intObj->value;
}

WORD Asm::parse_immediate_value() {
	match('#');
	Integer* intObj = match_and_get<Integer>(INT);
	return intObj->value;
}

// 解析直接寻址: *value
WORD Asm::parse_direct_address_value() {
	match('*');
	Integer* intObj = match_and_get<Integer>(INT);
	return intObj->value;
}

// 解析寄存器间接寻址: [$reg]
WORD Asm::parse_indirect_register_value() {
	match('[');
	match('$');
	Integer* regObj = match_and_get<Integer>(INT);
	match(']');
	return regObj->value;
}

WORD Asm::parse_address_value() {
	match('*');
	Integer* intObj = match_and_get<Integer>(INT);
	return intObj->value;
}

bool Asm::parse_addressing_mode(BYTE& opt, WORD& addr) {
	if (s->kind == '#') {
		opt |= MR_A; // 立即数寻址
		addr = parse_immediate_value();
		LOG_DEBUG("Asm.Parser", "Immediate addressing: opt=0x" + std::to_string(opt) + ", addr=0x" + std::to_string(addr));
		return true;
	} else if (s->kind == '*') {
		opt |= MR_B; // 直接寻址
		addr = parse_direct_address_value();
		LOG_DEBUG("Asm.Parser", "Direct addressing: opt=0x" + std::to_string(opt) + ", addr=0x" + std::to_string(addr));
		return true;
	} else if (s->kind == '[') {
		opt |= MR_INDIRECT; // 寄存器间接寻址
		addr = parse_indirect_register_value();
		LOG_DEBUG("Asm.Parser", "Indirect addressing: opt=0x" + std::to_string(opt) + ", reg=0x" + std::to_string(addr));
		return true;
	} else {
		LOG_ERROR("Asm.Parser", "Syntax error at line " + std::to_string(lexer->line) + ": expected immediate symbol '#', address symbol '*', or indirect symbol '[', but got '" + std::string(1, s->kind) + "'");
		return false;
	}
}

// 旧版解析函数（将被逐步替换）
Code* Asm::data() {
	match(DATA);
	Integer* intObj = match_and_get<Integer>(INT);
	WORD size = intObj->value;
	match(DATA);
	// 使用Factory创建Data指令
	return factory->createData(size);
}

Code* Asm::halt() {
 	match(HALT);
	return factory->createHalt();
}

Code* Asm::call() {
	match(CALL);
	string labelName = ((Word*)s)->word;
	match(ID);
	
	return factory->createCall(labelName);
}

Code* Asm::add() {
	match(ADD);
	BYTE reg1 = parse_register_value();
	match('#');
	Integer* reg2 = match_and_get<Integer>(INT);
	if (!reg2) return nullptr;
	Integer* reg1Obj = new Integer(INT, reg1);
	Add *a = factory->createAdd(reg1Obj, reg2);
	return a;
}

Code* Asm::ret() {
	match(RET);
	Ret *r = factory->createRet();
	return r;
}

Code* Asm::label() {
	match(LABEL);
	string labelName = ((Word*)s)->word;
	match(ID);
	match(':');
	
	return factory->createLabelCode(labelName);
}

Code* Asm::unary() {
	match('~');
	BYTE reg1, reg2;
	if (!parse_multiple_registers(reg1, reg2)) {
		return nullptr;
	}
	Integer* negObj = create_operation_object(NEG);
	Integer* reg1Obj = new Integer(INT, reg1);
	Integer* reg2Obj = new Integer(INT, reg2);
	Unary *u = factory->createUnary(negObj, reg1Obj, reg2Obj);
	return u;
}

Code* Asm::arith(BYTE b) {
	match(s->kind);
	BYTE reg1, reg2, reg3;
	if (!parse_multiple_registers(reg1, reg2, reg3)) {
		return nullptr;
	}
	Arith *a = factory->createArith(b, reg1, reg2, reg3);
	return a;
}

Code* Asm::jmp(BYTE b) {
	match(s->kind);
	string labelName = ((Word*)s)->word;
	match(ID);
	
	return factory->createJmp(b, labelName);
}

// 构造函数
Asm::Asm(string fp) {
	// 初始化日志系统
	Common::Logger::getInstance().initializeDefault("INFO", "Logs/asm.log");
	LOG_INFO("Asm.Parser", "Assembler initialized for file: " + fp);
	
	lexer = new Lexer(fp);
	factory = new InstructionFactory(lexer);
	error_count = 0;
}

// 析构函数
Asm::~Asm() {
	if (lexer) delete lexer;
	if (factory) delete factory;
	if (cs) delete cs;
}

// 主解析函数
void Asm::parse() {
	LOG_INFO("Asm.Parser", "Starting assembly parsing");
	cs = new Codes;
	s = lexer->scan();
	
	while (s->kind != END) {
		// 跳过注释
		if (s->kind == COMMENT) {
			match(COMMENT);
			continue;
		}
		
		Code *c = parse_statement();
		if (c) {
			cs->codes.push_back(c);
			// Factory已经自动管理了offset和内存布局
		} else {
			// 解析失败，停止汇编
			LOG_ERROR("Asm.Parser", "Assembly failed at line " + std::to_string(lexer->line) + ", stopping parsing");
			error_count++;
			break;
		}
	}
	LOG_INFO("Asm.Parser", "Assembly parsing completed. Total errors: " + std::to_string(error_count));
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
		case LEA:
			return parse_lea_statement();
		case HALT:
			return parse_halt_statement();
		case CALL:
			return parse_call_statement();
		case RET:
			return parse_ret_statement();
		case INT_INST:
		case IRET:
		case CLI_INST:
		case STI_INST:
			return parse_interrupt_statement();
		case PUSH:
			return parse_push_statement();
		case POP:
			return parse_pop_statement();
		case MOV:
			return parse_mov_statement();
		case IN:
		case OUT:
			return parse_io_statement();
		case NEG:
			return parse_neg_statement();
		case LOOP:
			return parse_loop_statement();
		case ADD:
			return parse_add_statement();
		case SUB:
		case MUL:
		case DIV:
		case MOD:
		case CMP:
		case SHL:
		case SHR:
		case SAL:
		case SAR:
		case SRL:
		case SRR:
			return parse_arithmetic_statement();
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
			// 检查是否是标签定义（后面跟着冒号）
			{
				string labelName = ((Word*)s)->word;
				match(ID);
				if (s->kind == ':') {
					match(':');
					return factory->createLabelCode(labelName);
				} else {
					LOG_ERROR("Asm.Parser", "Syntax error at line " + std::to_string(lexer->line) + ": unexpected identifier '" + labelName + "'");
					return nullptr;
				}
			}
	default:
		LOG_ERROR("Asm.Parser", "Syntax error at line " + std::to_string(lexer->line) + ": unsupported instruction or symbol '" + std::string(1, s->kind) + "' (code: " + std::to_string(s->kind) + ")");
		return nullptr;
		}
}

// 解析data语句: data <value>
Code* Asm::parse_data_statement() {
	match(DATA);
	if (s->kind != INT) {
		LOG_ERROR("Asm.Parser", "语法错误: 期望数值，但得到 '" + std::string(1, s->kind) + "'");
		return nullptr;
	}
	WORD size = ((Integer*)s)->value;
	match(INT);
	
	// 使用Factory创建Data指令
	return factory->createData(size);
}

// 解析变量定义语句: var <name> <value>
Code* Asm::parse_variable_statement() {
	match(VAR);

	// 解析变量名
	if (s->kind != ID) {
		LOG_ERROR("Asm.Parser", "Syntax error at line " + std::to_string(lexer->line) + ": expected variable name, but got '" + std::string(1, s->kind) + "'");
		error_count++;
		return nullptr;
	}
	string varName = ((Word*)s)->word;
	match(ID);

	// 解析变量值（可以是整数或字符串）
	if (s->kind == INT) {
		// 整数变量
		Integer* intObj = match_and_get<Integer>(INT);
		if (!intObj) return nullptr;
		return factory->createVariable(varName, intObj);
	} else if (s->kind == STRING) {
		// 字符串常量
		string stringValue = ((String*)s)->value;
		match(STRING);
		// 计算字符串在数据段中的地址
		WORD stringAddr = factory->getDataSize();
		return factory->createStringConstant(stringValue, stringAddr);
	} else {
		LOG_ERROR("Asm.Parser", "Syntax error at line " + std::to_string(lexer->line) + ": expected variable value (integer or string), but got '" + std::string(1, s->kind) + "'");
		error_count++;
		return nullptr;
	}
}

// 解析FS段寄存器设置语句: fs <value>
Code* Asm::parse_fs_statement() {
	match(FS);
	Integer* intObj = match_and_get<Integer>(INT);
	Data *d = new Data;
	d->line = lexer->line;
	d->width = intObj->value;
	// 这里可以设置FS段寄存器，但需要扩展Data结构体或创建新的结构体
	return d;
}

// 解析GS段寄存器设置语句: gs <value>
Code* Asm::parse_gs_statement() {
	match(GS);
	Integer* intObj = match_and_get<Integer>(INT);
	Data *d = new Data;
	d->line = lexer->line;
	d->width = intObj->value;
	// 这里可以设置GS段寄存器，但需要扩展Data结构体或创建新的结构体
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
	
	// 使用工厂创建Load指令，传递正确的操作码（包含寻址模式）
	return factory->createLoad(opt, reg, addr);
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
	
	// 使用工厂创建Store指令，传递正确的操作码（包含寻址模式）
	return factory->createStore(opt, reg, addr);
}

// 解析halt语句: halt
Code* Asm::parse_halt_statement() {
	match(HALT);
	return factory->createHalt();
}

// 解析push语句: push <register>
Code* Asm::parse_push_statement() {
	match(PUSH);
	BYTE reg = parse_register_value();
	Integer* regObj = new Integer(INT, reg);
	Push *p = factory->createPush(regObj);
	return p;
}

// 解析pop语句: pop <register>
Code* Asm::parse_pop_statement() {
	match(POP);
	BYTE reg = parse_register_value();
	Integer* regObj = new Integer(INT, reg);
	Pop *p = factory->createPop(regObj);
	return p;
}

// 解析mov语句: mov <register> <register>
Code* Asm::parse_mov_statement() {
	match(MOV);
	BYTE reg1, reg2;
	if (!parse_multiple_registers(reg1, reg2)) {
		return nullptr;
	}
	Integer* reg1Obj = new Integer(INT, reg1);
	Integer* reg2Obj = new Integer(INT, reg2);
	Mov *m = factory->createMov(reg1Obj, reg2Obj);
	return m;
}

// 解析中断指令
Code* Asm::parse_interrupt_statement() {
	BYTE OP = s->kind;
	match(OP);
	switch (OP) {
		case INT_INST: {
			Integer* intObj = match_and_get<Integer>(INT);
			if (!intObj) return nullptr;
			Int *i = factory->createInt(intObj);
			return i;
		}
		case IRET: {
			Iret *ir = factory->createIret();
			return ir;
		}
		case CLI_INST: {
			Cli *c = factory->createCli();
			c->width = 1;
			return c;
		}
		case STI_INST: {
			Sti *sti = factory->createSti();
			sti->width = 1;
			return sti;
		}
		default:
			return nullptr;
	}
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
	// 从Factory获取内存布局信息
	WORD dataSize = factory->getDataSize();
	WORD codeSize = factory->getCodeSize();
	WORD DS = 0;  // 数据段起始地址固定为0
	WORD CS = factory->getCS();  // 代码段起始地址 = 数据段结束地址

	LOG_INFO("Asm.Parser", "=== Toy架构二进制文件格式 ===");
	LOG_INFO("Asm.Parser", "文件头格式 (16字节):");
	LOG_INFO("Asm.Parser", "  DS (2字节): 数据段起始地址 = " + std::to_string(DS));
	LOG_INFO("Asm.Parser", "  CS (2字节): 代码段起始地址 = " + std::to_string(CS));
	LOG_INFO("Asm.Parser", "  dataSize (2字节): 数据段大小 = " + std::to_string(dataSize));
	LOG_INFO("Asm.Parser", "  codeSize (2字节): 代码段大小 = " + std::to_string(codeSize));
	LOG_INFO("Asm.Parser", "  reserved (8字节): 保留字段，用于未来扩展");
	LOG_INFO("Asm.Parser", "文件体格式:");
	LOG_INFO("Asm.Parser", "  数据段: 从地址DS开始，包含变量和字符串常量");
	LOG_INFO("Asm.Parser", "  代码段: 从地址CS开始，包含机器指令");
	LOG_INFO("Asm.Parser", "================================");

	// 写入文件头
	fwrite(&DS, sizeof(WORD), 1, fp);      // 数据段起始地址
	fwrite(&CS, sizeof(WORD), 1, fp);      // 代码段起始地址（数据段结束地址）
	fwrite(&dataSize, sizeof(WORD), 1, fp); // 数据段大小
	fwrite(&codeSize, sizeof(WORD), 1, fp); // 代码段大小
	
	// 写入保留字段（8字节）
	WORD reserved = 0;
	fwrite(&reserved, sizeof(WORD), 1, fp); // 保留字段1
	fwrite(&reserved, sizeof(WORD), 1, fp); // 保留字段2
	
	// 文件格式：内存镜像模式
	// 文件内容直接对应内存布局：数据段从DS开始，代码段从CS开始
	LOG_DEBUG("Asm.Parser", "Creating memory image: DS=" + std::to_string(DS) + ", CS=" + std::to_string(CS));
	
	// 计算总内存大小：从0到CS+codeSize
	WORD totalSize = CS + codeSize;
	LOG_DEBUG("Asm.Parser", "Total memory size: " + std::to_string(totalSize));
	
	// 创建内存镜像：先清零整个内存区域
	for (WORD i = 0; i < totalSize; i++) {
		fputc(0, fp);
	}
	
	// 跳过文件头，写入数据段
	fseek(fp, 16 + DS, SEEK_SET);
	LOG_DEBUG("Asm.Parser", "Writing data segment at file position: " + std::to_string(ftell(fp)));
	list<Code*>::iterator iter;
	for (iter = cs->codes.begin(); iter != cs->codes.end(); iter++) {
		if ((*iter)->opt == VAR) {
			(*iter)->code(fp);
		}
	}
	
	// 写入代码段到CS地址（跳过文件头）
	fseek(fp, 16 + CS, SEEK_SET);
	LOG_DEBUG("Asm.Parser", "Writing code segment at file position: " + std::to_string(ftell(fp)));
	for (iter = cs->codes.begin(); iter != cs->codes.end(); iter++) {
		if ((*iter)->opt != VAR) {
			(*iter)->code(fp);
		}
	}
}

// 打印指令列表（只记录到日志文件，不输出到屏幕）
void Asm::printInstructions() {
	LOG_INFO("Asm.Parser", "=== Assembly Instruction List ===");
	LOG_INFO("Asm.Parser", "Total instructions: " + std::to_string(getInstructionCount()));
	LOG_INFO("Asm.Parser", "Line\tWidth\tOffset\tInstruction");
	
	list<Code*>::iterator iter;
	int index = 0;
	for (iter = cs->codes.begin(); iter != cs->codes.end(); iter++) {
		// 构建完整的指令信息字符串
		std::string instruction_info = "[" + std::to_string(index++) + "]\t" + 
			std::to_string((*iter)->width) + "\t0x" + std::to_string((*iter)->offset) + "\t";
		
		// 获取指令的详细信息并添加到字符串中
		std::string desc = getInstructionDescription(*iter);
		instruction_info += desc;
		
		LOG_INFO("Asm.Parser", instruction_info);
	}
	LOG_INFO("Asm.Parser", "==================");
}

// 获取指令描述字符串
std::string Asm::getInstructionDescription(Code* code) {
	if (dynamic_cast<Register*>(code)) {
		Register* reg = dynamic_cast<Register*>(code);
		return "register\t$" + std::to_string(reg->reg_num);
	} else if (dynamic_cast<Immediate*>(code)) {
		Immediate* imm = dynamic_cast<Immediate*>(code);
		return "immediate\t#0x" + std::to_string(imm->value);
	} else if (dynamic_cast<Address*>(code)) {
		Address* addr = dynamic_cast<Address*>(code);
		return "address\t*0x" + std::to_string(addr->addr);
	} else if (dynamic_cast<Data*>(code)) {
		Data* data = dynamic_cast<Data*>(code);
		return "data:" + std::to_string(data->width);
	} else if (dynamic_cast<Variable*>(code)) {
		Variable* var = dynamic_cast<Variable*>(code);
		return "variable\t" + var->name + " = 0x" + std::to_string(var->value);
	} else if (dynamic_cast<StringConstant*>(code)) {
		StringConstant* str = dynamic_cast<StringConstant*>(code);
		return "string\t\"" + str->value + "\" at 0x" + std::to_string(str->addr);
	} else if (dynamic_cast<Load*>(code)) {
		Load* load = dynamic_cast<Load*>(code);
		return "load\t$" + std::to_string(load->opt) + " $" + std::to_string(load->reg) + " $0x" + std::to_string(load->addr);
	} else if (dynamic_cast<Store*>(code)) {
		Store* store = dynamic_cast<Store*>(code);
		return "store\t$" + std::to_string(store->opt) + " $" + std::to_string(store->reg) + " $0x" + std::to_string(store->addr);
	} else if (dynamic_cast<Lea*>(code)) {
		Lea* lea = dynamic_cast<Lea*>(code);
		return "lea\t$" + std::to_string(lea->opt) + " $" + std::to_string(lea->reg) + " $0x" + std::to_string(lea->addr);
	} else if (dynamic_cast<Halt*>(code)) {
		Halt* halt = dynamic_cast<Halt*>(code);
		return "halt\t$" + std::to_string(halt->opt);
	} else if (dynamic_cast<Push*>(code)) {
		Push* push = dynamic_cast<Push*>(code);
		return "push\t$" + std::to_string(push->opt) + " $" + std::to_string(push->reg);
	} else if (dynamic_cast<Pop*>(code)) {
		Pop* pop = dynamic_cast<Pop*>(code);
		return "pop\t$" + std::to_string(pop->opt) + " $" + std::to_string(pop->reg);
	} else if (dynamic_cast<Mov*>(code)) {
		Mov* mov = dynamic_cast<Mov*>(code);
		return "mov\t$" + std::to_string(mov->opt) + " $" + std::to_string(mov->reg1) + " $" + std::to_string(mov->reg2);
	} else if (dynamic_cast<In*>(code)) {
		In* in = dynamic_cast<In*>(code);
		return "in\t$" + std::to_string(in->reg) + " $" + std::to_string(in->port);
	} else if (dynamic_cast<Out*>(code)) {
		Out* out = dynamic_cast<Out*>(code);
		return "out\t$" + std::to_string(out->reg) + " $" + std::to_string(out->port);
	} else if (dynamic_cast<Add*>(code)) {
		Add* add = dynamic_cast<Add*>(code);
		return "add\t$" + std::to_string(add->opt) + " $" + std::to_string(add->reg1) + " $" + std::to_string(add->reg2);
	} else if (dynamic_cast<Ret*>(code)) {
		Ret* ret = dynamic_cast<Ret*>(code);
		return "ret\t$" + std::to_string(ret->opt);
	} else if (dynamic_cast<Int*>(code)) {
		Int* int_inst = dynamic_cast<Int*>(code);
		return "int\t$" + std::to_string(int_inst->opt) + " $" + std::to_string(int_inst->vector);
	} else if (dynamic_cast<Iret*>(code)) {
		Iret* iret = dynamic_cast<Iret*>(code);
		return "iret\t$" + std::to_string(iret->opt);
	} else if (dynamic_cast<Cli*>(code)) {
		Cli* cli = dynamic_cast<Cli*>(code);
		return "cli\t$" + std::to_string(cli->opt);
	} else if (dynamic_cast<Sti*>(code)) {
		Sti* sti = dynamic_cast<Sti*>(code);
		return "sti\t$" + std::to_string(sti->opt);
	} else if (dynamic_cast<Call*>(code)) {
		Call* call = dynamic_cast<Call*>(code);
		return "call\t$" + std::to_string(call->opt) + " $0x" + std::to_string(call->addr->offset);
	} else if (dynamic_cast<Jmp*>(code)) {
		Jmp* jmp = dynamic_cast<Jmp*>(code);
		return "jmp\t$" + std::to_string(jmp->opt) + " $0x" + std::to_string(jmp->addr->offset);
	} else if (dynamic_cast<Arith*>(code)) {
		Arith* arith = dynamic_cast<Arith*>(code);
		return "arith\t$" + std::to_string(arith->opt) + " $" + std::to_string(arith->reg1) + " $" + std::to_string(arith->reg2) + " $" + std::to_string(arith->reg3);
	} else if (dynamic_cast<Unary*>(code)) {
		Unary* unary = dynamic_cast<Unary*>(code);
		return "unary\t$" + std::to_string(unary->opt) + " $" + std::to_string(unary->reg1) + " $" + std::to_string(unary->reg2);
	} else if (dynamic_cast<Label*>(code)) {
		Label* label = dynamic_cast<Label*>(code);
		return "label\t" + label->w->word + ":";
	} else {
		return "unknown";
	}
}

// 清空指令列表
void Asm::clearInstructions() {
	list<Code*>::iterator iter;
	for (iter = cs->codes.begin(); iter != cs->codes.end(); iter++) {
		delete *iter;
	}
	cs->codes.clear();
}

// 解析I/O指令: in/out <register>
Code* Asm::parse_io_statement() {
	BYTE op = s->kind;
	match(op);
	match('$');
	Integer* reg = match_and_get<Integer>(INT);
	if (!reg) return nullptr;
	match('$');
	Integer* port = match_and_get<Integer>(INT);
	if (!port) return nullptr;
	if (op == IN) {
		return factory->createIn(reg, port);
	} else if (op == OUT) {
		return factory->createOut(reg, port);
	}
	LOG_ERROR("Asm.Parser", "Unknown I/O operation: " + std::to_string(op));
	return nullptr;
}

// 解析NEG指令: neg <register>
Code* Asm::parse_neg_statement() {
	match(NEG);
	
	// 解析寄存器
	BYTE reg = parse_register_value();
	if (reg == 0xFF) return nullptr;
	
	// 使用工厂创建NEG指令
	return factory->createArith(NEG, reg, 0, 0);
}

// 解析LOOP指令: loop <label>
Code* Asm::parse_loop_statement() {
	match(LOOP);
	
	// 解析标签
	if (s->kind != ID) {
		LOG_ERROR("Asm.Parser", "语法错误: 期望标签名，但得到 '" + std::string(1, s->kind) + "'");
		error_count++;
		return nullptr;
	}
	
	string labelName = ((Word*)s)->word;
	match(ID);
	
	// 使用工厂创建LOOP指令
	return factory->createJmp(LOOP, labelName);
}

// 解析LEA指令: lea <register> <address_or_variable>
Code* Asm::parse_lea_statement() {
	match(LEA);
	
	// 解析寄存器
	BYTE reg = parse_register_value();
	if (reg == 0xFF) return nullptr; // 解析失败
	
	// 解析寻址模式
	BYTE opt = LEA;
	WORD addr = 0;
	
	// LEA指令支持多种寻址模式：
	// 1. lea $reg *address - 直接地址
	// 2. lea $reg variable_name - 变量名（字符串常量地址）
	if (s->kind == '*') {
		// 直接地址寻址
		opt |= MR_B; // 直接寻址
		addr = parse_address_value();
		if (addr == 0xFFFF) return nullptr; // 解析失败
	} else if (s->kind == ID) {
		// 变量名寻址 - 获取字符串常量的地址
		string varName = ((Word*)s)->word;
		match(ID);
		
		// 查找变量/字符串常量的地址
		// 这里需要从工厂中获取变量的地址
		// 暂时使用一个占位符，实际实现需要从符号表中查找
		LOG_DEBUG("Asm.Parser", "LEA指令: 查找变量 '" + varName + "' 的地址");
		addr = 0; // 占位符，需要实际实现变量地址查找
	} else {
		LOG_ERROR("Asm.Parser", "Syntax error at line " + std::to_string(lexer->line) + ": expected address symbol '*' or variable name, but got '" + std::string(1, s->kind) + "'");
		return nullptr;
	}
	
	// 使用工厂创建LEA指令
	return factory->createLea(opt, reg, addr);
}


