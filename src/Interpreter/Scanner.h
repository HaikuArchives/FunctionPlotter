#ifndef SCANNER
#define SCANNER

#include <SupportDefs.h>


#define IDENT_LEN 255
	enum Token {
	T_EOF = 1024, T_IDENT, T_NUMBER, T_NAN, T_ERROR, T_IF, T_THEN, T_ELSE,
	T_EQUAL, T_NOT_EQUAL, T_GREATER_EQUAL, T_LESS_EQUAL,
	T_LESS = '<', T_GREATER = '>' 
	};

class Reader {
public:
	virtual ~Reader() {};
	virtual void Reset() = 0;
	virtual void NextChar(int16 &ch, int32 &pos) = 0;
};

class StringReader : public Reader {
	const char *text;
	int32 i;
public:
	StringReader(const char *text = NULL);
	void SetString(const char *text = NULL);
	void Reset();
	void NextChar(int16 &ch, int32 &pos);
};

typedef union {
	char ident[IDENT_LEN];
	double number;
} Result;

class Scanner {
	Reader *reader;
	int16 ch, tk; 
	int32 chPos, tkPos;
	
	double Integer(int &digits);
	void Number();
	void Ident();
	void SkipWhiteSpaces();
	void SkipLineComment();
public:
	Result value;

	Scanner(Reader *reader = NULL);
	void SetReader(Reader *reader = NULL);
	~Scanner();
	bool Reset();
	void NextToken(int16 &tk, int32 &pos);
};

#endif
