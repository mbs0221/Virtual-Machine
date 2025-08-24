#ifndef __LEXER_H_
#define __LEXER_H_

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <queue>
#include <cstring>

using namespace std;

enum Tag{
	IF = 256, THEN, ELSE, DO, WHILE, FOR, SWITCH, CASE, BREAK, CONTINUE, TRY, CATCH, FINALLY, THROW,
	BASIC, INT, CHAR,
	END,
	ID, NUM, FUNCTION,
	ASSIGN, // ASSIGN
	AND, OR, NOT, // LOGIC
	BIT_AND, BIT_OR, BIT_NOT, // BIT LOGIC
	EQ, NEQ, // REL
	LT, LEQ, GEQ, GT, // REL
	SHL, SHR, // SHIFT
	ADD, SUB, // TERM
	MUL, DIV, // FACTOR
	INC, DEC // UNARY
};

// �ʷ���Ԫ
struct Token{
	int	kind;
	Token(int tag) :kind(tag){  }
	virtual string place(){
		ostringstream s;
		s << kind;
		return s.str();
	}
};

struct Word :Token{
	string str;
	Word(int tag, string word) :Token(tag), str(word) {  }
	virtual string place(){
		ostringstream s;
		s << str;
		return s.str();
	}
	virtual string code(){
		return str;
	}
	const char* getName() { return str.c_str(); }
};

struct Type :Word{
	int width;
	static Type *Int, *Float, *Double, *Char, *Void;
	Type(int kind, string word, int width) :Word(kind, word), width(width){  }
	virtual string place(){
		ostringstream s;
		s << str << ":" << width;
		return s.str();
	}
	virtual string code(){
		return str;
	}
};

Type* Type::Int = new Type(BASIC, "qw", 4);
Type* Type::Float = new Type(BASIC, "dw", 2);
Type* Type::Double = new Type(BASIC, "qw", 4);
Type* Type::Char = new Type(BASIC, "db", 1);
Type* Type::Void = new Type(BASIC, "void", 0);

struct Integer :Token{
	int value;
	Integer(int tag, int value) :Token(tag), value(value) {  }
	virtual string place(){
		ostringstream s;
		s << value;
		return s.str();
	}
	virtual string code(){
		return "INT";
	}
};

class ReadBuffer {
private:
	FILE *file;
	char *buf;
	int front, rear;
public:
	const int size = 3;
	ReadBuffer() {
		front = 0, rear = 0;
	}
	~ReadBuffer() {
		if (file)
			fclose(file);
	}
	void open(char *filename) {
		file = fopen(filename, "rb");
	}
	char operator[](int i) {
		return buf[(front + i) % size];
	}
	char peak() {
		return buf[front % size];
	}
	void pop() {
		if (!isempty()) {
			front = (front + 1) % size;
			read();
		}
	}
	bool isempty() {
		return (front == rear);
	}
	bool isfull() {
		return ((rear + 1) % size) == front;
	}
	void read() {
		while (!isfull()) {
			size_t sz = fread(buf+rear, sizeof(char), 1, file);
			if (sz == 0) {
				// ��ֹ
				buf[rear-1] = '\0';
				break;
			}
			rear = (rear + sz) % size;
		}		
	}
};

