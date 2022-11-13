#include "crt.h"
#include "screenshoot.h"

using namespace draw;

screenshoot::screenshoot(rect rc, bool fade) : surface(rc.width(), rc.height(), getbpp()) {
	x = rc.x1;
	y = rc.y1;
	if(draw::canvas) {
		blit(0, 0, width, height, 0, *draw::canvas, x, y);
		if(fade) {
			auto push_canvas = canvas;
			auto push_clip = clipping;
			auto push_alpha = alpha;
			auto push_fore = fore;
			canvas = this;
			setclip();
			alpha = 128;
			fore = colors::form;
			//rectf({0, 0, width, height});
			fore = push_fore;
			alpha = push_alpha;
			clipping = push_clip;
			canvas = push_canvas;
		}
	}
}

screenshoot::screenshoot(bool fade) : screenshoot({0, 0, getwidth(), getheight()}, fade) {
}

screenshoot::~screenshoot() {
}

void screenshoot::restore() const {
	setclip();
	if(draw::canvas)
		draw::canvas->blit(x, y, width, height, 0, *this, 0, 0);
}

void screenshoot::blend(const surface& destination, unsigned milliseconds) const {
	if(!milliseconds)
		return;
	auto start = getcputime();
	auto finish = start + milliseconds;
	auto current = start;
	while(ismodal() && current < finish) {
		auto alpha = ((current - start) << 8) / milliseconds;
		restore();
		canvas->blend(destination, alpha);
		doredraw();
		waitcputime(1);
		current = getcputime();
	}
}

void screenshoot::fade(fnevent proc, unsigned milliseconds) {
	screenshoot before;
	proc();
	screenshoot after;
	before.blend(after, milliseconds);
	after.restore();
}