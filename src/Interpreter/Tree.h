#ifndef TREE
#define TREE

#include <SupportDefs.h>

enum NodeKind {  // = Token constants inclusive:
	LIST = 2048, // list 
	CALL,        // function call 
	SIGN,        // negative sign
	CONST,       // constant
	DEFINITION   // function defintion
};

enum LeafKind {
	IDENT,
	NUMBER,
	LOCAL,
	FUNCTION
}; 
/*
	Node-Structures for kinds
	-------------------------
	T_IF:
		left: Condition (Tree obect)
		right: T_ELSE (Node object)
			left: if-Expression (Tree object) 
			right: then-Expression (Tree object)
	LIST:
		left: data
		right: next list entry or NULL
	CALL:
		left: function name (Ident object)
		right: list of parameters or NULL (Node object; kind = LIST) 
	SIGN:
		left: expression 
		right: NULL
	CONST:
		left: constant name (Ident object)
		right: expression 
	DEFINITION:
		left: function name
		right: LIST Node
			left: list of formal parameters (each is an Ident object)
			right: expression
	or
		right: Function object
	LOCAL:
		index: local variable
*/
class Tree {
public:
	virtual ~Tree() {};
	virtual bool IsLeaf() = 0;
	virtual bool IsNode() = 0;
	virtual Tree *LeftNode() = 0;
	virtual Tree *RightNode() = 0;
	virtual void Print() = 0;
	virtual Tree *Clone() = 0;
};

typedef Tree *TreePtr;

class Node : public Tree {
public:
	Node(int16 kind, Tree *left, Tree *right);
	~Node();
	int16 kind;
	Tree *left, *right;
	bool IsLeaf();
	bool IsNode();
	Tree *LeftNode();
	Tree *RightNode();
	void Print();
	Tree *Clone();
};

class Leaf : public Tree {
public:
	virtual ~Leaf() {};
	bool IsLeaf();
	bool IsNode();
	virtual int Kind() = 0;
	virtual Tree *LeftNode();
	virtual Tree *RightNode();
	virtual void Print() = 0;
	virtual Tree *Clone() = 0;
};

class Ident : public Leaf {
public:
	const char *ident;
	Ident(const char *ident);
	~Ident();
	int Kind();
	void Print();
	Tree *Clone();
};

class Number : public Leaf {
public:
	double number;
	Number(double &n);	
	int Kind();
	void Print();
	Tree *Clone();
};

Node *Const(const char *name, double value);

class Local : public Leaf {
public:
	int index;
	Local(int i);
	int Kind();
	Tree *Clone();
	void Print();
};

class Interpreter;

class Function : public Leaf {
public:
	virtual ~Function();
	virtual bool Call(Interpreter *interpreter, double &result) = 0;
	int Kind();
	virtual Tree *Clone() = 0;
	virtual void Print() = 0;
};

#endif
