#include "crt.h"
#include "fraction.h"

BSDATAC(fractioni, fraction_maximum)

int	fractioni::getindex() const {
	return this - bsdata<fractioni>::elements;
}

static int index_realation[fraction_maximum * fraction_maximum];
static int start_realation[fraction_lenght];
static int current_realation[fraction_lenght];

void initialize_fractions() {
	for(auto& e : index_realation)
		e = -1;
	int partial_index = 0;
	for(auto f1 = 0; f1 < fraction_maximum; f1++) {
		for(auto f2 = 0; f2 < fraction_maximum; f2++) {
			if(f1 == f2)
				continue;
			if(index_realation[f1 * fraction_maximum + f2] != -1 || index_realation[f2 * fraction_maximum + f1] != -1)
				continue;
			index_realation[f1 * fraction_maximum + f2] = partial_index;
			partial_index++;
		}
	}
}

static int get_index(int f1, int f2) {
	if(f1 > f2)
		iswap(f1, f2);
	return index_realation[f1 * fraction_maximum + f2];
}

void add_relation(int f1, int f2, int value) {
	auto i = get_index(f1, f2);
	if(i == -1)
		return;
	current_realation[i] += value;
}

int get_relation(int f1, int f2) {
	auto i = get_index(f1, f2);
	if(i == -1)
		return 0;
	return current_realation[i];
}

void update_relations() {
	for(auto i = 0; i < fraction_lenght; i++) {
		auto v = current_realation[i];
		auto s = start_realation[i];
		if(v < s) {
			auto n = current_realation[i] + 1;
			if(n > s)
				n = s;
			current_realation[i] = n;
		} else {
			auto n = current_realation[i] - 1;
			if(n < s)
				n = s;
			current_realation[i] = n;
		}
	}
}