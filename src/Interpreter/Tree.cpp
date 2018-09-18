/* syntax tree  Michael Pfeiffer  15.04.00 */

#include <string.h>
#include <iostream.h>
#include "Interpreter/Tree.h"
#include <ctype.h>
 
// Node
Node::Node(int16 kind, Tree *left, Tree *right) {
	this->kind = kind; this->left = left; this->right = right;
}

Node::~Node() {
	if (left != NULL) {
		delete left; left = NULL;
	}
	
	if (right != NULL) {
		delete right; right = NULL;
	}
}

inline Tree *Clone(Tree *node) {
	if (node != NULL) 
		return node->Clone();
	else
		return NULL;
}

Tree *Node::Clone() {
Tree *l, *r, *cur;
Node *list;
	if (left != NULL)
		l = left->Clone();
	else
		l = NULL;
		
	if (right != NULL) { 
		// to avoid deep recursion depth, iterate through right pointer chain 
		// r = right->Clone(); replaced by:
		if (right->IsNode()) {
			r = list = new Node(((Node*)right)->kind, 
				::Clone(((Node*)right)->left), NULL);
				
			cur = ((Node*)right)->right;

			while ((cur != NULL) && cur->IsNode()) {
				list->right = new Node(((Node*)cur)->kind, 
										::Clone(((Node*)cur)->left), NULL);

				list = (Node*)list -> right;
				cur = ((Node*)cur) -> right;
			}

			if (cur != NULL) list->right = cur->Clone();
		} else {
			r = right->Clone();
		}
	} else
		r = NULL;
	
	return new Node(kind, l, r);
}

void Node::Print() {
	if (isprint(kind)) 
		cout << "(" << (char)kind  << ": ";
	else
		switch (kind) {
		case LIST:
			cout << "(LIST: ";
			break;
		case CALL:
			cout << "(CALL: ";
			break;
		case SIGN:
			cout << "(SIGN: ";
			break;
		case CONST:
			cout << "(CONST: ";
			break;
		case DEFINITION:
			cout << "(DEFINTION: ";
			break;
		default:
			cout << "(" << kind  << ": ";
		}
	if (left != NULL)
		left->Print();
	else
		cout << "NULL";

	cout << ", ";

	if (right != NULL)
		right->Print();
	else
		cout << "NULL";
	cout << ")" << "\n";	
}

bool Node::IsLeaf() {
	return false;
}

bool Node::IsNode() {
	return true;
}

Tree * Node::LeftNode() {
	return left;
}

Tree * Node::RightNode() {
	return right;
}

// Leaf
bool Leaf::IsLeaf() {
	return true;
}

bool Leaf::IsNode() {
	return false;
}

Tree * Leaf::LeftNode() {
	return NULL;
}

Tree * Leaf::RightNode() {
	return NULL;
}

// Ident
Ident::Ident(const char *ident) {
char *s;
	s = new char[strlen(ident)+1];
	strcpy(s, ident);
	this->ident = s;
}

Ident::~Ident() {
	if (ident != NULL) {
		delete ident; ident = NULL;
	}
}


Tree *Ident::Clone() {
	return new Ident(ident);
}

void Ident::Print() {
	if (ident != NULL)
		cout << ident;
	else
		cout << "NULL";
}

int Ident::Kind() {
	return IDENT;
}

// Number
Number::Number(double &n) {
	number = n;
}

Tree *Number::Clone() {
	return new Number(number);
}

void Number::Print() {
	cout << number;
}

int Number::Kind() {
	return NUMBER;
}

// Local
Local::Local(int i) {
	index = i;
}

int Local::Kind() {
	return LOCAL;
}

Tree * Local::Clone() {
	return new Local(index);
}

void Local::Print() {
	cout << "Local " << index;
}

// Function
int Function::Kind() {
	return FUNCTION;
}

Function::~Function() {
}

// global functions
Node *Const(const char *name, double value) {
	return new Node(CONST, new Ident(name), new Number(value));
}
