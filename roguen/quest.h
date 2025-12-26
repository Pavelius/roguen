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
	variant			drawobject, problem, reward, twist, level, final_level, modifier, entrance;
	char			rumor;
	constexpr operator bool() const { return problem.operator bool(); }
	void			clear();
};
extern quest* last_quest;

void add_quest(questn type, point position);
void add_quest(questn type, point position, variant modifier, variant level, variant reward);

quest* find_quest(point v);
