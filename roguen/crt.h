#pragma once

#include "adat.h"
#include "array.h"

#define FG(V) (1<<V)
#define FGT(F, V) ((F&FG(V))!=0)
#define	lastof(C) C+lenghtof(C)
#define maprnd(t) t[rand()%(sizeof(t)/sizeof(t[0]))]

extern "C" int						atexit(void(*func)(void));
extern "C" void						srand(unsigned seed); // Set random seed
extern "C" int						strcmp(const char* s1, const char* s2) noexcept(true); // Compare two strings

// Common used templates
inline int							ifloor(double n) { return (int)n; }

template<typename T>
struct funct {
	typedef void (T::*command)(); // typical object command
	typedef bool (T::*visible)() const; // object visibility
};
template<typename T, funct<T>::visible F> inline bool fntis(const void* p) { return (((T*)p)->*F)(); }
typedef void(*fnevent)(); // Callback function of any command executing
typedef bool(*fnchoose)(const void* object, array& source, void* pointer); // Callback function of choosing one element from array of many elements and storing it into `pointer`
typedef const char*(*fngetname)(const void* object); // Callback function of get object name
typedef void(*fncommand)(void* object); // Callback function of object command executing
typedef void(*fnread)(const char* url);

//int						isqrt(const int x); // Return aquare root of 'x'
//float						sqrt(const float x); // Return aquare root of 'x'
//const char*				skipcr(const char* p);
//const char*				szext(const char* path);
//const char*				szfind(const char* text, const char* name);
//const char*				szfname(const char* text); // Get file name from string (no fail, always return valid value)
char*						szfnamewe(char* result, const char* name); // get file name without extension (no fail)
const char*					szfurl(const char* url); // get full absolute url
bool						szmatch(const char* text, const char* name); //
bool						szstart(const char* text, const char* name);
char*						szurl(char* p, const char* path, const char* name, const char* ext = 0, const char* suffix = 0);
char*						szurlc(char* p1);