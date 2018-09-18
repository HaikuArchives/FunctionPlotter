/* parser  Michael Pfeiffer 15.04.00 */
/*
 When a parser function returns true, tree is a non NULL pointer to a Tree object.

 When a parser function returns false, it must remove all objects from the heap,
 which it created. The tree pointer may contain any value and must not be used.
*/  

#include <be/support/List.h>
#include "Interpreter/Parser.h"

// symbol table handling:

BList locals; 

Locals::~Locals() {
	MakeEmpty();
}

int32 Locals::Find(const char *ident) {
int32 i = 0;
const char *id;
	while ((NULL != (id = (const char*)locals.ItemAt(i))) && 
			(strcmp(id, ident) != 0)) i++;
	if (id != NULL) return i; else return -1;
}
// Is ident a local variable then return a index reference to it
bool Locals::IsLocal(const char *ident, TreePtr &tree) {
int32 i = Find(ident);
	if (i != -1) { 
		tree = new Local(i);
		return true;
	} else 
		return false;
}

bool Locals::Add(const char *ident) {
int32 i = Find(ident);
	if (i == -1) {
		char *local = new char[strlen(ident)+1];
		strcpy(local, ident);
		locals.AddItem(local);
		return true;
	} else return false;
}

void Locals::MakeEmpty() {
int32 i = 0;
char *id;
	while (NULL != (id = (char*)locals.ItemAt(i))) {
		delete id; i++;
	}	
	locals.MakeEmpty();
}

// Grammar 

// NotCond = ["!"] Expression.
bool Parser::NotCond(TreePtr &tree) {
bool n = (tk == '!');
	if (n) scanner->NextToken(tk, tkPos);
	if (!Expression(tree)) return false;
	if (n) tree = new Node('!', tree, NULL);
	return true;
}

// CompCond = NotCond [("==" | "!=" | "<" | ">" | "<=" | ">=") NotCond].
bool Parser::CompCond(TreePtr &tree) {
Tree *t;
int16 kind;
	if (!NotCond(tree)) return false;
	switch (tk) {
	case T_EQUAL:
	case T_NOT_EQUAL:
	case T_LESS:
	case T_GREATER:
	case T_LESS_EQUAL:
	case T_GREATER_EQUAL:
		kind = tk;
		scanner->NextToken(tk, tkPos);
		if (!NotCond(t)) { delete tree; return false; }
		tree = new Node(kind, tree, t);
		break;
	}
	return true;
}

// AndCond = CompCond {"&" CompCond}.
bool Parser::AndCond(TreePtr &tree) {
Tree *node = NULL;
Node *cur;
	if (CompCond(tree)) {
		if (tk != '&') return true;
		
		cur = (Node*)tree; tree = NULL;
		while (tk == '&') {
			scanner->NextToken(tk, tkPos);
			if (!CompCond(node)) { }
			if (tree == NULL) {
				tree = cur = new Node('&', cur, node);
			} else {
				cur->right = new Node('&', cur->right, node);
				cur = (Node*)cur->right;
			}
		}
		return true;
	} else
		return false;
}

// Condition = AndCond {"|" AndCond}.
bool Parser::Condition(TreePtr &tree) {
Tree *node = NULL;
Node *cur;
	if (AndCond(tree)) {
		if (tk != '|') return true;
		
		cur = (Node*)tree; tree = NULL;
		while (tk == '|') {
			scanner->NextToken(tk, tkPos);
			if (!AndCond(node)) { }
			if (tree == NULL) {
				tree = cur = new Node('|', cur, node);
			} else {
				cur->right = new Node('|', cur->right, node);
				cur = (Node*)cur->right;
			}
		}
		return true;
	} else
		return false;
}

// Identifier = ident.
bool Parser::Identifier(TreePtr &tree) {
	if (tk == T_IDENT) {
		if (!locals.IsLocal(scanner->value.ident, tree)) 
			tree = new Ident(scanner->value.ident);
		scanner->NextToken(tk, tkPos);
		return true;
	} else return false;
}

