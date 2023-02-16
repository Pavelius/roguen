#include "crt.h"
#include "draw.h"
#include "draw_object.h"
#include "screenshoot.h"

using namespace draw;

const size_t max_object_count = 1024;

BSDATAC(object, max_object_count)
BSDATAC(draworder, max_object_count)

fnevent				draw::object::afterpaintall;
fnevent				draw::object::beforepaintall;
fnevent				draw::object::correctcamera;
fnpaint				draw::object::afterpaint;
fnpaint				draw::object::afterpaintallpo;
static rect			last_screen, last_area;
static unsigned long timestamp;
static unsigned long timestamp_last;

long distance(point from, point to);

inline void copy(drawable& e1, drawable& e2) {
	e1 = e2;
}

static void remove_depends(draworder* p) {
	if(!p)
		return;
	for(auto& e : bsdata<draworder>()) {
		if(e.depend == p) {
			if(p->parent)
				copy(p->start, *p->parent);
			e.depend = 0;
		}
	}
}

unsigned long draw::getobjectstamp() {
	return timestamp;
}

void draworder::clear() {
	remove_depends(this);
	memset(this, 0, sizeof(*this));
}

static void remove_trail_orders() {
	auto pb = bsdata<draworder>::begin();
	auto pe = bsdata<draworder>::end();
	while(pe > pb) {
		pe--;
		if(*pe)
			break;
		bsdata<draworder>::source.count--;
	}
}

static void start_timer() {
	timestamp_last = getcputime();
}

static void update_timestamp() {
	auto c = getcputime();
	if(!timestamp_last || c < timestamp_last)
		timestamp_last = c;
	timestamp += c - timestamp_last;
	timestamp_last = c;
}

static void update_all_orders() {
	for(auto& e : bsdata<draworder>()) {
		if(e)
			e.update();
	}
}

static int calculate(int v1, int v2, int n, int m) {
	return v1 + (v2 - v1) * n / m;
}

void draworder::update() {
	if(depend || tick_start > timestamp)
		return;
	int m = tick_stop - tick_start;
	if(!m) {
		clear();
		return;
	}
	int n = timestamp - tick_start;
	if(n >= m)
		n = m;
	parent->position.x = (short)calculate(start.position.x, position.x, n, m);
	parent->position.y = (short)calculate(start.position.y, position.y, n, m);
	parent->alpha = (unsigned char)calculate(start.alpha, alpha, n, m);
	parent->fore.r = (unsigned char)calculate(start.fore.r, fore.r, n, m);
	parent->fore.g = (unsigned char)calculate(start.fore.g, fore.g, n, m);
	parent->fore.b = (unsigned char)calculate(start.fore.b, fore.b, n, m);
	if(n == m) {
		if(!alpha)
			parent->clear();
		clear();
	}
}

draworder* draworder::add(int milliseconds) {
	auto p = bsdata<draworder>::addz();
	copy(*p, *this);
	copy(p->start, *this);
	p->tick_start = timestamp;
	p->parent = parent;
	p->tick_stop = p->tick_start + milliseconds;
	return p;
}

draworder* object::add(int milliseconds, draworder* depend) {
	auto p = bsdata<draworder>::addz();
	if(depend) {
		copy(*p, *depend);
		copy(p->start, *depend);
		p->tick_start = depend->tick_stop;
		p->depend = depend;
	} else {
		copy(*p, *this);
		copy(p->start, *this);
		p->tick_start = timestamp;
	}
	p->parent = this;
	p->tick_stop = p->tick_start + milliseconds;
	return p;
}

void object::clear() {
	zclear(this);
}

void object::disappear(int milliseconds) {
	auto pr = add(milliseconds);
	pr->alpha = 0;
}

static void textcn(const char* string, int dy, unsigned feats) {
	auto push_caret = caret;
	caret.x -= textw(string) / 2;
	caret.y += dy;
	text(string, -1, 0);
	caret = push_caret;
}

void object::paintns() const {
	if(afterpaint)
		afterpaint(this);
	if(string) {
		auto push_font = draw::font;
		if(font)
			draw::font = font;
		textcn(string, 0, 0);
		draw::font = push_font;
	}
}

static void raw_beforemodal() {
	caret = {0, 0};
	width = getwidth();
	height = getheight();
	hot.cursor = cursor::Arrow;
	hot.hilite.clear();
}

void draw::splashscreen(unsigned milliseconds) {
	screenshoot push;
	raw_beforemodal();
	paintstart();
	paintobjects();
	screenshoot another;
	push.blend(another, milliseconds);
}

void object::paint() const {
	auto push_fore = draw::fore;
	auto push_alpha = draw::alpha;
	draw::fore = fore;
	draw::alpha = alpha;
	paintns();
	draw::alpha = push_alpha;
	draw::fore = push_fore;
}

static size_t getobjects(object** pb, object** pe) {
	auto ps = pb;
	for(auto& e : bsdata<object>()) {
		if(!e.position.in(last_area))
			continue;
		if(ps < pe)
			*ps++ = &e;
	}
	return ps - pb;
}

static int compare(const void* v1, const void* v2) {
	auto p1 = *((object**)v1);
	auto p2 = *((object**)v2);
	if(p1->type != p2->type)
		return p1->type - p2->type;
	auto r1 = p1->priority / 5;
	auto r2 = p2->priority / 5;
	if(r1 != r2)
		return r1 - r2;
	if(p1->position.y != p2->position.y)
		return p1->position.y - p2->position.y;
	if(p1->priority != p2->priority)
		return p1->priority - p2->priority;
	if(p1->position.x != p2->position.x)
		return p1->position.x - p2->position.x;
	return p1 - p2;
}

