// functions  Michael Pfeiffer  29.03.00
#include "Interpreter/Functions.h"
#include "Interpreter/Interpreter.h"
#include <iostream.h>
#include <math.h>

typedef struct {
	const char *name; double (*function)(double x);
} BIFunctions1;

static BIFunctions1 functions1[] = {
	{"sin", sin},
	{"cos", cos},
	{"tan", tan},
	{"asin", asin},
	{"acos", acos},
	{"atan", atan},
	{"sinh", sinh},
	{"cosh", cosh},
	{"tanh", tanh},
	{"exp", exp},
	{"log", log},
	{"log10", log10},
	{"sqrt", sqrt},
	{"ceil", ceil},
	{"floor", floor},
	{"abs", fabs},
	{NULL, NULL}
};


IFunction1::IFunction1(double (*function)(double x)) {
	this->function = function;
}

bool IFunction1::Call(Interpreter *i, double &result) {
	if (i->stack->FrameElems() == 1) {
		double v;
		i->stack->GetAt(0, v);
		result = function(v);
		return true;
	}
	return false;
}

void IFunction1::Print() {
	cout << "built in function";	
}

Tree *IFunction1::Clone() {
	return new IFunction1(function);
}

void InitFunctions(Global &globals) {
BIFunctions1 *f = functions1;
	while (f->name != NULL) { 
	Node *n = 
		new Node(DEFINITION, 
			new Ident(f->name), 
			new IFunction1(f->function));
		globals.Set(n);
		f++;
	}
}

void InitConstants(Global &globals) {
	globals.Set(Const("pi", M_PI));
	globals.Set(Const("e", M_E));
}