#include "ability.h"
#include "areaf.h"
#include "areamap.h"
#include "direction.h"
#include "draw.h"
#include "draw_object.h"
#include "moveable.h"
#include "screenshoot.h"
#include "visualeffect.h"

extern areamap area;

using namespace draw;

static object* add_object(point pt, void* data, unsigned char random, unsigned char priority = 10) {
	auto po = addobject(pt);
	po->data = data;
	po->priority = priority;
	po->random = random;
	po->alpha = 0xFF;
	return po;
}

static color getcolor(color_s format_color) {
	switch(format_color) {
	case ColorRed: return colors::red;
	case ColorGreen: return colors::green;
	case ColorBlue: return colors::blue;
	case ColorYellow: return colors::yellow;
	default: return colors::text;
	}
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

void movable::setdirection(direction_s v) {
	direction = v;
	switch(v) {
	case West: case NorthWest: case SouthWest: mirror = true; break;
	case East: case NorthEast: case SouthEast: mirror = false; break;
	}
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

void movable::fixaction() const {
	if(!area.is(position, Visible))
		return;
	auto po = draw::findobject(this);
	if(!po)
		return;
	auto pr = po->add(mst / 2);
	pr->position = to(po->position, direction, tsx / 8, tsy / 8);
	pr = pr->add(mst / 2);
	pr->position = po->position;
}

void movable::fixremove() const {
	auto po = draw::findobject(this);
	if(po)
		po->clear();
}

void movable::fixvalue(const char* format, color_s format_color) const {
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

void movable::fixvalue(int v) const {
	fixvalue(v, ColorGreen, ColorRed);
}

void movable::fixvalue(int v, color_s color_positive, color_s color_negative) const {
	char temp[260]; stringbuilder sb(temp);
	sb.add("%1i", v);
	if(color_negative == ColorNone)
		color_negative = color_positive;
	fixvalue(temp, v > 0 ? color_positive : color_negative);
}

void movable::fixability(ability_s i, int v) const {
	if(!area.is(position, Visible))
		return;
	char temp[260]; stringbuilder sb(temp);
	sb.add("%1%+2i", bsdata<abilityi>::elements[i].getname(), v);
	fixvalue(temp, v > 0 ? ColorGreen : ColorRed);
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
	auto po = add_object(getsposition(), pe, frame);
	po->position.y += pe->dy;
	if(!po)
		return;
	auto pr = po->add(mst * range / 3);
	pr->position = m2s(target);
	pr->position.y += pe->dy;
}

static void paint_black_screen() {
	rectpush push;
	width = getwidth();
	height = getheight();
	caret.clear();
	fillwindow();
}

static void paint_main_scene() {
	rectpush push;
	paintstart();
	paintobjects();
}

void movable::fixteleport(bool ishuman) const {
	auto po = draw::findobject(this);
	if(po) {
		if(ishuman) {
			paint_main_scene();
			screenshoot::fade(paint_black_screen, 1000);
			po->position = getsposition();
			screenshoot::fade(paint_main_scene, 1000);
		} else
			po->position = getsposition();
	}
}