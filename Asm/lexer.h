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
#include "code.h"

using namespace std;

// 词法单元
struct Token {
	int kind;
	Token(int tag) :kind(tag) { }
	virtual string place();
	virtual string code();
};

struct Word :Token {
	string word;
	Word(int tag, string word) :Token(tag), word(word) { }
	virtual string place();
	virtual string code();
};

struct Type :Word {
	int width;
	static Type* Int;
	Type(int kind, string word, int width) :Word(kind, word), width(width) { }
	virtual string place();
	virtual string code();
};

struct Integer :Token {
	int value;
	Integer(int tag, int value) :Token(tag), value(value) { }
	virtual string place();
	virtual string code();
};

struct String :Token {
	string value;
	String(int tag, string value) :Token(tag), value(value) { }
	virtual string place();
	virtual string code();
};

// 词法分析器
class Lexer {
	ifstream inf;
	map<string, Token*> words;
public:
	int line = 1;
	Lexer(string fp);
	~Lexer();
	Token *scan();
	
private:
	// 辅助函数
	void skipWhitespace();
	Token* scanIdentifier(char ch);
	Token* scanNumber(char ch);
	Token* scanDecimal(char ch);
	Token* scanOctal(char ch);
	Token* scanHexadecimal(char ch);
	Token* scanString();
	Token* scanComment();
};

#endif