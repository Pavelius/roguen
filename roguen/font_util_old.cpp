#include "math.h"
#include "slice.h"
#include "sprite_util.h"
#include "win.h"

using namespace util;

namespace {
class font {
	void*			hcnv;
	void*			hfnt;
public:
	font(const char* name, int size, bool antialiased);
	~font();
	int	glyphi(int glyph, int& width, int& height, int& dx, int& dy, int& ox, int& oy, unsigned char* buffer, int maxsize, int xscale, int yscale, int mode);
	int glyphi(int glyph, int& width, int& height, int& dx, int& dy, int& ox, int& oy);
	void info(int& width, int& height, int& dy1, int& dy2);
	static void write(const char* url, const char* name, int size);
};
}

static HDC hcnv, hfnt;
static unsigned short ascii_decoder[256];
static int height_diff;

static void wcpy(wchar_t* d, const char* s) {
	while(*s)
		*d++ = *s++;
	*d++ = 0;
}

static void font_create(const char* name, int size) {
	wchar_t name1[260]; wcpy(name1, name);
	void* hf = CreateFontW(size, 0, 0, 0, 400, 0, 0,
		0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, FF_DONTCARE,
		name1);
	hcnv = CreateCompatibleDC(0);
	hfnt = SelectObject(hcnv, hf);
}

static void font_delete() {
	DeleteObject(SelectObject(hcnv, hfnt));
	DeleteDC(hcnv);
}

static void font_info(int& width, int& height, int& dy1, int& dy2) {
	TEXTMETRICA tm;
	GetTextMetricsA(hcnv, &tm);
	width = tm.tmAveCharWidth;
	height = tm.tmHeight;
	dy1 = tm.tmAscent;
	dy2 = tm.tmDescent;
}

font::font(const char* name, int size, bool antialiased) {
	wchar_t name1[260];
	wcpy(name1, name);
	int q = antialiased ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY;
	void* hf = CreateFontW(size, 0, 0, 0, FW_NORMAL, 0, 0,
		0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, q, FF_DONTCARE,
		name1);
	hcnv = CreateCompatibleDC(0);
	hfnt = SelectObject(hcnv, hf);
}

font::~font() {
	DeleteObject(SelectObject(hcnv, hfnt));
	DeleteDC(hcnv);
}

// tool used to create font with SubPixel rendering from system font's
int font::glyphi(int glyph,
	int& width, int& height, int& dx, int& dy, int& ox, int& oy,
	unsigned char* buffer, int maxsize, int xscale, int yscale,
	int mode) {
	MAT2 scale3h;
	memset(&scale3h, 0, sizeof(MAT2));
	scale3h.eM11.value = xscale; // rgb color
	scale3h.eM22.value = yscale;
	GLYPHMETRICS gm;
	memset(&gm, 0, sizeof(gm));
	switch(mode) {
	case 1: mode = GGO_BITMAP; break;
	case 2: mode = GGO_GRAY2_BITMAP; break;
	case 4: mode = GGO_GRAY4_BITMAP; break;
	case 8: mode = GGO_GRAY8_BITMAP; break;
	default: mode = GGO_GRAY8_BITMAP; break;
	}
	int size = GetGlyphOutlineW(hcnv,
		glyph,
		mode,
		&gm,
		maxsize,
		buffer,
		&scale3h);
	width = gm.gmBlackBoxX;
	height = gm.gmBlackBoxY;
	dx = gm.gmCellIncX;
	dy = gm.gmCellIncY;
	ox = gm.gmptGlyphOrigin.x;
	oy = gm.gmptGlyphOrigin.y;
	return size;
}

int font::glyphi(int glyph,
	int& width, int& height, int& dx, int& dy, int& ox, int& oy) {
	MAT2 scale3h;
	memset(&scale3h, 0, sizeof(MAT2));
	scale3h.eM11.value = 1; // rgb color
	scale3h.eM22.value = 1;
	GLYPHMETRICS gm;
	memset(&gm, 0, sizeof(gm));
	int size = GetGlyphOutlineW(hcnv,
		glyph,
		GGO_GRAY8_BITMAP,
		&gm,
		0, 0,
		&scale3h);
	width = gm.gmBlackBoxX;
	height = gm.gmBlackBoxY;
	dx = gm.gmCellIncX;
	dy = gm.gmCellIncY;
	ox = gm.gmptGlyphOrigin.x;
	oy = gm.gmptGlyphOrigin.y;
	return size;
}

