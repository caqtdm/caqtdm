#ifndef VAPRINTF_H
#define VAPRINTF_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

char* vaPrintf(const char *fmt, ...);

#endif // VAPRINTF_H
