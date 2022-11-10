#include "variant.h"

#pragma once

class actable {
	variant			kind;
	short unsigned	name_id;
public:
	static void		actv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female, char separator);
	static void		actvf(stringbuilder& sb, const char* name, bool female, char separator, const char* format, ...);
	variant			getkind() const { return kind; }
	static const char* getlog();
	struct monsteri* getmonster() const;
	const char*		getname() const;
	bool			ischaracter() const;
	bool			iskind(variant v) const;
	static void		logv(const char* format, const char* format_param, const char* name, bool female);
	static void		pressspace();
	void			sayv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female) const;
	void			setkind(variant v) { kind = v; }
	void			setnoname() { name_id = 0xFFFF; }
	void			setname(unsigned short v) { name_id = v; }
};

