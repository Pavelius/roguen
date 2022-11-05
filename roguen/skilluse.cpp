#include "main.h"

BSDATAD(skilluse)

skilluse* skilluse::find(ability_s v, short unsigned player_id, short unsigned room_id) {
	if(player_id == 0xFFFF || room_id == 0xFFFF)
		return 0;
	for(auto& e : bsdata<skilluse>()) {
		if(e.ability == v && e.player_id == player_id && e.room_id == room_id)
			return &e;
	}
	return 0;
}

skilluse* skilluse::add(ability_s v, short unsigned player_id, short unsigned room_id) {
	if(player_id == 0xFFFF || room_id == 0xFFFF)
		return 0;
	auto p = find(v, player_id, room_id);
	if(!p)
		p = add(v, player_id, room_id);
	if(!p)
		return 0;
	p->stamp = game.getminutes();
	return p;
}