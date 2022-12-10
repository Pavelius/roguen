#pragma once

enum modifiers : unsigned char {
	NoModifier, OneOf, 
};
struct modifieri {
	const char*	id;
};
extern modifiers modifier;