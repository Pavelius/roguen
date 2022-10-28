#include "crt.h"

#pragma once

struct collectiona : adat<void*, 256> {
	void	select(array& source);
	void	select(array& source, fnvisible proc);
	void*	random() const;
};
template<typename T>
struct collection : collectiona {
	T**		begin() const { return (T**)data; }
	T**		end() const { return (T**)data + count; }
	T*		random() const { return (T*)collectiona::random(); }
	void	select() { collectiona::select(bsdata<T>::source); }
	void	select(fnvisible proc) { collectiona::select(bsdata<T>::source, proc); }
};