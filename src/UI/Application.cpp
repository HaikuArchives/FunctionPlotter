#include "UI/Application.h"
#include <SupportKit.h>
#include <StorageKit.h>
#include <TranslationKit.h>
#include <Roster.h>
#include <string.h>
#include <stdio.h>

#define APPLICATION "Function Plotter"
#define VERSION "0.9.2"

AppWindow::AppWindow(BRect aRect) 
	: BWindow(aRect, APPLICATION, B_TITLED_WINDOW, 0) {
	setup = NULL;
	SetSizeLimits(645, 2048, 480, 2048);
	// add menu bar
	BRect rect = BRect(0, 0, aRect.Width(), aRect.Height());
	menubar = new BMenuBar(rect, "menu_bar");
	BMenu *menu; // , *popUp; 
	BMenuItem *item;

	menu = new BMenu("File");
	//menu->AddItem(new BMenuItem("New", new BMessage(MENU_APP_NEW), 'N'));
	//menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Open ...", new BMessage(MENU_APP_OPEN), 'O'));
	menu->AddItem(new BMenuItem("Save ...", new BMessage(MENU_APP_SAVE), 'S'));
	menu->AddItem(new BMenuItem("Save Graph ...", new BMessage(MENU_APP_SAVE_GRAPH), 'G'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Page Setup ...", new BMessage(MENU_APP_PAGE_SETUP), 'P', B_SHIFT_KEY));
	menu->AddItem(new BMenuItem("Print ...", new BMessage(MENU_APP_PRINT), 'P'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("About ...", new BMessage(B_ABOUT_REQUESTED)));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q')); 
	menubar->AddItem(menu);

	menu = new BMenu("Edit");
	menu->AddItem(new BMenuItem("Copy", new BMessage(MENU_APP_COPY), 'C'));
	menubar->AddItem(menu);


	menu = new BMenu("Graph");
	BMessage update(MSG_UPDATE);
	menu->AddItem(bg_color = CreateColorMenu("Background Color", 16, COLOR_TABLE_SIZE, color_table, &update));
	menu->AddItem(axes_color = CreateColorMenu("Axes Color", 16, COLOR_TABLE_SIZE, color_table, &update));
	rgb_color blue = {0, 0, 255, 255}, white = {255, 255, 255, 255};
	MarkColorMenuItem(bg_color, white);
	MarkColorMenuItem(axes_color, blue);
//	menu->AddItem(new BMenuItem("ratio", NULL));
	menubar->AddItem(menu);

	menu = new BMenu("Window");
	menu->AddItem(item = new BMenuItem("Defintions", new BMessage(MENU_APP_PROGRAM)));
	item->SetMarked(true);
	menubar->AddItem(menu);
	
	AddChild(menubar);

	// controls
	rect.Set(0, menubar->Bounds().Height()+1, aRect.Width(), 150);
	data = new DataView(rect);
	AddChild(data);
	SetDefaultButton((BButton*)data->FindView("Update"));	

	BRect textRect(0, 0, aRect.Width(), 15);
	BRect r(textRect);
	r.OffsetTo(0, aRect.Height()-textRect.Height());
	statusLine = new BTextView(r, "statusLine", textRect, 
							   B_FOLLOW_RIGHT | B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, B_WILL_DRAW);
	AddChild(statusLine);

	// add graph view
	BRect viewRect(0, data->ConvertToParent(data->Bounds()).bottom+1, aRect.Width(), 
		statusLine->ConvertToParent(statusLine->Bounds()).top-1);
	/*BRect viewRect(scrollRect.left, scrollRect.top, 
					scrollRect.right-B_V_SCROLL_BAR_WIDTH-1,
					scrollRect.bottom-B_H_SCROLL_BAR_HEIGHT-1);*/
	view = new View(viewRect);
	AddChild(view);
	/*scrollView = new BScrollView("", view, B_FOLLOW_ALL_SIDES, 0, true, true);
	AddChild(scrollView);*/
	
#ifdef WATCH_CLIPBOARD
	be_clipboard->StartWatching(this);
#endif

	// Shortcuts
	AddShortcut('f', 0, new BMessage(MSG_ADD));
	AddShortcut('d', 0, new BMessage(MSG_REMOVE));
	AddShortcut('r', 0, new BMessage(MSG_REPLACE));
	
	// Open and Save Panel
	savePanel = new BFilePanel(B_SAVE_PANEL, new BMessenger(this));
	openPanel = new BFilePanel(B_OPEN_PANEL, new BMessenger(this));

	app_info ai;
	if (B_OK == be_app->GetAppInfo(&ai)) {
		BDirectory dir; BEntry entry(&ai.ref); 
		entry.GetParent(&dir);
		savePanel->SetPanelDirectory(&dir);
		openPanel->SetPanelDirectory(&dir);
	}
	BMessage msg(MSG_OPEN);
	openPanel->SetMessage(&msg);

	// Definitions Window
	program = new ProgramWindow(this, BRect(40, 40, 400, 400));
	program->Show();
	
	// make window visible
	Show();
	PostMessage(&update);
}

// Read data from view, parse and compile it
void AppWindow::ShowErrorPosition(BTextView *text, int32 pos) {
	beep();
	if (text->Window()->Lock()) {
		text->Select(pos, pos);
		text->MakeFocus();
		text->Window()->Unlock();
	}
}

// Read defintion and graph data, parse and evaluate it
bool AppWindow::UpdateGraph() {
int i;
StringReader *r;
Parser p(new Scanner(r = new StringReader()));
Expression *defs = view->graph->GetDefinitions();
	defs->SetText(program->view->Text());

	ColorMenuItem *c = (ColorMenuItem*)bg_color->FindMarked();
	if (c != NULL) view->graph->bg_color = c->GetColor();
	c = (ColorMenuItem*)axes_color->FindMarked();
	if (c != NULL) view->graph->axes_color = c->GetColor();

	if (!defs->ParseDefs(&p, r)) {
		statusLine->SetText("Syntax error in definitions");
		ShowErrorPosition(program->view, p.GetPos());
		return false;
	}
	
	view->ip->Reset();
	if (!defs->Evaluate(view->ip)) {
		statusLine->SetText("Error interpreting definitions");
		ShowErrorPosition(program->view, 0);
		return false;
	}

	ExprList *g = view->graph->GetGraphExprs();	
	for (i = 0; i <= 5; i++) {
		g->SetText(i, data->gCtrls[i]->Text());
	}

	if (!g->ParseExpr(&p, r, i)) {
		statusLine->SetText("Error parsing expression");
		ShowErrorPosition(data->gCtrls[i]->TextView(), p.GetPos());
		return false;
	}

	if (!g->Evaluate(view->ip, i)) {
		statusLine->SetText("Error evaluating expression");
		ShowErrorPosition(data->gCtrls[i]->TextView(), 0);
		return false;
	}
	return true;
}

// Parse and evaluate the expression in Functin1 object
bool AppWindow::CompileFunction(Function1 *f, bool display) {
StringReader *r;
Parser p(new Scanner(r = new StringReader()));
int i;
	for (i = 0; i < 3; i++) {
		if (!f->Get(i)->ParseExpr(&p, r)) {
			if (display) {
				statusLine->SetText("Syntax error in expression");
				ShowErrorPosition(data->fCtrls[i]->TextView(), p.GetPos());
			}
			return false;
		}
	}

	p.SetLocal("t");
	for (i = 3; i < 5; i++) {
		if (!f->Get(i)->ParseExpr(&p, r)) {
			if (display) {
				statusLine->SetText("Syntax error in expression");
				ShowErrorPosition(data->fCtrls[i]->TextView(), p.GetPos());
			}
			return false;
		}
	}
	p.EmptyLocals();
	f->Exprs()->SetState(S_EXPR);
	
	for (i = 0; i < 3; i++) {
		if (!f->Get(i)->Evaluate(view->ip)) {
			if (display) {
				statusLine->SetText("Error evaluating expression");
				ShowErrorPosition(data->fCtrls[i]->TextView(), 0);
			}
			return false;
		}	
	}
	f->Exprs()->SetState(S_VALUE);
	return true;
}

// Read all data, parse und evaluate it
bool AppWindow::CheckFunction() {
int i;
	UpdateGraph(); 
	for (i = 0; i < 5; i++) {
		view->f1->Exprs()->SetText(i, data->fCtrls[i]->Text());
	}
	view->f1->SetVisible(data->visible->Value());

	ColorMenuItem *c = (ColorMenuItem*)data->color->Menu()->FindMarked();
	if (c != NULL) view->f1->SetColor(c->GetColor());

	BMenuItem *item = data->style->Menu()->FindMarked();
	if (item != NULL) view->f1->SetStyle(data->style->Menu()->IndexOf(item));

	if (CompileFunction(view->f1, true)) {
		statusLine->SetText("ok");
		return true;
	} else
		return false;
}

void AppWindow::OnUpdate() {
	CheckFunction();
	view->Invalidate();
}

// Add a string representation of function f to the functions MenuField
void AppWindow::AddFunction(Function1 *f) {
	BMenu *m = data->functions->Menu();
	const char *xt, *yt;
	xt = f->Exprs()->GetText(XT); yt = f->Exprs()->GetText(YT);
	BString t("(");
	t << xt << ", " << yt << ")";
	BMenuItem *i = new BMenuItem(t.String(), new BMessage(MSG_SELECTED));
	m->AddItem(i);
	i->SetMarked(true);
}

// Add function to the functions MenuField
void AppWindow::OnAdd() {
	if (CheckFunction()) {
		AddFunction(view->f1);
		view->graph->AppendFunction(view->f1->Clone()); 
	}
	view->Invalidate();
}

// Replace the currently marked function
void AppWindow::OnReplace() {
	if (CheckFunction()) {
		BMenuItem *m = data->functions->Menu()->FindMarked();
		if (m != NULL) {
		int32 i = data->functions->Menu()->IndexOf(m);
			view->graph->DeleteFunction(i);
			data->functions->Menu()->RemoveItem(m);
			delete m;
		}
		AddFunction(view->f1);
		view->graph->AppendFunction(view->f1->Clone()); 
	}
	view->Invalidate();
}

// Delete currently marked function
void AppWindow::OnRemove() {
BMenuItem *m = data->functions->Menu()->FindMarked();
	if (m != NULL) {
		int32 i = data->functions->Menu()->IndexOf(m);
		view->graph->DeleteFunction(i);
		data->functions->Menu()->RemoveItem(m);
		delete m;
		BMenuItem *item = data->functions->Menu()->ItemAt(0);
		if (item != NULL) 
			item->SetMarked(true);
		else
			data->functions->MenuItem()->SetLabel("<empty>");
		OnSelect();
		view->Invalidate();
	}
}

// Display data of the selected function 
void AppWindow::OnSelect() {
BMenuItem *m = data->functions->Menu()->FindMarked();
	if (m != NULL) {
		int32 i = data->functions->Menu()->IndexOf(m);
		Function1 *f = view->graph->GetFunction(i);
		if (f != NULL) {
			delete view->f1; view->f1 = f->Clone();
			for (i = 0; i < 5; i++) data->fCtrls[i]->SetText(f->Exprs()->GetText(i));
		}
		
		data->visible->SetValue(f->IsVisible());
		data->MarkColorItem(f->GetColor());
		BMenuItem *item = data->style->Menu()->ItemAt(f->GetStyle());
		if (item != NULL) item->SetMarked(true);
	}
}

// Write graph rectangle to display
void AppWindow::UpdateGraphRect(const BRect graphRect) {
char string[255];
	sprintf(string, "%g", graphRect.top);
	data->gCtrls[TOP]->SetText(string);	
	sprintf(string, "%g", graphRect.bottom);
	data->gCtrls[BOTTOM]->SetText(string);	
	sprintf(string, "%g", graphRect.left);
	data->gCtrls[LEFT]->SetText(string);	
	sprintf(string, "%g", graphRect.right);
	data->gCtrls[RIGHT]->SetText(string);	

	UpdateGraph();
}

// Move relative graph use point p as a vector
void AppWindow::MoveGraphRect(BPoint p) {
float delta;
BRect rect = view->Bounds(),
	graphRect(view->graph->GetRect());
	if ((p.x == 0) && (p.y == 0)) return;
	delta = (graphRect.right-graphRect.left) * p.x / (rect.right-rect.left);
	graphRect.right += delta; graphRect.left += delta;

	delta = (graphRect.bottom-graphRect.top) * p.y / (rect.bottom-rect.top);
	graphRect.top += delta; graphRect.bottom += delta;

	UpdateGraphRect(graphRect);

	view->Invalidate();
}

// Zoom in or out
void AppWindow::Zoom(float factor) {
double m, d; 
BRect graphRect(view->graph->GetRect());
	d = factor * (graphRect.bottom - graphRect.top) / 2.0;
	m = (graphRect.bottom + graphRect.top) / 2.0;
	graphRect.bottom = m + d; graphRect.top = m - d;

	d = factor * (graphRect.right - graphRect.left) / 2.0;
	m = (graphRect.right + graphRect.left) / 2.0;
	graphRect.left = m - d; graphRect.right = m + d;

	UpdateGraphRect(graphRect);
	
	view->Invalidate();
}

// Page Setup
void AppWindow::PageSetup() {
status_t result;
	BPrintJob job("Function Plotter");
	
	if (setup) job.SetSettings(new BMessage(*setup));
	
	result = job.ConfigPage();
	
	if (result == B_OK) {
		if (setup) delete setup;
		setup = job.Settings();
	}
}

// Output graph to printer
void AppWindow::Print() {
status_t result;
	BPrintJob job("Function Plotter");
	
	if (setup) job.SetSettings(new BMessage(*setup));
	
	result = job.ConfigJob();
	
	if (result == B_OK) {
		if (setup) delete setup;
		setup = job.Settings();
		
		//paper_rect = job.PaperRect();
		BRect printable_rect = job.PrintableRect();
		
		job.BeginJob();
		if (job.FirstPage() == 1) {
			job.DrawView(view, printable_rect, BPoint(0,0)); 
			job.SpoolPage();
		}
		if (job.CanContinue()) job.CommitJob();
	}
}

// Create a bitmap of the graph
BBitmap * AppWindow::GetGraph(BRect &rect, color_space space) {
rect = view->Bounds();
BBitmap *bitmap = new BBitmap(rect, space, true);
View thisView(rect, view->ip, view->graph, view->f1);
	if (bitmap->Lock()) {
		bitmap->AddChild(&thisView);
		thisView.Draw(rect);
		thisView.Sync();
		bitmap->RemoveChild(&thisView);
		bitmap->Unlock();
		return bitmap;
	} else {
		delete bitmap;
		return NULL;
	}
}

// Copy graph as bitmap to clipboard
void AppWindow::OnCopy() {
BRect rect;
BBitmap *bitmap = GetGraph(rect, B_RGB32);
	if (bitmap != NULL) {
		if (be_clipboard->Lock()) {
			be_clipboard->Clear();
			BMessage *clip, data;
			bitmap->Archive(&data);
			if (NULL != (clip = be_clipboard->Data())) {
				clip->AddMessage("image/x-vnd.Be-bitmap", &data);
				clip->AddRect("rect", rect);
				be_clipboard->Commit();
			}
			be_clipboard->Unlock();
		}
		delete bitmap;
	}
}

// Open Save Panel
void AppWindow::OnSave() {
	BMessage msg(MSG_SAVE);
	savePanel->SetMessage(&msg);
	savePanel->Show();
}

// Save data
void AppWindow::OnSave(BMessage *msg) {
entry_ref ref;
BString name;
	msg->FindRef("directory", &ref);
	msg->FindString("name", &name);
	BDirectory dir(&ref);
	BFile file(&dir, name.String(), B_CREATE_FILE | B_WRITE_ONLY);
	if (file.InitCheck() == B_OK) {
		BMessage archive;
		view->graph->Archive(&archive);
		archive.Flatten(&file);
	} else {
		BAlert *a = new BAlert("Save Error", "Could not write file", "OK");
		a->Go();
	}
}

// Display Open Panel
void AppWindow::OnOpen() {
	openPanel->Show();
}

// Read data
void AppWindow::OnOpen(BMessage *msg) {
entry_ref ref;
BMessage archive;
	msg->FindRef("refs", &ref);
	BFile file(&ref, B_READ_ONLY);
	archive.Unflatten(&file);
	Graph *g = (Graph*)Graph::Instantiate(&archive);
	if (g != NULL) {
		delete view->graph; view->graph = g;
		BMenu *menu = data->functions->Menu();
		while (menu->RemoveItem((int32)0) != NULL);
		Function1 *f;
		int i;

		if (program->Lock()) {
			program->view->SetText(g->GetDefinitions()->GetText());
			program->Unlock();
		}
		
		MarkColorMenuItem(bg_color, view->graph->bg_color);
		MarkColorMenuItem(axes_color, view->graph->axes_color);
		
		ExprList *el = g->GetGraphExprs();
		for (i = 0; i < 6; i++) 
			data->gCtrls[i]->SetText(el->GetText(i));
		
		for (i = 0; i < 5; i++)
			data->fCtrls[i]->SetText("");

		UpdateGraph();
		for (i = 0; NULL != (f = g->GetFunction(i)); i++) {
			CompileFunction(f, false);
		}
		
		for (i = 0; NULL != (f = g->GetFunction(i)); i++)
			AddFunction(f);
		OnSelect();
		
		OnUpdate();	
	} else {
		BAlert *a = new BAlert("Open Error", "Could not read file.", "OK");
		a->Go();
	}
}

// Open Save Panel
void AppWindow::OnSaveGraph() {
	BMessage msg(MSG_SAVE_GRAPH);
	savePanel->SetMessage(&msg);
	savePanel->Show();
}

// Save graph as Bitmap
static struct {
	const char *extension;
	uint32 type;
} file_types[] = {
	{"TGA", 'TGA '},
	{"PPM", 'PPM'},
	{"PNG", 'PNG '},
	{"JPEG", 'JPEG'},
	{"BMP", 'BMP '},
	{NULL, 0}
};

void GetExtension(const BString &name, BString &string) {
int i = name.Length();
	while ((i >= 0) && (name[i] != '.')) i--;
	if (i >= 0) string = BString(&name.String()[i+1]);
	string.ToUpper();
}

void AppWindow::OnSaveGraph(BMessage *msg) {
entry_ref dir;
BString name, extension;
uint32 type; int i;
	msg->FindRef("directory", &dir);
	msg->FindString("name", &name);
	GetExtension(name, extension);
	for (i = 0; (file_types[i].extension != NULL) && 
				(extension != file_types[i].extension); i++);
	type = file_types[i].type;
	if (type != 0) {
		BDirectory dir2(&dir);
		BEntry file(&dir2, name.String());
		BPath path;
		file.GetPath(&path);

		BRect rect;
		BBitmap *bitmap = GetGraph(rect, B_CMAP8);
	
		if (bitmap != NULL) {
			BTranslatorRoster *roster = BTranslatorRoster::Default();
			BBitmapStream stream(bitmap);
			BFile file(path.Path(), B_CREATE_FILE | B_WRITE_ONLY);
			if (B_OK != roster->Translate(&stream, NULL, NULL, &file, type)) {
			BAlert *alert = new BAlert("Error", "Error in translator!\nCould not write file", "Ok");
				BEntry e(path.Path());
				e.Remove();
				alert->Go();
			}
			stream.DetachBitmap(&bitmap);
			delete bitmap;
		}
	} else {
		BAlert *alert = new BAlert("Error", "Unknown file format!", "Ok");
		alert->Go();
	}
/*	// print output translators to stdout
	translator_id *translators;
	int32 num_translators;
	BTranslatorRoster *roster = BTranslatorRoster::Default();
	
	roster->GetAllTranslators(&translators, &num_translators);

	for (int32 i = 0; i < num_translators; i++) {
		const translation_format *fmts;
		int32 num_fmts;
		const char *tname, *tinfo;
		int32 tversion;
		roster->GetTranslatorInfo(translators[i], &tname, &tinfo, &tversion);
		printf("%s: %s (%.2f)\n", tname, tinfo, tversion/100.0);
		roster->GetOutputFormats(translators[i], &fmts, &num_fmts);
		
		for (int32 j = 0; j < num_fmts; j++) {
			if (fmts[j].group == B_TRANSLATOR_BITMAP) { 
				if (fmts[j].type == B_TRANSLATOR_BITMAP) continue;
				printf("%d %d [%s] %s \"%4.4s\"\n", i, j, fmts[j].MIME, fmts[j].name,
						&fmts[j].type);
				break;
			}
		}
	}
	
	delete [] translators;
	fflush(stdout);
*/
}

void AppWindow::OnMove(BMessage *msg) {
	if (view->graph->GetGraphExprs()->GetState() != S_VALUE) return;
	BPoint p;
	msg->FindPoint("Diff", &p);
	MoveGraphRect(p);
}

void AppWindow::OnZoom(BMessage *msg) {
	if (view->graph->GetGraphExprs()->GetState() != S_VALUE) return;
	float f;
	msg->FindFloat("f", &f);
	Zoom(f);
}

bool AppWindow::QuitRequested() {
	if (program != NULL) {
		program->SetHide(false);
		program->PostMessage(B_QUIT_REQUESTED);
	}
	be_app->PostMessage(B_QUIT_REQUESTED);
	return(true);
}

void AppWindow::AboutRequested() {
	BAlert *about = new BAlert(APPLICATION, 
		APPLICATION " " VERSION "\nis freeware under GPL.\n\n"
		"Written 2000.\n\n"
		"By Michael Pfeiffer.\n\n"
		"EMail: k3079e6@lion.cc.","Close");
		about->Go();
}

#ifdef WATCH_CLIPBOARD
void PrintType(uint32 type) {
	printf("'%c%c%c%c'", (type >> 24) & 255, (type >> 16) & 255, 
				(type >> 8) & 255, type & 255);
}

void DisplayMessageData(BMessage *msg) {
char *name;
uint32 type;
int32 count;
	for (int32 i = 0; msg->GetInfo(B_ANY_TYPE, i, &name, &type, &count) == B_OK; i++) {
		printf("%d %s ", i, name); PrintType(type); printf("\n");
		switch (type) {
		case 'RECT': {
			BRect r; ssize_t size;
			msg->FindRect(name, 0, &r);
			printf("Rect(%f, %f, %f, %f)\n", r.left, r.top, r.right, r.bottom); 
			break;  }
		case 'CSTR': {
			const char *s;
			msg->FindString(name, 0, &s);
			printf("%s\n", s);
			break; }
		case 'LONG': {
			int64 i;
			msg->FindInt64(name, 0, &i);
			printf("%d\n", i);
			break; }
		} 
	}

}

void AppWindow::OnClipboardChanged(BMessage *msg) {
char *name;
uint32 type;
int32 count;
	DisplayMessageData(msg);
	
	BMessage *clip = NULL;	
	if (be_clipboard->Lock()) {
		if ((clip = be_clipboard->Data())) {
			DisplayMessageData(clip);
		}
		be_clipboard->Unlock();
	}
	fflush(stdout);
}
#endif

void AppWindow::MarkDefinitionsItem(bool mark) {
	BMenuItem *item = menubar->FindItem("Defintions");
	if (item != NULL) item->SetMarked(mark);
}

// Message Received
void AppWindow::MessageReceived(BMessage *message) {
	switch(message->what) {
	case MENU_APP_NEW: 
		break; 
	case B_ABOUT_REQUESTED:	AboutRequested();
		break;

	case MSG_MOVE: OnMove(message); 
		break;
	case MSG_ZOOM: OnZoom(message); 
		break;
	case MSG_REMOVE: OnRemove();
		break;
	case MSG_ADD: OnAdd();
		break;
	case MSG_REPLACE: OnReplace();
		break;
	case MSG_SELECTED: OnSelect(); 
		break;
	case MENU_APP_PAGE_SETUP: PageSetup(); 
		break;
	case MENU_APP_PRINT: Print(); 
		break;
	case MENU_APP_COPY: OnCopy();
		break;
	case MENU_APP_SAVE_GRAPH: OnSaveGraph();
		break;
	case MENU_APP_SAVE: OnSave();
		break;
	case MENU_APP_OPEN: OnOpen();
		break;
	case MSG_SAVE_GRAPH: OnSaveGraph(message);
		break;
	case MSG_SAVE: OnSave(message);
		break;
	case MSG_OPEN: OnOpen(message);
		break;
	case MENU_APP_PROGRAM:
		if (program == NULL) {
			program = new ProgramWindow(this, BRect(40, 40, 400, 400));
			MarkDefinitionsItem(true);
		} else {
			if (program->Lock()) {
				if (program->IsHidden()) program->Show();
				else program->Hide();
				MarkDefinitionsItem(!program->IsHidden());
				program->Unlock();
			}
		}
		break;
	case MSG_CLOSE_PROGRAM:
		MarkDefinitionsItem(false);
		program = NULL;
		break;
	case MSG_COMPILE_PROGRAM:
		if (UpdateGraph()) statusLine->SetText("ok");
		break;
	case MSG_HIDE_PROGRAM:
		MarkDefinitionsItem(false);
		break;
#ifdef WATCH_CLIPBOARD
	case B_CLIPBOARD_CHANGED:
		OnClipboardChanged(message);
		break;
#endif
	case MSG_UPDATE:
		OnUpdate();
		break;
	default:
		BWindow::MessageReceived(message);
	}
}

// BApplication

App::App() : BApplication("application/x-vnd.functionplotter") {
	BRect aRect;
	// set up a rectangle and instantiate a new window
	aRect.Set(100, 80, 850, 680);
	InitColorTable(); 
	window = NULL;
	window = new AppWindow(aRect);		
}

int main(int argc, char *argv[]) { 
	be_app = NULL;
	new App();
	be_app->Run();
	delete be_app; 
	return 0;
}