/* interpreter  Michael Pfeiffer 15.04.00 */
#include "Interpreter/Tree.h"
#include "Interpreter/Interpreter.h"
#include "Interpreter/Stack.h"
#include "Interpreter/Global.h"
#include "Interpreter/Functions.h"
#include "Interpreter/Scanner.h"
#include <math.h>

Interpreter::Interpreter() {
	maxCallDepth = 100; callDepth = 0;
	stack = new Stack(); globals = new Global();
	Init();
}

Interpreter::~Interpreter() {
	if (stack != NULL) { delete stack; stack = NULL; }
	if (globals != NULL) { delete globals; globals = NULL; }
}

void Interpreter::Reset() {
	if (stack != NULL) delete stack;
	stack = new Stack();
	if (globals != NULL) delete globals;
	globals = new Global();
	Init();
}

void Interpreter::Init() {
	InitFunctions(*globals);
	InitConstants(*globals);
}

bool Interpreter::EvalParamList(StackFrame *frame, Node *p) {
double v;
	while (p != NULL) {
		if (!Eval(p->left, v)) return false;
		frame->Append(v);
		p = (Node*)p->right;
	}
	return true;
}

static int32 CountFormalParams(Node *n) {
int32 i = 0;
	while (n != NULL) {
		n = (Node*)n->right; i++;
	}
	return i;
}

bool Interpreter::Call(Node *tree, double &result) {
Tree *f;
	if (callDepth >= maxCallDepth) return false;
	if (globals->Find(((Ident*)tree->left)->ident, f)) {
		bool ok;
		StackFrame *frame = new StackFrame();
		if (!EvalParamList(frame, (Node*)tree->right)) {
			delete frame;
			ok = false;
		} else {
			callDepth++;
			
			stack->NewFrame(frame);
			if (((Node*)f)->right->IsNode()) {
			// assert: f->right->kind == LIST
				Node *n = (Node*)((Node*)f)->right;
				if (CountFormalParams((Node*)n->left) == stack->FrameElems()) {
					ok = Eval(n->right, result);
				} else
					ok = false;

			} else {
			// assert: f->right->Kind() == FUNCTION
				ok = ((Function*)((Node*)f)->right)->Call(this, result);
			}

			stack->DeleteFrame();

			callDepth--;
		}
		return ok;
	} else
		return false;
}

bool Interpreter::GetGlobalValue(Ident *id, double &result) {
Tree *tree;
	if (globals->Find(id->ident, tree) &&
		(((Node*)tree)->kind == CONST)) {
		result = ((Number*)((Node*)tree)->right)->number;
		return true;
	} 
	return false;
}

bool Interpreter::GetLocalValue(Local *l, double &result) {
	return stack->GetAt(l->index, result);
}

bool Interpreter::Define(Tree *tree, double &result) {
Node *cur = (Node*)tree;
	do {
		if (!Eval(cur->left, result)) return false;
		cur = (Node*)cur->right;
	} while (cur != NULL);
	return true;
}

bool Interpreter::ConstDef(Node *node, double &result) {
	if (Eval(node->right, result)) {
		globals->Set(new Node(CONST, node->left->Clone(), new Number(result)));
		return true;
	} else
		return false;
}

bool Interpreter::FuncDef(Node *node, double &result) {
	globals->Set((Node*)node->Clone()); 
	return true;
}

bool Interpreter::IfElse(Node *node, double &result) {
	if (!Eval(node->left, result)) return false;
	if (result != 0) // if-Expression 
		return Eval(((Node*)node->right)->left, result);
	else // then-Expression
		return Eval(((Node*)node->right)->right, result);
}

bool Interpreter::Or(Node *node, double &result) {
Tree *cur = node->right;
	while(true) {
		if (!Eval(node->left, result)) return false;
		if (result != 0) return true;
		if (cur->IsNode()) {
			node = (Node*)node->right; cur = node->right;
		} else
			return Eval(node->right, result);
	}
}

bool Interpreter::And(Node *node, double &result) {
Tree *cur = node->right;
	while(true) {
		if (!Eval(node->left, result)) return false;
		if (result == 0) return true;
		if (cur->IsNode()) {
			node = (Node*)node->right; cur = node->right;
		} else
			return Eval(node->right, result);
	}
}

bool Interpreter::Not(Node *node, double &result) {
	if (Eval(node->left, result)) {
		result = !result; return true;
	} else
		return false;
}

bool Interpreter::Compare(Node *node, double &result) {
double r;
	if (!Eval(node->left, result)) return false;
	if (!Eval(node->right, r)) return false;
	switch (node->kind) {
	case T_EQUAL:
		result = result == r;
		break;
	case T_NOT_EQUAL:
		result = result != r;
		break;
	case T_LESS:
		result = result < r;
		break;
	case T_GREATER:
		result = result > r;
		break;
	case T_LESS_EQUAL:
		result = result <= r;
		break;
	case T_GREATER_EQUAL:
		result = result >= r;
		break;
	}
	return true;
}

bool Interpreter::Eval(TreePtr tree, double &result) {
double x, y;
	if (tree->IsNode()) {
		switch(((Node*)tree)->kind) {
		case '*':
		case '/':
		case '+':
		case '-':
		case '^':
		case '%':
			if (!Eval(((Node*)tree)->left, x)) return false;
			if (!Eval(((Node*)tree)->right, y)) return false;
			switch (((Node*)tree)->kind) {
			case '*':
				result = x * y;
				break;
			case '/':
				if (y == 0) return false;
				result = x / y;
				break;
			case '+':
				result = x + y;
				break;
			case '-':
				result = x - y;
				break;
			case '^':
				result = pow(x, y);
				break;
			case '%':
				result = fmod(x, y);
				break;
			}
			break;
			
		case SIGN:
			if (!Eval(((Node*)tree)->left, x)) return false;
			result = -x;
			break;
			
		case CALL:
			return Call((Node*)tree, result);
		
		case LIST: // definition list
			return Define(tree, result);
			break;
		
		case CONST:
			return ConstDef((Node*)tree, result);
			break;
		
		case DEFINITION:
			return FuncDef((Node*)tree, result);
			break;
		
		case T_IF:
			return IfElse((Node*)tree, result);
			break;
			
		case '|':
			return Or((Node*)tree, result);
			break;

		case '&':
			return And((Node*)tree, result);
			break;
		
		case '!':
			return Not((Node*)tree, result);
			break;
		
		case T_EQUAL:
		case T_NOT_EQUAL:
		case T_LESS:
		case T_GREATER:
		case T_LESS_EQUAL:
		case T_GREATER_EQUAL:
			return Compare((Node*)tree, result);
			break; 
		
		default:
			return false;
		}
	} else {
		switch (((Leaf*)tree)->Kind()) {
		case NUMBER: result = ((Number*)tree)->number;
			break;
		case IDENT:
			return GetGlobalValue((Ident*)tree, result);
		case LOCAL:
			return GetLocalValue((Local*)tree, result);
		default:
			return false;
		}
	}
	return isfinite(result);
}

bool Interpreter::Evaluate(TreePtr tree, double &result) {
	if (tree == NULL) { result = NAN; return true; }
	return Eval(tree, result);
}

bool Interpreter::Evaluate(TreePtr tree, double value, double &result) {
	if (!isfinite(value)) return false;
	if (tree == NULL) { result = NAN; return true; }
	stack->NewFrame();
	stack->Append(value);
	bool ok = Eval(tree, result);
	stack->DeleteFrame();
	return ok;
}
