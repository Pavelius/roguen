#pragma once

#include "nameable.h"
#include "flagable.h"
#include "variant.h"

typedef void(*fncommand)(void* object); // Callback function of object command executing

struct phrasei {
	short			index, next;
	variants		elements;
	const char*		text;
	void			clear() { memset((void*)this, 0, sizeof(*this)); }
	const phrasei*	nextanswer() const;
	bool			isanswer() const { return next != -1; }
};
extern const phrasei* last_phrase;
struct talki : nameable {
	sliceu<phrasei>	elements;
	flagable<16>	visited;
	void			clear() { memset((void*)this, 0, sizeof(*this)); }
	bool			isvisited(short v) const { return visited.is(v); }
	phrasei*		find(short v) const;
	void			setvisit(short v) { visited.set(v); }
};
talki* find_talk(const phrasei* p);

void read_talk(const char* url);
bool talk_opponent(const char* id, fncommand proc);