#include "collection.h"
#include "nameable.h"
#include "variant.h"

#pragma once

enum ability_s : unsigned char;
enum duration_s : unsigned char;
struct sitei;

struct siteskilli : nameable {
	ability_s		skill;
	unsigned		target;
	char			bonus;
	duration_s		retry;
	variants		conditions;
	variants		effect, fail;
	void			apply() const;
	bool			isusable() const;
};
typedef collection<siteskilli> siteskilla;
extern siteskilla last_actions;
extern siteskilli* last_action;