void font::info(int& width, int& height, int& dy1, int& dy2) {
	TEXTMETRICA tm;
	GetTextMetricsA(hcnv, &tm);
	width = tm.tmAveCharWidth;
	height = tm.tmHeight;
	dy1 = tm.tmAscent;
	dy2 = tm.tmDescent;
}

static int primary[256];
static int secondary[256];
static int tertiary[256];
static unsigned char buf1[128 * 128];
static unsigned char buf2[128 * 128];
static short int widths[4096];
static int intervals[][2] = {{0x21, 0x7E},
	{0x410, 0x44F},
	{0x456, 0x457}, // ”краинские буквы маленькие
	{0x406, 0x407}, // ”краинские буквы большие
};

inline int floor(double f) {
	return ((int)(f * 10.00)) / 10;
}

const int num_levels = 65; // Constant GGO_GRAY8_BITMAP generate 65 levels of gray

static void lcd_init(double prim, double second, double tert) {
	double norm = (255.0 / ((double)num_levels)) / (prim + second + tert);
	prim *= norm;
	second *= norm;
	tert *= norm;
	for(int i = 0; i < num_levels; i++) {
		primary[i] = floor(prim * i);
		secondary[i] = floor(second * i);
		tertiary[i] = floor(tert * i);
	}
}

static void lcd_prepare_glyph(unsigned char* gbuf2, int& w2, const unsigned char* gbuf1, int w1, int h1) {
	int src_stride = (w1 + 3) / 4 * 4;
	int dst_width = src_stride + 4;
	for(int y = 0; y < h1; ++y) {
		const unsigned char* src_ptr = gbuf1 + src_stride * y;
		unsigned char* dst_ptr = gbuf2 + dst_width * y;
		for(int x = 0; x < w1; ++x) {
			unsigned v = *src_ptr++;
			dst_ptr[0] = imin((int)dst_ptr[0] + tertiary[v], 255);
			dst_ptr[1] = imin((int)dst_ptr[1] + secondary[v], 255);
			dst_ptr[2] = imin((int)dst_ptr[2] + primary[v], 255);
			dst_ptr[3] = imin((int)dst_ptr[3] + secondary[v], 255);
			dst_ptr[4] = imin((int)dst_ptr[4] + tertiary[v], 255);
			++dst_ptr;
		}
	}
	w2 = dst_width;
}

static void lcd_prepare_glyph2(unsigned char* gbuf2, int& w2, const unsigned char* gbuf1, int w1, int h1) {
	int src_stride = (w1 + 3) / 4 * 4;
	int dst_width = src_stride + 4;
	for(int y = 0; y < h1; ++y) {
		const unsigned char* src_ptr = gbuf1 + src_stride * y;
		unsigned char* dst_ptr = gbuf2 + dst_width * y;
		unsigned char v;
		for(int x = 0; x < w1; ++x) {
			v = *src_ptr++;
			*dst_ptr = (v >> 1) << 2;
			++dst_ptr;
		}
	}
	w2 = dst_width;
}

static void apply_level(unsigned char* p, int w, int h, int s, int min, int shift) {
	for(auto y = 0; y < h; y++) {
		for(auto x = 0; x < w; x++) {
			auto v = p[y * s + x];
			if(v >= min)
				v = 255;
			else
				v = v << shift;
			p[y * s + x] = v;
		}
	}
}

static void glyph(font& font, int g, sprite& ei) {
	int w1 = 0, w2 = 0, dx, dy, ox, oy, w9, h9;
	int h1 = 0;
	memset(buf1, 0, sizeof(buf1));
	memset(buf2, 0, sizeof(buf2));
	int size = font.glyphi(g, w1, h1, dx, dy, ox, oy, buf1, sizeof(buf1), 3, 1, 0);
	if(!size)
		return;
	lcd_prepare_glyph(buf2, w2, buf1, w1, h1);
	font.glyphi(g, w9, h9, dx, dy, ox, oy);
	sprite_store(&ei, buf2, w2, ((w1 + 5) / 3) * 3, h1, -ox, oy, sprite::ALC);
	int id = ei.glyph(g);
	widths[id] = dx;
}