// �ʷ�������
class Lexer{
	map<string, Token*> words;
	ReadBuffer buffer;
public:
	int line = 1;
	Lexer(){
		words["int"] = Type::Int;
		words["char"] = Type::Char;
		words["float"] = Type::Float;
		words["double"] = Type::Double;
		words["void"] = Type::Void;
		words["if"] = new Word(IF, "if");
		words["then"] = new Word(THEN, "then");
		words["else"] = new Word(ELSE, "else");
		words["do"] = new Word(DO, "do");
		words["while"] = new Word(WHILE, "while");
		words["for"] = new Word(FOR, "for");
		words["case"] = new Word(CASE, "case");
		words["break"] = new Word(BREAK, "break");
		words["continue"] = new Word(CONTINUE, "continue");
		words["end"] = new Word(END, "end");
		words["try"] = new Word(TRY, "try");
		words["catch"] = new Word(CATCH, "catch");
		words["finally"] = new Word(FINALLY, "finally");
		words["throw"] = new Word(THROW, "throw");
		// Operator
		words["&"] = new Word(BIT_AND, "&"); // BIT LOGIC
		words["|"] = new Word(BIT_OR, "|");
		words["~"] = new Word(BIT_NOT, "~"); 
		words["&&"] = new Word(AND, "AND"); // LOGIC
		words["||"] = new Word(OR, "OR");
		words["<<"] = new Word(SHL, "SHL"); // SHIFT
		words[">>"] = new Word(SHR, "SHR");
		words["=="] = new Word(EQ, "EQ");  // COMP
		words["!="] = new Word(NEQ, "NEQ");
		words["<"] = new Word(LT, "LT");
		words["<="] = new Word(LEQ, "LEQ");
		words[">="] = new Word(GEQ, "GEQ");
		words[">"] = new Word(GT, "GT");
	}
	~Lexer(){
		printf("~Lexer\n");
		words.clear();
	}
	void open(char *filename) {
		buffer.open(filename);
	}
	Integer* match_integer(ReadBuffer & buffer) {
		int value = 0;
		char ch = buffer.peak();
		buffer.pop();
		if (ch == '0') {
			ch = buffer.peak();
			if (ch == 'x' || ch == 'X') {
				ch = buffer.peak();
				buffer.pop();
				if (isdigit(ch) || (ch >= 'a'&&ch <= 'f') || (ch >= 'A'&&ch <= 'F')) {
					do {
						if (isalpha(ch)) {
							value = 16 * value + ch - 'A' + 10;
						}
						else {
							value = 16 * value + ch - '0';
						}
						ch = buffer.peak();
						buffer.pop();
					} while (isdigit(ch) || (ch >= 'a'&&ch <= 'f') || (ch >= 'A'&&ch <= 'F'));
					return new Integer(NUM, value);
				}
				else {
					printf("�����ʮ������!");
					return nullptr;
				}
			}
			else if (ch >= '0'&&ch <= '7') {
				//�˽�������
				do {
					value = 8 * value + ch - '0';
					ch = buffer.peak();
					buffer.pop();
				} while (ch >= '0'&&ch <= '7');
				buffer.read();
				return new Integer(NUM, value);
			}
			else {
				//ʮ��������0
				buffer.read();
				return new Integer(NUM, 0);
			}
		}
		else {
			//��0��ʮ��������,5״̬
			do {
				value = 10 * value + ch - '0';
				ch = buffer.peak();
				buffer.pop();
			} while (isdigit(ch));
			return new Integer(NUM, value);
		}
	}
	Token* match_operator(ReadBuffer & buffer) {
		char ch[3];
		memset(ch, 0, sizeof(char) * 3);
		ch[0] = buffer.peak();
		buffer.pop();
		ch[1] = buffer.peak();
		switch (ch[1]) {
			case '&':
			case '|':
			case '<':
			case '>':
			case '=':
				buffer.pop();
				return words[ch];
				break;
			default:
				break;
		}
		ch[1] = '\0';
		if (words.find(ch) != words.end()) {
			return words[ch];
		}
		return new Token(ch[0]);
	}
	Token *scan()
	{
		char ch;
		do{
			ch = buffer.peak();
			if (ch == '\n')line++;
			buffer.pop();
		} while (ch == ' ' || ch == '\n' || ch == '\t');
		if (isalpha(ch)){
			string str;
			do{
				str.push_back(ch);
				ch = buffer.peak();
				buffer.read();
			} while (isalnum(ch) || ch == '_');
			if (words.find(str) == words.end()){
				return new Word(ID, str);
			}
			return words[str];
		}
		if (isdigit(ch)){
			return match_integer(buffer);
		}
		
		return match_operator(buffer);
	}
};

#endif