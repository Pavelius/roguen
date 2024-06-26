#pragma once

#ifdef _MSC_VER
#define xva_start(v) (((const char*)&v) + sizeof(v))
#else
#define xva_start(v) (((const char*)&v) + sizeof(v)*4)
#endif

enum gender_s : unsigned char;

class stringbuilder {
	struct grammar;
	struct genderi;
	char*				p;
	char*				pb;
	const char*			pe;
	const char*			readformat(const char* format, const char* format_param);
	const char*			readvariable(const char* format);
	void				add(const char* s, const grammar* source, const char* def = 0);
public:
	typedef void (*fncustom)(stringbuilder& sb, const char* id);
	constexpr stringbuilder(char* pb, const char* pe) : p(pb), pb(pb), pe(pe) {}
	template<unsigned N> constexpr stringbuilder(char(&result)[N]) : stringbuilder(result, result + N - 1) {}
	constexpr operator char*() const { return pb; }
	explicit constexpr operator bool() const { return pb[0]; }
	static fncustom		custom;
	void				add(const char* format, ...) { addv(format, xva_start(format)); }
	void				add(char sym);
	void				addby(const char* s);
	void				addch(char sym);
	void				addcount(const char* id, int count, const char* format = 0);
	void				addicon(const char* id, int value);
	void				addint(int value, int precision, const int radix);
	void				adjective(const char* name, int m);
	void				addlocalefile(const char* folder, const char* name, const char* ext = 0);
	void				addlocaleurl();
	void				addn(const char* format, ...) { addx('\n', format, xva_start(format)); }
	void				addnounf(const char* s);
	void				addnouni(const char* s);
	void				addnounx(const char* s);
	void				addnz(const char* format, unsigned count);
	void				addof(const char* s);
	void				adds(const char* format, ...) { addx(' ', format, xva_start(format)); }
	void				addsep(char separator);
	void				addsym(int v);
	void				addsz() { if(p < pe) *p++ = 0; }
	void				addto(const char* s);
	void				addv(const char* format, const char* format_param);
	void				addx(char separator, const char* format, const char* format_param);
	void				addx(const char* separator, const char* format, const char* format_param);
	void				adduint(unsigned value, int precision, const int radix);
	const char*			begin() const { return pb; }
	void				change(char s1, char s2);
	void				change(const char* s1, const char* s2);
	void				clear() { pb[0] = 0; p = pb; }
	void				copy(const char* v);
	static void			defidentifier(stringbuilder& sb, const char* id);
	const char*			end() const { return pe; }
	char*				get() const { return p; }
	static const char*	getbycount(const char* id, int count);
	static int			getgender(const char* s);
	static int			getnum(const char* v);
	unsigned			getlenght() const { return p - pb; }
	unsigned			getmaximum() const { return pe - pb - 1; }
	bool				isempthy() const { return !pb || pb[0] == 0; }
	static bool			ischa(unsigned char sym) { return (sym >= 'A' && sym <= 'Z') || (sym >= 'a' && sym <= 'z') || sym >= 0xC0; }
	bool				isfull() const { return p >= pe; }
	static bool			isnum(unsigned char sym) { return sym >= '0' && sym <= '9'; }
	bool				ispos(const char* v) const { return p == v; }
	static unsigned char lower(unsigned char sym);
	void				lower();
	const char*			psidf(const char* pb);
	const char*			psline(const char* pb);
	const char*			psstr(const char* p, char end_symbol);
	const char*			psstrlf(const char* p);
	static const char*	read(const char* p, long& result);
	static const char*	read(const char* p, int& result);
	static const char*	read(const char* p, short& result);
	void				set(char* v) { p = v; p[0] = 0; }
	static void			setlocale(const char* id);
	void				trimr();
	static unsigned char upper(unsigned char sym);
	void				upper();
};
typedef const char* (*fntext)(const void* object, stringbuilder& sb);
typedef void (*fnstatus)(const void* object, stringbuilder& sb);
typedef void (*fnprint)(stringbuilder& sb);
typedef void (*fnoutput)(const char* format);

const char* ids(const char* p1, const char* p2);
const char* ids(const char* p1, const char* p2, const char* p3);
const char* str(const char* format, ...);

void print(fnoutput proc, const char* format, ...);