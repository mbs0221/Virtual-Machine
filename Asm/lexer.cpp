#include "lexer.h"
#include "Common/Logger.h"

// 全局变量定义
Type* Type::Int = new Type(INT, "int", 2);

// Token类实现
string Token::place() {
	ostringstream s;
	s << kind;
	return s.str();
}

string Token::code() {
	ostringstream s;
	s << kind;
	return s.str();
}

// Word类实现
string Word::place() {
	ostringstream s;
	s << word;
	return s.str();
}

string Word::code() {
	return "";
}

// Type类实现
string Type::place() {
	ostringstream s;
	s << word << ":" << width;
	return s.str();
}

string Type::code() {
	return "";
}

// Integer类实现
string Integer::place() {
	ostringstream s;
	s << value;
	return s.str();
}

string Integer::code() {
	return "";
}

// String类实现
string String::place() {
	ostringstream s;
	s << "\"" << value << "\"";
	return s.str();
}

string String::code() {
	return value;
}

// Lexer类实现
Lexer::Lexer(string fp) {
	words["data"] = new Word(DATA, "data");
	words["code"] = new Word(CODE, "code");
	words["int"] = new Word(INT, "int");
	words["load"] = new Word(LOAD, "load");
	words["store"] = new Word(STORE, "store");
	words["lea"] = new Word(LEA, "lea");
	words["halt"] = new Word(HALT, "halt");
	words["label"] = new Word(LABEL, "label");
	words["jmp"] = new Word(JMP, "jmp");
	words["jb"] = new Word(JB, "jb");
	words["je"] = new Word(JE, "je");
	words["jne"] = new Word(JNE, "jne");
	words["jg"] = new Word(JG, "jg");
	words["jge"] = new Word(JGE, "jge");
	words["jbe"] = new Word(JBE, "jbe");
	words["call"] = new Word(CALL, "call");
	words["ret"] = new Word(RET, "ret");
	words["push"] = new Word(PUSH, "push");
	words["pop"] = new Word(POP, "pop");
	words["mov"] = new Word(MOV, "mov");
	words["in"] = new Word(IN, "in");
	words["out"] = new Word(OUT, "out");
	words["neg"] = new Word(NEG, "neg");
	words["loop"] = new Word(LOOP, "loop");
	words["add"] = new Word(ADD, "add");
	words["sub"] = new Word(SUB, "sub");
	words["mul"] = new Word(MUL, "mul");
	words["div"] = new Word(DIV, "div");
	words["mod"] = new Word(MOD, "mod");
	words["cmp"] = new Word(CMP, "cmp");
	words["shl"] = new Word(SHL, "shl");
	words["shr"] = new Word(SHR, "shr");
	words["sal"] = new Word(SAL, "sal");
	words["sar"] = new Word(SAR, "sar");
	words["srl"] = new Word(SRL, "srl");
	words["srr"] = new Word(SRR, "srr");
	words["var"] = new Word(VAR, "var");
	words["fs"] = new Word(FS, "fs");
	words["gs"] = new Word(GS, "gs");
	words["int"] = new Word(INT_INST, "int");
	words["iret"] = new Word(IRET, "iret");
	words["cli"] = new Word(CLI_INST, "cli");
	words["sti"] = new Word(STI_INST, "sti");
	inf.open(fp, ios::in);
}

Lexer::~Lexer() {
	inf.close();
	words.clear();
	// 移除析构函数的日志，避免在程序结束时输出到屏幕
}

Token* Lexer::scan() {
	char ch;
	
	// 跳过空白字符
	skipWhitespace();
	
	// 检查文件结束
	if (inf.eof()) {
		return new Token(END);
	}
	
	// 读取当前字符
	inf.read(&ch, sizeof(ch));
	if (inf.eof() || ch == EOF) {
		return new Token(END);
	}
	
	
		// 根据字符类型分发到相应的解析函数
	if (isalpha(ch)) {
		return scanIdentifier(ch);
	}

	if (isdigit(ch)) {
		return scanNumber(ch);
	}

	// 字符串字面量
	if (ch == '"') {
		return scanString();
	}

	// 注释处理
	if (ch == ';') {
		return scanComment();
	}

	// 其他字符（如运算符、分隔符等）
	return new Token(ch);
}

// 跳过空白字符
void Lexer::skipWhitespace() {
	char ch;
	do {
		if (inf.eof()) {
			return;
		}
		inf.read(&ch, sizeof(ch));
		if (ch == '\n') line++;
	} while (ch == ' ' || ch == '\n' || ch == '\t');
	
	// 回退一个字符，让调用者处理
	if (!inf.eof()) {
		inf.seekg(-1, ios::cur);
	}
}

