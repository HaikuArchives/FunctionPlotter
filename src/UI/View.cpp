#include "UI/Application.h"
#include <stdio.h>

View::View(BRect rect) : BView(rect, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE) {
	SetViewColor(B_TRANSPARENT_COLOR);
	mouseDown = false;
	mono = true; 
	deleteGraph = true;
	graph = new Graph(); f1 = new Function1(); ip = new Interpreter();
}

View::View(BRect rect, Interpreter *ip, Graph *graph, Function1 *f1) : BView(rect, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE) {
	SetViewColor(B_TRANSPARENT_COLOR);
	mouseDown = false;
	mono = true; 
	deleteGraph = false;
	this->ip = ip; this->graph = graph; this->f1 = f1;
}

View::~View() {
	if (deleteGraph) {
		if (ip) delete ip;
		if (graph) delete graph;
		if (f1) delete f1;
	}
}

void View::Plot(Function1 *f, float w, float h, BRect r, BRect g) {
double x0, y0, t, from, to, by;
bool old = false;
BPoint p, pOld; 
//Interpreter *inter = &((AppWindow*)Window())->ip;
Tree *xt, *yt;
	if ((f->Exprs()->GetState() < S_EXPR) || !f->IsVisible()) return;

	for (int i = 0; i < 3; i++) 
		if (!f->Get(i)->Evaluate(ip)) return;

	f->GetInterval(from, to, by);
	t = (to - from) / by;
	if (isnan(t) || !isfinite(t) || (t <= 0) || (t > 1000) ) return;

	f->GetFunctions(xt, yt);
	SetHighColor(f->GetColor());
	
#define PLOT_LINE					\
	if (old)						\
		StrokeLine(p);				\
	else {							\
		StrokeLine(p, p);			\
		old = true;					\
	}								\
	pOld = p;

#define PLOT_POINT					\
	StrokeLine(p, p);

#define PLOT_CROSS					\
	StrokeLine(BPoint(p.x-2, p.y), BPoint(p.x+2, p.y)); \
	StrokeLine(BPoint(p.x, p.y-2), BPoint(p.x, p.y+2)); 

#define PLOT_RECT1							\
	if (old) {								\
		StrokeLine(BPoint(p.x, pOld.y));	\
		StrokeLine(p);						\
	} else {								\
		StrokeLine(p, p); 					\
		old = true;							\
	}										\
	pOld = p;	

#define PLOT_RECT2							\
	if (old) {								\
		StrokeLine(BPoint(pOld.x, p.y));	\
		StrokeLine(p);						\
	} else {								\
		StrokeLine(p, p); 					\
		old = true;							\
	}										\
	pOld = p;	

#define PLOT(method)												\
   {																	\
		if (ip->Evaluate(xt, t, x0) && ip->Evaluate(yt, t, y0)) {	\
			p = GraphToView(BPoint(x0, y0), w, h, g);					\
			method														\
		} else old = false;												\
	}

#define PLOTALL(style, method) \
	case style: \
		if (from < to) \
			for (t = from; t <= to; t += by) PLOT(method) \
		else \
			for (t = from; t >= to; t += by) PLOT(method) \
		if (t != to) { t = to; PLOT(method) } \
	break;

	switch(f->GetStyle()) {
		PLOTALL(0, PLOT(PLOT_LINE))
		PLOTALL(1, PLOT_POINT)
		PLOTALL(2, PLOT_CROSS)
		PLOTALL(3, PLOT_RECT1)
		PLOTALL(4, PLOT_RECT2)
	}

}

#define N 10

void Delta(BRect &r, bool xy, float w, float h, float &d, float &x) {
float x0, f;
float sign = 1;
	if (xy) {
		x0 = r.left;
		f = w / fabs(r.right - r.left);
		if (r.left < r.right) sign = -1;
	} else {
		x0 = r.top;
		f = h / fabs(r.bottom - r.top);
		if (r.top < r.bottom) sign = -1;
	}
	x0 -= d * floor(x0 / d);

	d = f * d;
	
	x = sign * x0 * f;	
} 

void View::DrawTicks(float x0, float y0, float w, float h, BRect &rect) {
float d, x, y;
BPoint from(0, y0-3), to(0, y0+3);
	d = graph->GetGrid().x;
	Delta(rect, true, w, h, d, x);
	if (d >= 2.0)
		while (x < w) {
			from.x = to.x = x;
			StrokeLine(from, to);
			x += d;
		}
	
	from.x = x0-3; to.x = x0+3;
	d = /*((AppWindow*)Window())->*/graph->GetGrid().y;
	Delta(rect, false, w, h, d, y);
	if (d >= 2.0)
		while (y < h) {
			from.y = to.y = y;
			StrokeLine(from, to);
			y += d;
		}
} 

#define PH 5
#define PL 10

