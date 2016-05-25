//  edm - extensible display manager

//  Copyright (C) 1999 John W. Sinclair

//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef __expString_h
#define __expString_h 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STATE_DONE -1
#define STATE_DONE -1
#define STATE_COPY_OUT 1
#define STATE_FIND_LEFT_PAREN 2
#define STATE_FIND_NON_WHITE 3
#define STATE_COPY_MACRO 4

#define MAX_EXPAND_SIZE 1024

#define EXPSTR_SUCCESS 1
#define EXPSTR_OUTOVFL 100
#define EXPSTR_MACOVFL 102
#define EXPSTR_SYNTAX  104

#ifdef __expString_cc

static int expand (
  int numMacros,
  char *macro[],
  char *expansion[],
  int preserveSymbols,
  char *inString,
  int inStringSize,
  int inStringLen,
  char **outString,
  int *outStringSize,
  int *outStringLen,
  int numPossibleSymbols,
  int *numSymbolsFound );

#endif

class expStringClass {

private:

friend int expand (
  int numMacros,
  char *macro[],
  char *expansion[],
  int preserveSymbols,
  char *inString,
  int inStringSize,
  int inStringLen,
  char **outString,
  int *outStringSize,
  int *outStringLen,
  int numPossibleSymbols,
  int *numSymbolsFound );

char *expandedString1, *expandedString, *expandedString2;
int rawStringSize, rawStringLen;
int expandedString1Size, expandedString1Len, numPossibleSymbols1;
int expandedStringSize, expandedStringLen, numPossibleSymbols;
int expandedString2Size, expandedString2Len, numPossibleSymbols2;

public:

char *rawString;

expStringClass ( void );

~expStringClass ( void );

void copy ( const expStringClass &source );

char *getRaw ( void );

int setRaw (
  char *str );

int containsPrimaryMacros ( void ) {

  if ( numPossibleSymbols1 )
    return 1;
  else
    return 0;

}

int containsSecondaryMacros ( void ) {

  if ( numPossibleSymbols2 )
    return 1;
  else
    return 0;

}

char *getExpanded ( void );

int expand1st (
  int numMacros,
  char *macro[],
  char *expansion[] );

int expand2nd (
  int numMacros,
  char *macro[],
  char *expansion[] );

int expandFinal ( void );

void reset ( void );

};

#endif
