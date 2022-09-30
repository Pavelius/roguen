#include "anymap.h"
#include "direction.h"

point to(point m, direction_s v, unsigned char mps) {
	static const point error = {short(-1), short(-1)};
	if(m.x < 0)
		return error;
	switch(v) {
	case North:
		if(m.y == 0)
			return error;
		m.y--;
		break;
	case NorthWest:
		if(m.y == 0 || m.x == 0)
			return error;
		m.x--; m.y--;
		break;
	case NorthEast:
		if(m.y == 0 || m.x >= mps - 1)
			return error;
		m.x++; m.y--;
		break;
	case South:
		if(m.y >= mps - 1)
			return error;
		m.y++;
		break;
	case SouthWest:
		if(m.y >= mps - 1 || m.x == 0)
			return error;
		m.x--; m.y++;
		break;
	case SouthEast:
		if(m.y >= mps - 1 || m.x >= mps - 1)
			return error;
		m.x++; m.y++;
		break;
	case West:
		if(m.x == 0)
			return error;
		m.x--;
		break;
	case East:
		if(m.x >= mps - 1)
			return error;
		m.x++;
		break;
	default:
		break;
	}
	return m;
}