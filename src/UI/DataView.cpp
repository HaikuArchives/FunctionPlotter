#include "UI/DataView.h"
#include "UI/MsgConsts.h"
#include "UI/Graph.h"

BTextControl *NewTextControl(BRect &rect, const char *label, const char *value, int divider = -1) {
BTextControl *ctrl;
	ctrl = new BTextControl(rect,"", label, value, NULL);
	if (divider != -1) ctrl->SetDivider(divider); 
	ctrl->SetAlignment(B_ALIGN_LEFT, B_ALIGN_RIGHT);
	return ctrl;
}

DataView::DataView(BRect rect) : BView(rect, NULL, B_FOLLOW_LEFT | B_FOLLOW_TOP, 0) {
float height;
BRect r;
	// Interval
	rect.Set(0, 0, 80, 10);
	rect.OffsetBy(5, 3);
	AddChild(fCtrls[FROM] = NewTextControl(rect, "t=", "left", 20));
	height = fCtrls[FROM]->Bounds().Height();
	
	rect.OffsetBy(90, 0);
	AddChild(fCtrls[TO]  = NewTextControl(rect, "To:", "right", 20));
	rect.OffsetBy(90, 0);
	AddChild(fCtrls[BY]   = NewTextControl(rect, "By:", "(to-from)/100", 20));

	// Functions x(t), y(t)
	rect.Set(0, 0, 250, height); rect.OffsetTo(5, height+6);
	AddChild(fCtrls[XT]   = NewTextControl(rect, "x(t)=", "t", 30));
	rect.OffsetBy(0, height+3);
	AddChild(fCtrls[YT]   = NewTextControl(rect, "y(t)=", "t^2", 30));
	
	// Visible CheckBox
	rect.OffsetBy(0, height+3); rect.right = 80;
	AddChild(visible = new BCheckBox(rect, "visible", "visible", new BMessage(MSG_UPDATE)));
	visible->SetValue(1);
	
	//	Color Menu;
	rect.OffsetBy(rect.Width()+10, 0); // rect.right += 90;
	BMessage update(MSG_UPDATE);
	BMenu *m = CreateColorMenu("Color", 16, COLOR_TABLE_SIZE, color_table, &update);
	color = new BMenuField(rect, "", "", m, true);
	color->SetDivider(0 /* 40 */);
	rgb_color red = {255, 0, 0, 255}; 
	MarkColorMenuItem(m, red);
	AddChild(color);

	// Style Menu
	rect.OffsetBy(rect.Width()+10, 0);
	m = new BMenu("Style");
	m->SetRadioMode(true);
	BMenuItem *item;
	m->AddItem(item = new BMenuItem("Lines", new BMessage(MSG_UPDATE)));
	item->SetMarked(true);
	m->AddItem(new BMenuItem("Points", new BMessage(MSG_UPDATE)));
	m->AddItem(new BMenuItem("Crosses", new BMessage(MSG_UPDATE)));
	m->AddItem(new BMenuItem("Steps 1", new BMessage(MSG_UPDATE)));
	m->AddItem(new BMenuItem("Steps 2", new BMessage(MSG_UPDATE)));
	style = new BMenuField(rect, "", "", m, true);
	style->SetDivider(0 /* 40 */); 
	AddChild(style);

	// Graph Rectangle
	r.Set(0, 0, 310, height*4); r.OffsetTo(300, 5);
	BBox *box = new BBox(r); box->SetLabel("Graph");
	AddChild(box);
	rect.Set(0, 0, 90, 10); rect.OffsetTo(5, height);
	box->AddChild(gCtrls[LEFT] = NewTextControl(rect, "Left", "-10", 30));
	rect.OffsetBy(100, 0); r.Set(0, 0, 100, 10); r.OffsetTo(rect.left, rect.top);
	box->AddChild(gCtrls[RIGHT] = NewTextControl(r, "Right", "10", 40));
	rect.OffsetBy(110, 0);
	box->AddChild(gCtrls[GRID_X] = NewTextControl(rect, "Grid X", "1", 30));

	rect.OffsetTo(5, height*2+5);
	box->AddChild(gCtrls[TOP] = NewTextControl(rect, "Top", "10", 30));
	rect.OffsetBy(100, 0); r.OffsetTo(rect.left, rect.top);
	box->AddChild(gCtrls[BOTTOM] = NewTextControl(r, "Bottom", "-10", 40));
	rect.OffsetBy(110, 0);
	box->AddChild(gCtrls[GRID_Y] = NewTextControl(rect, "Grid Y", "1", 30));

	// Functions Menu
	rect.Set(0, 0, 250, 20); rect.OffsetBy(5, height * 4 + 6 * 3);
	BPopUpMenu *popUp = new BPopUpMenu("Empty");
	functions = new BMenuField(rect, "", "Functions:", popUp, true);  
	functions->SetDivider(60); 
	AddChild(functions);

	// Buttons
	rect.Set(0, 0, 60, 10);
	BButton *button;
	rect.OffsetBy(260, height * 4 + 6 * 3);
	AddChild(new BButton(rect, NULL, "Add", new BMessage(MSG_ADD)));
	rect.OffsetBy(70, 0);
	AddChild(button = new BButton(rect, NULL, "Replace", new BMessage(MSG_REPLACE)));
	rect.OffsetBy(70, 0);
	AddChild(new BButton(rect, NULL, "Delete", new BMessage(MSG_REMOVE)));
	rect.OffsetBy(70, 0);
	AddChild(button = new BButton(rect, "Update", "Update", new BMessage(MSG_UPDATE)));
}

void DataView::MarkColorItem(const rgb_color &color) {
	MarkColorMenuItem(this->color->Menu(), color);
}
