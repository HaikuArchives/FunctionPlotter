#ifndef DATA_VIEW
#define DATA_VIEW

#include <InterfaceKit.h>
#include "UI/ColorMenuItem.h"

class DataView : public BView {
public:
	BTextControl
		*fCtrls[5],
		*gCtrls[6];
	
	BCheckBox *visible;
	BMenuField *color, *style;
	
//	BPopUpMenu *popUp;
	BMenuField *functions;
	
	void MarkColorItem(const rgb_color &color);
	
	DataView(BRect rect);
};

#endif
