#ifndef __LEXER_H_
#define __LEXER_H_

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

using namespace std;

//关键字
enum Tag{
	IF = 256, THEN, ELSE, DO, WHILE, FOR, CASE, ID, INT, END, FUNC, RETURN,
	// 新增关键字
	VOID, CHAR, FLOAT, DOUBLE, BOOL, STRING,
	TRUE, FALSE, NULL_VAL,
	BREAK, CONTINUE, SWITCH, DEFAULT,
	AND, OR, NOT,
	INCLUDE, DEFINE, CONST,
	PRINT, SCAN, MAIN
};

// 符号
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
	string word;
	Word(int tag, string word) :Token(tag), word(word) {  }
	virtual string place(){
		ostringstream s;
		s << word;
		return s.str();
	}
	virtual string code(){
		return "";
	}
};

struct Type :Word{
	int width;
	static Type* Int;
	static Type* Char;
	static Type* Float;
	static Type* Bool;
	static Type* Void;
	Type(int kind, string word, int width) :Word(kind, word), width(width){  }
	virtual string place(){
		ostringstream s;
		s << word << ":" << width;
		return s.str();
	}
	virtual string code(){
		return "";
	}
};

Type* Type::Int = new Type(INT, "int", 2);
Type* Type::Char = new Type(CHAR, "char", 1);
Type* Type::Float = new Type(FLOAT, "float", 4);
Type* Type::Bool = new Type(BOOL, "bool", 1);
Type* Type::Void = new Type(VOID, "void", 0);

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

struct Float :Token{
	float value;
	Float(int tag, float value) :Token(tag), value(value) {  }
	virtual string place(){
		ostringstream s;
		s << value;
		return s.str();
	}
	virtual string code(){
		return "";
	}
};

struct String :Token{
	string value;
	String(int tag, string value) :Token(tag), value(value) {  }
	virtual string place(){
		return value;
	}
	virtual string code(){
		return "";
	}
};

// �ʷ�������
class Lexer{
	ifstream inf;
	map<string, Token*> words;
public:
	int line = 1;
	Lexer(string fp){
		// 基本类型
		words["int"] = new Word(INT, "int");
		words["char"] = new Word(CHAR, "char");
		words["float"] = new Word(FLOAT, "float");
		words["bool"] = new Word(BOOL, "bool");
		words["void"] = new Word(VOID, "void");
		
		// 控制流
		words["if"] = new Word(IF, "if");
		words["then"] = new Word(THEN, "then");
		words["else"] = new Word(ELSE, "else");
		words["do"] = new Word(DO, "do");
		words["while"] = new Word(WHILE, "while");
		words["for"] = new Word(FOR, "for");
		words["switch"] = new Word(SWITCH, "switch");
		words["case"] = new Word(CASE, "case");
		words["default"] = new Word(DEFAULT, "default");
		words["break"] = new Word(BREAK, "break");
		words["continue"] = new Word(CONTINUE, "continue");
		
		// 函数相关
		words["func"] = new Word(FUNC, "func");
		words["return"] = new Word(RETURN, "return");
		words["main"] = new Word(MAIN, "main");
		
		// 字面量
		words["true"] = new Word(TRUE, "true");
		words["false"] = new Word(FALSE, "false");
		words["null"] = new Word(NULL_VAL, "null");
		
		// 逻辑运算符
		words["and"] = new Word(AND, "and");
		words["or"] = new Word(OR, "or");
		words["not"] = new Word(NOT, "not");
		
		// 预处理器
		words["include"] = new Word(INCLUDE, "include");
		words["define"] = new Word(DEFINE, "define");
		words["const"] = new Word(CONST, "const");
		
		// I/O函数
		words["print"] = new Word(PRINT, "print");
		words["scan"] = new Word(SCAN, "scan");
		
		words["end"] = new Word(END, "end");
		inf.open(fp, ios::in);
	}
	~Lexer(){
		inf.close();
		words.clear();
	}
	Token *scan()
	{
		char ch;
		do{
			inf.read(&ch, sizeof(ch));
			if (ch == '\n')line++;
		} while (ch == ' ' || ch == '\n' || ch == '\t');
		if (ch == EOF){
			return new Token(END);
		}
		
		// 处理单行注释 //
		if (ch == '/'){
			inf.read(&ch, sizeof(ch));
			if (ch == '/'){
				// 跳过整行注释
				do{
					inf.read(&ch, sizeof(ch));
					if (ch == '\n')line++;
				} while (ch != '\n' && ch != EOF);
				// 递归调用以获取下一个token
				return scan();
			} else {
				// 不是注释，回退一个字符
				inf.seekg(-1, ios::cur);
				ch = '/';
			}
		}
		if (isalpha(ch)){
			string str;
			do{
				str.push_back(ch);
				inf.read(&ch, sizeof(ch));
			} while (isalnum(ch));  //1״̬
			inf.seekg(-1, ios::cur);//����һ���ַ�
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
							}
							else{
								value = 16 * value + ch - '0';
							}
							inf.read(&ch, sizeof(ch));
						} while (isdigit(ch) || (ch >= 'a'&&ch <= 'f') || (ch >= 'A'&&ch <= 'F'));
						inf.seekg(-1, ios::cur);
						return new Integer(INT, value);
					}
					else{
						printf("�����ʮ������!");
					}
				}
				else if (ch >= '0'&&ch <= '7'){
					//�˽�������
					do{
						value = 8 * value + ch - '0';
						inf.read(&ch, sizeof(ch));
					} while (ch >= '0'&&ch <= '7');
					inf.seekg(-1, ios::cur);
					return new Integer(INT, value);
				}
				else{
					//ʮ��������0
					inf.seekg(-1, ios::cur);
					return new Integer(INT, 0);
				}
			}
			else{
				//��0��ʮ��������,5״̬
				do{
					value = 10 * value + ch - '0';
					inf.read(&ch, sizeof(ch));
				} while (isdigit(ch));
				inf.seekg(-1, ios::cur);//����һ���ַ�
				return new Integer(INT, value);
			}
		}
		
		// 字符串字面量
		if (ch == '"'){
			string str;
			inf.read(&ch, sizeof(ch));
			while (ch != '"' && ch != EOF){
				if (ch == '\\'){
					inf.read(&ch, sizeof(ch));
					switch(ch){
						case 'n': str += '\n'; break;
						case 't': str += '\t'; break;
						case 'r': str += '\r'; break;
						case '\\': str += '\\'; break;
						case '"': str += '"'; break;
						default: str += ch; break;
					}
				} else {
					str += ch;
				}
				inf.read(&ch, sizeof(ch));
			}
			return new String(STRING, str);
		}
		
		// 字符字面量
		if (ch == '\''){
			inf.read(&ch, sizeof(ch));
			char charValue = ch;
			if (ch == '\\'){
				inf.read(&ch, sizeof(ch));
				switch(ch){
					case 'n': charValue = '\n'; break;
					case 't': charValue = '\t'; break;
					case 'r': charValue = '\r'; break;
					case '\\': charValue = '\\'; break;
					case '\'': charValue = '\''; break;
					default: charValue = ch; break;
				}
			}
			inf.read(&ch, sizeof(ch)); // 跳过结束的'
			return new Integer(CHAR, charValue);
		}
		
		return new Token(ch);
	}
};

#endif