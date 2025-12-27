#include "rand.h"
#include "rollrange.h"

int rollrange::roll() const {
	return xrand(n1, n2);
}

void rollrange::normalize() {
	if(n1 < 0)
		n1 = 0;
	if(n1 > n2)
		n2 = n1;
}

void rollrange::add(int bonus) {
	n1 += bonus;
	n2 += bonus;
	normalize();
}