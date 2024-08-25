#pragma once

namespace log {
extern bool allowparse;
typedef void(*fnread)(const char* url);
void			close();
bool			checksym(const char* p, char sym);
void			errorp(const char* position, const char* format, ...);
extern int		errors;
void			errorv(const char* position, const char* format);
const char*		read(const char* url, bool error_if_not_exist = true);
void			readf(fnread proc, const char* url, const char* filter);
void			readlf(fnread proc, const char* url, const char* filter);
void            setfile(const char* v);
void            seturl(const char* v);
const char*		skipws(const char* p);
const char*		skipwscr(const char* p);
}
