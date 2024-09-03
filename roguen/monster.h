#include "ability.h"
#include "nameable.h"
#include "randomizer.h"
#include "variant.h"

#pragma once

struct itemi;

struct monsteri : nameable, statable {
	unsigned short	avatar;
	unsigned char	appear;
	variants		use;
	monsteri*		parent;
	randomizeri*	minions;
	itemi*			rest;
	monsteri*		ally() const;
	bool			islower(ability_s v, int value) const;
	const monsteri& getbase() const { return parent ? parent->getbase() : *this; }
};
bool is_boss(const void* p);
void check_monsters();