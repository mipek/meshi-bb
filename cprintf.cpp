#include "cprintf.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string>
#include <sstream>

#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define KDEF  "\x1b[0m"


// c_printf("{g}Success {d}good\n");

static const char *CharToColor(char k) {
	switch(k){
	// Maybe I will add the word instead of a letter
	// or both. Let's see whats better.
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


std::string FormatColorString(const char *in)
{
	std::stringstream ss;
	std::string s = in;
	char previousColor = 'd';
	for (size_t i=0; i<s.length(); ++i) {
		char color = previousColor;
		if (s[i] == '{' && s[i+2] == '}') {
			color = s[i+1];
			i += 3;
		}
		if (color != previousColor) {
			ss << CharToColor(color);
			previousColor = color;
		}
		ss << s[i];
	}
	return ss.str();
}

void c_printf(const char *cf, ...)
{
	va_list args;
	va_start(args, cf);
	std::string f = FormatColorString(cf);
	if (!f.empty()) {
		vprintf(f.c_str(), args);
		va_end(args);
	}
}