#ifndef PARSER
#define PARSER
#include "Interpreter/Tree.h"
#include "Interpreter/Scanner.h"
#include <SupportKit.h>

class Locals {
	BList locals; // formal parameters of function definition
public:
	~Locals();
	int32 Find(const char *ident);
	bool IsLocal(const char *ident, TreePtr &tree);
	bool Add(const char *ident);
	void MakeEmpty();
};

class Parser {
	Scanner *scanner;
	Locals locals;
	
	bool NotCond(TreePtr &tree);
	bool CompCond(TreePtr &tree);
	bool AndCond(TreePtr &tree);
	bool Condition(TreePtr &tree);
	bool Identifier(TreePtr &tree);
	bool Call(TreePtr &tree);
	void Sign(bool sign, TreePtr &tree);
	bool Literal(TreePtr &tree);
	bool Pow(TreePtr &tree);
	bool Factor(TreePtr &tree);
	bool Expression(TreePtr &tree);
	bool IdentList(TreePtr &tree);
	bool Definition(TreePtr &tree);
	bool Definitions(TreePtr &tree);
	bool Reset();
	
	int16 tk; int32 tkPos;
public:
	Parser(Scanner *scanner = NULL);
	~Parser();
	void SetScanner(Scanner *scanner = NULL);
	void SetLocal(const char *variable);
	void EmptyLocals();
	bool ParseExpr(TreePtr &ptr);
	bool ParseDefs(TreePtr &ptr);
	int32 GetPos() { return tkPos; }
};
#endif
