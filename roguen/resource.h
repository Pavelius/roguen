#pragma once

struct sprite;
struct resource {
	const char*		name;
	sprite*			data;
	bool			notfound;
};
