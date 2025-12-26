#pragma once

#include "adat.h"
#include "bsdata.h"

typedef bool(*fnallow)(const void* drawobject, int index); // Callback function of status probing. Return true if `object` allow `index` status.
typedef const char*(*fngetname)(const void* drawobject); // Callback function of get drawobject name
typedef bool(*fnvisible)(const void* drawobject); // Callback function of checking some functionality of `object`
typedef void* fngroup(const void* drawobject);

struct collectiona : adat<void*, 256> {
	void* choose(fngetname proc, const char* title, const char* cancel, bool autochoose) const;
	void* pick();
	void* random() const;
	bool choose(fngetname proc, const char* title, const char* cancel) const;
	void distinct();
	void group(fngroup proc);
	void match(fnvisible proc, bool keep);
	void match(fnallow proc, int param, bool keep);
	void match(const collectiona& source, bool keep);
	void select(array& source);
	void select(array& source, fnvisible proc);
	void shuffle();
	void sort(fngetname proc);
	void sort(fncompare proc);
	template<typename N> slice<N> records() const { return slice<N>((N*)data, count); }
};
extern collectiona records;
template<typename T>
struct collection : collectiona {
	constexpr T* operator[](unsigned i) const { return (T*)data[i]; }
	T** begin() { return (T**)data; }
	T** end() const { return (T**)data + count; }
	T* pick() { return (T*)collectiona::pick(); }
	T* random() const { return (T*)collectiona::random(); }
	void select() { collectiona::select(bsdata<T>::source); }
	void select(fnvisible proc) { collectiona::select(bsdata<T>::source, proc); }
};