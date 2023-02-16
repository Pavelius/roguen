#include "ability.h"
#include "nameable.h"
#include "randomizer.h"
#include "variant.h"

#pragma once

struct monsteri : nameable, statable {
	unsigned short	avatar;
	char			friendly;
	bool			unique;
	variants		use;
	monsteri*		parent;
	randomizeri*	minions;
	variants		treasure;
	monsteri*		ally() const;
	const monsteri& getbase() const { return parent ? parent->getbase() : *this; }
	static bool		isboss(const void* p) { return ((monsteri*)p)->minions != 0; }
};