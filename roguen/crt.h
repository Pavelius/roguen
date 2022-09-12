#pragma once

typedef decltype(sizeof(0)) size_t;

#ifdef _DEBUG
#define assert(e) if(!(e)) {exit(255);}
#else
#define assert(e)
#endif

#ifdef _MSC_VER
#define	BSDATATMPL
#else
#define	BSDATATMPL template<>
#endif

#define FG(V) (1<<V)
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

extern "C" int						atexit(void(*func)(void));
extern "C" void*					bsearch(const void* key, const void* base, unsigned num, size_t size, int(*compar)(const void*, const void*));
extern "C" void						exit(int exit_code);
extern "C" int						memcmp(const void* p1, const void* p2, size_t size) noexcept(true);
extern "C" void*					memmove(void* destination, const void* source, size_t size) noexcept(true);
extern "C" void*					memchr(const void* ptr, int value, long unsigned num);
extern "C" void*					memcpy(void* destination, const void* source, long unsigned size) noexcept(true);
extern "C" void*					memset(void* destination, int value, long unsigned size) noexcept(true);
extern "C" void						qsort(void* base, unsigned num, long unsigned size, int(*compar)(const void*, const void*));
extern "C" int						rand(void); // Get next random value
extern "C" void						srand(unsigned seed); // Set random seed
extern "C" int						strcmp(const char* s1, const char* s2) noexcept(true); // Compare two strings

unsigned long						getcputime();
unsigned                            randomseed();
void								waitcputime(unsigned v);

