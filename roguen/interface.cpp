#include "answers.h"
#include "bsreq.h"
#include "draw.h"
#include "draw_object.h"
#include "keyname.h"
#include "log.h"
#include "main.h"
#include "resource.h"

using namespace draw;

const int tsx = 64;
const int tsy = 48;
const int mst = 260;
const int window_width = 480;
const int window_height = 280;
const int panel_width = 120;

static const void* focus_pressed;
static unsigned long last_tick_message;
static unsigned long start_stamp;
static int wears_offset = 80;

void set_dark_theme();
void initialize_translation(const char* locale);
void initialize_png();

point m2s(point v) {
	return point{(short)(v.x * tsx), (short)(v.y * tsy)};
}

point s2m(point v) {
	return point{(short)(v.x / tsx), (short)(v.y / tsy)};
}

static point right(point v) {
	return {(short)(v.x - tsx), v.y};
}

static point left(point v) {
	return {(short)(v.x + tsx), v.y};
}

static point up(point v) {
	return {v.x, (short)(v.y - tsy)};
}

static point down(point v) {
	return {v.x, (short)(v.y + tsy)};
}

static color getcolor(int format_color) {
	switch(format_color) {
	case 1: return colors::red;
	case 2: return colors::green;
	case 3: return colors::blue;
	case 4: return colors::yellow;
	default: return colors::text;
	}
}

static void strokedown() {
	rectpush push;
	auto push_fore = fore;
	fore = push_fore.mix(colors::text, 216);
	line(caret.x, caret.y + height - 1);
	line(caret.x + width - 1, caret.y);
	fore = push_fore.mix(colors::window, 128);
	line(caret.x, caret.y - height + 1);
	line(caret.x - width + 1, caret.y);
	fore = push_fore;
}

static void strokeup() {
	rectpush push;
	auto push_fore = fore;
	fore = push_fore.mix(colors::window, 128);
	line(caret.x, caret.y + height - 1);
	line(caret.x + width -1, caret.y);
	fore = push_fore.mix(colors::text, 216);
	line(caret.x, caret.y - height + 1);
	line(caret.x - width + 1, caret.y);
	fore = push_fore;
}

