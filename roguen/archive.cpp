#include "archive.h"

bool archive::signature(const char* id) {
	char temp[4];
	if(writemode) {
		memset(temp, 0, sizeof(temp));
		zcpy(temp, id, sizeof(temp) - 1);
		set(temp, sizeof(temp));
	} else {
		set(temp, sizeof(temp));
		if(szcmpi(temp, id) != 0)
			return false;
	}
	return true;
}

bool archive::version(short major, short minor) {
	short major_reader = major;
	short minor_reader = minor;
	set(major_reader);
	set(minor_reader);
	if(!writemode) {
		if(major_reader < major)
			return false;
		else if(major_reader == major && minor_reader < minor)
			return false;
	}
	return true;
}

void archive::set(array& v) {
	set(v.count);
	set(v.size);
	if(!writemode)
		v.reserve(v.count);
	set(v.data, v.size * v.count);
}

void archive::set(void* value, unsigned size) {
	if(writemode)
		source.write(value, size);
	else
		source.read(value, size);
}