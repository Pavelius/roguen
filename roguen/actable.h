#include "variant.h"

#pragma once

class actable {
	variant			kind;
	short unsigned	name_id;
public:
	variant			getkind() const { return kind; }
	struct monsteri* getmonster() const;
	const char*		getname() const;
	bool			ischaracter() const;
	bool			iskind(variant v) const;
	bool			isnamed() const { return name_id != 0xFFFF; }
	void			setkind(variant v) { kind = v; }
	void			setnoname() { name_id = 0xFFFF; }
	void			setname(unsigned short v) { name_id = v; }
};