#ifndef CONNECT_APP_H
#define CONNECT_APP_H

#include <AppKit.h>
#include <InterfaceKit.h>
#include <StorageKit.h>
#include "Interpreter/Parser.h"
#include "Interpreter/Interpreter.h"
#include "UI/MsgConsts.h"
#include "UI/Graph.h"
#include "UI/ProgramWindow.h"
#include "UI/DataView.h"
#include "UI/View.h"

//#define WATCH_CLIPBOARD // print clipboard changes to stdout

class AppWindow : public BWindow {
public:
	BMenuBar *menubar;
	BMenu *bg_color, *axes_color;
	View *view; /*BScrollView *scrollView;*/
	DataView *data; 
	BTextView *statusLine;
	BMessage *setup; // page setup 
	BFilePanel *savePanel, *openPanel;
	ProgramWindow *program;

	AppWindow(BRect);

	bool QuitRequested();
	void AboutRequested();	

	void MessageReceived(BMessage *message);

	void ShowErrorPosition(BTextView *text, int32 pos);
	bool CompileFunction(Function1 *f, bool display);
	bool UpdateGraph();
	bool CheckFunction();
	void OnUpdate();

	void AddFunction(Function1 *f);
	void OnAdd();
	void OnReplace();
	void OnRemove();
	void OnSelect();

	void UpdateGraphRect(const BRect r);
	void MoveGraphRect(BPoint p);
	void Zoom(float factor);
	void OnZoom(BMessage *msg);
	void OnMove(BMessage *msg);

	BBitmap *GetGraph(BRect &rect, color_space space);

	void OnCopy();

	void OnSaveGraph();
	void OnSaveGraph(BMessage *msg);
	void OnSave();
	void OnSave(BMessage *msg);
	void OnOpen();
	void OnOpen(BMessage *msg);

	void PageSetup();
	void Print();

	void MarkDefinitionsItem(bool mark);
#ifdef WATCH_CLIPBOARD
	void OnClipboardChanged(BMessage *msg);
#endif
};

class App : public BApplication {
public:
	AppWindow *window;
	App();
};

#define my_app ((App*)be_app)
#endif