enum class codepage { No, W1251, UTF8, U16BE, U16LE };
namespace metrics {
const codepage						code = codepage::W1251;
}
// Common used templates
inline int							ifloor(double n) { return (int)n; }
template<class T> inline T			imax(T a, T b) { return a > b ? a : b; }
template<class T> inline T			imin(T a, T b) { return a < b ? a : b; }
template<class T> inline T			iabs(T a) { return a > 0 ? a : -a; }
template<class T> inline void		iswap(T& a, T& b) { T i = a; a = b; b = i; }
// Inline sequence functions
template<class T> inline void		seqclear(T* p) { T* z = p->next; while(z) { T* n = z->next; z->next = 0; delete z; z = n; } p->next = 0; } // Use to clean up sequenced resources if destructor. Use it like 'clear(this)'.
template<class T> inline T*			seqlast(T* p) { while(p->next) p = p->next; return p; } // Return last element in sequence.
template<class T> inline void		seqlink(T* p) { p->next = 0; if(!T::first) T::first = p; else seqlast(T::first)->next = p; }
// Inline strings functions
template<class T> inline const T*	zchr(const T* p, T e) { while(*p) { if(*p == e) return p; p++; } return 0; }
template<class T> inline void		zcpy(T* p1, const T* p2) { while(*p2) *p1++ = *p2++; *p1 = 0; }
template<class T> inline void		zcpy(T* p1, const T* p2, int max_count) { while(*p2 && max_count-- > 0) *p1++ = *p2++; *p1 = 0; }
template<class T> constexpr T*		zend(T* p) { while(*p) p++; return p; }
template<class T> inline void		zcat(T* p1, const T e) { p1 = zend(p1); p1[0] = e; p1[1] = 0; }
template<class T> inline void		zcat(T* p1, const T* p2) { zcpy(zend(p1), p2); }
template<class T> constexpr size_t	zlen(T* p) { return zend(p) - p; }
template<class T> inline void		zshuffle(T* p, int count) { for(int i = 0; i < count; i++) iswap(p[i], p[rand() % count]); }
// Storge like vector
template<class T, size_t count_max = 128>
struct adat {
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
};
// Simple slice object
template<class T>
class slice {
	T*								data;
	size_t							count;
public:
	typedef T data_type;
	constexpr slice() : data(0), count(0) {}
	template<size_t N> constexpr slice(T(&v)[N]) : data(v), count(N) {}
	template<int N> constexpr slice(adat<T, N>& v) : data(v), count(v.count) {}
	constexpr slice(T* data, unsigned count) : data(data), count(count) {}
	constexpr slice(T* p1, const T* p2) : data(p1), count(p2 - p1) {}
	explicit operator bool() const { return count != 0; }
	constexpr T*					begin() const { return data; }
	constexpr T*					end() const { return data + count; }
	constexpr unsigned				size() const { return count; }
};
// Abstract array vector
class array {
	size_t							count_maximum;
	void							grow(unsigned offset, size_t delta);
	void							shrink(unsigned offset, size_t delta);
	void							zero(unsigned offset, size_t delta);
public:
	void*							data;
	size_t							count, size;
	typedef int(*pcompare)(const void* p1, const void* p2, void* param);
	constexpr array(size_t size = 0) : count_maximum(0), data(0), count(0), size(size) {}
	constexpr array(void* data, size_t size, size_t count) : count_maximum(count | 0x80000000), data(data), count(count), size(size) {}
	constexpr array(void* data, size_t size, size_t count, unsigned count_maximum) : count_maximum(count_maximum | 0x80000000), data(data), count(count), size(size) {}
	constexpr explicit operator bool() const { return count != 0; }
	~array();
	void*							add();
	void*							addz() { auto p = add(); memset(p, 0, size); return p; }
	void*							add(const void* element);
	void*							addfind(const char* id);
	void*							addu(const void* element, unsigned count);
	const char*						addus(const char* element, unsigned count);
	char*							begin() const { return (char*)data; }
	void							change(unsigned offset, int size);
	void							clear();
	char*							end() const { return (char*)data + size * count; }
	int								find(const char* value, unsigned offset) const { return findps(value, offset, zlen(value)); }
	int								find(int i1, int i2, void* value, unsigned offset, size_t size) const;
	int								find(void* value, unsigned offset, size_t size) const { return find(0, -1, value, offset, size); }
	int								findps(const char* value, unsigned offset, size_t size) const;
	const void*						findu(const void* value, size_t size) const;
	const char*						findus(const char* value, size_t size) const;
	void*							findv(const char* value, unsigned offset) const;
	size_t							getmaximum() const { return count_maximum & 0x7FFFFFFF; }
	size_t							getcount() const { return count; }
	size_t							getsize() const { return size; }
	constexpr bool					have(const void* element) const { return element >= data && element < ((char*)data + size * count); }
	int								indexof(const void* element) const;
	void*							insert(int index, const void* element);
	bool							is(const void* e) const { return e >= data && e < (char*)data + count * size; }
	bool							isgrowable() const { return (count_maximum & 0x80000000) == 0; }
	void*							ptr(int index) const { return (char*)data + size * index; }
	void*							ptrs(int index) const { return (((unsigned)index) < count) ? (char*)data + size * index : 0; }
	template<class T> slice<T> records() const { return slice<T>((T*)data, count); }
	void							remove(int index, int elements_count = 1);
	void							shift(int i1, int i2, size_t c1, size_t c2);
	void							setcount(unsigned value) { count = value; }
	void							setup(size_t size);
	void							sort(int i1, int i2, pcompare compare, void* param);
	void							sort(pcompare compare, void* param) { sort(-1, -1, compare, param); }
	void							swap(int i1, int i2);
	void							reserve(unsigned count);
};
template<class T> class vector : public array {
public:
	typedef T data_type;
	constexpr vector() : array(sizeof(T)) {}
	~vector() { for(auto& e : *this) e.~T(); }
	constexpr T& operator[](int index) { return ((T*)data)[index]; }
	constexpr const T& operator[](int index) const { return ((T*)data)[index]; }
	constexpr explicit operator bool() const { return count != 0; }
	T*								add() { return (T*)array::add(); }
	void							add(const T& v) { *((T*)array::add()) = v; }
	constexpr const T*				begin() const { return (T*)data; }
	constexpr T*					begin() { return (T*)data; }
	constexpr const T*				end() const { return (T*)data + count; }
	constexpr T*					end() { return (T*)data + count; }
	constexpr int					indexof(const T* e) const { if(e >= (T*)data && e < (T*)data + count) return e - (T*)data; return -1; }
	constexpr int					indexof(const T t) const { for(auto& e : *this) if(e == t) return &e - (T*)data; return -1; }
	constexpr bool					is(const T* t) const { return t >= data && t < data + count; }
	constexpr T*					ptr(size_t index) const { return (T*)data + index; }
	constexpr T*					ptrs(size_t index) const { return (index < count) ? (T*)data + index : 0; }
};
// Abstract data access class
template<typename T> struct bsdata {
	static T						elements[];
	static array					source;
	static constexpr array*			source_ptr = &source;
	static T*						add() { return (T*)source.add(); }
	static T*						addz() { for(auto& e : bsdata<T>()) if(!e) return &e; return add(); }
	static constexpr bool			have(const void* p) { return source.have(p); }
	static T*						find(const char* id) { return (T*)source.findv(id, 0); }
	static constexpr T&				get(int i) { return begin()[i]; }
	static constexpr T*				get(const void* p) { return have(p) ? (T*)p : 0; }
	static constexpr T*				begin() { return (T*)source.data; }
	static constexpr T*				end() { return (T*)source.data + source.getcount(); }
};
template<> struct bsdata<int> { static constexpr array* source_ptr = 0; };
NOBSDATA(unsigned)
NOBSDATA(short)
NOBSDATA(unsigned short)
NOBSDATA(char)
NOBSDATA(unsigned char)
NOBSDATA(const char*)
NOBSDATA(bool)
template<typename T> struct sliceu {
	unsigned						start, count;
	constexpr sliceu() : start(0), count(0) {}
	template<size_t N> sliceu(T(&v)[N]) { set(v, N); }
	constexpr explicit operator bool() const { return count != 0; }
	T*								begin() const { return (T*)bsdata<T>::source.ptr(start); }
	void							clear() { start = count = 0; }
	T*								end() const { return (T*)bsdata<T>::source.ptr(start + count); }
	void							set(const T* v, unsigned count) { start = bsdata<T>::source.indexof(bsdata<T>::source.addu(v, count)); this->count = count; }
	constexpr unsigned				size() const { return count; }
};
// Callback function of any command executing
typedef void(*fnevent)();
// Callback function of status probing. Return true if `object` allow `index` status.
typedef bool(*fnallow)(const void* object, int index);
// Callback function of choosing one element from array of many elements and storing it into `pointer`
typedef bool(*fnchoose)(const void* object, array& source, void* pointer);
// Callback function of checking some functionality of `object`
typedef bool(*fnvisible)(const void* object);
// Callback function of get object name
typedef const char*(*fngetname)(const void* object);
// Callback function of object command executing
typedef void(*fncommand)(void* object);
// Common functions
bool								equal(const char* s1, const char* s2);
template<typename T> short unsigned getbsi(const T* v) { return bsdata<T>::source.indexof(v); }
const char*							getdescription(const char* id);
int									getdigitscount(unsigned number); // Get digits count of number. For example if number=100, result be 3.
const char*							getnm(const char* id);
const char*							getnm(const char* id, int count);
const char*							getnmof(const char* id);
const char*							getnmpl(const char* id);
const char*							getnmsh(const char* id);
bool								ischa(unsigned u); // is alphabetical character?
inline bool							isnum(unsigned u) { return u >= '0' && u <= '9'; } // is numeric character?
int									isqrt(const int x); // Return aquare root of 'x'
void*								loadb(const char* url, int* size = 0, int additional_bytes_alloated = 0); // Load binary file.
char*								loadt(const char* url, int* size = 0); // Load text file and decode it to system codepage.
bool								matchuc(const char* name, const char* filter);
float								sqrt(const float x); // Return aquare root of 'x'
inline const char*					skipsp(const char* p) { if(p) while(*p == ' ' || *p == '\t') p++; return p; }
inline const char*					skipspcr(const char* p) { if(p) while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++; return p; }
const char*							skipcr(const char* p);
void								szchange(char* p, char s1, char s2);
void								szencode(char* output, int output_count, codepage output_code, const char* input, int input_count, codepage input_code);
unsigned							szget(const char** input, codepage page = metrics::code);
int									szcmpi(const char* p1, const char* p2);
int									szcmpi(const char* p1, const char* p2, int count);
const char*							szdup(const char* text);
const char*							szext(const char* path);
const char*							szfind(const char* text, const char* name);
const char*							szfname(const char* text); // Get file name from string (no fail, always return valid value)
char*								szfnamewe(char* result, const char* name); // get file name without extension (no fail)
const char*							szfurl(const char* url); // get full absolute url
unsigned							szlower(unsigned u); // to lower reg
void								szlower(char* p); // to lower reg
bool								szmatch(const char* text, const char* name); //
bool								szpmatch(const char* text, const char* pattern);
void								szput(char** output, unsigned u, codepage page = metrics::code);
char*								szput(char* output, unsigned u, codepage page = metrics::code); // Fast symbol put function. Return 'output'.
bool								szstart(const char* text, const char* name);
unsigned							szupper(unsigned u);
void								szupper(char* p); // to upper reg
char*								szurl(char* p, const char* path, const char* name, const char* ext = 0, const char* suffix = 0);
char*								szurlc(char* p1);
inline int							xrand(int n1, int n2) { return n1 + rand() % (n2 - n1 + 1); }
// Get base type
template<class T> struct meta_decoy { typedef T value; };
template<> struct meta_decoy<const char*> { typedef const char* value; };
template<class T> struct meta_decoy<T*> : meta_decoy<T> {};
template<class T> struct meta_decoy<const T*> : meta_decoy<T> {};
template<class T, size_t N> struct meta_decoy<T[N]> : meta_decoy<T> {};
template<class T> struct meta_decoy<T[]> : meta_decoy<T> {};
template<class T> struct meta_decoy<const T> : meta_decoy<T> {};
template<class T> struct meta_decoy<vector<T>> : meta_decoy<T> {};
template<class T> struct meta_decoy<sliceu<T>> : meta_decoy<T> {};
template<class T> struct meta_decoy<slice<T>> : meta_decoy<T> {};
template<class T, size_t N> struct meta_decoy<adat<T, N>> : meta_decoy<T> {};