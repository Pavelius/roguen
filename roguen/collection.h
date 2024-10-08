#pragma once

#include "adat.h"
#include "bsdata.h"

typedef bool(*fnallow)(const void* object, int index); // Callback function of status probing. Return true if `object` allow `index` status.
typedef const char*(*fngetname)(const void* object); // Callback function of get object name
typedef bool(*fnvisible)(const void* object); // Callback function of checking some functionality of `object`

struct collectiona : adat<void*, 256> {
	typedef void* fngroup(const void* object);
	void*	choose(fngetname proc, const char* title, const char* cancel, bool autochoose) const;
	bool	chooseu(fngetname proc, const char* title, const char* cancel, int offset);
	void	distinct();
	void	group(fngroup proc);
	void	match(fnvisible proc, bool keep);
	void	match(fnallow proc, int param, bool keep);
	void	match(const collectiona& source, bool keep);
	void*	random() const;
	slice<void*> middle(int offset, int count) { return slice<void*>(data + offset, count); }
	void*	pick();
	void	select(array& source);
	void	select(array& source, fnvisible proc);
	void	shuffle();
	void	sort(fngetname proc);
	void	sort(fncompare proc);
};
template<typename T>
struct collection : collectiona {
	constexpr T* operator[](unsigned i) const { return (T*)data[i]; }
	T**		begin() const { return (T**)data; }
	T*		choose(const char* title, const char* cancel = 0, bool autochoose = true) const {
		return (T*)collectiona::choose(T::getname, title, cancel, autochoose);
	}
	bool	chooseu(const char* title, const char* cancel = 0, int offset = 0) { return collectiona::chooseu(T::getname, title, cancel, offset); }
	T**		end() const { return (T**)data + count; }
	T*		pick() { return (T*)collectiona::pick(); }
	T*		random() const { return (T*)collectiona::random(); }
	void	select() { collectiona::select(bsdata<T>::source); }
	void	select(fnvisible proc) { collectiona::select(bsdata<T>::source, proc); }
};