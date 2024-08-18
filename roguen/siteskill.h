#include "collection.h"
#include "nameable.h"
#include "variant.h"

#pragma once

enum ability_s : unsigned char;
enum duration_s : unsigned char;
struct sitei;

struct siteskilli : nameable {
	ability_s		skill;
	char			bonus;
	duration_s		retry;
	variants		effect;
	unsigned		key;
	const char*		keyid;
	variant			tool;
	bool			isusable() const;
	bool			ishotkeypresent() const;
	void			fixuse() const;
	void			usetool();
};
typedef collection<siteskilli> siteskilla;
extern siteskilla last_actions;
extern siteskilli* last_action;

void site_skills_initialize();