// 解析标识符
Token* Lexer::scanIdentifier(char ch) {
	string str;
	do {
		str.push_back(ch);
		if (inf.eof()) break;
		inf.read(&ch, sizeof(ch));
		if (inf.eof()) break;  // 添加额外的文件末尾检查
	} while (isalnum(ch) || ch == '_');  // 支持字母、数字和下划线
	
	// 回退一个字符
	if (!inf.eof()) {
		inf.seekg(-1, ios::cur);
	}
	
	// 检查是否是关键字
	if (words.find(str) == words.end()) {
		return new Word(ID, str);
	}
	return words[str];
}

// 解析数字
Token* Lexer::scanNumber(char ch) {
	if (ch == '0') {
		// 处理以0开头的数字
		if (inf.eof()) {
			return new Integer(INT, 0);
		}
		
		char nextCh;
		inf.read(&nextCh, sizeof(nextCh));
		
		if (nextCh == 'x' || nextCh == 'X') {
			// 十六进制
			return scanHexadecimal(nextCh);
		} else if (nextCh >= '0' && nextCh <= '7') {
			// 八进制
			return scanOctal(nextCh);
		} else {
			// 十进制0
			inf.seekg(-1, ios::cur);
			return new Integer(INT, 0);
		}
	} else {
		// 十进制数字
		return scanDecimal(ch);
	}
}

// 解析十进制数字
Token* Lexer::scanDecimal(char ch) {
	int value = ch - '0'; // 第一个数字
	
	char nextCh;
	while (!inf.eof()) {
		inf.read(&nextCh, sizeof(nextCh));
		if (isdigit(nextCh)) {
			value = 10 * value + (nextCh - '0');
		} else {
			// 回退一个字符
			inf.seekg(-1, ios::cur);
			break;
		}
	}
	
	return new Integer(INT, value);
}

// 解析八进制数字
Token* Lexer::scanOctal(char ch) {
	int value = ch - '0'; // 第一个数字
	
	char nextCh;
	while (!inf.eof()) {
		inf.read(&nextCh, sizeof(nextCh));
		if (nextCh >= '0' && nextCh <= '7') {
			value = 8 * value + (nextCh - '0');
		} else {
			// 回退一个字符
			inf.seekg(-1, ios::cur);
			break;
		}
	}
	
	return new Integer(INT, value);
}

// 解析十六进制数字
Token* Lexer::scanHexadecimal(char ch) {
	// ch 是 'x' 或 'X'，需要读取下一个字符
	if (inf.eof()) {
		return new Integer(INT, 0);
	}
	
	char nextCh;
	inf.read(&nextCh, sizeof(nextCh));
	
	if (!isdigit(nextCh) && 
		!((nextCh >= 'a' && nextCh <= 'f') || (nextCh >= 'A' && nextCh <= 'F'))) {
		LOG_ERROR("Asm.Lexer", "错误的十六进制数!");
		return new Integer(INT, 0);
	}
	
	int value = 0;
	do {
		if (isdigit(nextCh)) {
			value = 16 * value + (nextCh - '0');
		} else if (isalpha(nextCh)) {
			value = 16 * value + (toupper(nextCh) - 'A' + 10);
		}
		
		if (inf.eof()) break;
		inf.read(&nextCh, sizeof(nextCh));
	} while (isdigit(nextCh) || 
			 (nextCh >= 'a' && nextCh <= 'f') || 
			 (nextCh >= 'A' && nextCh <= 'F'));
	
	        // 回退一个字符
        if (!inf.eof()) {
                inf.seekg(-1, ios::cur);
        }

        return new Integer(INT, value);
}

// 解析字符串字面量
Token* Lexer::scanString() {
	string value = "";
	char ch;
	
	while (!inf.eof()) {
		inf.read(&ch, sizeof(ch));
		if (ch == '"') {
			// 字符串结束
			break;
		} else if (ch == '\\') {
			// 转义字符
			if (!inf.eof()) {
				inf.read(&ch, sizeof(ch));
				switch (ch) {
					case 'n': value += '\n'; break;
					case 't': value += '\t'; break;
					case 'r': value += '\r'; break;
					case '\\': value += '\\'; break;
					case '"': value += '"'; break;
					default: value += ch; break;
				}
			}
		} else {
			value += ch;
		}
	}
	
	return new String(STRING, value);
}

// 解析注释
Token* Lexer::scanComment() {
	string comment = "";
	char ch;
	
	while (!inf.eof()) {
		inf.read(&ch, sizeof(ch));
		if (ch == '\n') {
			line++; // 注释结束，行号增加
			break;
		}
		comment += ch;
	}
	
	// 返回一个特殊的注释Token，汇编器会忽略它
	// 注意：这里不需要调用scan()，因为主循环会处理下一个token
	return new Token(COMMENT);
}
