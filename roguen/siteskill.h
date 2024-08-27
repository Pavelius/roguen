#include "collection.h"
#include "nameable.h"
#include "variant.h"

#pragma once

enum ability_s : unsigned char;
struct sitei;

struct siteskilli : nameable {
	ability_s		skill;
	char			bonus;
	variants		effect;
	unsigned		key;
	const char*		keyid;
	variant			tool;
	bool			isusable() const;
	bool			ishotkeypresent() const;
	void			usetool();
};
typedef collection<siteskilli> siteskilla;
extern siteskilla last_actions;
extern siteskilli* last_action;

void check_site_skills();