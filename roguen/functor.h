#pragma once

template<class T>
struct funct {
	typedef void (T::*command)(); // typical object command
	typedef bool (T::*visible)() const; // object visibility
};
template<class T, funct<T>::visible F> inline bool fntis(const void* p) { return (((T*)p)->*F)(); }