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
		paintstart();
		fillform();
		setoffset(metrics::padding, metrics::padding);
		mainscene();
		pause_keys();
		paintfinish();
		domodal();
	}
	return getresult();
}