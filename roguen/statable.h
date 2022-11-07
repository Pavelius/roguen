#pragma once

enum ability_s : unsigned char;

struct statable {
	char		abilities[64];
	void		add(ability_s i, int v = 1) { abilities[i] += v; }
};
