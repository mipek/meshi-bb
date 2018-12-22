#include "blackbox.h"
#include "cprintf.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string>
#include <sstream>

#if PLAT == PLAT_WINDOWS
#	include <Windows.h>

static WORD char_to_attribute(char k)
{
	switch (k) {
	case 'r': return 12;
	case 'g': return 10;
	case 'y': return 6;
	case 'b': return 9;
	case 'm': return 5;
	case 'c': return 11;
	case 'w': return 8;
	case 'd': // fall trough
	default:
		return 7;
	}
}
#else
#	define KRED  "\x1B[31m"
#	define KGRN  "\x1B[32m"
#	define KYEL  "\x1B[33m"
#	define KBLU  "\x1B[34m"
#	define KMAG  "\x1B[35m"
#	define KCYN  "\x1B[36m"
#	define KWHT  "\x1B[37m"
#	define KDEF  "\x1b[0m"

static const char *char_to_color(char k)
{
	switch(k) {
	case 'r': return KRED;
	case 'g': return KGRN;
	case 'y': return KYEL;
	case 'b': return KBLU;
	case 'm': return KMAG;
	case 'c': return KCYN;
	case 'w': return KWHT;
	case 'd': // fall trough
	default:
		return KDEF;		
	}
}
#endif

void output_color_string(const char *format, va_list &args)
{
	std::stringstream ss;
	size_t inlen;
#if PLAT == PLAT_WINDOWS
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	char s[1024];
	inlen = _vsnprintf_s(s, sizeof(s), format, args);
#else
	std::string s = format;
	inlen = s.length();
#endif

	char previous_color = 'd';
	for (size_t i=0; i<inlen; ++i) {
		char color = previous_color;
		if (s[i] == '{' && s[i+2] == '}') {
			color = s[i+1];
			i += 3;
		}

		// did the color change?
		if (color != previous_color) {
#if PLAT == PLAT_WINDOWS
			SetConsoleTextAttribute(hConsole, char_to_attribute(color));
#else
			ss << char_to_color(color);
#endif
			previous_color = color;
		}

#if PLAT == PLAT_WINDOWS
		fputc(s[i], stderr);
#else
		ss << s[i];
#endif
	}

#if PLAT != PLAT_WINDOWS
	std::string f = ss.str();
	if (!f.empty()) {
		vprintf(f.c_str(), args);
	}
#endif
}

void c_printf(const char *cf, ...)
{
	va_list args;
	va_start(args, cf);

#if PLAT == PLAT_WINDOWS
	// save console state
	CONSOLE_SCREEN_BUFFER_INFO inf;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hConsole, &inf);
#endif

	output_color_string(cf, args);

#if PLAT == PLAT_WINDOWS
	// restore
	SetConsoleTextAttribute(hConsole, inf.wAttributes);
#endif

	va_end(args);
}