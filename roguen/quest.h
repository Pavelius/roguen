#include "nameable.h"
#include "point.h"
#include "variant.h"

#pragma once

enum questn : unsigned char {
	KillBossQuest, RescueQuest
};
struct questni : nameable {};

struct quest {
	point			position;
	questn			type;
	variant			object, problem, reward, twist, level, final_level, modifier, entrance;
	char			rumor;
	constexpr operator bool() const { return problem.operator bool(); }
	static quest*	add(point position);
	static quest*	add(point position, variant modifier, variant type, variant reward);
	void			clear() { zclear(this); }
	static quest*	find(point v);
};
