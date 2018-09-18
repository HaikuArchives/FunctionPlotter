#include "UI/ColorMenuItem.h"
#include <View.h>

ColorMenuItem::ColorMenuItem(rgb_color color, BMessage *message, 
	char shortcut = 0,	uint32 modifiers = 0) :
	BMenuItem("item", message, shortcut, modifiers) {
	this->color = color;
}

void ColorMenuItem::Draw /*Content*/() {
float w, h;
BPoint p;
	BMenu *m = Menu();
	if (m == NULL) return;
	
	p = ContentLocation();
	GetContentSize(&w, &h);	

	rgb_color c = color;
	if (!IsEnabled()) {
		c.green = c.blue = c.red = 
			(uint8) (0.4 * c.red + 0.2 * c.blue + 0.4 * c.green);
	}

	m->SetHighColor(c);
	m->FillRect(BRect(p.x+2, p.y+2, p.x+w-2, p.y+h-2));
	
	if (IsMarked()) {
	rgb_color blue={0, 0, 255, 255};
		m->SetHighColor(blue);
		m->StrokeRect(BRect(p.x, p.y, p.x+w, p.y+h));
	}

	if (IsSelected()) {
	rgb_color red={0, 0, 255, 255};
		m->SetHighColor(red);
		m->StrokeRect(BRect(p.x, p.y, p.x+w, p.y+h));
	}
}

void ColorMenuItem::GetSize(float &w, float &h) {
	w = 15; h = 15;
}

void ColorMenuItem::GetContentSize(float *w, float *h) {
	// BMenuItem::GetContentSize(w, h);
	*w = 14; *h = 14;
}

rgb_color ColorMenuItem::GetColor() const {
	return color;
}

ColorMenuItem::ColorMenuItem(BMessage *data) : BMenuItem(data) {
	data->FindInt8("red", (int8*)&color.red);
	data->FindInt8("green", (int8*)&color.green);
	data->FindInt8("blue", (int8*)&color.blue);
	data->FindInt8("alpha", (int8*)&color.alpha);
}

BArchivable *ColorMenuItem::Instantiate(BMessage *data) {
	if (validate_instantiation(data, "ColorMenuItem"))
		return new ColorMenuItem(data);
	else
		return NULL;
}

status_t ColorMenuItem::Archive(BMessage *archive, bool deep = true) {
	BMenuItem::Archive(archive, deep);
	archive->AddString("class", "ColorMenuItem");
	ArchiveColor(NULL, archive, color);
	return B_OK;
}

BMenu *CreateColorMenu(const char *name, int columns, int n, rgb_color *colors, BMessage *msg) {
	if (columns > n) columns = n;
	int rows = (n + columns-1) / columns; // ceil((float)n / (float)columns)

	ColorMenuItem cmi(colors[0], NULL);
	float w, h;
	cmi.GetSize(w, h);
	
	BMenu *menu = new BMenu(name, columns * w, rows * h); 
	menu->SetRadioMode(true);
	
	BRect frame; float left, top;
	int i = 0, x, y; BMessage *m = NULL; 
	for (y = 0; y < rows; y++)
		for (x = 0; x < columns; x++) {
			left = x * w;
			top = y * h;
			frame.Set(left, top, left + w - 1, top + h - 1);
			
			if (msg != NULL) {
				m = new BMessage(*msg);
				ArchiveColor(NULL, m, colors[i]);
			}
			menu->AddItem(new ColorMenuItem(colors[i], m), frame);
			i++; if (i == n) return menu;
		}
	return menu;
}

void MarkColorMenuItem(BMenu *m, const rgb_color &c1) {
ColorMenuItem *item;
	for (int i = 0; NULL != (item = (ColorMenuItem*)m->ItemAt(i)); i++) {
	rgb_color c2 = item->GetColor();
		if ((c1.red == c2.red) && (c1.blue == c2.blue) && 
			(c1.green == c2.green) && (c1.alpha == c2.alpha)) {
			item->SetMarked(true);
			return;
		}
	}
	item = (ColorMenuItem*)m->ItemAt(0);
	if (item != NULL) item->SetMarked(true);
}


static uint8 colors[] = {
	0, 64, 128, 192, 255
};

rgb_color color_table[COLOR_TABLE_SIZE];

void InitColorTable() {
int i, j, k, l = 0;
	for (i = 0; i < COLORS; i++) 
		for (j = 0; j < COLORS; j++)
			for (k = 0; k < COLORS; k++) {
				color_table[l].red = colors[i];
				color_table[l].green = colors[j];
				color_table[l].blue = colors[k];
				color_table[l].alpha = 255;
				l ++;
			} 
}

void ArchiveColor(const char *name, BMessage *archive, const rgb_color &color) { 
	if (name == NULL) {
		archive->AddInt8("red", color.red);
		archive->AddInt8("green", color.green);
		archive->AddInt8("blue", color.blue);
		archive->AddInt8("alpha", color.alpha);
	} else {
		BMessage msg;
		msg.AddInt8("red", color.red);
		msg.AddInt8("green", color.green);
		msg.AddInt8("blue", color.blue);
		msg.AddInt8("alpha", color.alpha);
		archive->AddMessage(name, &msg);
	}
}

void UnarchiveColor(const char *name, BMessage *archive, rgb_color &color) { 
	if (name == NULL) {
		archive->FindInt8("red", (int8*)&color.red);
		archive->FindInt8("green", (int8*)&color.green);
		archive->FindInt8("blue", (int8*)&color.blue);
		archive->FindInt8("alpha", (int8*)&color.alpha);
	} else {
		BMessage msg;
		archive->FindMessage(name, &msg);
		msg.FindInt8("red", (int8*)&color.red);
		msg.FindInt8("green", (int8*)&color.green);
		msg.FindInt8("blue", (int8*)&color.blue);
		msg.FindInt8("alpha", (int8*)&color.alpha);
	}
}


