#ifndef __LEXER_H_
#define __LEXER_H_

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "code.h"

using namespace std;

// 词法单元
struct Token{
	int	kind;
	Token(int tag) :kind(tag){  }
	virtual string place(){
		ostringstream s;
		s << kind;
		return s.str();
	}
	virtual string code(){
		ostringstream s;
		s << kind;
		return s.str();
	}
};

struct Word :Token{
	string str;
	Word(int tag, string str) :Token(tag), str(str) {  }
	virtual string place(){
		ostringstream os;
		os << str;
		return os.str();
	}
	virtual string code(){
		return "";
	}
};

struct Type :Word{
	int width;
	static Type *Int, *Reg;
	Type(int kind, string word, int width) :Word(kind, word), width(width){  }
	virtual string place(){
		ostringstream s;
		s << str << ":" << width;
		return s.str();
	}
	virtual string code(){
		return "";
	}
};

Type* Type::Int = new Type(NUM, "int", 2);
Type* Type::Reg = new Type(REG, "int", 1);

struct Integer :Token{
	int value;
	Integer(int tag, int value) :Token(tag), value(value) {  }
	virtual string place(){
		ostringstream s;
		s << value;
		return s.str();
	}
	virtual string code(){
		return "";
	}
};

#define DEF_KEY_WORD(key, type, value) words[key] = new Integer(type, value)


// 词法分析器
class Lexer{
	ifstream inf;
	map<string, Token*> words;
	int hexToDec(char word[20])
	{
		char *str;
		int l = strtol(word, &str, 16);
		return l;
	}
	int octToDec(char word[20])
	{
		char *str;
		int l = strtol(word, &str, 8);
		return l;
	}
public:
	int line = 1;
	Lexer(string fp){
		inf.open(fp, ios::in);
		words["data"] = new Word(DATA, "data");
		words["stack"] = new Word(STACK, "stack");
		words["code"] = new Word(CODE, "code");
		// 数据操作
		words["load"] = new Word(LOAD, "load");
		words["store"] = new Word(STORE, "store");
		// 停机指令
		words["halt"] = new Word(HALT, "halt");
		// 算术逻辑运算
		words["sub"] = new Word(SUB, "add");
		words["add"] = new Word(ADD, "sub");
		// 栈操作指令
		words["push"] = new Word(PUSH, "push");
		words["pop"] = new Word(POP, "pop");
		// 跳转指令
		words["jmp"] = new Word(JMP, "jmp");
		words["jb"] = new Word(JB, "jb");
		words["jbe"] = new Word(JBE, "jbe");
		words["je"] = new Word(JE, "je");
		words["jge"] = new Word(JGE, "jge");
		words["jg"] = new Word(JG, "jg");
		words["jne"] = new Word(JNE, "jne");
		// 函数调用
		words["proc"] = new Word(PROC, "proc");
		words["endp"] = new Word(ENDP, "endp");
		words["call"] = new Word(CALL, "call");
		// 段寄存器
		words["ds"] = new Integer(REG, Reg::DS);
		words["cs"] = new Integer(REG, Reg::CS);
		words["ss"] = new Integer(REG, Reg::SS);
		words["es"] = new Integer(REG, Reg::ES);
		// 寄存器
		words["bp"] = new Integer(REG, Reg::BP);
		words["sp"] = new Integer(REG, Reg::SP);
		words["si"] = new Integer(REG, Reg::SI);
		words["di"] = new Integer(REG, Reg::DI);
	}
	// MIPS指令集
	void MIPS(){
		// R-type
		DEF_KEY_WORD("add",RTYPE, 0x00000020);
		DEF_KEY_WORD("addu",RTYPE, 0x00000021);
		DEF_KEY_WORD("sub",RTYPE, 0x00000022);
		DEF_KEY_WORD("subu",RTYPE, 0x00000023);
		DEF_KEY_WORD("and",RTYPE, 0x00000024);
		DEF_KEY_WORD("or",RTYPE, 0x00000025);
		DEF_KEY_WORD("xor",RTYPE, 0x00000026);
		DEF_KEY_WORD("nor",RTYPE, 0x00000027);
		DEF_KEY_WORD("slt",RTYPE, 0x0000002A);
		DEF_KEY_WORD("sltu",RTYPE, 0x0000002B);
		DEF_KEY_WORD("sll",RTYPE, 0x00000000);
		DEF_KEY_WORD("srl",RTYPE, 0x00000002);
		DEF_KEY_WORD("ara",RTYPE, 0x00000003);
		DEF_KEY_WORD("add",RTYPE, 0x00000004);
		DEF_KEY_WORD("add",RTYPE, 0x00000005);
		DEF_KEY_WORD("srav",RTYPE, 0x00000006);
		// I-Type

		// J-Type
	}
	~Lexer(){
		inf.close();
		words.clear();
		printf("~Lexer");
	}
	Token *scan()
	{
		int i = 0;
		char ch;
		do{
			inf.read(&ch, sizeof(ch));
			if (ch == ';'){
				while (ch != '\n'){
					//printf("skip:%c\n", ch);
					inf.read(&ch, sizeof(ch));
				}
			}
			if (ch == '\n')line++;
		} while (ch == ' ' || ch == '\n' || ch == '\t');
		if (inf.eof()){
			printf("end of file\n");
			return new Token(END);
		}
		if (isalpha(ch)){
			string str;
			do{
				str.push_back(ch);
				inf.read(&ch, sizeof(ch));
			} while (isalnum(ch));  //1状态
			inf.seekg(-1, ios::cur);//回退一个字符
			if (words.find(str) == words.end()){
				return new Word(ID, str);
			}
			return words[str];
		}
		if (isdigit(ch)){
			int value = 0;
			if (ch == '0'){
				inf.read(&ch, sizeof(ch));
				if (ch == 'x' || ch == 'X'){
					inf.read(&ch, sizeof(ch));
					if (isdigit(ch) || (ch >= 'a'&&ch <= 'f') || (ch >= 'A'&&ch <= 'F')){
						do{
							if (isalpha(ch)){
								value = 16 * value + ch - 'A' + 10;
							}else{
								value = 16 * value + ch - '0';
							}
							inf.read(&ch, sizeof(ch));
						} while (isdigit(ch) || (ch >= 'a'&&ch <= 'f') || (ch >= 'A'&&ch <= 'F'));
						inf.seekg(-1, ios::cur);
						return new Integer(NUM, value);
					}else{
						printf("错误的十六进制!");
					}
				}else if (ch >= '0'&&ch <= '7'){
					//八进制整数
					do{
						value = 8 * value + ch - '0';
						inf.read(&ch, sizeof(ch));
					} while (ch >= '0'&&ch <= '7');
					inf.seekg(-1, ios::cur);
					return new Integer(NUM, value);
				}else{
					//十进制整数0
					inf.seekg(-1, ios::cur);
					return new Integer(NUM, 0);
				}
			}else{
				//除0外十进制整数,5状态
				do{
					value = 10 * value + ch - '0';
					inf.read(&ch, sizeof(ch));
				} while (isdigit(ch));
				inf.seekg(-1, ios::cur);//回退一个字符
				return new Integer(NUM, value);
			}
		}
		return new Token(ch);
	}
};

#endif