#include "color.h"
#include "draw.h"
#include "point.h"

#pragma once

namespace draw {
struct object;
typedef void(*fnpaint)(const object* pointer);
struct drawable {
	point			position;
	color			fore;
	unsigned char	alpha;
};
struct draworder : drawable {
	object*			parent;
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
	enum type_s : unsigned char {
		NoObject, Background, Object, Overlay
	};
	const void*		data;
	const char*		string;
	unsigned char	priority, random;
	type_s			type;
	constexpr explicit operator bool() const { return data != 0 || string != 0; }
	static fnevent	beforepaintall, afterpaintall, correctcamera;
	static fnpaint	afterpaint, afterpaintallpo;
	draworder*		add(int milliseconds = 1000, draworder* depend = 0);
	void			clear();
	void			disappear(int milliseconds);
	void			move(point goal, int speed, int correct = 0);
	void			paint() const;
	void			paintns() const;
};
object*				addobject(point pt);
bool				cameravisible(point goal, int border = 48);
void*				chooseobject();
void				shrink();
void				clearobjects();
void				focusing(point goal);
object*				findobject(const void* p);
void				paintobjects();
unsigned long		getobjectstamp();
void				setcamera(point v);
void				slidecamera(point v, int step = 16);
void				splashscreen(unsigned milliseconds);
void				showobjects();
void				waitall();
}