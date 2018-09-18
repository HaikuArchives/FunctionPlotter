#ifndef VIEW_H
#define VIEW_H

#include "Interpreter/Tree.h"
#include "UI/Graph.h"

class View : public BView {
public:
	bool mono; // print in black and white only
	bool deleteGraph;
	Graph *graph; 
	Function1 *f1;
	Interpreter *ip;		

	BRect bounds;
	BPoint mouseStart;
	bool mouseDown;

	View(BRect rect);
	View(BRect rect, Interpreter *ip, Graph *graph, Function1 *f1);
	~View();
	
	inline BPoint ViewToGraph(const BPoint &p, float w, float h, BRect &g);
	inline BPoint GraphToView(const BPoint &p, float w, float h, BRect &g);

	void Plot(Function1 *f, float w, float h, BRect r, BRect g);
	void DrawTicks(float x0, float y0, float w, float h, BRect &rect);
	void DrawPointers(float x0, float y0, float w, float h, BRect &rect);
	void Draw(BRect updateRect);

	void MouseDown(BPoint point);
	void MouseUp(BPoint point);
	void MouseMoved(BPoint point, uint32 transit, const BMessage *message);
	void Zoom(float factor);
	void Position(BPoint point, float &x, float &y);
};
#endif
