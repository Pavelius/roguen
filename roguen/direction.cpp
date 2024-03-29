#include "direction.h"

point all_directions[] = {
	{0, -1}, {1, 0}, {0, 1}, {-1, 0},
	{1, -1}, {1, 1}, {-1, 1}, {-1, -1},
};

direction_s round(direction_s d, direction_s v) {
	switch(v) {
	case NorthEast:
		switch(d) {
		case North: return NorthEast;
		case NorthEast: return East;
		case East: return SouthEast;
		case SouthEast: return South;
		case South: return SouthWest;
		case SouthWest: return West;
		case West: return NorthWest;
		case NorthWest: return North;
		default: return d;
		}
		break;
	case NorthWest:
		switch(d) {
		case North: return NorthWest;
		case NorthWest: return West;
		case West: return SouthWest;
		case SouthWest: return South;
		case South: return SouthEast;
		case SouthEast: return East;
		case East: return NorthEast;
		case NorthEast: return North;
		default: return d;
		}
		break;
	case East:
		switch(d) {
		case North: return East;
		case East: return South;
		case South: return West;
		case West: return North;
		default: return d;
		}
		break;
	case West:
		switch(d) {
		case North: return West;
		case West: return South;
		case South: return East;
		case East: return North;
		default: return d;
		}
		break;
	case South:
		switch(d) {
		case North: return South;
		case West: return East;
		case South: return North;
		case East: return West;
		default: return d;
		}
		break;
	default:
		return d;
	}
}

point to (point m, direction_s d) {
	return m + all_directions[d];
}