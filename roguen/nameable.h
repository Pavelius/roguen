#pragma once

struct nameable {
	const char*		id;
	const char*		getname() const;
	const char*		getid() const { return id; }
	static const char* getname(const void* p) { return ((nameable*)p)->getname(); }
};
