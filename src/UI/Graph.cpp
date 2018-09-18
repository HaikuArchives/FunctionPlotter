#include "UI/Graph.h"
#include "UI/ColorMenuItem.h"
// Expr 
/*
Expr::Expr(Expr &e) {
	if (e.expr != NULL)
		expr = e.expr->Clone();
	else
		expr = NULL;
}
Expr::Expr(Tree *expr = NULL) {
	this->expr = expr;
}

Expr::~Expr() {
	if (expr != NULL) {
		delete expr; expr = NULL;
	}
}

void Expr::Set(Tree *expr) {
	if (this->expr != NULL) delete this->expr;
	this->expr = expr;
}

Tree *Expr::Get() {
	return expr;
}
*/
// Expression
Expression::Expression() {
	expression = NULL; value = 0; state = S_UNDEF;
}

Expression::Expression(Expression &e) : 
	name(e.name), text(e.text), value(e.value), state(e.state)  {
	if (e.expression != NULL)
		expression = e.expression->Clone();
	else
		expression = NULL;
}

Expression::~Expression() {
	if (expression != NULL) {
		delete expression; expression = NULL;
	}
}

Expression *Expression::Clone() {
Expression *e = new Expression(*this);
	return e;
}

State Expression::GetState() {
	return state;
}

void Expression::SetName(const char *name) {
	this->name.SetTo(name);
}

void Expression::SetText(const char *text) {
	this->text.SetTo(text);
	state = S_TEXT;
}

const char * Expression::GetText() {
	return text.String();
}


bool Expression::ParseExpr(Parser *parser, StringReader *reader) {
Tree *expr;
	reader->SetString(GetText());
	bool ok = parser->ParseExpr(expr);
	if (expression != NULL) delete expression;
	if (ok) {
		expression = expr; state = S_EXPR;
	} else {
		expression = NULL; state = S_TEXT;
	}
	return ok;
}

bool Expression::ParseDefs(Parser *parser, StringReader *reader) {
Tree *expr;
	reader->SetString(GetText());
	bool ok = parser->ParseDefs(expr);
	if (expression != NULL) delete expression;
	if (ok) {
		expression = expr; state = S_EXPR;
	} else {
		expression = NULL; state = S_TEXT;
	}
	return ok;
}

bool Expression::Evaluate(Interpreter *interpreter) {
	if (state >= S_EXPR) {
		if (interpreter->Evaluate(expression, value)) {
			state = S_VALUE;
			if (name.Length() > 0) 
				interpreter->globals->Set(Const(name.String(), value));
			return true;
		}
		state = S_EXPR;
	} 
	return false;
}

double Expression::GetValue() {
	return value;
}

Tree * Expression::GetTree() {
	return expression;
}

status_t Expression::Archive(BMessage *archive, bool deep = true) const {
	archive->AddString("class", "Expression");
	archive->AddString("name", name);
	archive->AddString("text", text);
	return B_OK;
}

BArchivable *Expression::Instantiate(BMessage *archive) {
	if (validate_instantiation(archive, "Expression")) 
		return new Expression(archive);
	else
		return NULL;
}

Expression::Expression(BMessage *archive) {
	archive->FindString("name", &name);
	archive->FindString("text", &text);
	expression = NULL;
	state = S_TEXT;
}

// ExprList
ExprList::ExprList(ExprList &e) : numExprs(e.numExprs), state(e.state) {
	exprs = new ExpressionPtr[e.numExprs];
	for (int i = 0; i < numExprs; i++) exprs[i] = e.exprs[i]->Clone();
}

ExprList::~ExprList() {
	if (exprs != NULL) {
		for (int i = 0; i < numExprs; i++) delete exprs[i];
		delete [] exprs; exprs = NULL;
	}
}

ExprList * ExprList::Clone() {
ExprList *l = new ExprList(*this);
	return l;
}

ExprList::ExprList(int n) : numExprs(n), state(S_UNDEF) {
	exprs = new ExpressionPtr[n];
	for (int i = 0; i < numExprs; i++)
		exprs[i] = new Expression();
}

void ExprList::SetText(int component, const char *text) {
	state = S_TEXT;
	if ((component >= 0) && (component < numExprs))
		exprs[component]->SetText(text);
}

const char * ExprList::GetText(int component) {
	if ((component >= 0) && (component <= numExprs))
		return exprs[component]->GetText();
	else
		return NULL;
}

