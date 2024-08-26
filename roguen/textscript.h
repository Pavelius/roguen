#pragma once

#include "stringbuilder.h"

struct textscript {
	const char*	id;
	fnprint		proc;
};

void actv(stringbuilder& sb, const char* format, const char* format_param, char separator);
void actvf(stringbuilder& sb, char separator, const char* format, ...);
void logv(const char* format, const char* format_param);
void logv(const char* format);
void sayva(stringbuilder& sb, const char* format, const char* format_param);
void initialize_strings();

const char* getlog();