void View::DrawPointers(float x, float y, float w, float h, BRect &rect) {
BRect b;
	if (x < 0) x = 0;
	else if (x > w) x = w;
	
	if (y < 0) y = 0;
	else if (y > h) y = h;
	
	if (rect.top > rect.bottom) { // up
		b = BRect(x-PH, 0, x+PH, PL);
		FillRect(b, B_SOLID_LOW);
		FillTriangle(BPoint(x, 0), BPoint(x+PH, PL), BPoint(x-PH, PL), b);
	} else { // down
		b = BRect(x-PH, h-PL, x+PH, h);
		FillRect(b, B_SOLID_LOW);
		FillTriangle(BPoint(x, h), BPoint(x+PH, h-PL), BPoint(x-PH, h-PL), b);
	}

	if (rect.left < rect.right) { // right
		b = BRect(w-PL, y-PH, w, y+PH);
		FillRect(b, B_SOLID_LOW);
		FillTriangle(BPoint(w-PL,  y-PH), BPoint(w, y), BPoint(w-PL, y+PH), b);
	} else { // left
		b = BRect(0, y-PH, PL, y+PH);
		FillRect(b, B_SOLID_LOW);
		FillTriangle(BPoint(PL,  y-PH), BPoint(0, y), BPoint(PL, y+PH), b);
	}
}

void View::Draw(BRect updateRect) {
BRect rect, g; 
Function1 *f;
	if (IsPrinting()) {
		BRect b = bounds; 
		rect = updateRect;
		float f1 = b.bottom / b.right, f2 = rect.bottom / rect.right;
		if (f1 < f2) {
			rect.bottom = rect.right * f1;
		} else {
			rect.right = rect.bottom / f1;
		}
		//SetLowColor(255, 255, 255);
		updateRect = rect;
		BRegion r(updateRect);
		ConstrainClippingRegion(&r);
	} else { 
		bounds = rect = Bounds();
		//SetLowColor(192, 192, 192);
	}

	SetLowColor(graph->bg_color);
	FillRect(updateRect, B_SOLID_LOW);
	//AppWindow *w = (AppWindow*)Window();
	//Graph *graph;

	//if ((w != NULL) && (NULL != (graph = w->graph)) && 
	 if		(graph->GetGraphExprs()->GetState() == S_VALUE) {
		float x0, y0;
		g = graph->GetRect();

		if ((g.left == g.right) || (g.bottom == g.top)) return;

/*		if (IsPrinting() && mono) 
			SetHighColor(0, 0, 0);
		else 
			SetHighColor(0, 0, 255);*/
		SetHighColor(graph->axes_color);
		x0 = -g.left / (g.right - g.left) * rect.right;
		y0 = g.top / (g.top - g.bottom) * rect.bottom;
		StrokeLine(BPoint(0, y0), BPoint(rect.right, y0));
		StrokeLine(BPoint(x0, 0), BPoint(x0, rect.bottom));
		DrawTicks(x0, y0, rect.right, rect.bottom, g);
		DrawPointers(x0, y0, rect.right, rect.bottom, g);
		
	 	if (!IsPrinting() || !mono) SetHighColor(255, 0, 0);

		for (int i = 0; (f = graph->GetFunction(i)) != NULL; i++)
			Plot(f, rect.right, rect.bottom, updateRect, g);

		f = /*((AppWindow*)Window())->*/f1;
		Plot(f, rect.right, rect.bottom, updateRect, g);		
	} 
}

inline BPoint View::ViewToGraph(const BPoint &vp, float w, float h, BRect &g) {
BPoint gp;
	gp.x = vp.x / w * (g.right - g.left) + g.left;
	gp.y = vp.y / h * (g.bottom - g.top) + g.top; 
	return gp;
}

inline BPoint View::GraphToView(const BPoint &gp, float w, float h, BRect &g) {
BPoint vp;
	vp.x = (gp.x - g.left) / (g.right - g.left) * w;
	vp.y = (g.top - gp.y) / (g.top - g.bottom) * h;
	return vp;
}


void View::Zoom(float factor) {
BMessage msg(MSG_ZOOM);
	msg.AddFloat("f", factor);
	Window()->PostMessage(&msg);
}

void View::MouseDown(BPoint point) {
	if (modifiers() & B_SHIFT_KEY) Zoom(0.5);
	else if (modifiers() & B_CONTROL_KEY) Zoom(2);
	else {
		mouseStart = point; mouseDown = true; 
	}
}

void View::Position(BPoint point, float &x, float &y) {
BRect r = Bounds(), g = /*((AppWindow*)Window())->*/graph->GetRect();
	x = point.x / r.right * fabs(g.right-g.left) + g.left;
	y = g.top - point.y / r.bottom * fabs(g.top-g.bottom);
}

void View::MouseMoved(BPoint point, uint32 transit, const BMessage *message) {
char string[80]; float x, y;
	Position(point, x, y);
	sprintf(string, "Function Plotter (%G, %G)", x, y);
	Window()->SetTitle(string);
	if (mouseDown) {
		BPoint p = mouseStart - point;
		if ((p.x == 0) && (p.y == 0)) return;
		mouseStart = point; 
		
		BMessage msg(MSG_MOVE);
		msg.AddPoint("Diff", p);
		Window()->PostMessage(&msg); 
	}
}

void View::MouseUp(BPoint point) {
	if (mouseDown) {
		mouseDown = false;
		BPoint p = mouseStart - point;
		if ((p.x == 0) && (p.y == 0)) return;

		BMessage msg(MSG_MOVE);
		msg.AddPoint("Diff", p);
		Window()->PostMessage(&msg); 
	}
}
