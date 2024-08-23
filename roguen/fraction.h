#pragma once
#include "nameable.h"

const int fraction_maximum = 32;
const int fraction_lenght = (fraction_maximum - 1) * fraction_maximum / 2;

struct fractioni : nameable {
	int			getindex() const;
};
extern fractioni* last_fraction;

void add_relation(int f1, int f2, int value);
void initialize_fractions();
void update_relations();