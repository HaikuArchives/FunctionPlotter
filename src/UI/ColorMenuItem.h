#ifndef COLOR_MENU_ITEM
#define COLOR_MENU_ITEM

#include <MenuItem.h>
#include <GraphicsDefs.h>

class _EXPORT ColorMenuItem : public BMenuItem {
	rgb_color color;
public:
	ColorMenuItem(rgb_color color, BMessage *message, char shortcut = 0,
		uint32 modifiers = 0);
	void Draw/*Content*/();
	void GetContentSize(float *w, float *h);
	rgb_color GetColor() const;

	void GetSize(float &w, float &h);

	ColorMenuItem(BMessage *data);
	static BArchivable *Instantiate(BMessage *data);
	status_t Archive(BMessage *msg, bool deep = true);
};

#define COLORS 5
#define COLOR_TABLE_SIZE COLORS * COLORS * COLORS
extern rgb_color color_table[COLOR_TABLE_SIZE];
void InitColorTable();

void ArchiveColor(const char *name, BMessage *archive, const rgb_color &color);
void UnarchiveColor(const char *name, BMessage *archive, rgb_color &color);

BMenu *CreateColorMenu(const char *name, int columns, int n, rgb_color *colors, BMessage *msg);
void MarkColorMenuItem(BMenu *m, const rgb_color &color);

#endif
