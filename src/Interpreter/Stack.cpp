// stack Michael Pfeiffer 29.3.00
#include "Interpreter/Stack.h"
#include <List.h>
#include <assert.h>

StackFrame::StackFrame() {
	next = NULL;
}

static bool deleteLocals(void *item) {
	delete (double*)item;
	return false;
}

StackFrame::~StackFrame() {
	locals.DoForEach(deleteLocals);
/*
int32 i = 0;
double *d;
	for (i = 0; NULL != (d = (double*)locals.ItemAt(i)); i++) delete d;
*/
	locals.MakeEmpty();
}

void StackFrame::Append(double value) {
	locals.AddItem(new double(value));
}

Stack::Stack() {
	stack = new StackFrame();
}

Stack::~Stack() {
	if (stack != NULL) {
	StackFrame *old;
		do {
			old = stack; stack = stack->next;
			delete old;
		} while (stack != NULL);	
	}
}

void Stack::NewFrame() {
StackFrame *f = new StackFrame();
	f->next = stack; stack = f;
}

void Stack::NewFrame(StackFrame *f) {
	f->next = stack; stack = f;
}

void Stack::DeleteFrame() {
StackFrame *old = stack; 
	if (old != NULL) {
		stack = stack->next; delete old;
	}
}

void Stack::Append(double value) {
	stack->locals.AddItem(new double(value));
}

bool Stack::GetAt(int i, double &value) {
double *d = (double*)stack->locals.ItemAt(i);
	if (d != NULL) { value = *d; return true; }
	else return false;
}

int32 Stack::FrameElems() {
	return stack->locals.CountItems();
}
