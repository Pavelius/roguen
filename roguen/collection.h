#include "crt.h"

#pragma once

struct collectiona : adat<void*, 256> {
	typedef void* fngroup(const void* object);
	void*	choose(fngetname proc, const char* title, const char* cancel, bool autochoose) const;
	void	distinct();
	void	group(fngroup proc);
	void*	random() const;
	void	select(array& source);
	void	select(array& source, fnvisible proc);
	void	sort(fngetname proc);
};
template<typename T>
struct collection : collectiona {
	constexpr T* operator[](unsigned i) const { return (T*)data[i]; }
	T**		begin() const { return (T**)data; }
	T*		choose(const char* title, const char* cancel = 0, bool autochoose = false) const {
		return (T*)collectiona::choose(T::getname, title, cancel, autochoose);
	}
	T**		end() const { return (T**)data + count; }
	T*		random() const { return (T*)collectiona::random(); }
	void	select() { collectiona::select(bsdata<T>::source); }
	void	select(fnvisible proc) { collectiona::select(bsdata<T>::source, proc); }
};