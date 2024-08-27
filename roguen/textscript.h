#pragma once

#include "stringbuilder.h"

struct textscript {
	const char*	id;
	fnprint		proc;
};

void actv(stringbuilder& sb, const char* format, const char* format_param, char separator);
bool actn(stringbuilder& sb, const char* id, const char* action, const char* format_param, char separator);
void actvf(stringbuilder& sb, char separator, const char* format, ...);
void logv(const char* format, const char* format_param);
void logv(const char* format);
void sayv(stringbuilder& sb, const char* format, const char* format_param);
void sayv(stringbuilder& sb, const char* format);
void initialize_strings();

const char* getlog();
