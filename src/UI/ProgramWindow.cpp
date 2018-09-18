#include <InterfaceKit.h>
#include "UI/ProgramWindow.h"
#include "UI/MsgConsts.h"

ProgramWindow::ProgramWindow(BWindow *win, BRect aRect) 
	: BWindow(aRect, "Definitions", B_TITLED_WINDOW, 0) {
	parent = win;
	BRect r = Bounds();
	SetSizeLimits(200, 2048, 200, 2048);
	hide = true; // hide window instead of closing it

	r.right -= B_V_SCROLL_BAR_WIDTH + 1;
	r.bottom -= /*B_H_SCROLL_BAR_HEIGHT +*/ 35;
	view = new BTextView(r, "", r, B_FOLLOW_ALL_SIDES, 
		B_WILL_DRAW | B_NAVIGABLE);
	AddChild(new BScrollView("", view, B_FOLLOW_ALL_SIDES,
		0, false, true));

	BRect br(0, 0, 90, 10);
	br.OffsetTo(r.right-100, r.bottom /*+ B_H_SCROLL_BAR_HEIGHT*/ + 7);

	BButton *button;
	AddChild(button = new BButton(br, "", "Check Syntax", 
			new BMessage(MSG_SYNTAX_CHECK), 
			B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM));
}

void ProgramWindow::MessageReceived(BMessage *message) {
	switch(message->what) {
	case MSG_SYNTAX_CHECK:
		parent->PostMessage(MSG_COMPILE_PROGRAM);
		break;
	default:
		BWindow::MessageReceived(message);
	}
}

void ProgramWindow::FrameResized(float w, float h) {
	view->SetTextRect(BRect(0, 0, w - B_V_SCROLL_BAR_WIDTH - 1,
				   h /*- B_H_SCROLL_BAR_HEIGHT*/ - 35)); 
}

void ProgramWindow::SetHide(bool h) {
	hide = h;
}

bool ProgramWindow::QuitRequested() {
	if(hide) {
		Hide(); 
		parent->PostMessage(MSG_HIDE_PROGRAM);
		return false;
	} else {
		parent->PostMessage(MSG_CLOSE_PROGRAM);
		return true;
	}
}

