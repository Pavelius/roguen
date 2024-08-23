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
extern const phrasei* last_phrase;
struct talki : nameable {
	sliceu<phrasei>	elements;
	flagable<16>	visited;
	void			clear() { memset(this, 0, sizeof(*this)); }
	bool			isvisited(short v) const { return visited.is(v); }
	phrasei*		find(short v) const;
	void			setvisit(short v) { visited.set(v); }
};
talki* find_talk(const phrasei* p);

void read_talk();
void read_talk(const char* url);