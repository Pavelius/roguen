#include "draw.h"

const int glyph_start = 32;
const int glyph_count = 256 - glyph_start;

void draw::glyph(int n, unsigned flags) {
	if(n <= glyph_start)
		return;
	auto push_fore = fore;
	image(font, n - glyph_start, 0);
	if(flags & TextBold) {
		fore = fore_stroke;
		image(font, n - glyph_start + glyph_count * 1, 0);
	}
	if(flags & TextItalic) {
		fore = fore_stroke;
		image(font, n - glyph_start + glyph_count * 2, 0);
	}
	fore = push_fore;
}

int draw::textw(int n) {
	if(n <= glyph_start)
		n = 'l';
	auto widths = (short int*)font->ptr(font->size - glyph_count * sizeof(short int));
	return widths[n - glyph_start];
}

int draw::texth() {
	if(!font)
		return 0;
	return font->height;
}