static void glyphna(font& font, int g, sprite& ei) {
	int dx, dy, ox, oy, w, h;
	memset(buf1, 0, sizeof(buf1));
	int size = font.glyphi(g, w, h, dx, dy, ox, oy, buf1, sizeof(buf1), 1, 1, 4);
	if(!size)
		return;
	auto w1 = (w + 3) / 4 * 4;
	apply_level(buf1, w, h, w1, 16, 3);
	sprite_store(&ei, buf1, w1, w, h, -ox, oy, sprite::ALC8);
	int id = ei.glyph(g);
	widths[id] = dx;
}

static int corrected(int symbol, int width) {
	//switch(symbol) {
	//case 0x43A: return width - 1;
	//default: return width;
	//}
	return width;
}

static void glyphmh(font& font, int g, sprite& ei) {
	int dx, dy, ox, oy, w, h;
	auto s = buf1;
	memset(buf1, 0, sizeof(buf1));
	int size = font.glyphi(g, w, h, dx, dy, ox, oy, buf1, sizeof(buf1), 1, 1, 1);
	if(!size)
		return;
	auto w1 = ((w + 31) / 32) * 4;
	sprite_store(&ei, buf1, w1, w, h, -ox, oy, sprite::RAW1);
	int id = ei.glyph(g);
	widths[id] = corrected(g, w + 1);
}

void pma_write(const char* url, pma** pp);

void font_write(const char* url, const char* name, int size, sprite::encodes encode) {
	int height, width;
	int ascend;
	int descend;
	lcd_init(2.0, 0.2, 0.05);
	font font(name, size, false);
	int glyph_count = 0;
	for(auto& e : intervals)
		glyph_count += e[1] - e[0] + 1;
	auto p = (sprite*)new char[256 * 256 * 8];
	if(!p)
		return;
	sprite_create(p, glyph_count);
	sprite_add(p, intervals, sizeof(intervals));
	for(auto& e : intervals) {
		for(int i = e[0]; i <= e[1]; i++) {
			switch(encode) {
			case sprite::ALC8: glyphna(font, i, *p); break;
			case sprite::RAW1: glyphmh(font, i, *p); break;
			default: glyph(font, i, *p); break;
			}
		}
	}
	sprite_add(p, widths, sizeof(widths[0]) * glyph_count);
	font.info(width, height, ascend, descend);
	p->height = height;
	p->width = draw::textw(p);
	p->ascend = ascend;
	p->descend = descend;
	sprite_write(url, p);
	delete[] p;
}

static void initialize_ascii_decoder() {
	memset(ascii_decoder, 0, sizeof(ascii_decoder));
	for(auto i = 0; i < 128; i++)
		ascii_decoder[i] = i;
	for(auto i = 0xA0; i < 256; i++)
		ascii_decoder[i] = i;
	ascii_decoder[149] = 0x2022;
	ascii_decoder[165] = 0x490;
	ascii_decoder[168] = 0x401;
	ascii_decoder[170] = 0x404;
	ascii_decoder[175] = 0x407;
	ascii_decoder[178] = 0x406;
	ascii_decoder[179] = 0x456;
	ascii_decoder[180] = 0x491;
	ascii_decoder[184] = 0x451;
	ascii_decoder[186] = 0x454;
	ascii_decoder[191] = 0x457;
	for(auto i = 0xC0; i < 256; i++)
		ascii_decoder[i] = 0x410 + (i - 0xC0);
}

static void put_mono(unsigned char* d, unsigned dn, unsigned char* s, unsigned sn, int w, int h) {
	for(auto y = 0; y < h; y++) {
		for(auto x = 0; x < w; x++) {
			unsigned char n = 0;
			if(s[x / 8] & (0x80 >> (x % 8)))
				n = 1;
			d[x] = n;
		}
		s += sn;
		d += dn;
	}
}

static void put_gray(unsigned char* d, int dn, unsigned char* s, int sn, int w, int h, unsigned char sym) {
	while(h-- > 0) {
		for(auto x = 0; x < w; x++) {
			if(s[x])
				d[x] = sym;
		}
		d += dn;
		s += sn;
	}
}

static void put_gray(unsigned char* d, int dn, unsigned char* s, int sn, int w, int h, unsigned char sym, unsigned char sym_dest) {
	while(h-- > 0) {
		for(auto x = 0; x < w; x++) {
			if(s[x] == sym)
				d[x] = sym_dest;
			else
				d[x] = 0;
		}
		d += dn;
		s += sn;
	}
}


