#include "dice.h"
#include "rand.h"

int dice::roll() const {
	int v = min;
	if(min < max)
		v += rand() % (max - min + 1);
	return v;
}

void dice::correct() {
	if(min < 0)
		min = 1;
	if(max < min)
		max = min;
}