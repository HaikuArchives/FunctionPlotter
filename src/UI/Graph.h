#ifndef GRAPH
#define GRAPH

#include <SupportKit.h>
#include <GraphicsDefs.h>
#include <Rect.h>
#include <Archivable.h>
#include <Message.h>
#include "Interpreter/Tree.h"
#include "Interpreter/Scanner.h"
#include "Interpreter/Parser.h"
#include "Interpreter/Interpreter.h"

// wrapper for Tree 
enum State {
	S_UNDEF, S_TEXT, S_EXPR, S_VALUE
};

// text -> Parser -> expression -> Interpreter -> value
class _EXPORT Expression : public BArchivable {
	BString name;
	BString text;
	//Expr expression;
	Tree *expression;
	double value;
	State state;
public:
	Expression();
	Expression(Expression &e);
	~Expression();

	State GetState();
	void SetName(const char *name);
	void SetText(const char *text);
	const char *GetText();
	double GetValue();
	Tree *GetTree();
	bool ParseExpr(Parser *parser, StringReader *reader);
	bool ParseDefs(Parser *parser, StringReader *reader);
	bool Evaluate(Interpreter *interpreter);
	Expression *Clone();
	
	Expression(BMessage *archive);
	status_t Archive(BMessage *archive, bool deep = true) const;
	static BArchivable *Instantiate(BMessage *archive);
}; 

typedef Expression *ExpressionPtr;

class _EXPORT ExprList : public BArchivable {
	ExpressionPtr *exprs;
	const int16 numExprs;

	State state;
public:
	ExprList(int n);
	ExprList(ExprList &e);
	~ExprList();

	void SetText(int i, const char *text);
	const char *GetText(int i);
	bool ParseExpr(Parser *parser, StringReader *reader, int &i);
	bool ParseDefs(Parser *parser, StringReader *reader, int &i);
	bool Evaluate(Interpreter *interpreter, int &i);
	State GetState();
	void SetState(State state);
	double GetValue(int i);
	Tree *GetTree(int i);
	ExprList *Clone();
	Expression *Get(int i);
	
	ExprList(BMessage *archive);
	status_t Archive(BMessage *archive, bool deep = true) const;
	static BArchivable *Instantiate(BMessage *archive);
};

// enum Component {FROM, TO, BY, XT, YT};
#define FROM 0
#define TO   1
#define BY   2
#define XT   3
#define YT   4

class _EXPORT Function1 : BArchivable{
	ExprList *exprs;
	bool visible;
	rgb_color color;
	int8 style;
public:
	Function1(Function1 &f);
	Function1();
	~Function1();	
	void GetInterval(double &from, double &to, double &by);
	void GetFunctions(TreePtr &xt, TreePtr &yt);
	Expression *Get(int i);
	ExprList *Exprs();
	void SetVisible(bool visible);
	bool IsVisible();
	void SetColor(const rgb_color &color);
	rgb_color GetColor();
	void SetStyle(int8 style);
	int8 GetStyle();
	Function1 *Clone();

	Function1(BMessage *archive);
	status_t Archive(BMessage *archive, bool deep = true) const;
	static BArchivable *Instantiate(BMessage *archive);
};

// enum Component { DEFINITIONS, LEFT, RIGHT, TOP, BOTTOM, GRID_X, GRID_Y };
#define LEFT        0
#define RIGHT       1
#define TOP         2
#define BOTTOM      3
#define GRID_X      4
#define GRID_Y      5

class _EXPORT Graph : public BArchivable {
	ExprList *exprs;
	Expression *definitions;
	BList functions;
	void FreeFunctions();
public:
	rgb_color bg_color, axes_color;

	Graph();
	~Graph();
	
	Expression *GetDefinitions();
	ExprList *GetGraphExprs();
	Function1 *GetFunction(int i);
	void AppendFunction(Function1 *f);
	void DeleteFunction(int i);
	int CountFunctions();
	BRect GetRect();
	BPoint GetGrid();
	Graph *Clone();


	Graph(BMessage *archive);
	status_t Archive(BMessage *archive, bool deep = true) const;
	static BArchivable *Instantiate(BMessage *archive);
};

#endif