// tool used to create font with SubPixel rendering from system font's
static int font_glyphi(int glyph, int& width, int& height, int& dx, int& dy, int& ox, int& oy, unsigned char* buffer, int maxsize, int xscale, int yscale) {
	MAT2 scale3h = {0};
	scale3h.eM11.value = xscale; // rgb color
	scale3h.eM22.value = yscale;
	GLYPHMETRICS gm = {0};
	int size = GetGlyphOutlineW(hcnv,
		glyph,
		GGO_BITMAP,
		&gm,
		maxsize,
		buffer,
		&scale3h);
	width = gm.gmBlackBoxX;
	height = gm.gmBlackBoxY;
	dx = gm.gmCellIncX;
	dy = gm.gmCellIncY;
	ox = gm.gmptGlyphOrigin.x;
	oy = gm.gmptGlyphOrigin.y;
	return size;
}

static void glyph_mono(int g, sprite& ei, sprite::encodes encode, int index) {
	const int glyph_start = 32;
	const int glyph_count = 256 - glyph_start;
	int dx, dy, ox, oy, w, h;
	memset(buf1, 0, sizeof(buf1));
	memset(buf2, 0, sizeof(buf2));
	int size = font_glyphi(g, w, h, dx, dy, ox, oy, buf1, sizeof(buf1), 1, 1);
	if(size <= 0)
		return;
	auto w1 = w + 4, h1 = h + 4;
	auto sn = w1;
	ox++; oy++;
	put_mono(buf2, sn, buf1, ((w + 31) / 32) * 4, w, h);
	memset(buf1, 0, sizeof(buf1));
	put_gray(buf1 + 0 + 0 * sn, sn, buf2, sn, w, h, 3);
	put_gray(buf1 + 1 + 0 * sn, sn, buf2, sn, w, h, 3);
	put_gray(buf1 + 2 + 0 * sn, sn, buf2, sn, w, h, 3);
	put_gray(buf1 + 0 + 1 * sn, sn, buf2, sn, w, h, 3);
	put_gray(buf1 + 0 + 2 * sn, sn, buf2, sn, w, h, 3);
	put_gray(buf1 + 2 + 1 * sn, sn, buf2, sn, w, h, 2);
	put_gray(buf1 + 2 + 2 * sn, sn, buf2, sn, w, h, 2);
	put_gray(buf1 + 1 + 2 * sn, sn, buf2, sn, w, h, 2);
	put_gray(buf1 + 1 + 1 * sn, sn, buf2, sn, w, h, 1);
	// Copy back image
	memset(buf2, 0, sizeof(buf2)); put_gray(buf2, sn, buf1, sn, w1, h1, 1, 1);
	sprite_store(&ei, buf2, sn, w1, h1, -ox, oy - height_diff, encode, 0, 0, index, 0);
	memset(buf2, 0, sizeof(buf2)); put_gray(buf2, sn, buf1, sn, w1, h1, 2, 1);
	sprite_store(&ei, buf2, sn, w1, h1, -ox, oy - height_diff, encode, 0, 0, index + glyph_count * 1, 0);
	memset(buf2, 0, sizeof(buf2)); put_gray(buf2, sn, buf1, sn, w1, h1, 3, 1);
	sprite_store(&ei, buf2, sn, w1, h1, -ox, oy - height_diff, encode, 0, 0, index + glyph_count * 2, 0);
	widths[index] = dx;
}

void font_write_ascii(const char* url, const char* name, int size, sprite::encodes encode) {
	font_create(name, size);
	auto p = (sprite*)new char[256 * 256 * 8];
	if(!p)
		return;
	initialize_ascii_decoder();
	const int glyph_start = 32;
	const int glyph_count = 256 - glyph_start;
	sprite_create(p, glyph_count * 3);
	int height, width, ascend, descend;
	font_info(width, height, ascend, descend);
	p->height = height;
	p->ascend = ascend;
	p->descend = descend;
	height_diff = p->height - p->descend;
	for(auto i = glyph_start; i < 256; i++) {
		auto g = i - glyph_start;
		auto u = ascii_decoder[i];
		glyph_mono(u, *p, encode, g);
	}
	sprite_add(p, widths, sizeof(widths[0]) * glyph_count);
	p->width = widths['l' - glyph_start];
	sprite_write(url, p);
	font_delete();
	delete[] p;
}