static point to(point pt, direction_s d, int sx, int sy) {
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

static void remove_temp_objects(const array& source) {
	for(auto& e : bsdata<object>()) {
		if(source.have(e.data))
			e.clear();
	}
}

static void remove_temp_objects() {
	for(auto& e : bsdata<object>()) {
		if(!e.data)
			e.clear();
	}
}

static void add_object(point pt, void* data, unsigned char random, unsigned char priority = 10) {
	auto po = addobject(pt);
	po->data = data;
	po->priority = priority;
	po->random = random;
	po->alpha = 0xFF;
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

void movable::fixdisappear() const {
	auto po = draw::findobject(this);
	if(po)
		po->disappear(mst);
}

void movable::fixeffect(point position, const char* id) {
	auto pv = bsdata<visualeffect>::find(id);
	if(!pv)
		return;
	position.y += pv->dy;
	auto po = draw::addobject(position);
	po->data = pv;
	po->priority = pv->priority;
	po->alpha = 0xFF;
	po->add(mst);
}

void movable::fixeffect(const char* id) const {
	auto pt = getposition(); pt.y -= tsy / 2;
	fixeffect(pt, id);
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

void movable::fixremove() const {
	auto po = draw::findobject(this);
	if(po)
		po->clear();
}

void movable::fixvalue(const char* format, int format_color) const {
	auto pt = getposition(); pt.y -= tsy;
	auto pa = addobject(pt);
	pa->alpha = 0xFF;
	pa->string = szdup(format);
	pa->priority = 20;
	pa->fore = colors::red;
	auto po = pa->add(mst);
	pt.y -= tsy;
	po->position = pt;
}

void movable::fixvalue(int v) const {
	char temp[260]; stringbuilder sb(temp);
	sb.add("%1i", v);
	fixvalue(temp, (v > 0) ? 2 : 1);
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

static void paint_items() {
	remove_temp_objects(bsdata<itemi>::source);
	auto p1 = s2m(camera);
	rect rc = {p1.x, p1.y, p1.x + getwidth() / tsx + 1, p1.y + getheight() / tsy + 1};
	for(auto& e : bsdata<itemground>()) {
		if(!e)
			continue;
		auto pt = i2m(e.index);
		if(!pt.in(rc))
			continue;
		add_object(m2s(pt), &const_cast<itemi&>(e.geti()), 0, 6);
	}
}

static void link_camera() {
	if(player) {
		auto po = findobject(player);
		if(po)
			setcamera(po->position);
	}
}

static void paint_overlapped(indext i, tile_s tile) {
	for(auto& ei : bsdata<tilei>()) {
		if(ei.borders == -1)
			continue;
		auto t0 = (tile_s)(&ei - bsdata<tilei>::elements);
		if(tile == t0)
			return;
		auto f = area.getindex(i, t0);
		if(!f)
			continue;
		auto pi = gres(res::Borders);
		image(pi, ei.borders * 15 + (f - 1), 0);
	}
}

static bool iswall(indext i, direction_s d) {
	auto i1 = to(i, d);
	if(i1 == Blocked)
		return true;
	if(!area.is(i1, Explored))
		return true;
	return bsdata<tilei>::elements[area.tiles[i1]].iswall();
}

static void fillwalls() {
	rectpush push;
	pushvalue push_fore(fore);
	caret.x -= tsx / 2; caret.y -= tsy / 2;
	width = tsx; height = tsy;
	fore = color(39, 45, 47);
	rectf();
}

static void fillfow() {
	rectpush push;
	pushvalue push_fore(fore);
	caret.x -= tsx / 2; caret.y -= tsy / 2;
	width = tsx; height = tsy;
	fore = color(11, 12, 11);
	rectf();
}

static void paint_wall(point p0, indext i, unsigned char r, const tilei& ei) {
	auto pi = gres(res::Walls);
	auto bs = ei.walls.start + ei.walls.count;
	auto bw = 0;
	auto wn = iswall(i, North);
	auto ws = iswall(i, South);
	auto we = iswall(i, East);
	auto ww = iswall(i, West);
	auto ss = false;
	auto sn = false;
	if(ws) {
		fillwalls();
		if(!ww) {
			if(iswall(i, SouthWest))
				image(pi, bs + 5, 0);
			else
				image(pi, bs + 1, 0);
		} else if(!iswall(i, SouthWest))
			image(pi, bs + 3, 0);
		if(!we) {
			if(iswall(i, SouthEast))
				image(pi, bs + 6, 0);
			else
				image(pi, bs + 2, 0);
		} else if(!iswall(i, SouthEast))
			image(pi, bs + 4, 0);
	} else {
		image(pi, ei.walls.get(r), 0);
		if(!ww)
			image(pi, bs + 10, 0);
		if(!we)
			image(pi, bs + 9, 0);
		add_object(down(p0), bsdata<resource>::elements + (int)res::Shadows, bw + 0, 6);
		sn = true;
	}
	auto pu = up(p0);
	if(!wn) {
		add_object(pu, bsdata<resource>::elements + (int)res::Walls, bs + 0, 12);
		if(!we)
			add_object(pu, bsdata<resource>::elements + (int)res::Walls, bs + 7, 13);
		if(!ww)
			add_object(pu, bsdata<resource>::elements + (int)res::Walls, bs + 8, 13);
		add_object(pu, bsdata<resource>::elements + (int)res::Shadows, bw + 1, 6);
		ss = true;
	}
	if(!ww) {
		add_object(right(p0), bsdata<resource>::elements + (int)res::Shadows, bw + 2, 6);
		if(ss)
			add_object(right(pu), bsdata<resource>::elements + (int)res::Shadows, bw + 6, 6);
		if(sn)
			add_object(right(down(p0)), bsdata<resource>::elements + (int)res::Shadows, bw + 4, 6);
	}
	if(!we) {
		add_object(left(p0), bsdata<resource>::elements + (int)res::Shadows, bw + 3, 6);
		if(ss)
			add_object(left(pu), bsdata<resource>::elements + (int)res::Shadows, bw + 7, 6);
		if(sn)
			add_object(left(down(p0)), bsdata<resource>::elements + (int)res::Shadows, bw + 5, 6);
	}
}

static void paint_floor() {
	remove_temp_objects(bsdata<featurei>::source);
	remove_temp_objects(bsdata<resource>::source);
	auto pi = gres(res::Floor);
	auto pd = gres(res::Decals);
	auto pf = gres(res::Features);
	auto x1 = (camera.x - tsx / 2) / tsx, x2 = (camera.x + getwidth() + tsx / 2) / tsx;
	auto y1 = (camera.y - tsy / 2) / tsy, y2 = (camera.y + getheight() + tsx / 2) / tsy;
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
			auto t = area.tiles[i];
			auto& ei = bsdata<tilei>::elements[t];
			if(ei.iswall())
				paint_wall(pt, i, r, ei);
			else {
				if(ei.floor) {
					image(pi, ei.floor.start + (r % ei.floor.count), 0);
					paint_overlapped(i, t);
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
				auto f = area.features[i];
				if(f) {
					auto& ei = bsdata<featurei>::elements[f];
					if(ei.is(BetweenWalls)) {
						auto a = area.is(i, Activated) ? 1 : 0;
						if(area.iswall(i, East) && area.iswall(i, West))
							image(pf, ei.features.start + a, 0);
						else if(area.iswall(i, North) && area.iswall(i, South))
							image(pf, ei.features.start + 2 + a, 0);
					} else
						add_object(pt, &ei, r, ei.priority);
				}
			}
		}
	}
}

static void paint_fow() {
	rectpush push;
	height = tsy; width = tsx;
	auto pi = gres(res::Fow);
	auto x1 = camera.x / tsx, x2 = (camera.x + getwidth() + tsx / 2) / tsx;
	auto y1 = camera.y / tsy, y2 = (camera.y + getheight() + tsy / 2) / tsy;
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
			if(!area.is(i, Explored))
				fillfow();
			else {
				auto f = area.getfow(i);
				if(f == 0) {
					if(!area.isb(to(i, SouthEast), Explored))
						image(pi, 2, ImageMirrorH);
					if(!area.isb(to(i, NorthWest), Explored))
						image(pi, 2, ImageMirrorV);
					if(!area.isb(to(i, NorthEast), Explored))
						image(pi, 2, ImageMirrorV | ImageMirrorH);
					if(!area.isb(to(i, SouthWest), Explored))
						image(pi, 2, 0);
				} else {
					if((f & 1) != 0)
						image(pi, 0, ImageMirrorV);
					if((f & 2) != 0)
						image(pi, 0, 0);
					if((f & 4) != 0)
						image(pi, 1, 0);
					if((f & 8) != 0)
						image(pi, 1, ImageMirrorH);
				}
			}
		}
	}
}

static int getavatar(int race, bool female, int armor) {
	return race * 6 + (female ? 1 : 0) + armor * 2;
}

static void fillbar(int value) {
	rectpush push;
	if(value > 0) {
		auto push_width = width;
		width = value;
		rectf();
		width = push_width - value;
		caret.x += value;
	}
	filldark();
}

static void bar(int value, int maximum, color m) {
	if(!maximum)
		return;
	auto push_fore = fore;
	fore = m;
	fillbar(value * width / maximum);
	strokeborder();
	fore = push_fore;
}

void creature::paintbars() const {
	const int dy = 4;
	rectpush push;
	caret.y -= tsy * 7 / 4; caret.x -= tsx / 4;
	width = tsx / 2; height = 4;
	bar(get(Hits), get(HitsMaximum), colors::red); caret.y += dy - 1;
	bar(get(Mana), get(ManaMaximum), colors::blue);
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
			image(pa, 9, flags);
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
			image(pa, wears[MeleeWeapon].getavatar(), flags);
		else if(wears[MeleeWeaponOffhand])
			image(pa, 10 + wears[MeleeWeaponOffhand].getavatar(), flags);
		else
			image(pa, 10 + 25, flags);
	}
	if(player == this)
		paintbars();
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

void visualeffect::paint() const {
	auto pi = gres(resid);
	if(!pi)
		return;
	auto pc = pi->gcicle(frame);
	if(!pc)
		return;
	unsigned long current = getobjectstamp() - start_stamp;
	auto tk = current * pc->count / mst;
	if(tk >= pc->count)
		return;
	image(pi, pc->start + tk, 0);
}

static void object_afterpaint(const object* p) {
	if(bsdata<creature>::have(p->data))
		((creature*)p->data)->paint();
	else if(bsdata<featurei>::have(p->data))
		((featurei*)p->data)->paint(p->random);
	else if(bsdata<itemi>::have(p->data))
		((itemi*)p->data)->paint();
	else if(bsdata<visualeffect>::have(p->data))
		((visualeffect*)p->data)->paint();
	else if(bsdata<resource>::have(p->data))
		image(((resource*)p->data)->get(), p->random, 0);
}

static void fieldh(const char* format) {
	char temp[260]; stringbuilder sb(temp);
	sb.add("%1:", format);
	text(temp);
}

static void field(const char* id, int width, const char* format) {
	fieldh(id);
	caret.x += width;
	text(format);
	caret.x += width;
}

static void field(const char* id, int width, int value) {
	char temp[32]; stringbuilder sb(temp);
	sb.add("%1i", value);
	field(id, width, temp);
}

static void fillbuttonpress() {
	auto push_fore = fore;
	fore = colors::button.mix(colors::form, 96);
	rectf();
	fore = push_fore;
}

static void paint_button(const char* format, bool pressed) {
	auto push_caret = caret;
	height = texth();
	if(width == -1)
		width = textw(format) + 6;
	auto push_fore = fore;
	fore = colors::button;
	rectf();
	if(pressed)
		strokedown();
	else
		strokeup();
	fore = push_fore;
	caret.x += 1 + (width - textw(format) + 1) / 2;
	if(pressed)
		caret.y += 1;
	text(format);
	caret = push_caret;
}

static bool button(unsigned key, int format_width) {
	if(!key)
		return false;
	char temp[2] = {(char)key, 0};
	auto pn = findnamebykey(key);
	if(!pn)
		pn = temp;
	auto push_width = width;
	width = format_width;
	auto result = draw::button(temp, key, draw::pbutton, false);
	width = push_width;
	return result;
}

static void player_info() {
	char temp[1024]; stringbuilder sb(temp); sb.clear();
	player->getinfo(sb);
	textf(temp);
}

static void paint_status() {
	auto push_caret = caret;
	auto push_height = height;
	auto push_width = width;
	caret.x = getwidth() - panel_width;
	caret.y = 0;
	width = panel_width - 1;
	height = getheight() - 1;
	fillform();
	strokeborder();
	setoffset(metrics::border, metrics::border);
	if(player)
		player_info();
	caret = push_caret;
	height = push_height;
	width = push_width - panel_width;
}

static void before_paint_all() {
	auto push_caret = caret;
	auto push_clip = clipping;
	paint_status();
	setclipall();
	link_camera();
	paint_floor();
	paint_items();
	caret = push_caret;
	clipping = push_clip;
}

static void paint_message() {
	auto p = console.begin();
	if(!p || !p[0])
		return;
	rectpush push;
	width = window_width;
	textfs(p);
	caret.y = metrics::padding * 2;
	caret.x = (getwidth() - width - panel_width) / 2;
	strokeout(fillwindow, metrics::padding, metrics::padding);
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

static void after_paint_all() {
	paint_fow();
	paint_message();
	reset_message();
}

static void execute_script() {
	auto pn = (hotkey::fnevent)hot.object;
	if(pn)
		pn(hot.param);
}

static void presskey(const sliceu<hotkey>& source) {
	for(auto& e : source) {
		if(hot.key == e.key) {
			execute(execute_script, 0, 0, e.proc);
			return;
		}
	}
}

static void animate_figures() {
	start_stamp = getobjectstamp();
	waitall();
	remove_temp_objects();
	remove_temp_objects(bsdata<visualeffect>::source);
}

void adventure_mode() {
	auto pk = bsdata<hotkeylist>::find("AdventureKeys");
	if(!pk)
		return;
	animate_figures();
	auto start = player->getwait();
	while(player && start == player->getwait() && ismodal()) {
		paintstart();
		paintobjects();
		presskey(pk->elements);
		paintfinish();
		domodal();
	}
}

static point answer_end;

static void answer_before_paint() {
	paintobjects();
	caret.x = (getwidth() - window_width - panel_width) / 2;
	caret.y = (getheight() - window_height) / 2;
	answer_end = caret;
	answer_end.y += window_height - texth() - 2;
	width = window_width;
	height = window_height;
	strokeout(fillform, metrics::padding, metrics::padding);
	auto push_fore = fore;
	fore = colors::form;
	strokeout(strokeup, metrics::padding, metrics::padding);
	strokeout(strokeup, metrics::padding - 1, metrics::padding - 1);
	fore = push_fore;
	if(answers::prompa) {
		auto push_font = font;
		auto push_fore = fore;
		font = metrics::h2;
		fore = colors::h2;
		texta(answers::prompa, AlignCenter);
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
	auto push_caret = caret;
	unsigned key = value ? answer_key(index) : KeyEscape;
	auto need_execute = button(key, 24);
	if(bsdata<creature>::have(value)) {
		auto pc = bsdata<creature>::elements + bsdata<creature>::source.indexof(value);
		auto pi = pc->getwear(value);
		auto st = pc->getwearslot(pi);
		if(pi && st >= MeleeWeapon && st <= Elbows) {
			text(bsdata<weari>::elements[st].getname());
			caret.x += wears_offset; width -= wears_offset;
		}
	}
	text(format);
	if(need_execute)
		execute(proc, (long)value);
	caret = push_caret;
	caret.y += texth() + 1;
}

static void answer_after_paint() {
	auto push_caret = caret;
	caret = answer_end;
	auto push_width = width; width = -1;
	if(answers::cancel_text) {
		if(draw::button(answers::cancel_text, KeyEscape, draw::pbutton, false))
			execute(buttoncancel);
	}
	width = push_width;
	caret = push_caret;
}

static void correct_camera() {
	if(camera.x < -tsx / 2)
		camera.x = -tsx / 2;
	if(camera.y < -tsy / 2)
		camera.y = -tsy / 2;
	auto w = getwidth() - panel_width;
	auto h = getheight();
	if(camera.x > tsx * mps - w - tsx / 2)
		camera.x = tsx * mps - w - tsx / 2;
	if(camera.y > tsy * mps - h - tsy / 2)
		camera.y = tsy * mps - h - tsy / 2;
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
	metrics::border = 4;
	metrics::padding = 4;
	pbutton = paint_button;
	object::beforepaintall = before_paint_all;
	object::afterpaint = object_afterpaint;
	object::afterpaintall = after_paint_all;
	object::correctcamera = correct_camera;
	answers::paintcell = answer_paint_cell;
	answers::beforepaint = answer_before_paint;
	answers::afterpaint = answer_after_paint;
	draw::width = 640;
	draw::height = 360;
	initialize(getnm("AppTitle"));
	setnext(proc);
	start();
	return 0;
}