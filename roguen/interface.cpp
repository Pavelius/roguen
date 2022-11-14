#include "answers.h"
#include "bsreq.h"
#include "draw.h"
#include "draw_object.h"
#include "keyname.h"
#include "log.h"
#include "main.h"
#include "resource.h"
#include "screenshoot.h"
#include "visualeffect.h"

using namespace draw;

const int tsx = 64;
const int tsy = 48;
const int mst = 260;
const int panel_width = 130;

void set_dark_theme();
void initialize_translation(const char* locale);
void initialize_png();

static const void*		focus_pressed;
static unsigned long	last_tick_message;
static unsigned long	start_stamp;
static int				wears_offset = 80;
static rect				message_rect;
static keybind*			keybinds;
bool					show_floor_rect;
int						window_width = 480;
int						window_height = 280;

static point m2s(point v) {
	return point{(short)(v.x * tsx), (short)(v.y * tsy)};
}

static point s2m(point v) {
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
	line(caret.x + width - 1, caret.y);
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

static object* add_object(point pt, void* data, unsigned char random, unsigned char priority = 10) {
	auto po = addobject(pt);
	po->data = data;
	po->priority = priority;
	po->random = random;
	po->alpha = 0xFF;
	return po;
}

point movable::getsposition() const {
	return m2s(getposition());
}

void movable::fixmovement() const {
	auto po = draw::findobject(this);
	if(!po)
		return;
	auto pt = getsposition();
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
	if(!area.is(position, Visible))
		return;
	auto pt = getsposition(); pt.y -= tsy / 2;
	fixeffect(pt, id);
}

void movable::fixappear() const {
	auto po = draw::findobject(this);
	if(po)
		return;
	po = addobject(getsposition());
	po->data = this;
	po->alpha = 0xFF;
	po->priority = 11;
}

void movable::fixremove() const {
	auto po = draw::findobject(this);
	if(po)
		po->clear();
}

void movable::fixvalue(const char* format, int format_color) const {
	auto pt = getsposition(); pt.y -= tsy;
	auto pa = addobject(pt);
	pa->alpha = 0xFF;
	pa->string = szdup(format);
	pa->priority = 20;
	pa->fore = getcolor(format_color);
	auto po = pa->add(mst);
	pt.y -= tsy;
	po->position = pt;
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

const char* missilename(direction_s v) {
	switch(v) {
	case North: return "MissileNorth";
	case South: return "MissileSouth";
	case East: return "MissileEast";
	case West: return "MissileWest";
	default: return "MissileNorthWest";
	}
}

void movable::fixshoot(point target, int frame) const {
	fixthrown(target, missilename(direction), frame);
}

void movable::fixthrown(point target, const char* id, int frame) const {
	if(!area.isvalid(target))
		return;
	auto pe = bsdata<visualeffect>::find(id);
	if(!pe)
		return;
	auto range = area.getrange(getposition(), target);
	if(range == 0xFFFF)
		return;
	auto po = add_object(m2s(getposition()), pe, frame);
	po->position.y += pe->dy;
	if(!po)
		return;
	auto pr = po->add(mst * range / 3);
	pr->position = m2s(target);
	pr->position.y += pe->dy;
}

void gamei::next(fnevent proc) {
	setnext(proc);
}

static void paint_items() {
	remove_temp_objects(bsdata<itemi>::source);
	auto p1 = s2m(camera);
	rect rc = {p1.x, p1.y, p1.x + getwidth() / tsx + 1, p1.y + getheight() / tsy + 1};
	for(auto& e : bsdata<itemground>()) {
		if(!e)
			continue;
		if(!area.is(e.position, Explored))
			continue;
		if(!e.position.in(rc))
			continue;
		add_object(m2s(e.position), &const_cast<itemi&>(e.geti()), 0, 6);
	}
}

static void link_camera() {
	if(player) {
		auto po = findobject(player);
		if(po)
			setcamera(po->position);
	}
}

static void paint_overlapped(point i, tile_s tile) {
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

static bool iswall(point i, direction_s d) {
	auto i1 = to(i, d);
	if(!area.isvalid(i1))
		return true;
	if(!area.is(i1, Explored))
		return true;
	return bsdata<tilei>::elements[area[i1]].iswall();
}

static void fillwalls() {
	rectpush push;
	pushvalue push_fore(fore);
	caret.x -= tsx / 2; caret.y -= tsy / 2;
	width = tsx; height = tsy;
	fore = color(39, 45, 47);
	rectf();
}

static void filllos() {
	rectpush push;
	pushvalue push_fore(fore);
	pushvalue push_alpha(alpha, (unsigned char)64);
	caret.x -= tsx / 2; caret.y -= tsy / 2;
	width = tsx; height = tsy;
	fore = color(0, 0, 0);
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

static void paint_wall(point p0, point i, unsigned char r, const tilei& ei) {
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

static void floorrect() {
	rectpush push;
	caret.x -= tsx / 2 - 1;
	caret.y -= tsx / 2 - 1;
	width = tsx - 2;
	height = tsy - 2;
	rectb();
}

static void paint_floor() {
	point i;
	remove_temp_objects(bsdata<featurei>::source);
	remove_temp_objects(bsdata<resource>::source);
	auto pi = gres(res::Floor);
	auto pd = gres(res::Decals);
	auto pf = gres(res::Features);
	short x1 = (camera.x - tsx / 2) / tsx, x2 = (camera.x + getwidth() + tsx / 2) / tsx;
	short y1 = (camera.y - tsy / 2) / tsy, y2 = (camera.y + getheight() + tsx / 2) / tsy + 2;
	for(i.y = y1; i.y <= y2; i.y++) {
		if(i.y < 0)
			continue;
		if(i.y >= area.mps)
			break;
		for(i.x = x1; i.x <= x2; i.x++) {
			if(i.x < 0)
				continue;
			if(i.x >= area.mps)
				break;
			auto pt = m2s(i);
			if(!area.is(i, Explored))
				continue;
			setcaret(pt);
			auto r = area.random[i];
			auto t = area[i];
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
							image(pd, ei.decals.start + fw, 0);
					}
				}
				for(auto f = Explored; f <= Webbed; f = (areaf)(f + 1)) {
					if(!area.is(i, f))
						continue;
					auto& ei = bsdata<areafi>::elements[f];
					if(ei.features)
						image(pf, ei.features.get(r), 0);
				}
				auto& ef = area.getfeature(i);
				if(ef.isvisible()) {
					if(ef.is(BetweenWalls)) {
						if(area.iswall(i, East) && area.iswall(i, West))
							add_object(pt, bsdata<resource>::elements + (int)res::Features, ef.features.start, ef.priority);
						else if(area.iswall(i, North) && area.iswall(i, South))
							add_object(pt, bsdata<resource>::elements + (int)res::Features, ef.features.start + 1, ef.priority);
					} else
						add_object(pt, const_cast<featurei*>(&ef), r, ef.priority);
				}
				if(show_floor_rect)
					floorrect();
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
		if(y >= area.mps)
			break;
		for(short x = x1; x <= x2; x++) {
			if(x < 0)
				continue;
			if(x >= area.mps)
				break;
			point i = {x, y};
			auto pt = m2s({x, y});
			setcaret(pt);
			if(!area.is(i, Explored))
				fillfow();
			else {
				auto f = area.getfow(i);
				if((f & 1) != 0)
					image(pi, 0, ImageMirrorV);
				if((f & 2) != 0)
					image(pi, 0, 0);
				if((f & 4) != 0)
					image(pi, 1, 0);
				if((f & 8) != 0)
					image(pi, 1, ImageMirrorH);
				if((f & (2 | 8)) == 0 && !area.isb(to(i, SouthEast), Explored))
					image(pi, 2, ImageMirrorH);
				if((f & (1 | 4)) == 0 && !area.isb(to(i, NorthWest), Explored))
					image(pi, 2, ImageMirrorV);
				if((f & (1 | 8)) == 0 && !area.isb(to(i, NorthEast), Explored))
					image(pi, 2, ImageMirrorV | ImageMirrorH);
				if((f & (2 | 4)) == 0 && !area.isb(to(i, SouthWest), Explored))
					image(pi, 2, 0);
			}
		}
	}
}

static void paint_los() {
	rectpush push;
	height = tsy; width = tsx;
	auto pi = gres(res::Los);
	auto x1 = camera.x / tsx, x2 = (camera.x + getwidth() + tsx / 2) / tsx;
	auto y1 = camera.y / tsy, y2 = (camera.y + getheight() + tsy / 2) / tsy;
	for(short y = y1; y <= y2; y++) {
		if(y < 0)
			continue;
		if(y >= area.mps)
			break;
		for(short x = x1; x <= x2; x++) {
			if(x < 0)
				continue;
			if(x >= area.mps)
				break;
			point i = {x, y};
			if(!area.is(i, Explored))
				continue;
			auto pt = m2s({x, y});
			setcaret(pt);
			if(!area.is(i, Visible))
				filllos();
			else {
				auto f = area.getlos(i);
				if(f != area.getfow(i)) {
					// 1 - North
					// 2 - South
					// 4 - West
					// 8 - East
					switch(f) {
					case 1: image(pi, 0, ImageMirrorV); break;
					case 2: image(pi, 0, 0); break;
					case 1 + 2: image(pi, 0, ImageMirrorV); image(pi, 0, 0); break;
					case 4: image(pi, 1, 0); break;
					case 1 + 4: image(pi, 3, ImageMirrorV); break;
					case 2 + 4: image(pi, 3, 0); break;
					case 1 + 2 + 4: image(pi, 3, 0); image(pi, 0, ImageMirrorV); break;
					case 8: image(pi, 1, ImageMirrorH); break;
					case 1 + 8: image(pi, 3, ImageMirrorV | ImageMirrorH); break;
					case 2 + 8: image(pi, 3, ImageMirrorH); break;
					case 1 + 2 + 8: image(pi, 0, ImageMirrorV); image(pi, 3, ImageMirrorH); break;
					case 4 + 8: image(pi, 1, 0); image(pi, 1, ImageMirrorH); break;
					case 1 + 4 + 8: image(pi, 3, ImageMirrorV); image(pi, 1, ImageMirrorH); break;
					case 2 + 4 + 8: image(pi, 3, 0); image(pi, 1, ImageMirrorH); break;
					default: break;
					}
				}
				if((f & (2 | 8)) == 0 && !area.isb(to(i, SouthEast), Visible))
					image(pi, 2, ImageMirrorH);
				if((f & (1 | 4)) == 0 && !area.isb(to(i, NorthWest), Visible))
					image(pi, 2, ImageMirrorV);
				if((f & (1 | 8)) == 0 && !area.isb(to(i, NorthEast), Visible))
					image(pi, 2, ImageMirrorV | ImageMirrorH);
				if((f & (2 | 4)) == 0 && !area.isb(to(i, SouthWest), Visible))
					image(pi, 2, 0);
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

static void fillbarnd(int value) {
	rectpush push;
	if(value > 0) {
		setoffset(1, 1);
		auto push_width = width;
		width = value;
		rectf();
		width = push_width - value;
		caret.x += value;
	}
}

static void bar_shade(int value, int maximum, color m) {
	if(!maximum || !value)
		return;
	if(value > maximum)
		value = maximum;
	auto push_fore = fore;
	fore = m;
	fore.a = 128;
	fillbarnd(value * width / maximum);
	fore = push_fore;
}

static point get_top_position(variant v) {
	point result = {0, 0};
	if(v.iskind<monsteri>()) {
		auto ps = gres(res::Monsters);
		auto& fr = ps->get(bsdata<monsteri>::elements[v.value].avatar);
		result.y -= fr.oy + 4 * 2;
	} else if(v.iskind<racei>()) {
		result.y = -76;
	}
	return result;
}

static void paint_bars(const creature* player) {
	const int dy = 4;
	rectpush push;
	caret.y += get_top_position(player->getkind()).y;
	caret.x -= tsx / 4;
	width = tsx / 2; height = 4;
	bar(player->get(Hits), player->get(HitsMaximum), colors::red);
	bar_shade(player->get(Poison), player->get(HitsMaximum), colors::green);
	caret.y += dy - 1;
	bar(player->get(Mana), player->get(ManaMaximum), colors::blue);
}

void creature::paintbarsall() const {
	if(!area.is(getposition(), Visible))
		return;
	if(game.getowner() == this || is(Enemy))
		paint_bars(this);
}

void creature::paint() const {
	auto feats = ismirror() ? ImageMirrorH : 0;
	auto kind = getkind();
	if(!area.is(getposition(), Visible))
		return;
	if(kind.iskind<monsteri>()) {
		auto pi = gres(res::Monsters);
		image(pi, bsdata<monsteri>::elements[kind.value].avatar, feats);
	} else {
		auto pb = gres(res::PCBody);
		auto pa = gres(res::PCArms);
		auto pc = gres(res::PCAccessories);
		// Missile weapon if any
		if(wears[RangedWeapon])
			image(pc, wears[RangedWeapon].getavatar(), feats);
		// Cloacks
		if(wears[Backward])
			image(pc, wears[Backward].getavatar(), feats);
		// Primary arm
		if(wears[MeleeWeapon].is(TwoHanded))
			image(pa, 9, feats);
		else if(wears[MeleeWeapon])
			image(pa, 36 + wears[MeleeWeapon].getavatar(), feats);
		else
			image(pa, 36 + 25, feats);
		// Torso and armor
		image(pb, getavatar(kind.value, is(Female), wears[Torso].getavatar()), feats);
		// Secondanary arm
		if(wears[MeleeWeapon].is(TwoHanded))
			image(pa, wears[MeleeWeapon].getavatar(), feats);
		else if(wears[MeleeWeaponOffhand])
			image(pa, 10 + wears[MeleeWeaponOffhand].getavatar(), feats);
		else
			image(pa, 10 + 25, feats);
	}
}

void featurei::paint(int r) const {
	if(!isvisible())
		return;
	auto pi = gres(res::Features);
	image(pi, features.get(r), 0);
	if(overlay)
		image(pi, overlay.get(r >> 4), 0);
}

void itemi::paint() const {
	auto pi = gres(res::Items);
	image(pi, avatar, 0);
}

void visualeffect::paint(unsigned char random) const {
	auto pi = gres(resid);
	if(!pi)
		return;
	if(pi->cicles_offset) {
		auto pc = pi->gcicle(frame);
		if(pc) {
			unsigned long current = getobjectstamp() - start_stamp;
			auto tk = current * pc->count / mst;
			if(tk < pc->count)
				image(pi, pc->start + tk, feats);
		}
	} else
		image(pi, random + frame, feats);
}

static void object_afterpaintpo(const object* p) {
	if(bsdata<creature>::have(p->data))
		((creature*)p->data)->paintbarsall();
}

static void object_afterpaint(const object* p) {
	if(bsdata<creature>::have(p->data))
		((creature*)p->data)->paint();
	else if(bsdata<featurei>::have(p->data))
		((featurei*)p->data)->paint(p->random);
	else if(bsdata<itemi>::have(p->data))
		((itemi*)p->data)->paint();
	else if(bsdata<visualeffect>::have(p->data))
		((visualeffect*)p->data)->paint(p->random);
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
	caret.y += 1;
	rectf();
	if(pressed)
		strokedown();
	else
		strokeup();
	caret.y -= 1;
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
	char temp[32]; stringbuilder sb(temp);
	addkeyname(sb, key);
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

static void before_paint_all() {
	auto push_caret = caret;
	auto push_clip = clipping;
	setclipall();
	link_camera();
	paint_floor();
	paint_items();
	paint_los();
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
	message_rect.set(caret.x, caret.y, caret.x + width, caret.y + height);
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

static void presskey() {
	for(auto& e : bsdata<hotkey>()) {
		if(hot.key == e.key) {
			execute(execute_script, 0, 0, e.proc);
			return;
		}
	}
}

void animate_figures() {
	start_stamp = getobjectstamp();
	waitall();
	remove_temp_objects();
	remove_temp_objects(bsdata<visualeffect>::source);
}

void adventure_mode() {
	animate_figures();
	auto human = player;
	if(!human)
		return;
	auto start = human->getwait();
	while((start == human->getwait()) && ismodal()) {
		paintstart();
		paintobjects();
		presskey();
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
	if(answers::prompa) {
		auto push_font = font;
		font = metrics::h2;
		fore = colors::h2;
		texta(answers::prompa, AlignCenter);
		caret.y += texth();
		font = push_font;
	}
	fore = push_fore;
}

static unsigned answer_key(int index) {
	switch(index) {
	case 0: case 1: case 2:
	case 3: case 4: case 5:
	case 6: case 7: case 8:
		return '1' + index;
	case 9: return '0';
	default: return 'A' + (index - 10);
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
	if(current_columns) {
		auto total_width = current_columns->totalwidth();
		char temp[260]; stringbuilder sb(temp);
		if(width >= total_width) {
			caret.x = push_caret.x + width - total_width;
			for(auto p = current_columns; *p; p++) {
				sb.clear();
				auto push_caret = caret;
				textf(p->proc(value, sb));
				caret = push_caret;
				caret.x += p->width;
			}
		}
	}
	if(need_execute)
		execute(proc, (long)value);
	caret = push_caret;
	caret.y += texth() + 1;
}

static void answer_paint_cell_small(int index, const void* value, const char* format, fnevent proc) {
	auto push_caret = caret;
	auto push_width = width;
	unsigned key = answer_key(index);
	auto need_execute = button(key, 24);
	width -= caret.x - push_caret.x;
	textf(format);
	if(need_execute)
		execute(proc, (long)value);
	width = push_width;
	caret = push_caret;
	caret.y += texth() + 1;
}

static void get_total_height(const answers& source) {
	auto push_clipping = clipping;
	auto total_height = 0;
	width = window_width - 60;
	auto p = console.begin();
	textfs(p);
	total_height += height;
	auto minimal_width = width;
	auto minimal_height = texth() + 1;
	if(source)
		total_height += metrics::padding;
	for(auto& e : source) {
		width = window_width - 24 - metrics::padding;
		textfs(e.text);
		width += 24 + metrics::padding;
		if(minimal_width < width)
			minimal_width = width;
		if(height < minimal_height)
			height = minimal_height;
		total_height += height;
	}
	width = minimal_width;
	height = total_height;
}

static void paint_message(const answers& source, int window_width) {
	auto p = console.begin();
	if(!p || !p[0])
		return;
	rectpush push;
	width = window_width;
	caret.y = metrics::padding * 2;
	caret.x = (getwidth() - window_width - panel_width) / 2;
	textf(p);
	caret.y += metrics::padding;
	auto index = 0;
	for(auto& e : source)
		answer_paint_cell_small(index++, e.value, e.text, buttonparam);
}

void* answers::choose() const {
	rectpush push;
	screenshoot screen;
	while(ismodal()) {
		screen.restore();
		get_total_height(*this);
		auto window_width = width;
		caret.y = metrics::padding * 2;
		caret.x = (getwidth() - window_width - panel_width) / 2;
		strokeout(fillwindow, metrics::padding, metrics::padding);
		strokeout(strokeborder, metrics::padding, metrics::padding);
		paint_message(*this, window_width);
		domodal();
	}
	screen.restore();
	::console.clear();
	return (void*)getresult();
}

static bool backward_button(const char* format, void* value) {
	auto format_width = textw(format);
	auto push_width = width;
	auto push_caret = caret;
	width = format_width;
	caret.x -= width;
	auto result = draw::button(format, 0, draw::pbutton, false);
	width = push_width;
	return result;
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
	if(camera.x > tsx * area.mps - w - tsx / 2)
		camera.x = tsx * area.mps - w - tsx / 2;
	if(camera.y > tsy * area.mps - h - tsy / 2)
		camera.y = tsy * area.mps - h - tsy / 2;
}

/*static void paint_world() {
	rectpush push;
	pushvalue push_fore(fore);
	const int z = 16;
	point origin;
	origin.x = (width - world.mps * z) / 2;
	origin.y = (height - world.mps * z) / 2;
	height = width = z;
	for(short y = 0; y < world.mps; y++) {
		for(short x = 0; x < world.mps; x++) {
			auto t = world[{x, y}];
			if(!t)
				continue;
			auto p = bsdata<sitei>::elements + t;
			fore = p->minimap;
			caret.x = origin.x + x * width;
			caret.y = origin.y + y * height;
			rectf();
		}
	}
}*/

static void fillfade(color cv, unsigned char av = 128) {
	pushvalue push_fore(fore);
	pushvalue push_alpha(alpha);
	fore = cv;
	alpha = av;
	rectf();
}

static point m2a(point m, int z) {
	point r;
	r.x = m.x * z;
	r.y = m.y * z;
	return r;
}

static void paint_minimap_items(point origin, int z) {
	rectpush push;
	height = width = z;
	for(auto& e : bsdata<itemground>()) {
		if(!e)
			continue;
		auto i = e.position;
		if(!area.is(i, Explored))
			continue;
		caret.x = origin.x + i.x * width;
		caret.y = origin.y + i.y * height;
		fillfade(color(255, 255, 0), 192);
	}
}

static void paint_minimap_creatures(point origin, int z, bool use_hearing) {
	rectpush push;
	height = width = z;
	for(auto& e : bsdata<creature>()) {
		if(e.worldpos != game)
			continue;
		auto i = e.getposition();
		if(use_hearing) {
			if(player && !player->canhear(e.getposition()))
				continue;
		} else if(!area.is(i, Explored))
			continue;
		caret.x = origin.x + i.x * width;
		caret.y = origin.y + i.y * height;
		if(e.is(Enemy))
			fillfade(color(255, 0, 0), 192);
		else if(e.is(Ally))
			fillfade(color(0, 255, 0), 192);
		else
			fillfade(color(255, 255, 255), 192);
	}
}

static void paint_area(point origin, int z) {
	rectpush push;
	pushvalue push_fore(fore);
	height = width = z;
	point i;
	for(i.y = 0; i.y < area.mps; i.y++) {
		for(i.x = 0; i.x < area.mps; i.x++) {
			auto t = area[i];
			if(!t || !area.is(i, Explored))
				continue;
			auto p = bsdata<tilei>::elements + t;
			caret.x = origin.x + i.x * width;
			caret.y = origin.y + i.y * height;
			fore = p->minimap;
			rectf();
			auto pf = bsdata<featurei>::elements + (int)area.features[i];
			if(pf->isvisible()) {
				color v = pf->minimap; v.a = 0;
				fillfade(v, pf->minimap.a);
			}
		}
	}
}

static void paint_area_screen(point origin, int z) {
	if(!player)
		return;
	auto pc = player->getposition();
	auto s1 = s2m(camera);
	rectpush push;
	pushvalue push_fore(fore);
	caret.x = origin.x + s1.x * z;
	caret.y = origin.y + s1.y * z;
	width = ((draw::getwidth() - panel_width) / tsx + 2) * z;
	height = (draw::getheight() / tsy + 1) * z;
	fore = colors::white;
	rectb();
}

static void paint_logs(const char* format, int& origin, int& format_origin, int& maximum) {
	if(!format)
		return;
	rectpush push;
	if(maximum == -1) {
		rectpush push;
		origin = 0;
		format_origin = 0;
		textfs(format);
		maximum = height;
		height = push.height;
		width = push.width;
		caret = push.caret;
		if(maximum > height) {
			auto push_clip = clipping;
			setclip();
			caret.y -= maximum - height;
			textfs(format, origin, format_origin);
			clipping = push_clip;
		}
	}
	auto push_clip = clipping; setclip();
	caret.y += origin;
	textf(format + format_origin);
	clipping = push_clip;
}

static void text_header(const char* format) {
	auto push_caret = caret;
	auto push_font = font;
	auto push_fore = fore;
	font = metrics::h2;
	fore = colors::h2;
	caret.x += (width - textw(format)) / 2;
	text(format);
	caret = push_caret;
	caret.y += texth();
	font = push_font;
	fore = push_fore;
}

static void small_header(const char* format) {
	auto push_caret = caret;
	auto push_font = font;
	auto push_fore = fore;
	font = metrics::h3;
	fore = colors::h3;
	caret.x += (width - textw(format)) / 2;
	text(format);
	caret = push_caret;
	caret.y += texth();
	font = push_font;
	fore = push_fore;
}

static void pause_keys() {
	if(hot.key == KeySpace || hot.key == KeyEscape)
		execute(buttoncancel);
}

/*static void scene_world() {
	fillwindow();
	paint_world();
	pause_keys();
}*/

static void paint_legends(point origin, int z) {
	auto push_caret = caret;
	auto push_fore = fore;
	auto push_font = font;
	font = metrics::font;
	auto index = 1;
	for(auto& e : bsdata<roomi>()) {
		if(e != game || !e.is(ShowMinimapBullet))
			continue;
		caret.x = origin.x + center(e.rc).x * z + z / 2;
		caret.y = origin.y + center(e.rc).y * z + z / 2;
		fore = colors::white;
		circlef(7);
		fore = colors::black;
		circle(7);
		char temp[260]; stringbuilder sb(temp);
		sb.add("%1i", index);
		caret.y -= texth() / 2;
		caret.x -= textw(temp) / 2;
		text(temp);
		index++;
	}
	font = push_font;
	fore = push_fore;
	caret = push_caret;
}

static void paint_legends_text(point origin) {
	auto push_caret = caret;
	auto push_fore = fore;
	auto push_font = font;
	font = metrics::font;
	caret = origin;
	auto index = 1;
	char temp[260]; stringbuilder sb(temp);
	for(auto& e : bsdata<roomi>()) {
		if(e != game || !e.is(ShowMinimapBullet))
			continue;
		caret.x = origin.x;
		sb.clear(); sb.add("%1i.", index);
		text(temp);
		caret.x += 18;
		text(e.getname());
		caret.y += texth();
		index++;
	}
	font = push_font;
	fore = push_fore;
	caret = push_caret;
}

static void scene_area() {
	fillwindow();
	if(game.level)
		print(text_header, "%1 (%Level %2i)", last_location->getname(), game.level);
	else
		text_header(last_location->getname());
	print(small_header, getnm("GlobalMapPosition"), game.position.x, game.position.y);
	const int z = 4;
	point origin;
	origin.x = 16;
	origin.y = (height - area.mps * z) / 2;
	paint_area(origin, z);
	paint_minimap_creatures(origin, z, true);
	paint_minimap_items(origin, z);
	paint_legends(origin, z);
	paint_legends_text({(short)(16 + area.mps * z + 16), origin.y});
	paint_area_screen(origin, z);
	pause_keys();
}

static void paint_minimap() {
	const auto z = 2;
	paint_area(caret, z);
	paint_minimap_creatures(caret, z, true);
	paint_minimap_items(caret, z);
	paint_area_screen(caret, z);
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
	caret.x = getwidth() - panel_width + 1;
	caret.y = draw::getheight() - 128 - 1;
	paint_minimap();
	caret = push_caret;
	height = push_height;
	width = push_width - panel_width;
}

static void before_paint() {
	message_rect.clear();
	paint_status();
}

void show_area(int bonus) {
	scene(scene_area);
}

void show_logs(int bonus) {
	int maximum = -1, format_origin = 0, origin = 0;
	game.writelog();
	while(ismodal()) {
		paintstart();
		fillwindow();
		setoffset(metrics::padding, metrics::padding);
		paint_logs(actable::getlog(), origin, format_origin, maximum);
		pause_keys();
		paintfinish();
		domodal();
	}
}

static void textcn(const char* format) {
	auto push_caret = caret;
	caret.x -= (textw(format) + 1) / 2;
	caret.y -= texth();
	text(format);
	caret = push_caret;
}

void visualize_images(res pid, point size, point offset) {
	auto p = gres(pid);
	auto origin = 0;
	point d = {(short)(getwidth() / size.x), (short)(getheight() / size.y)};
	auto per_screen = d.x * d.y;
	auto maximum_origin = p->count - per_screen;
	if(maximum_origin < 0)
		maximum_origin = 0;
	while(ismodal()) {
		fillform();
		if(origin < 0)
			origin = 0;
		if(origin > maximum_origin)
			origin = maximum_origin;
		for(auto y = 0; y < d.y; y++) {
			for(auto x = 0; x < d.x; x++) {
				auto i = origin + y * d.x + x;
				caret.x = size.x * x;
				caret.y = size.x * y;
				caret = caret + offset;
				image(p, i, 0);
				print(textcn, "%1i", i);
			}
		}
		paintfinish();
		domodal();
		switch(hot.key) {
		case KeyUp: origin -= d.x; break;
		case KeyPageUp: origin -= per_screen; break;
		case KeyDown: origin += d.x; break;
		case KeyPageDown: origin += per_screen; break;
		case KeyEscape: breakmodal(0); break;
		}
	}
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
	pbackground = before_paint;
	object::beforepaintall = before_paint_all;
	object::afterpaint = object_afterpaint;
	object::afterpaintallpo = object_afterpaintpo;
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