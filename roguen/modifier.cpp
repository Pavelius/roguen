#include "modifier.h"
#include "script.h"

BSDATA(modifieri) = {
	{"NoModifier"},
	{"InPlayerBackpack"},
	{"InPosition"},
	{"InRoomToBuy"},
	{"InRoomToBuySpecial"},
};
assert_enum(modifieri, InRoomToBuySpecial)

short get_token(modifiern v) {
	switch(v) {
	case InRoomToBuy: return -1001;
	case InRoomToBuySpecial: return -1002;
	default: return -1000;
	}
}