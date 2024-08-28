#pragma once

#include "ability.h"
#include "nameable.h"
#include "variant.h"

enum craftn : unsigned char {
	AlchemyCraft, BlacksmithCraft
};
extern craftn last_craft;
struct crafti : nameable {
	ability_s	skill;
	variants	elements;
};
struct receipti {
	unsigned	receipts[BlacksmithCraft + 1];
	unsigned	iscraft(craftn i, int v) const { return (receipts[i] & (1<<v))!=0; }
	void		setcraft(craftn i, int v) { receipts[i] |= (1<<v); }
};
