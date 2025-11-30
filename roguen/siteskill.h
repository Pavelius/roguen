#include "collection.h"
#include "nameable.h"
#include "variant.h"

#pragma once

enum abilityn : unsigned char;
struct sitei;

struct siteskilli : nameable {
	abilityn		skill;
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