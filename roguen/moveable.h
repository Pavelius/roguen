#include "actable.h"
#include "point.h"

#pragma once

enum direction_s : unsigned char;
enum ability_s : unsigned char;

const int tsx = 64;
const int tsy = 48;
const int mst = 260;

inline point		m2s(point v) { return point{(short)(v.x * tsx), (short)(v.y * tsy)}; }
inline point		s2m(point v) { return point{(short)(v.x / tsx), (short)(v.y / tsy)}; }

class movable : public actable {
	point			position;
	direction_s		direction;
	bool			mirror;
public:
	void			fixaction() const;
	void			fixappear() const;
	void			fixability(ability_s i, int v) const;
	void			fixdisappear() const;
	void			fixeffect(const char* id) const;
	static void		fixeffect(point position, const char* id);
	void			fixmovement() const;
	void			fixremove() const;
	void			fixshoot(point target, int frame) const;
	void			fixteleport(bool ishuman) const;
	void			fixthrown(point target, const char* id, int frame) const;
	void			fixvalue(const char* v, int color = 0) const;
	void			fixvalue(int v) const;
	void			fixvalue(int v, int color_positive, int color_negative = -1) const;
	bool			in(const rect& rc) const { return position.in(rc); }
	bool			ismirror() const { return mirror; }
	point			getposition() const { return position; }
	point			getsposition() const { return m2s(position); }
	void			setdirection(direction_s v);
	void			setposition(point m) { position = m; }
};
