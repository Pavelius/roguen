#include "main.h"

bool siteskilli::isvalid(const void* object) {
	auto p = (siteskilli*)object;
	return p->site == last_site
		&& player->get(p->skill) != 0;
}