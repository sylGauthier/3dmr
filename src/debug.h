#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
static void dbg_printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	fprintf(stdout, "Debug:");
	vfprintf(stdout, fmt, args);
	va_end(args);
}
#else
static void dbg_printf(const char *fmt, ...) {}
#endif

#endif
