#pragma once

struct sprite;
struct resource {
	const char*		name;
	const sprite*	data;
	bool			notfound;
	const sprite*	get();
};
