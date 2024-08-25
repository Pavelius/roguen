#include "bsdata.h"
#include "dialog.h"
#include "draw.h"

using namespace draw;

static void pause_keys() {
	if(hot.key == KeySpace || hot.key == KeyEscape)
		execute(buttoncancel);
}

int dialogi::open() const {
	if(beforeopen)
		beforeopen();
	while(ismodal()) {
		if(std_paint)
			paintstart();
		fillform();
		setoffset(metrics::padding, metrics::padding);
		mainscene();
		if(!no_keys)
			pause_keys();
		if(std_paint)
			paintfinish();
		domodal();
	}
	return getresult();
}

void open_dialog(const char* id) {
	auto p = bsdata<dialogi>::find(id);
	if(!p)
		return;
	p->open();
}