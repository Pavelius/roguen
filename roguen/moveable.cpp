#include "main.h"

void movable::setposition(point v) {
	position = v;
}

void movable::setdirection(direction_s v) {
	direction = v;
	switch(v) {
	case West: case NorthWest: case SouthWest: mirror = true; break;
	case East: case NorthEast: case SouthEast: mirror = false; break;
	}
}