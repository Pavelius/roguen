#pragma once

#include "array.h"

#ifdef _MSC_VER
#define	BSDATATMPL
#else
#define	BSDATATMPL template<>
#endif

#define FG(V) (1<<V)
#define FGT(F, V) ((F&FG(V))!=0)
#define	FO(T,R) ((size_t)&((T*)0)->R)
#define	lenghtof(C) (sizeof(C)/sizeof(C[0]))
#define	lastof(C) C+lenghtof(C)
#define maptbl(t, id) (t[imax((unsigned long)0, imin((unsigned long)id, (unsigned long)(sizeof(t)/sizeof(t[0])-1)))])
#define maprnd(t) t[rand()%(sizeof(t)/sizeof(t[0]))]
#define BSDATA(e) BSDATATMPL e bsdata<e>::elements[]
#define BSDATAD(e) BSDATATMPL array bsdata<e>::source(sizeof(e));
#define BSDATAE(e) BSDATATMPL array bsdata<e>::source(bsdata<e>::elements, sizeof(bsdata<e>::elements[0]), 0, sizeof(bsdata<e>::elements)/sizeof(bsdata<e>::elements[0]));
#define BSDATAF(e) BSDATATMPL array bsdata<e>::source(bsdata<e>::elements, sizeof(bsdata<e>::elements[0]), sizeof(bsdata<e>::elements)/sizeof(bsdata<e>::elements[0]), sizeof(bsdata<e>::elements)/sizeof(bsdata<e>::elements[0]));
#define BSDATAC(e, c) BSDATATMPL e bsdata<e>::elements[c]; BSDATAE(e)
#define NOBSDATA(e) template<> struct bsdata<e> : bsdata<int> {};
#define assert_enum(e, last) static_assert(sizeof(bsdata<e>::elements) / sizeof(bsdata<e>::elements[0]) == static_cast<int>(last) + 1, "Invalid count of " #e " elements"); BSDATAF(e)

typedef int (*fncompare)(const void*, const void*);

extern "C" int						atexit(void(*func)(void));
extern "C" void*					bsearch(const void* key, const void* base, unsigned num, size_t size, fncompare proc);
extern "C" void						qsort(void* base, unsigned num, long unsigned size, fncompare proc);
extern "C" void						srand(unsigned seed); // Set random seed
extern "C" int						strcmp(const char* s1, const char* s2) noexcept(true); // Compare two strings

unsigned long						getcputime();
unsigned                            randomseed();
void								waitcputime(unsigned v);

// Common used templates
inline int							ifloor(double n) { return (int)n; }
template<class T> inline T			imax(T a, T b) { return a > b ? a : b; }
template<class T> inline T			imin(T a, T b) { return a < b ? a : b; }
//template<class T> inline void		zclear(T* p) { memset(p, 0, sizeof(*p)); }
//template<class T> inline void		zclear(T& e) { memset(&e, 0, sizeof(e)); }
//template<class T> inline void		zcpy(T* p1, const T* p2) { while(*p2) *p1++ = *p2++; *p1 = 0; }
//template<class T> inline void		zcpy(T* p1, const T* p2, int max_count) { while(*p2 && max_count-- > 0) *p1++ = *p2++; *p1 = 0; }
//template<class T> inline void		zcat(T* p1, const T e) { p1 = zend(p1); p1[0] = e; p1[1] = 0; }
//template<class T> inline void		zcat(T* p1, const T* p2) { zcpy(zend(p1), p2); }

template<class T, size_t count_max = 128>
struct adat { // Storge like vector
	size_t							count;
	T								data[count_max];
	constexpr adat() : data{}, count(0) {}
	constexpr const T& operator[](unsigned index) const { return data[index]; }
	constexpr T& operator[](unsigned index) { return data[index]; }
	explicit operator bool() const { return count != 0; }
	typedef T data_type;
	T*								add() { if(count < count_max) return data + (count++); return data; }
	void							add(const T& e) { if(count < count_max) data[count++] = e; }
	T*								begin() { return data; }
	const T*						begin() const { return data; }
	void							clear() { count = 0; }
	T*								end() { return data + count; }
	const T*						end() const { return data + count; }
	const T*						endof() const { return data + count_max; }
	int								find(const T t) const { for(auto& e : *this) if(e == t) return &e - data; return -1; }
	int								getcount() const { return count; }
	size_t							getmaximum() const { return count_max; }
	int								indexof(const void* e) const { if(e >= data && e < data + count) return (T*)e - data; return -1; }
	bool							is(const T t) const { for(auto& e : *this) if(e == t) return true; return false; }
	void							remove(int index, int remove_count = 1) { if(index < 0) return; if(index<int(count - 1)) memcpy(data + index, data + index + 1, sizeof(data[0]) * (count - index - 1)); count--; }
	void							remove(const T t) { remove(find(t), 1); }
	void							top(size_t v) { if(count > v) count = v; }
};

