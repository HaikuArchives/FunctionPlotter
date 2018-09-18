// global  Michael Pfeiffer  29.03.00
#include "Interpreter/Global.h"

static bool remove(void *t) {
	delete ((Node*)t);
	return false;
} 

Global::~Global() {
	globals.DoForEach(remove);
	globals.MakeEmpty();	
}

static Tree *foundTree;

static bool find(void *t, void *n) {
// assert: ((Tree*)t)->IsNode() && ((Node*)t)->kind in {DEFINITION, CONST}
	if (strcmp(((Ident*)((Node*)t)->left)->ident, (const char*)n) == 0) {
		foundTree = (Tree*)t;
		return true;
	} else
		return false;
}

bool Global::Find(const char *name, TreePtr &tree) {
	foundTree = NULL;
	globals.DoForEach(find, (void*)name);
	if (foundTree != NULL) {
		tree = foundTree; return true;
	} else
		return false;
}

void Global::Set(Node *node) {
Tree *old;
	if (Find(((Ident*)node->left)->ident, old)) {
		globals.RemoveItem(old); delete old;
	}
	globals.AddItem(node);
}



