#ifndef STACK
#define STACK

#include "be/support/List.h"

class StackFrame {
public:
	BList locals; 
	StackFrame *next;
	StackFrame();
	~StackFrame();
	void Append(double value);
};

class Stack {
	StackFrame *stack;	
public:
	Stack();
	~Stack();
	
	void NewFrame();
	void NewFrame(StackFrame *f);
	void DeleteFrame();
	
	void Append(double value);
	bool GetAt(int i, double &value);
	int32 FrameElems(); // number of local variables
};

#endif
