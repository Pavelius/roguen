#pragma once

struct nameable {
	const char*		id;
	constexpr explicit operator bool() const { return id != 0; }
	const char*		getname() const;
	const char*		getid() const { return id; }
	static const char* getname(const void* p) { return ((nameable*)p)->getname(); }
};
