#include "answers.h"
#include "bsreq.h"
#include "draw.h"
#include "draw_object.h"
#include "log.h"
#include "main.h"

using namespace draw;

const int tsx = 64;
const int tsy = 48;
const int mst = 200;
const int window_width = 420;
const int window_height = 280;

static unsigned long last_tick_message;

void set_dark_theme();
void initialize_translation(const char* locale);
void initialize_png();

point m2s(point v) {
	return point{(short)(v.x * tsx), (short)(v.y * tsy)};
}

point s2m(point v) {
	return point{(short)(v.x / tsx), (short)(v.y / tsy)};
}

void movable::fixmovement() const {
	auto po = draw::findobject(this);
	if(!po)
		return;
	auto pt = getposition();
	if(po->position == pt)
		return;
	auto pr = po->add(mst);
	pr->position = pt;
}

void movable::fixappear() const {
	auto po = draw::findobject(this);
	if(po)
		return;
	po = addobject(getposition());
	po->data = this;
	po->alpha = 0;
	po->priority = 11;
	auto pr = po->add(mst);
	pr->alpha = 0xFF;
}

point to(point pt, direction_s d, int sx, int sy) {
	if(d == North || d == NorthEast || d == NorthWest)
		pt.y -= sy;
	if(d == South || d == SouthEast || d == SouthWest)
		pt.y += sy;
	if(d == East || d == SouthEast || d == NorthEast)
		pt.x += sx;
	if(d == West || d == SouthWest || d == NorthWest)
		pt.x -= sx;
	return pt;
}

void movable::fixaction() const {
	auto po = draw::findobject(this);
	if(!po)
		return;
	auto pr = po->add(mst / 2);
	pr->position = to(po->position, direction, tsx / 8, tsy / 8);
	pr = pr->add(mst / 2);
	pr->position = po->position;
}

static void remove_temp_objects(array* source) {
	if(!source)
		return;
	for(auto& e : bsdata<object>()) {
		if(source->have(e.data))
			e.clear();
	}
}

static void add_object(point pt, void* data, unsigned char random, unsigned char priority = 10) {
	auto po = addobject(pt);
	po->data = data;
	po->alpha = 0xFF;
	po->priority = priority;
	po->random = random;
}

static void paint_items() {
	remove_temp_objects(bsdata<itemi>::source_ptr);
	auto p1 = s2m(camera);
	rect rc = {p1.x, p1.y, p1.x + getwidth() / tsx, p1.y + getheight() / tsy};
	for(auto& e : bsdata<itemground>()) {
		auto pt = i2m(e.index);
		if(!pt.in(rc))
			continue;
		add_object(m2s(pt), &const_cast<itemi&>(e.geti()), 0, 6);
	}
}

static void paint_floor() {
	remove_temp_objects(bsdata<featurei>::source_ptr);
	auto pi = gres(res::Floor);
	auto pd = gres(res::Decals);
	auto pf = gres(res::Features);
	auto x1 = camera.x / tsx, x2 = (camera.x + getwidth()) / tsx;
	auto y1 = camera.y / tsy, y2 = (camera.y + getheight()) / tsy;
	for(short y = y1; y <= y2; y++) {
		if(y < 0)
			continue;
		if(y >= mps)
			break;
		for(short x = x1; x <= x2; x++) {
			if(x < 0)
				continue;
			if(x >= mps)
				break;
			auto i = m2i({x, y});
			auto pt = m2s({x, y});
			setcaret(pt);
			auto r = area.random[i];
			auto& ei = bsdata<tilei>::elements[area.tiles[i]];
			if(ei.floor) {
				image(pi, ei.floor.start + (r % ei.floor.count), 0);
				if(ei.decals) {
					auto fw = r >> 3;
					if(fw < ei.decals.count)
						image(pd, fw, 0);
				}
			}
			for(auto f = Explored; f <= Webbed; f = (mapf_s)(f + 1)) {
				if(!area.is(i, f))
					continue;
				auto& ei = bsdata<areafi>::elements[f];
				if(ei.features)
					image(pf, ei.features.get(r), 0);
			}
			if(area.features[i]) {
				auto& ei = bsdata<featurei>::elements[area.features[i]];
				add_object(pt, &ei, r, ei.priority);
			}
		}
	}
}

static int getavatar(int race, bool female, int armor) {
	return race * 6 + (female ? 1 : 0) + armor * 2;
}

void creature::paint() const {
	auto flags = ismirror() ? ImageMirrorH : 0;
	auto kind = getkind();
	if(kind.iskind<monsteri>()) {
		auto pi = gres(res::Monsters);
		image(pi, kind.value, flags);
	} else {
		auto pb = gres(res::PCBody);
		auto pa = gres(res::PCArms);
		auto pc = gres(res::PCAccessories);
		// Missile weapon if any
		if(wears[RangedWeapon])
			image(pc, wears[RangedWeapon].getavatar(), flags);
		// Cloacks
		if(wears[Backward])
			image(pc, wears[Backward].getavatar(), flags);
		// Primary arm
		if(wears[MeleeWeapon].is(TwoHanded))
			image(pa, wears[MeleeWeapon].getavatar(), flags);
		else if(wears[MeleeWeapon])
			image(pa, 36 + wears[MeleeWeapon].getavatar(), flags);
		else
			image(pa, 36 + 25, flags);
		// Torso and armor
		image(pb, getavatar(kind.value, is(Female), wears[Torso].getavatar()), flags);
		// Thrown weapon
		//if(wears[ThrownWeapon])
		//	image(pc, 3, 0); // Throwing
		// Secondanary arm
		if(wears[MeleeWeapon].is(TwoHanded))
			image(pa, 9, flags);
		else if(wears[MeleeWeaponOffhand])
			image(pa, 10 + wears[MeleeWeaponOffhand].getavatar(), flags);
		else
			image(pa, 10 + 25, flags);
	}
}

