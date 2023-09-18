#pragma once

enum modifiers : unsigned char {
	NoModifier, Permanent, InPlayerBackpack
};
struct modifieri {
	const char*	id;
};
extern modifiers modifier;