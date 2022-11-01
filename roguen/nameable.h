#pragma once

struct nameable {
	const char*		id;
	const char*		getname() const;
	static const char* getname(const void* p) { return ((nameable*)p)->getname(); }
};
