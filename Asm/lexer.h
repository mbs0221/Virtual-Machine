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

// �ʷ���Ԫ
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

// �ʷ�������
class Lexer{
	ifstream inf;
	map<string, Token*> words;
public:
	int line = 1;
	Lexer(string fp){
		words["data"] = new Word(DATA, "data");
		words["code"] = new Word(CODE, "code");
		words["int"] = new Word(INT, "int");
		words["load"] = new Word(LOAD, "load");
		words["store"] = new Word(STORE, "store");
		words["halt"] = new Word(HALT, "halt");
		words["label"] = new Word(LABEL, "label");
		words["jmp"] = new Word(JMP, "jmp");
		words["jb"] = new Word(JB, "jb");
		words["je"] = new Word(JE, "je");
		words["jne"] = new Word(JNE, "jne");
		words["jg"] = new Word(JG, "jg");
		words["call"] = new Word(CALL, "call");
		words["ret"] = new Word(RET, "ret");
		words["push"] = new Word(PUSH, "push");
		words["pop"] = new Word(POP, "pop");
		words["mov"] = new Word(MOV, "mov");
		words["add"] = new Word(ADD, "add");
		inf.open(fp, ios::in);
	}
	~Lexer(){
		inf.close();
		words.clear();
		printf("~Lexer");
	}
	Token *scan()
	{
		char ch;
		do{
			if (inf.eof()) {
				return new Token(END);
			}
			inf.read(&ch, sizeof(ch));
			if (ch == '\n')line++;
		} while (ch == ' ' || ch == '\n' || ch == '\t');
		
		if (inf.eof() || ch == EOF){
			return new Token(END);
		}
		
		if (isalpha(ch)){
			string str;
			do{
				str.push_back(ch);
				if (inf.eof()) break;
				inf.read(&ch, sizeof(ch));
			} while (isalnum(ch) || ch == '_');  // 支持字母、数字和下划线
			if (!inf.eof()) {
				inf.seekg(-1, ios::cur);//回退一个字符
			}
			if (words.find(str) == words.end()){
				return new Word(ID, str);
			}
			return words[str];
		}
		
		if (isdigit(ch)){
			int value = 0;
			if (ch == '0'){
				if (inf.eof()) {
					return new Integer(INT, 0);
				}
				inf.read(&ch, sizeof(ch));
				if (ch == 'x' || ch == 'X'){
					if (inf.eof()) {
						return new Integer(INT, 0);
					}
					inf.read(&ch, sizeof(ch));
					if (isdigit(ch) || (ch >= 'a'&&ch <= 'f') || (ch >= 'A'&&ch <= 'F')){
						do{
							if (isalpha(ch)){
								value = 16 * value + ch - 'A' + 10;
							}else{
								value = 16 * value + ch - '0';
							}
							if (inf.eof()) break;
							inf.read(&ch, sizeof(ch));
						} while (isdigit(ch) || (ch >= 'a'&&ch <= 'f') || (ch >= 'A'&&ch <= 'F'));
						if (!inf.eof()) {
							inf.seekg(-1, ios::cur);
						}
						return new Integer(INT, value);
					}else{
						printf("错误的十六进制数!");
						return new Integer(INT, 0);
					}
				}else if (ch >= '0'&&ch <= '7'){
					//八进制数字
					do{
						value = 8 * value + ch - '0';
						if (inf.eof()) break;
						inf.read(&ch, sizeof(ch));
					} while (ch >= '0'&&ch <= '7');
					if (!inf.eof()) {
						inf.seekg(-1, ios::cur);
					}
					return new Integer(INT, value);
				}else{
					//十进制数字0
					inf.seekg(-1, ios::cur);
					return new Integer(INT, 0);
				}
			}else{
				// 0-9的十进制数
				do{
					value = 10 * value + ch - '0';
					if (inf.eof()) break;
					inf.read(&ch, sizeof(ch));
				} while (isdigit(ch));
				if (!inf.eof()) {
					inf.seekg(-1, ios::cur);// 移除最后一个字符
				}
				return new Integer(INT, value);
			}
		}
		
		return new Token(ch);
	}
};

#endif