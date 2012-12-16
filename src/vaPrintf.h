#ifndef VAPRINTF_H
#define VAPRINTF_H

#ifdef _MSC_VER
	#include <windows.h>
#else
	#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>

char* vaPrintf(const char *fmt, ...);

#endif // VAPRINTF_H
