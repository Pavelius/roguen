#include "color.h"
#include "crt.h"
#include "point.h"

#pragma once

typedef void(*fnevent)();

namespace draw {
struct object;
typedef void(*fnpaint)(const object* pointer);
struct drawable {
	enum type_s : unsigned char {
		None, Background, Object, Overlay
	};
	point			position;
	unsigned char	alpha, priority, random;
	type_s			type;
	color			fore;
	constexpr explicit operator bool() const { return type != None; }
	void			clear();
};
struct draworder : drawable {
	drawable*		parent;
	drawable		start;
	draworder*		depend;
	unsigned long	tick_start, tick_stop;
	explicit operator bool() const { return parent != 0; }
	draworder*		add(int milliseconds = 1000);
	void			clear();
	void			setduration(int v) { tick_stop = tick_start + v; }
	void			update();
	void			wait();
};
struct object : drawable {
	const void*		data;
	const char*		string;
	static fnevent	correctcamera, beforepaint, afterpaint;
	static fnpaint	painting;
	draworder*		add(int milliseconds = 1000, draworder* depend = 0);
	void			clear();
	void			disappear(int milliseconds);
	void			move(point goal, int speed, int correct = 0);
	void			paint() const;
};
extern rect last_screen, last_area;
extern adat<object*, 512> objects;
object*				addobject(point pt, object::type_s type = object::Object);
bool				cameravisible(point goal, int border = 48);
void*				chooseobject();
void				shrink();
void				clearobjects();
void				focusing(point goal);
object*				findobject(const void* p);
void				paintobjects();
void				removeobjects(const array& source);
unsigned long		getobjectstamp();
void				setcamera(point v);
void				slidecamera(point v, int step = 16);
void				splashscreen(unsigned milliseconds);
void				showobjects();
void				waitall();
}