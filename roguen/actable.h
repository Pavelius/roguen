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
	void			sayv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female) const;
	void			setkind(variant v) { kind = v; }
	void			setnoname() { name_id = 0xFFFF; }
	void			setname(unsigned short v) { name_id = v; }
};
void				actv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female, char separator);
void				actvf(stringbuilder& sb, const char* name, bool female, char separator, const char* format, ...);
const char*			getlog();
void				logv(const char* format, const char* format_param, const char* name, bool female);
void				logv(const char* format);
