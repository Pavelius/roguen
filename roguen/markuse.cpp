#include "markuse.h"

BSDATAD(markuse)

markuse* markuse_find(variant v, point position, short unsigned player_id) {
	if(player_id == 0xFFFF)
		return 0;
	for(auto& e : bsdata<markuse>()) {
		if(e.player_id == player_id && e.ability == v && e.position == position)
			return &e;
	}
	return 0;
}

markuse* markuse_add(variant v, point position, short unsigned player_id, unsigned stamp) {
	if(player_id == 0xFFFF)
		return 0;
	auto p = markuse_find(v, position, player_id);
	if(!p)
		p = bsdata<markuse>::add();
	if(!p)
		return 0;
	p->ability = v;
	p->player_id = player_id;
	p->position = position;
	p->stamp = stamp;
	return p;
}

bool markused(variant v, point position, short unsigned player_id) {
	return markuse_find(v, position, player_id) != 0;
}