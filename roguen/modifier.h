#pragma once

enum modifiern : unsigned char {
	NoModifier, InPlayerBackpack, InPosition, InRoomToBuy, InRoomToBuySpecial,
};
short get_token(modifiern v);