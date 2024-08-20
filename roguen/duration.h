#pragma once

struct durationi;

enum duration_s : unsigned char {
	Instant,
	Minutes, Hours, Days
};

int get_duration(duration_s v, int additional = 0);