// Call = "(" [Expr {"," Expr}] ")". 
// Note: tree contains an Ident object on call
bool Parser::Call(TreePtr &tree) {
TreePtr node;
Node *first = NULL, *param = NULL;
	scanner->NextToken(tk, tkPos);
	if (tk != ')') {
		if (!Expression(node)) return false;
		first = param = new Node(LIST, node, NULL); 		
		while (tk == ',') {
			scanner->NextToken(tk, tkPos);
			if (!Expression(node)) {
				delete param; return false;
			}
			param->right = new Node(LIST, node, NULL);
			param = (Node*)param->right;
		};
		tree = new Node(CALL, tree, first);
	} else {
		tree = new Node(CALL, tree, NULL);
	}
	if (tk != ')') return false;
	scanner->NextToken(tk, tkPos);
	return true;
}

void Parser::Sign(bool sign, TreePtr &tree) {
	if (sign) tree = new Node(SIGN, tree, NULL);
}

// Literal = {"-"} (ident ["(" Call ] | number | "(" Expression ")" | 
//            "if" Condition "then" Expression "else" Expression ).
bool Parser::Literal(TreePtr &tree) {
Tree *a, *b;
int sign = 0;
	while (tk == '-') { scanner->NextToken(tk, tkPos); sign++; };
	sign %= 2;
	switch (tk) {
	case T_IDENT: 
		Identifier(tree);
		if ((tk == '(') && !Call(tree)) { delete tree; return false; }
		Sign(sign, tree);
		break;
	
	case T_NUMBER:
		if (sign) {
			double n = -scanner->value.number; tree = new Number(n);
		} else
			tree = new Number(scanner->value.number);		
		scanner->NextToken(tk, tkPos);
		break;
		
	case '(':
		scanner->NextToken(tk, tkPos);
		if (!Expression(tree)) return false;
		if (tk != ')') { delete tree; return false; }
		Sign(sign, tree);
		scanner->NextToken(tk, tkPos);
		break;

	case T_IF:
		scanner->NextToken(tk, tkPos);
		if (!Condition(tree)) return false;
		if (tk != T_THEN) goto if_error1;
		scanner->NextToken(tk, tkPos);
		if (!Expression(a)) goto if_error1;
		if (tk != T_ELSE) goto if_error2;
		scanner->NextToken(tk, tkPos);
		if (!Expression(b)) goto if_error2;
		tree = new Node(T_IF, tree, new Node(T_ELSE, a, b));
		Sign(sign, tree);
		break;
		
	default:
		return false;
	}
	return true;

if_error2:
	delete a;
if_error1:
	delete tree;
	return false;
}

// Pow = Literal ["^" Literal].
bool Parser::Pow(TreePtr &tree) {
TreePtr node = NULL; 
int16 kind;
	if (Literal(tree)) {
		if (tk == '^') {
			kind = tk;
			scanner->NextToken(tk, tkPos);
			if (!Literal(node)) {
				delete tree; return false;
			}
			tree = new Node(kind, tree, node);
		}
		
		return true;
	} else return false;
}

// Factor = Pow [(["*"] | "/" | "%") Factor].
bool Parser::Factor(TreePtr &tree) {
TreePtr node = NULL; 
int16 kind;
	if (Pow(tree)) {
		for(;;) {
			switch(tk) {
			case T_IDENT: case T_NUMBER: case '(': case T_IF:
				if (!Pow(node)) {
					delete tree; return false;
				}
				tree = new Node('*', tree, node);
				break;
			case '*': case '/': case '%':
				kind = tk;
				scanner->NextToken(tk, tkPos);
				if (!Pow(node)) {
					delete tree; return false;
				}
				tree = new Node(kind, tree, node);
				break;
			default:
				return true;
			}
		}
		return true;
	} else return false;
}

