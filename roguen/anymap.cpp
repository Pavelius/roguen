#include "anymap.h"
#include "direction.h"

point to(point m, direction_s v, unsigned char mps) {
	if(m.x<0)
		return m;
	switch(v) {
	case North:
		if(m.y == 0)
			return point{(short)-1, (short)-1};
		return point{m.x, m.y - 1};
	case NorthWest:
		if(m.y == 0 || m.x == 0)
			return point{(short)-1, (short)-1};
		return point{short(m.x - 1), (short)(m.y - 1)};
	case NorthEast:
		if(m.y == 0 || m.x >= mps - 1)
			return point{(short)-1, (short)-1};
		return point{short(m.x + 1), short(m.y - 1)};
	case South:
		if(m.y >= mps - 1)
			return point{(short)-1, (short)-1};
		return point{m.x, short(m.y + 1)};
	case SouthWest:
		if(m.y >= mps - 1 || m.x == 0)
			return point{(short)-1, (short)-1};
		return point{short(m.x - 1), (short)(m.y + 1)};
	case SouthEast:
		if(m.y >= mps - 1 || m.x >= mps - 1)
			return point{(short)-1, (short)-1};
		return point{short(m.x + 1), short(m.y + 1)};
	case West:
		if(m.x == 0)
			return point{(short)-1, (short)-1};
		return point{short(m.x - 1), m.y};
	case East:
		if(m.x >= mps - 1)
			return point{(short)-1, (short)-1};
		return point{short(m.x + 1), m.y};
	default:
		return m;
	}
}