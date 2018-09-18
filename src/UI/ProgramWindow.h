#ifndef PROGWIN
#define PROGWIN
#include <Window.h>
#include <TextView.h>

class ProgramWindow : public BWindow {
	BWindow *parent;
	bool hide;
public:
	BTextView *view;
	ProgramWindow(BWindow *win, BRect rect);
	void MessageReceived(BMessage *message);
	void FrameResized(float w, float h);
	bool QuitRequested();
	void SetHide(bool h);
};

#endif