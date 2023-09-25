#pragma once

enum modifiers : unsigned char {
	NoModifier, InPlayerBackpack
};
struct modifieri {
	const char*	id;
};
extern modifiers modifier;