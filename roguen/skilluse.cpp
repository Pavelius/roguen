#include "skilluse.h"

BSDATAD(skilluse)

skilluse* skilluse::find(variant v, point position, short unsigned player_id) {
	if(player_id == 0xFFFF)
		return 0;
	for(auto& e : bsdata<skilluse>()) {
		if(e.player_id == player_id && e.ability == v && e.position == position)
			return &e;
	}
	return 0;
}

skilluse* skilluse::add(variant v, point position, short unsigned player_id, unsigned stamp) {
	if(player_id == 0xFFFF)
		return 0;
	auto p = find(v, position, player_id);
	if(!p)
		p = bsdata<skilluse>::add();
	if(!p)
		return 0;
	p->ability = v;
	p->player_id = player_id;
	p->position = position;
	p->stamp = stamp;
	return p;
}