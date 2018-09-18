#ifndef BUILT_IN_FUNCTIONS
#define BUILT_IN_FUNCTIONS

#include "Interpreter/Tree.h"
#include "Interpreter/Global.h"

class IFunction1 : public Function {
	double (*function)(double x);
public:
	IFunction1(double (*function)(double x));
	bool Call(Interpreter *i, double &result);
	void Print();
	Tree *Clone();
};

void InitFunctions(Global &globals);
void InitConstants(Global &globals);

#endif
