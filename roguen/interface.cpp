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

void set_dark_theme();
void initialize_translation(const char* locale);
void initialize_png();

point m2s(point v) {
	return point{(short)(v.x * tsx), (short)(v.y * tsy)};
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

void creature::paint() const {
	auto flags = mirror ? ImageMirrorH : 0;
	if(kind.iskind<monsteri>()) {
		auto pi = gres(res::Monsters);
		image(pi, kind.value, flags);
	} else {
		auto pb = gres(res::PCBody);
		auto pa = gres(res::PCArms);
		auto pc = gres(res::PCAccessories);
		if(wears[RangedWeapon]) // Missile
			image(pc, wears[RangedWeapon].getavatar(), 0);
		image(pc, 1, 0); // Cloack
		if(wears[MeleeWeapon].is(TwoHanded)) {
			image(pa, 9, 0); // Arm
			image(pa, 4, 0); // Left Arms
		} else {
			image(pa, 10 + wears[MeleeWeapon].getavatar(), 0); // Arm
			image(pa, 36 + wears[MeleeWeaponOffhand].getavatar(), 0); // Left Arms
		}
		image(pb, wears[Torso].getavatar(), 0); // Body
		image(pc, 3, 0); // Throwing
		image(pa, 4, 0); // Left Arms
	}
}

void featurei::paint(int r) const {
	auto pi = gres(res::Features);
	image(pi, features.get(r), 0);
	if(overlay)
		image(pi, overlay.get(r >> 4), 0);
}

static void object_afterpaint(const object* p) {
	if(bsdata<creature>::have(p->data))
		((creature*)p->data)->paint();
	else if(bsdata<featurei>::have(p->data))
		((featurei*)p->data)->paint(p->random);
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
}

static void presskey(const slice<hotkey>& source) {
	for(auto& e : source) {
		if(hot.key == e.key) {
			execute(e.proc);
			return;
		}
	}
}

static void move_left() {
	player->movestep(West);
	adventure_mode();
}

static void move_right() {
	player->movestep(East);
	adventure_mode();
}

static void move_up() {
	player->movestep(North);
	adventure_mode();
}

static void move_down() {
	player->movestep(South);
	adventure_mode();
}

static void move_up_left() {
	player->movestep(NorthWest);
	adventure_mode();
}

static void move_up_right() {
	player->movestep(NorthEast);
	adventure_mode();
}

static void move_down_left() {
	player->movestep(SouthWest);
	adventure_mode();
}

static void move_down_right() {
	player->movestep(SouthEast);
	adventure_mode();
}

static void attack_forward() {
	player->fixaction();
	adventure_mode();
}

static hotkey adventure_keys[] = {
	{"AttackForward", 'A', attack_forward},
	{"MoveDown", KeyDown, move_down},
	{"MoveDownLeft", KeyEnd, move_down_left},
	{"MoveDownRight", KeyPageDown, move_down_right},
	{"MoveLeft", KeyLeft, move_left},
	{"MoveRight", KeyRight, move_right},
	{"MoveUp", KeyUp, move_up},
	{"MoveUpRight", KeyPageUp, move_up_right},
	{"MoveUpLeft", KeyHome, move_up_left},
};

static void run_adventure_mode() {
	waitall();
	while(ismodal()) {
		paintstart();
		paintobjects();
		presskey(adventure_keys);
		paintfinish();
		domodal();
	}
}

void adventure_mode() {
	setnext(run_adventure_mode);
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
	object::afterpaint = object_afterpaint;
	//answers::paintcell = menubutton;
	//answers::beforepaint = menubeforepaint;
	draw::width = 640;
	draw::height = 360;
	initialize(getnm("AppTitle"));
	setnext(proc);
	start();
	return 0;
}