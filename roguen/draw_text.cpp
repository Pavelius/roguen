#include "draw.h"

inline short* fwidth(const sprite* font) {
	return (short*)((char*)font + font->size - font->count * sizeof(short));
}

inline int wsymbol(const sprite* font, unsigned u) {
	return (u <= 0x20) ? 't' - 0x21 : font->glyph(u);
}

int draw::textw(int sym) {
	if(!font)
		return 0;
	return fwidth(font)[wsymbol(font, sym)];
}

int draw::textw(const sprite* font) {
	if(!font)
		return 0;
	return fwidth(font)[wsymbol(font, 'A')];
}

void draw::glyph(int sym, unsigned feats) {
	static unsigned char koeff[] = {128, 160};
	int id = font->glyph(sym);
	if(sym >= 0x21) {
		if(feats&TextStroke) {
			color push_fore = fore;
			fore = fore_stroke;
			stroke(caret.x, caret.y + font->ascend, font, id, feats, 2, koeff);
			fore = push_fore;
		}
		image(caret.x, caret.y + font->ascend, font, id, feats);
	}
}

int draw::texth() {
	if(!font)
		return 0;
	return font->height;
}