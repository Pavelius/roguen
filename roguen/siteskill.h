#include "collection.h"
#include "nameable.h"
#include "variant.h"

#pragma once

enum ability_s : unsigned char;
enum duration_s : unsigned char;
struct sitei;

struct siteskilli : nameable {
	const sitei*	site;
	ability_s		skill;
	char			bonus;
	duration_s		retry;
	variants		effect, fail;
	static bool		isvalid(const void* object);
};
typedef collection<siteskilli> siteskilla;