static void sortobjects(object** pb, size_t count) {
	qsort(pb, count, sizeof(pb[0]), compare);
}

void draw::paintobjects() {
	rectpush push;
	auto push_clip = clipping;
	if(object::beforepaintall)
		object::beforepaintall();
	last_screen = {caret.x, caret.y, caret.x + width, caret.y + height};
	setclip(last_screen);
	object* source[max_object_count];
	last_area = last_screen; last_area.move(camera.x, camera.y);
	last_area.offset(-128, -128);
	auto count = getobjects(source, source + sizeof(source) / sizeof(source[0]));
	sortobjects(source, count);
	for(size_t i = 0; i < count; i++) {
		auto p = source[i];
		if(p->type == object::Object)
			draw::caret = p->position - draw::camera;
		else
			draw::caret = p->position;
		p->paint();
	}
	if(object::afterpaintall)
		object::afterpaintall();
	if(object::afterpaintallpo) {
		for(size_t i = 0; i < count; i++) {
			auto p = source[i];
			if(p->type==object::Object)
				draw::caret = p->position - draw::camera;
			else
				draw::caret = p->position;
			object::afterpaintallpo(p);
		}
	}
	shrink();
	clipping = push_clip;
}

void* draw::chooseobject() {
	draw::scene(paintobjects);
	return (void*)getresult();
}

static void paintobjectsshowmode() {
	paintobjects();
	if(hot.key == KeyEscape)
		execute(buttoncancel);
	if(!hot.pressed && hot.key == MouseLeft)
		execute(buttoncancel);
}

void draw::showobjects() {
	draw::scene(paintobjectsshowmode);
}

object*	draw::addobject(point pt) {
	auto p = bsdata<object>::addz();
	p->clear();
	p->type = object::Object;
	p->position = pt;
	return p;
}

object* draw::findobject(const void* p) {
	for(auto& e : bsdata<object>()) {
		if(e.data == p)
			return &e;
	}
	return 0;
}

void draw::clearobjects() {
	bsdata<object>::source.clear();
}

void draw::shrink() {
	auto pb = bsdata<object>::begin();
	auto pe = bsdata<object>::end();
	while(pe > pb) {
		if(pe[-1])
			break;
		pe--;
	}
	bsdata<object>::source.count = pe - bsdata<object>::elements;
}

static rect getcorrectarea(int offs) {
	return {camera.x + offs, camera.y + offs,
		camera.x + last_screen.width() - offs,
		camera.y + last_screen.height() - offs};
}

static void correct_camera(point result, int offs) {
	if(last_screen) {
		rect area = getcorrectarea(offs);
		if(!result.in(area)) {
			if(result.x < area.x1)
				camera.x -= area.x1 - result.x;
			if(result.y < area.y1)
				camera.y -= area.y1 - result.y;
			if(result.x > area.x2)
				camera.x += result.x - area.x2;
			if(result.y > area.y2)
				camera.y += result.y - area.y2;
		}
	}
}

static void moving(point& result, point goal, int step, int corrent) {
	auto start = result;
	auto maxds = distance(start, goal);
	auto curds = 0;
	while(curds < maxds && ismodal()) {
		result.x = (short)(start.x + (goal.x - start.x) * curds / maxds);
		result.y = (short)(start.y + (goal.y - start.y) * curds / maxds);
		if(corrent)
			correct_camera(result, corrent);
		paintstart();
		paintobjects();
		doredraw();
		waitcputime(1);
		curds += step;
		if(curds > maxds)
			curds = maxds;
	}
	result = goal;
}

void object::move(point goal, int speed, int correct) {
	moving(position, goal, speed, correct);
}

void draw::setcamera(point v) {
	auto w = last_screen.width();
	if(!w)
		w = getwidth();
	auto h = last_screen.height();
	if(!h)
		h = getheight();
	v.x -= w / 2;
	v.y -= h / 2;
	camera = v;
	if(object::correctcamera)
		object::correctcamera();
}

bool draw::cameravisible(point goal, int border) {
	rect rc = {camera.x, camera.y, camera.x + last_screen.width(), camera.y + last_screen.height()};
	rc.offset(-border);
	return goal.in(rc);
}

void draw::slidecamera(point goal, int step) {
	auto push_camera = camera;
	setcamera(goal);
	goal = camera;
	camera = push_camera;
	auto start = camera;
	auto maxds = distance(start, goal);
	if(!maxds)
		return;
	auto curds = 0;
	while(curds < maxds && ismodal()) {
		curds += step;
		if(curds > maxds)
			curds = maxds;
		camera.x = (short)(start.x + (goal.x - start.x) * curds / maxds);
		camera.y = (short)(start.y + (goal.y - start.y) * curds / maxds);
		paintstart();
		paintobjects();
		doredraw();
		waitcputime(1);
	}
}

void draw::focusing(point goal) {
	if(!cameravisible(goal))
		slidecamera(goal);
}

void draw::waitall() {
	start_timer();
	while(bsdata<draworder>::source.count > 0 && ismodal()) {
		update_timestamp();
		update_all_orders();
		paintstart();
		paintobjects();
		doredraw();
		waitcputime(1);
		remove_trail_orders();
	}
}

void draw::draworder::wait() {
	if(!(*this))
		return;
	start_timer();
	while((*this) && bsdata<draworder>::source.count > 0 && ismodal()) {
		update_timestamp();
		update_all_orders();
		paintstart();
		paintobjects();
		doredraw();
		waitcputime(1);
		remove_trail_orders();
	}
}