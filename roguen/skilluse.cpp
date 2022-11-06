#include "main.h"

BSDATAD(skilluse)

skilluse* skilluse::find(variant v, short unsigned player_id, short unsigned room_id) {
	if(player_id == 0xFFFF || room_id == 0xFFFF)
		return 0;
	for(auto& e : bsdata<skilluse>()) {
		if(e.ability == v && e.player_id == player_id && e.room_id == room_id)
			return &e;
	}
	return 0;
}

skilluse* skilluse::add(variant v, short unsigned player_id, short unsigned room_id) {
	if(player_id == 0xFFFF || room_id == 0xFFFF)
		return 0;
	auto p = find(v, player_id, room_id);
	if(!p)
		p = bsdata<skilluse>::add();
	if(!p)
		return 0;
	p->ability = v;
	p->player_id = player_id;
	p->room_id = room_id;
	p->stamp = game.getminutes();
	return p;
}