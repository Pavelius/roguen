#pragma once

template<class T> inline T iabs(T a) { return a > 0 ? a : -a; }
template<class T> inline T imax(T a, T b) { return a > b ? a : b; }
template<class T> inline T imin(T a, T b) { return a < b ? a : b; }

#define maptbl(t, id) (t[imax((unsigned long)0, imin((unsigned long)id, (unsigned long)(sizeof(t)/sizeof(t[0])-1)))])

