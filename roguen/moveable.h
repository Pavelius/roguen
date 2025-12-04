#include "actable.h"
#include "point.h"

#pragma once

enum abilityn : unsigned char;
enum color_s : unsigned char;
enum directionn : unsigned char;

const int tsx = 64;
const int tsy = 48;
const int mst = 260;

inline point m2s(point v) { return point{(short)(v.x * tsx), (short)(v.y * tsy)}; }
inline point s2m(point v) { return point{(short)(v.x / tsx), (short)(v.y / tsy)}; }

typedef void(*fnevent)(); // Callback function of any command executing

class movable : public actable {
	point			position;
	directionn		direction;
	bool			mirror;
	void			fixvalue(void* data, fnevent proc, color_s color) const;
public:
	void			fixactivity() const;
	void			fixappear(fnevent fpaint) const;
	void			fixability(abilityn i, int v) const;
	void			fixdisappear() const;
	void			fixeffect(const char* id) const;
	static void		fixeffect(point position, const char* id);
	void			fixmovement() const;
	void			fixremove() const;
	void			fixshoot(point target, int frame) const;
	void			fixteleport(bool ishuman) const;
	void			fixthrown(point target, const char* id, int frame) const;
	void			fixvalue(const char* v, color_s color) const;
	void			fixvalue(int v) const;
	void			fixvalue(int v, color_s color) const;
	void			fixvalue(int v, color_s color_positive, color_s color_negative) const;
	bool			in(const rect& rc) const { return position.in(rc); }
	bool			ismirror() const { return mirror; }
	point			getposition() const { return position; }
	point			getsposition() const { return m2s(position); }
	void			setdirection(directionn v);
	void			setposition(point m) { position = m; }
};
