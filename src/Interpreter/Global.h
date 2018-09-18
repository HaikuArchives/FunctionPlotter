#ifndef GLOBAL
#define GLOBAL

#include <be/support/List.h>
#include "Interpreter/Tree.h"

class Global {
	BList globals;
public:
	~Global();
	bool Find(const char *name, TreePtr &tree);
	void Set(Node *tree);
};
#endif