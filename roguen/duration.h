#pragma once

struct durationi;

enum duration_s : unsigned char {
	Instant,
	Minute10, Minute10PL,
	Minute20, Minute20PL,
	Minute30,
	Hour1, Hour2, Hour4, Hour1PL,
	Day1,
};

int get_duration(duration_s v, int level = 0);
