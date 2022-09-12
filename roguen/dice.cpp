#include "crt.h"
#include "dice.h"

int dice::roll() const {
	int r = b;
	for(auto i = c; i >= 0; i--)
		r += 1 + rand() % d;
	return r;
}