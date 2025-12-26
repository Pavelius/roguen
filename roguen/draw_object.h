#include "color.h"
#include "adat.h"
#include "point.h"

#pragma once

struct drawobject;

struct drawable {
	point			position;
	unsigned char	alpha, priority, param, flags;
	constexpr explicit operator bool() const { return priority != 0; }
	void			clear();
};
struct draworder : drawable {
	drawable*		parent;
	drawable		start;
	draworder*		depend;
	bool			cleanup;
	unsigned long	tick_start, tick_stop;
	explicit operator bool() const { return parent != 0; }
	draworder*		add(int milliseconds = 1000, bool cleanup = false);
	void			clear();
	void			setduration(int v) { tick_stop = tick_start + v; }
	void			update();
	void			wait();
};
struct drawobject : drawable {
	const void*		data;
	fnevent			proc;
	static fnevent	correctcamera, beforepaint, afterpaint;
	draworder*		add(int milliseconds = 1000, draworder* depend = 0, bool cleanup = false);
	void			clear();
	void			disappear(int milliseconds);
	void			move(point goal, int speed, int correct = 0);
	void			paint() const;
};
extern drawobject* last_object;
extern rect last_screen, last_area;
extern adat<drawobject*, 512> objects;

drawobject*	addobject(point pt, fnevent proc, void* data, unsigned char param, unsigned char priority = 10, unsigned char alpha = 0xFF, unsigned char flags = 0);

void*				chooseobject();

drawobject*	findobject(const void* p);
drawobject*	findobject(point pt, fnevent proc);

unsigned long getobjectstamp();

bool cameravisible(point goal, int border = 48);
void clearobjects();
void focusing(point goal);
void paintobjects();
void removeobjects(const array& source);
void shrink();
void setcamera(point v);
void slidecamera(point v, int step = 16);
void splashscreen(unsigned milliseconds);
void showobjects();
void waitall();

template<typename T> void ftpaint() { ((T*)last_object->data)->paint(); }