#pragma once

struct rollrange {
	int n1, n2;
	int roll() const;
	int maximum() const { return n2; }
	int minimum() const { return n1; }
	void add(int bonus);
	void normalize();
};
