#pragma once

enum direction_s : unsigned char {
	North, East, South, West,
	NorthEast, SouthEast, SouthWest, NorthWest
};
direction_s round(direction_s d, direction_s v);
