#include "crt.h"
#include "dice.h"

int dice::roll() const {
	return min + ((max == min) ? 0 : (rand() % (max - min + 1)));
}

void dice::correct() {
	if(min < 0)
		min = 1;
	if(max < min)
		max = min;
}