void featurei::paint(int r) const {
	auto pi = gres(res::Features);
	image(pi, features.get(r), 0);
	if(overlay)
		image(pi, overlay.get(r >> 4), 0);
}

void itemi::paint() const {
	auto pi = gres(res::Items);
	image(pi, this - bsdata<itemi>::elements, 0);
}

static void object_afterpaint(const object* p) {
	if(bsdata<creature>::have(p->data))
		((creature*)p->data)->paint();
	else if(bsdata<featurei>::have(p->data))
		((featurei*)p->data)->paint(p->random);
	else if(bsdata<itemi>::have(p->data))
		((itemi*)p->data)->paint();
}

static void fieldh(const char* format) {
	char temp[260]; stringbuilder sb(temp);
	sb.add("%1:", format);
	text(temp);
}

static void field(const char* id, int width, const char* format) {
	auto push_caret = caret;
	fieldh(id);
	caret.x += width;
	text(format);
	caret = push_caret;
	caret.y += texth();
}

static void player_info() {
	if(!player)
		return;
	field("ST", 20, "10");
	field("DX", 20, "10");
}

static void paint_map_background() {
	rectpush push;
	setclipall();
	paint_floor();
	paint_items();
}

static void paint_message() {
	auto p = console.begin();
	if(!p || !p[0])
		return;
	rectpush push;
	width = window_width;
	textfs(p);
	caret.y = metrics::padding*2;
	caret.x = (getwidth() - width) / 2;
	auto push_alpha = alpha; alpha = 0xE0;
	strokeout(fillwindow, metrics::padding, metrics::padding);
	alpha = push_alpha;
	strokeout(strokeborder, metrics::padding, metrics::padding);
	textf(p);
}

static void reset_message() {
	auto current_tick = getcputime();
	if(!last_tick_message)
		last_tick_message = current_tick;
	auto delay = (current_tick - last_tick_message);
	if(delay >= 4000) {
		last_tick_message = current_tick;
		console.clear();
	}
}

static void paint_console() {
	paint_message();
	reset_message();
}

static void execute_script() {
	auto pn = (const char*)hot.object;
	if(!pn)
		return;
	auto ps = bsdata<script>::find(pn);
	if(ps)
		ps->proc(hot.param);
}

static void presskey(const slice<hotkey>& source) {
	for(auto& e : source) {
		if(hot.key == e.key) {
			execute(execute_script, 0, 0, e.id);
			return;
		}
	}
}

static hotkey adventure_keys[] = {
	{"AttackForward", 'A'},
	{"MoveDown", KeyDown},
	{"MoveDownLeft", KeyEnd},
	{"MoveDownRight", KeyPageDown},
	{"MoveLeft", KeyLeft},
	{"MoveRight", KeyRight},
	{"MoveUp", KeyUp},
	{"MoveUpRight", KeyPageUp},
	{"MoveUpLeft", KeyHome},
	{"Inventory", 'I'},
};

void adventure_mode() {
	waitall();
	auto start = player->getwait();
	while(player && start==player->getwait() && ismodal()) {
		paintstart();
		paintobjects();
		presskey(adventure_keys);
		paintfinish();
		domodal();
	}
}

static void answer_before_paint() {
	paintobjects();
	caret.x = (getwidth() - window_width - metrics::padding) / 2;
	caret.y = 40;
	width = window_width;
	height = window_height;
	auto push_alpha = alpha; alpha = 0xE0;
	strokeout(fillwindow, metrics::padding, metrics::padding);
	alpha = push_alpha;
	strokeout(strokeborder, metrics::padding, metrics::padding);
	if(answers::header) {
		auto push_font = font;
		auto push_fore = fore;
		font = metrics::h2;
		fore = colors::h2;
		texta(answers::header, AlignCenter);
		caret.y += texth();
		fore = push_fore;
		font = push_font;
	}
}

static unsigned answer_key(int index) {
	switch(index) {
	case 0: case 1: case 2:
	case 3: case 4: case 5:
	case 6: case 7: case 8:
		return '1' + index;
	case 9:
		return '0';
	default:
		return 'A' + (index - 10);
	}
}

static void answer_paint_cell(int index, const void* value, const char* format, fnevent proc) {
	unsigned key = value ? answer_key(index) : KeyEscape;
	auto need_execute = (key == hot.key);
	text(format);
	caret.y += texth();
	if(need_execute)
		execute(proc, (long)value);
}

int start_application(fnevent proc, fnevent initializing) {
	initialize_png();
	if(!proc)
		return -1;
	set_dark_theme();
	bsreq::read("rules/Basic.txt");
	initialize_translation("ru");
	if(initializing)
		initializing();
	if(log::geterrors())
		return -1;
	pbackground = paint_map_background;
	metrics::border = 1;
	metrics::padding = 4;
	//pfinish = paint_console;
	object::afterpaint = object_afterpaint;
	object::afterpaintall = paint_console;
	answers::paintcell = answer_paint_cell;
	answers::beforepaint = answer_before_paint;
	draw::width = 640;
	draw::height = 360;
	initialize(getnm("AppTitle"));
	setnext(proc);
	start();
	return 0;
}