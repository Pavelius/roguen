#include "color.h"
#include "point.h"
#include "stringbuilder.h"

#pragma once

typedef void(*fnevent)();
typedef void(*fnbutton)(const char* title, bool pressed);

enum draw_event_s : unsigned {
	KeyBackspace = 8, KeyTab = 9, KeyEnter = 10, KeyEscape = 0x1B, KeySpace = 0x20, KeyDelete = 0x7F,
	// input events
	InputSymbol = 0x80, InputTimer, InputKeyUp, InputIdle, InputNeedUpdate, InputUpdate, InputNoUpdate,
	// Keyboard and mouse input (can be overrided by flags)
	MouseLeft, MouseLeftDBL, MouseRight,
	MouseMove, MouseWheelUp, MouseWheelDown,
	KeyLeft, KeyRight, KeyUp, KeyDown, KeyPageUp, KeyPageDown, KeyHome, KeyEnd,
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
	// support
	CommandMask = 0x000000FF,
	Ctrl = 0x00000100,
	Alt = 0x00000200,
	Shift = 0x00000400,
};
enum image_flag_s {
	ImageMirrorV = 0x0001,
	ImageMirrorH = 0x0002,
	ImageGrayscale = 0x0004,
	ImageNoOffset = 0x0008,
	ImageTransparent = 0x0010,
	ImageColor = 0x0020,
	ImagePallette = 0x0040,
	TextStroke = 0x0080,
	TextItalic = 0x0100,
	TextBold = 0x0200,
	TextUscope = 0x0400,
	TextSingleLine = 0x0800,
	AlignLeft = 0x0000,
	AlignCenter = 0x1000,
	AlignRight = 0x2000,
	AlignLeftCenter = 0x3000,
	AlignCenterCenter = 0x4000,
	AlignRightCenter = 0x5000,
	AlignLeftBottom = 0x6000,
	AlignCenterBottom = 0x7000,
	AlignRightBottom = 0x8000,
	AlignWidth = 0xE000,
	AlignMask = 0xF000,
};
enum window_flag_s {
	WFResize = 0x0010,
	WFMinmax = 0x0020,
	WFMaximized = 0x0040,
	WFAbsolutePos = 0x0080,
};
enum class cursor : unsigned char {
	Arrow, Hand, LeftRight, UpDown, All, No, Edit, Wait,
};
enum class res;
struct pma {
	char				name[4]; // Identifier of current block
	int					size; // Size of all block
	int					count; // Count of records in this block
	const pma*			getheader(const char* name) const;
};
struct sprite : pma {
	enum encodes { Auto, RAW, RLE, ALC, RAW8, RLE8, ALC8, RAW1, ALC41, ALC1 };
	struct frame {
		short 			sx, sy;
		short			ox, oy;
		encodes			encode;
		unsigned		pallette;
		unsigned		offset;
		rect			getrect(int x, int y, unsigned feats) const;
	};
	struct cicle {
		short unsigned	start;
		short unsigned	count;
	};
	short int			width, height; // common size of all frames (if applicable)
	short int			ascend, descend; // top or down ascend
	short unsigned		feats; // must be zero
	unsigned			cicles; // count of anim structure
	unsigned			cicles_offset;
	frame				frames[1];
	int					esize() const { return frames[0].offset - (sizeof(sprite) + sizeof(frame)*(count - 1)); }
	const unsigned char* edata() const { return (const unsigned char*)this + sizeof(sprite) + sizeof(frame)*(count - 1); }
	int					ganim(int index, int tick) const;
	const frame&		get(int id) const { return frames[(id >= count) ? 0 : id]; }
	cicle*				gcicle(int index) const { return (cicle*)ptr(cicles_offset) + index; }
	int					glyph(unsigned sym) const;
	const unsigned char* ptr(unsigned o) const { return (unsigned char*)this + o; }
};
namespace colors {
extern color			button, form, window;
extern color			border, active;
extern color			text, h1, h2, h3, special;
namespace tips {
extern color			back;
extern color			text;
}
}
namespace metrics {
extern sprite*			font;
extern sprite*			small;
extern sprite*			h1;
extern sprite*			h2;
extern sprite*			h3;
extern sprite*			icons;
extern int				border, padding;
}
namespace draw {
struct hoti {
	enum cursor			cursor; // set this mouse cursor
	unsigned			key; // if pressed key or mouse this field has key
	point				mouse; // current mouse coordinates
	bool				pressed; // flag if any of mouse keys is pressed
	long				param, param2; // command context or parameters
	const void*			object; // command object
	rect				hilite, focus;
	explicit operator bool() const { return key != 0; }
};
extern hoti				hot;
struct surface {
	struct plugin {
		const char*		name;
		const char*		filter;
		plugin*			next;
		static plugin*	first;
		plugin(const char* name, const char* filter);
		virtual bool	decode(unsigned char* output, int output_bpp, const unsigned char* input, unsigned input_size) = 0;
		virtual bool	inspect(int& w, int& h, int& bpp, const unsigned char* input, unsigned size) = 0;
	};
	int					width;
	int					height;
	int					scanline;
	int					bpp;
	unsigned char*		bits;
	surface();
	surface(int width, int height, int bpp);
	surface(const char* url, color* pallette = 0);
	~surface();
	constexpr explicit operator bool() const { return bits != 0; }
	static unsigned char* allocator(unsigned char* bits, unsigned size);
	void				blend(const surface& source, int alpha);
	void				blit(int x, int y, int width, int height, bool transparent, const surface& source, int x_source, int y_source);
	void				blit(int x, int y, int width, int height, bool transparent, const surface& source, int x_source, int y_source, int width_source, int height_source);
	void				clear() { resize(0, 0, 0, true); }
	void				convert(int bpp, color* pallette);
	void				flipv();
	unsigned char*		ptr(int x, int y);
	bool				read(const char* url, color* pallette = 0, int need_bpp = 0);
	void				resize(int width, int height, int bpp, bool alloc_memory);
	void				rotate();
	void				write(const char* url, color* pallette);
};
extern surface*			canvas;
extern rect				clipping; // Clipping area
extern fnevent			domodal;
extern point			dragmouse, caret, camera, tips_caret, tips_size;
extern color			fore, fore_stroke;
extern const void*		hilite_object;
extern point			hilite_position;
extern int				hilite_size;
extern unsigned char    alpha;
extern int				width, height, fsize, dialog_width;
extern bool				text_clipped, control_hilited;
extern const sprite*	font; // Currently selected font
extern double			linw;
extern stringbuilder	tips_sb;
extern long				text_params[16];
extern color*			palt;
extern int				tab_pixels;
extern fnevent          pbackground, ptips, pfinish, pinput;
extern fnevent          pbeforemodal, pleavemodal, psetfocus;
extern fnbutton			pbutton;
struct rectpush {
	point				caret;
	int					width, height;
	constexpr rectpush() : caret(draw::caret), width(draw::width), height(draw::height) {}
	~rectpush() { draw::caret = caret; draw::width = width; draw::height = height; }
};
int						aligned(int x, int width, unsigned state, int string_width);
int						alignedh(const rect& rc, const char* string, unsigned state);
void					circle(int size);
void					circlef(int size);
bool					dragactive(const void* p);
bool					dragactive();
void					dragbegin(const void* p);
void					dropshadow();
void					execute(fnevent proc, long value = 0, long value2 = 0, const void* object = 0);
void					fhexagon();
void					fillactive();
void					fillbutton();
void					filldark();
void					fillform();
void					fillwindow();
int						getbpp();
inline rect				getrect() { return {caret.x, caret.y, caret.x + width, caret.y + height}; }
int						getheight();
int						getwidth();
void					glyph(int sym, unsigned feats);
void					gradv(const color c1, const color c2, int skip = 0);
void					gradh(const color c1, const color c2, int skip = 0);
const sprite*			gres(res id);
void					hexagon();
int						hittest(int x, int test_x, const char* string, int lenght);
int						hittest(rect rc, const char* string, unsigned state, point mouse);
bool					ishilite(const rect& rc);
inline bool				ishilite() { return ishilite(getrect()); }
inline bool				ishilite(int size) { return ishilite({caret.x-size, caret.y - size, caret.x + size, caret.y + size}); }
void					image(int x, int y, const sprite* e, int id, int feats);
inline void				image(const sprite* e, int id, int feats) { image(caret.x, caret.y, e, id, feats); }
void					image(const sprite* e, int id, int feats, color* pal);
void					imager(int x, int y, const sprite* p, int id, int radius);
void					key2str(stringbuilder& sb, int key);
void					line(int x, int y); // Draw line
void					linet(int x, int y);
void					loadposition(rect& v);
void					paintstart();
void					paintfinish();
void					pixel(int x, int y);
void					pixel(int x, int y, unsigned char alpha);
unsigned char*			ptr(int x, int y);
int						rawinput();
void					rectb(); // Draw rectangle border
void					rectb3d(); // Draw rectangle border
void					rectf(); // Draw rectangle area-> Right and bottom side is one pixel less.
void					rectfe(rect rc, int radius);
void					rectx();
void					rectfocus();
void					saveposition(rect& v);
void					setcamera(point v);
void					setcaret(point v);
void					setcaption(const char* string);
void					setclip(rect rc);
inline void				setclip() { clipping.set(0, 0, getwidth(), getheight()); }
inline void				setclipall() { setclip({caret.x, caret.y, caret.x + width, caret.y + height}); }
void					setoffset(int x, int y);
void					setpos(int x, int y);
void					setpos(int x, int y, int width, int height);
void					settimer(unsigned milleseconds);
const char*				skiptr(const char* string);
void					stroke(int x, int y, const sprite* e, int id, int feats, unsigned char thin = 1, unsigned char* koeff = 0);
void					strokeactive();
void					strokeborder();
void					strokeline();
void					strokeout(fnevent proc, int dx = 0, int dy = 0);
void					syscursor(bool enable);
void					text(const char* string, int count = -1, unsigned feats = 0);
int						text(rect rc, const char* string, unsigned state = 0, int* max_width = 0);
void					texta(const char* string, unsigned state = 0);
void					textas(const char* string);
void					textc(const char* string, int count = -1, unsigned feats = 0);
void					textcj(const char* string);
int						textbc(const char* string, int width);
int						texte(rect rc, const char* string, unsigned feats, int i1, int i2);
void					textf(const char* string);
void					textf(const char* string, int& origin, int& maximum, int& cashe_origin, int& cashe_string);
void					textfs(const char* string);
int						texth();
void					textrp(const char* string, int count = -1, unsigned feats = 0);
int						texth(const char* string, int width);
int						textw(int sym);
int						textw(const char* string, int count = -1);
int						textw(rect& rc, const char* string);
int						textw(const sprite* font);
void					tipspos();
inline void				tooltips(const char* format, ...) { tips_sb.addv(format, xva_start(format)); }
void					updatewindow();
void					write(const char* url, unsigned char* bits, int width, int height, int bpp, int scanline, color* pallette);
void					vertical(fnevent proc);
}
namespace draw {
void breakmodal(long result);
bool button(const char* title, unsigned key, fnbutton proc, bool vertical = true, unsigned* keybinds = 0);
void buttoncancel();
void buttonok();
void buttonparam();
void cbsetsht();
void cbsetint();
void cbsetptr();
void doredraw();
void fire(bool run, fnevent proc, long value = 0, long value2 = 0, const void* object = 0);
long getresult();
bool isnext();
void initialize(const char* title);
bool ismodal();
void* scene(fnevent proc);
void scene();
void setneedupdate();
void setnext(fnevent v);
void start();
}
namespace draw {
bool isclipped(int size);
void setposct();
void setposld();
void setposlu();
void setposru();
}