template<class T>
class vector : public array {
public:
	typedef T data_type;
	constexpr vector() : array(sizeof(T)) {}
	~vector() { for(auto& e : *this) e.~T(); }
	constexpr T& operator[](int index) { return ((T*)data)[index]; }
	constexpr const T& operator[](int index) const { return ((T*)data)[index]; }
	constexpr explicit operator bool() const { return count != 0; }
	T*						add() { return (T*)array::add(); }
	void					add(const T& v) { *((T*)array::add()) = v; }
	constexpr const T*		begin() const { return (T*)data; }
	constexpr T*			begin() { return (T*)data; }
	constexpr const T*		end() const { return (T*)data + count; }
	constexpr T*			end() { return (T*)data + count; }
	constexpr int			indexof(const T* e) const { if(e >= (T*)data && e < (T*)data + count) return e - (T*)data; return -1; }
	constexpr int			indexof(const T t) const { for(auto& e : *this) if(e == t) return &e - (T*)data; return -1; }
	constexpr bool			is(const T* t) const { return t >= data && t < data + count; }
	constexpr T*			ptr(size_t index) const { return (T*)data + index; }
	constexpr T*			ptrs(size_t index) const { return (index < count) ? (T*)data + index : 0; }
};

template<typename T>
struct bsdata {
	static T				elements[];
	static array			source;
	static constexpr array*	source_ptr = &source;
	static T*				add() { return (T*)source.add(); }
	static T*				addz() { for(auto& e : bsdata<T>()) if(!e) return &e; return add(); }
	static constexpr bool	have(const void* p) { return source.have(p); }
	static T*				find(const char* id) { return (T*)source.findv(id, 0); }
	static constexpr T&		get(int i) { return begin()[i]; }
	static constexpr T*		get(const void* p) { return have(p) ? (T*)p : 0; }
	static constexpr T*		begin() { return (T*)source.data; }
	static constexpr T*		end() { return (T*)source.data + source.getcount(); }
	static constexpr T*		ptr(short unsigned i) { return (i==0xFFFF) ? 0 : (T*)source.ptr(i); }
};
template<typename T> inline void	bsset(short unsigned& v, const T* p) { v = (p == 0) ? 0xFFFF : bsdata<T>::source.indexof(p); }
template<typename T> inline unsigned short bsid(const T* p) { return bsdata<T>::source.indexof(p); }
template<> struct bsdata<int> { static constexpr array* source_ptr = 0; };
NOBSDATA(unsigned)
NOBSDATA(short)
NOBSDATA(unsigned short)
NOBSDATA(char)
NOBSDATA(unsigned char)
NOBSDATA(const char*)
NOBSDATA(bool)

template<typename T>
struct sliceu {
	unsigned				start, count;
	constexpr sliceu() : start(0), count(0) {}
	constexpr sliceu(unsigned start, unsigned count) : start(start), count(count) {}
	template<size_t N> sliceu(T(&v)[N]) { set(v, N); }
	constexpr explicit operator bool() const { return count != 0; }
	T*						begin() const { return (T*)bsdata<T>::source.ptr(start); }
	void					clear() { start = count = 0; }
	T*						end() const { return (T*)bsdata<T>::source.ptr(start + count); }
	sliceu<T>				left(unsigned v) const { return sliceu<T>(start, v <= count ? v : count); }
	void					set(unsigned p1, unsigned count) { start = p1; this->count = count; }
	void					set(const T* v, unsigned count) { start = bsdata<T>::source.indexof(bsdata<T>::source.addu(v, count)); this->count = count; }
	constexpr unsigned		size() const { return count; }
};

template<typename T>
struct funct {
	typedef void (T::*command)(); // typical object command
	typedef bool (T::*visible)() const; // object visibility
};
template<typename T, funct<T>::visible F> inline bool fntis(const void* p) { return (((T*)p)->*F)(); }
typedef void(*fnevent)(); // Callback function of any command executing
typedef bool(*fnallow)(const void* object, int index); // Callback function of status probing. Return true if `object` allow `index` status.
typedef bool(*fnchoose)(const void* object, array& source, void* pointer); // Callback function of choosing one element from array of many elements and storing it into `pointer`
typedef bool(*fnvisible)(const void* object); // Callback function of checking some functionality of `object`
typedef const char*(*fngetname)(const void* object); // Callback function of get object name
typedef void(*fncommand)(void* object); // Callback function of object command executing
typedef void(*fnread)(const char* url);

bool						equal(const char* s1, const char* s2);
int							getdigitscount(unsigned number); // Get digits count of number. For example if number=100, result be 3.
int							isqrt(const int x); // Return aquare root of 'x'
float						sqrt(const float x); // Return aquare root of 'x'
const char*					skipcr(const char* p);
void						szchange(char* p, char s1, char s2);
const char*					szext(const char* path);
const char*					szfind(const char* text, const char* name);
const char*					szfname(const char* text); // Get file name from string (no fail, always return valid value)
char*						szfnamewe(char* result, const char* name); // get file name without extension (no fail)
const char*					szfurl(const char* url); // get full absolute url
bool						szmatch(const char* text, const char* name); //
bool						szstart(const char* text, const char* name);
char*						szurl(char* p, const char* path, const char* name, const char* ext = 0, const char* suffix = 0);
char*						szurlc(char* p1);