bool ExprList::ParseExpr(Parser *p, StringReader *r, int &component) {
	for (int i = 0; i < numExprs; i++) {
		if (!exprs[i]->ParseExpr(p, r)) {
			component = i; state = S_TEXT; return false;
		}
	}
	state = S_EXPR;
	return true;
}

bool ExprList::ParseDefs(Parser *p, StringReader *r, int &component) {
	for (int i = 0; i < numExprs; i++) {
		if (!exprs[i]->ParseDefs(p, r)) {
			component = i; state = S_TEXT; return false;
		}
	}
	state = S_EXPR;
	return true;
}

bool ExprList::Evaluate(Interpreter *interpreter, int &component) {
	for (int i = 0; i < numExprs; i++) {
		if (!exprs[i]->Evaluate(interpreter)) {
			component = i; state = S_EXPR; return false;
		}
	}
	state = S_VALUE;
	return true;	
}

State ExprList::GetState() {
	return state;
}

void ExprList::SetState(State state) {
	this->state = state;
}

Expression *ExprList::Get(int i) {
	if ((i >= 0) && (i < numExprs)) 
		return exprs[i];
	else
		return 0;
}

double ExprList::GetValue(int i) {
	if ((i >= 0) && (i < numExprs)) 
		return exprs[i]->GetValue();
	else
		return 0;
}

Tree * ExprList::GetTree(int i) {
	if ((i >= 0) && (i < numExprs)) 
		return exprs[i]->GetTree();
	else
		return NULL;
}

status_t ExprList::Archive(BMessage *archive, bool deep = true) const {
	archive->AddString("class", "ExprList");
	archive->AddInt16("numExprs", numExprs);
	for (int i = 0; i < numExprs; i++) {
		BMessage msg;
		exprs[i]->Archive(&msg, deep);
		archive->AddMessage("exprs", &msg);
	}
	return B_OK;
}

BArchivable *ExprList::Instantiate(BMessage *archive) {
	if (validate_instantiation(archive, "ExprList")) 
		return new ExprList(archive);
	else
		return NULL;
}

ExprList::ExprList(BMessage *archive) {
	int16 n;
	archive->FindInt16("numExprs", &n);
	((int16&)numExprs) = n;
	exprs = new ExpressionPtr[numExprs];
	for (int i = 0; i < numExprs; i++) {
		BMessage msg;
		archive->FindMessage("exprs", i, &msg);
		exprs[i] = (Expression*)Expression::Instantiate(&msg);
	}
}

// Function
Function1::Function1() {
	exprs = new ExprList(5); 
	exprs->Get(FROM)->SetName("from"); 
	exprs->Get(TO)->SetName("to"); 
	exprs->Get(BY)->SetName("by"); 
	exprs->Get(XT)->SetName("xt"); 
	exprs->Get(YT)->SetName("yt"); 
	visible = true;
	color.red = 255; color.green = color.blue = color.alpha = 0;
	style = 0;
};

Function1::Function1(Function1 &f) {
	exprs = f.exprs->Clone();
	visible = f.visible;
	color = f.color;
	style = f.style;
}

Function1::~Function1() {
	delete exprs;
}

Function1 * Function1::Clone() {
Function1 *f = new Function1(*this);
	return f;
}

void Function1::GetInterval(double &from, double &to, double &by) {
	from = exprs->GetValue(FROM);
	to = exprs->GetValue(TO);
	by = exprs->GetValue(BY);
}

void Function1::GetFunctions(TreePtr &xt, TreePtr &yt) {
	xt = exprs->GetTree(XT); yt = exprs->GetTree(YT);
}

ExprList *Function1::Exprs() {
	return exprs;
}

Expression *Function1::Get(int i) {
	return exprs->Get(i);
}

void Function1::SetVisible(bool visible) {
	this->visible = visible;
}

bool Function1::IsVisible() {
	return visible;
}

void Function1::SetColor(const rgb_color &color) {
	this->color = color;
}

rgb_color Function1::GetColor() {
	return color;
}

void Function1::SetStyle(int8 style) {
	this->style = style;
}

int8 Function1::GetStyle() {
	return style;
}

status_t Function1::Archive(BMessage *archive, bool deep = true) const {
	archive->AddString("class", "Function1");
	archive->AddBool("visible", visible);
	archive->AddInt8("red", color.red);
	archive->AddInt8("green", color.green);
	archive->AddInt8("blue", color.blue);
	archive->AddInt8("alpha", color.alpha);
	archive->AddInt8("style", style);
		
	BMessage msg;
	exprs->Archive(&msg, deep);
	archive->AddMessage("exprs", &msg);

	return B_OK;
}