// Expression = Factor ["+" | "-" Expression].
bool Parser::Expression(TreePtr &tree) {
TreePtr node = NULL;
int16 kind;
	if (Factor(tree)) {
		while ((tk == '+') || (tk == '-')) {
			kind = tk;
			scanner->NextToken(tk, tkPos);
			if (!Factor(node)) {
				delete tree; return false;
			}
			tree = new Node(kind, tree, node); 
		}
		return true;
	} else return false;
}

// IdentList = [ident {"," ident}].
bool Parser::IdentList(TreePtr &tree) {
Node *param;
Tree *ident;
	if (tk != T_IDENT) return true;
	Identifier(ident);
	locals.Add(((Ident*)ident)->ident);
	tree = param = new Node(LIST, ident, NULL);
	while (tk == ',') {
		scanner->NextToken(tk, tkPos);
		if (!Identifier(ident)  || !locals.Add(((Ident*)ident)->ident)) {
			delete tree;
			return false;
		}
		
		param->right = new Node(LIST, ident, NULL);
		param = (Node*)param->right;
	}
	return true;
}

// Definition = ident ["(" IdentList ")"] "=" Expression ";".
bool Parser::Definition(TreePtr &tree) {
Tree *expr = NULL, *param = NULL;
bool function = false;
	if (!Identifier(tree)) return false;
	if (tk == '(') {
		function = true;
		scanner->NextToken(tk, tkPos);
		if (!IdentList(param)) {
			locals.MakeEmpty();
			delete tree; return false;
		}
		if (tk != ')') {
			delete tree; if (param != NULL) delete param;
			locals.MakeEmpty();
			return false;
		}
		scanner->NextToken(tk, tkPos);
	}
	if (tk != '=') {
		delete tree; if (param != NULL) delete param; 
		locals.MakeEmpty();
		return false;
	}
	scanner->NextToken(tk, tkPos);

	if (!Expression(expr)) {
		delete tree; if (param != NULL) delete param;
		locals.MakeEmpty();
		return false;
	}

	locals.MakeEmpty();

	if (function) {
		tree = new Node(DEFINITION, tree, new Node(LIST, param, expr));
	} else {
		tree = new Node(CONST, tree, expr);
	}

	if (tk != ';') { delete tree; return false; }
	scanner->NextToken(tk, tkPos);

	return true;	
}

// Definitions = { Definition }.
bool Parser::Definitions(TreePtr &tree) {
Tree *def;
Node *cur;
	if (tk != T_IDENT) { tree = NULL; return true; }
	if (!Definition(tree)) return false;
	tree = cur = new Node(LIST, tree, NULL);

	while (tk == T_IDENT) {
		if (!Definition(def)) { delete tree; return false; }
		cur->right = new Node(LIST, def, NULL);
		cur = (Node*)cur->right;		
	}
	return true;
}

bool Parser::ParseExpr(TreePtr &tree) {
	if (!Reset()) return false;
	
	if (Expression(tree)) {
		if (tk == T_EOF) return true;
		delete tree; 
	}
	tree = NULL;
	return false;
}

bool Parser::ParseDefs(TreePtr &tree) {
	if (!Reset()) return false;
	
	if (Definitions(tree)) {
		if (tk == T_EOF) return true;
		if (tree != NULL) { delete tree; tree = NULL; }
	}
	return false;
}

bool Parser::Reset() {
	if ((scanner != NULL) && scanner->Reset()) {
		scanner->NextToken(tk, tkPos);
		return true;
	} else
		return false;
}

Parser::Parser(Scanner *scanner = NULL) {
	this->scanner = scanner;
}

Parser::~Parser() {
	if (scanner != NULL) {
		delete scanner; scanner = NULL;
	}
}

void Parser::SetScanner(Scanner *scanner = NULL) {
	if (this->scanner != NULL) delete this->scanner;
	this->scanner = scanner;
}

void Parser::SetLocal(const char *variable) {
	locals.Add(variable);
}

void Parser::EmptyLocals() {
	locals.MakeEmpty();
}

