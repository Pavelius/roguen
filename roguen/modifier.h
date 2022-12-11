#pragma once

enum modifiers : unsigned char {
	NoModifier, Permanent, 
};
struct modifieri {
	const char*	id;
};
extern modifiers modifier;