BArchivable *Function1::Instantiate(BMessage *archive) {
	if (validate_instantiation(archive, "Function1")) 
		return new Function1(archive);
	else
		return NULL;
}

Function1::Function1(BMessage *archive) {
	archive->FindBool("visible", &visible);
	archive->FindInt8("red", (int8*)&color.red);
	archive->FindInt8("green", (int8*)&color.green);
	archive->FindInt8("blue", (int8*)&color.blue);
	archive->FindInt8("alpha", (int8*)&color.alpha);
	archive->FindInt8("style", &style);
		
	BMessage msg;
	archive->FindMessage("exprs", &msg);
	exprs = (ExprList*)ExprList::Instantiate(&msg);
}

// Graph
Graph::Graph() {
	exprs = new ExprList(6);
	definitions = new Expression();
	exprs->Get(LEFT)->SetName("left");
	exprs->Get(RIGHT)->SetName("right");
	exprs->Get(TOP)->SetName("top");
	exprs->Get(BOTTOM)->SetName("bottom");
	exprs->Get(GRID_X)->SetName("gridX");
	exprs->Get(GRID_Y)->SetName("gridY");
	rgb_color white = {255, 255, 255, 255}, blue = {0, 0, 255, 255};
	bg_color = white;
	axes_color = blue;
};

Graph::~Graph() {
	if (exprs) { delete exprs; exprs = NULL; }
	if (definitions) { delete definitions; definitions = NULL; }
	FreeFunctions();
}

static bool freeFunction(void *f) {
	if (f) delete (Function1*)f;
	return false;
}

void Graph::FreeFunctions() {
	functions.DoForEach(freeFunction);
}

Function1 * Graph::GetFunction(int i) {
	return (Function1*)functions.ItemAt(i);
}

int Graph::CountFunctions() {
	return functions.CountItems();
}

void Graph::AppendFunction(Function1 *f) {
	functions.AddItem(f);
}

void Graph::DeleteFunction(int i) {
Function1 *f = (Function1*)functions.RemoveItem(i);
	if (f != NULL) delete f; 
}

Expression * Graph::GetDefinitions() {
	return definitions;
}

ExprList * Graph::GetGraphExprs() {
	return exprs;
}

BRect Graph::GetRect() {
	return BRect(exprs->GetValue(LEFT), exprs->GetValue(TOP),
		exprs->GetValue(RIGHT), exprs->GetValue(BOTTOM));
}

BPoint Graph::GetGrid() {
	return BPoint(exprs->GetValue(GRID_X), exprs->GetValue(GRID_Y));
}


status_t Graph::Archive(BMessage *archive, bool deep = true) const {
	archive->AddString("class", "Graph");
	BMessage msg;
	exprs->Archive(&msg, deep);
	archive->AddMessage("exprs", &msg);
	msg.MakeEmpty();
	
	definitions->Archive(&msg, deep);
	archive->AddMessage("definitions", &msg);
	msg.MakeEmpty();

	ArchiveColor("bg_color", archive, bg_color);
	ArchiveColor("axes_color", archive, axes_color);
	
	archive->AddInt32("numFunctions", functions.CountItems());
	Function1 *f;
	for (int i = 0; NULL != (f = (Function1*)functions.ItemAt(i)); i++) {
		f->Archive(&msg, deep);
		archive->AddMessage("functions", &msg);
		msg.MakeEmpty();
	}
	return B_OK;
}

BArchivable *Graph::Instantiate(BMessage *archive) {
	if (validate_instantiation(archive, "Graph")) {
		return new Graph(archive);
	} else
		return NULL;
}

Graph::Graph(BMessage *archive) {
	BMessage msg;
	archive->FindMessage("exprs", &msg);
	exprs = (ExprList*)ExprList::Instantiate(&msg);
	msg.MakeEmpty();
	
	archive->FindMessage("definitions", &msg);
	definitions = (Expression*)Expression::Instantiate(&msg);
	msg.MakeEmpty();
	
	UnarchiveColor("bg_color", archive, bg_color);
	UnarchiveColor("axes_color", archive, axes_color);
	
	int32 n;
	archive->FindInt32("numFunctions", &n);
	for (int i = 0; i < n; i++) {
		archive->FindMessage("functions", i, &msg);
		functions.AddItem(Function1::Instantiate(&msg));
		msg.MakeEmpty();
	}
}