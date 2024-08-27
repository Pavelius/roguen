#pragma once

struct charname;

const char*	get_charname(unsigned short v);

short unsigned select_charname(short unsigned* pb, short unsigned* pe, const char* pattern);
short unsigned random_charname(const char* pattern);

void read_charname(const char* url);