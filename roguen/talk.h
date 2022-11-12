#include "nameable.h"
#include "flagable.h"
#include "variant.h"

#pragma once

struct phrasei {
	short			index, next;
	variants		elements;
	const char*		text;
	void			clear() { memset(this, 0, sizeof(*this)); }
	const phrasei*	nextanswer() const;
	bool			isanswer() const { return next != -1; }
};
struct talki : nameable {
	sliceu<phrasei>	elements;
	flagable<16>	visited;
	void			clear() { memset(this, 0, sizeof(*this)); }
	bool			isvisited(short v) const { return visited.is(v); }
	phrasei*		find(short v) const;
	static talki*	owner(const phrasei* p);
	static void		read(const char* url);
	static void		read();
	void			setvisit(short v) { visited.set(v); }
};