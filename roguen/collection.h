#include "crt.h"

#pragma once

struct collectiona : adat<void*, 256> {
	typedef void* fngroup(const void* object);
	void*	choose(fngetname proc, const char* title, const char* cancel, bool autochoose) const;
	bool	chooseu(fngetname proc, const char* title, const char* cancel) const;
	void	distinct();
	void	group(fngroup proc);
	void	match(fnvisible proc, bool keep);
	void	match(fnallow proc, int param, bool keep);
	void	match(const collectiona& source, bool keep);
	void*	random() const;
	void*	pick();
	void	select(array& source);
	void	select(array& source, fnvisible proc);
	void	shuffle();
	void	sort(fngetname proc);
};
template<typename T>
struct collection : collectiona {
	constexpr T* operator[](unsigned i) const { return (T*)data[i]; }
	T**		begin() const { return (T**)data; }
	T*		choose(const char* title, const char* cancel = 0, bool autochoose = true) const {
		return (T*)collectiona::choose(T::getname, title, cancel, autochoose);
	}
	bool	chooseu(const char* title, const char* cancel = 0) const { return collectiona::chooseu(T::getname, title, cancel); }
	T**		end() const { return (T**)data + count; }
	T*		pick() { return (T*)collectiona::pick(); }
	T*		random() const { return (T*)collectiona::random(); }
	void	select() { collectiona::select(bsdata<T>::source); }
	void	select(fnvisible proc) { collectiona::select(bsdata<T>::source, proc); }
};