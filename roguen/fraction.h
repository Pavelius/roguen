#pragma once
#include "nameable.h"

enum {
	PlayerFraction,
};
struct fractioni : nameable {
	int			getindex() const;
};

const int fraction_maximum = 32;
const int fraction_lenght = (fraction_maximum - 1) * fraction_maximum / 2;

void add_relation(int f1, int f2, int value);
void initialize_fractions();
void normalize_relations(int maximum);