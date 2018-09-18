#ifndef INTERPRETER
#define INTERPRETER
#include "Interpreter/Tree.h"
#include "Interpreter/Global.h"
#include "Interpreter/Stack.h"

class Interpreter {
	bool EvalParamList(StackFrame *frame, Node *p);	
	bool Call(Node *tree, double &result);
	bool GetGlobalValue(Ident *id, double &result);
	bool GetLocalValue(Local *l, double &result);
	bool Define(Tree *tree, double &result);
	bool ConstDef(Node *node, double &result);
	bool FuncDef(Node *node, double &result);
	bool IfElse(Node *node, double &result);
	bool And(Node *node, double &result);
	bool Or(Node *node, double &result);
	bool Not(Node *node, double &result);
	bool Compare(Node *node, double &result);
	bool Eval(TreePtr tree, double &result);
	void Init();
	
	int16 maxCallDepth;
	int16 callDepth;
public:
	Stack *stack;
	Global *globals;

	Interpreter();
	~Interpreter();
	void Reset();
	
	bool Evaluate(TreePtr tree, double &result);
	bool Evaluate(TreePtr tree, double value, double &result);
};

#endif
