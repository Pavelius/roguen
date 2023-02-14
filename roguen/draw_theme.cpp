#include "draw.h"

void set_dark_theme() {
	colors::window = color(32, 32, 32);
	colors::active = color(172, 128, 0);
	colors::border = color(73, 73, 80);
	colors::form = color(45, 45, 48);
	colors::text = color(255, 255, 255);
	colors::button = color(55, 55, 58);
	colors::special = color(255, 244, 32);
	colors::border = color(63, 63, 70);
	colors::tips::text = color(255, 255, 255);
	colors::tips::back = color(100, 100, 120);
	colors::h1 = colors::text.mix(colors::button, 64);
	colors::h2 = colors::text.mix(colors::button, 96);
	colors::h3 = colors::text.mix(colors::button, 128);
}