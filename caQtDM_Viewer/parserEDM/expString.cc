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

#define __expString_cc 1

#include "expString.h"
#include "utility.h"
#include "environment.str"

static char *g_expStrBlank = "";

static int valFromExec (
  char *val,
  int max,
  int *execBufSize,
  char *execBuf
) {

FILE *f;
int i, i0, i1, saveLoc, result;
static int ignoreExec = -1;

  if ( ignoreExec == -1 ) {
    if ( getenv( environment_str35 ) ) {
      ignoreExec = 1;
    }
    else {
      ignoreExec = 0;
    }
  }

  execBuf[0] = 0;
  *execBufSize = 0;

  if ( ignoreExec ) return 0;

  result = 0;
  saveLoc = 0;

  if ( val ) {

    if ( val[0] == '`' ) {

      i0 = 1;
      i1 = strlen( val ) - 1;
      if ( i1 >= i0 ) {

	for ( i=i1; i>i0; i-- ) {
          if ( val[i] == '`' ) {
            val[i] = 0;
            saveLoc = i;
	  }
	}

        val[i1] = 0;
        f = popen( &val[1], "r" );
        if ( f ) {

          fgets( execBuf, max, f );
          execBuf[max] = 0;
          trimWhiteSpace( execBuf );
          *execBufSize = strlen( execBuf );
          fclose( f );

	  result = 1;

	}

	if ( saveLoc ) val[saveLoc] = '`';

      }

    }

  }

  return result;

}

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
  int *numSymbolsFound )
{

char buf[MAX_EXPAND_SIZE+1];
char execBuf[MAX_EXPAND_SIZE+1];
char thisMacro[MAX_EXPAND_SIZE+1];
int state, foundOne, i, ii, nOut, nIn, nMacro,
 outStrLen, execBufSize, needSpace;

  *numSymbolsFound = 0;

  if ( numPossibleSymbols == 0 ) return EXPSTR_SUCCESS;

  nOut = nIn = nMacro = 0;
  outStrLen = 0;
  state = STATE_COPY_OUT;

  while ( state != STATE_DONE ) {

    // fprintf( stderr, "state = %-d, char = %c\n", state, inString[nIn] );

    switch ( state ) {

    // ====================================================================

    case STATE_COPY_OUT:

      if ( nIn >= inStringLen ) {

        if ( nOut+1 >= MAX_EXPAND_SIZE ) return EXPSTR_OUTOVFL;
        buf[nOut] = 0;

        state = STATE_DONE;

      }
      else if ( inString[nIn] == '$' ) {

        strcpy( thisMacro, "" );
        nMacro = 0;
        state = STATE_FIND_LEFT_PAREN;

      }
      else {

        if ( nOut+1 >= MAX_EXPAND_SIZE ) return EXPSTR_OUTOVFL;
        buf[nOut] = inString[nIn];
        nOut++;

      }

      break;

    // ====================================================================

    case STATE_FIND_LEFT_PAREN:

      if ( nIn >= inStringLen ) {

        if ( nOut+1 >= MAX_EXPAND_SIZE ) return EXPSTR_OUTOVFL;
        buf[nOut] = '$';
        nOut++;

	state = STATE_DONE;

      }
      else if ( inString[nIn] == '(' ) {

        state = STATE_FIND_NON_WHITE;

      }
      else {

        if ( nOut+1 >= MAX_EXPAND_SIZE ) return EXPSTR_OUTOVFL;
        buf[nOut] = '$';
        nOut++;

        if ( nOut+1 >= MAX_EXPAND_SIZE ) return EXPSTR_OUTOVFL;
        buf[nOut] = inString[nIn];
        nOut++;

        state = STATE_COPY_OUT;

      }

      break;

    // ====================================================================

    case STATE_FIND_NON_WHITE:

      if ( nIn >= inStringLen ) {

        return EXPSTR_SYNTAX;

      }
      else if ( inString[nIn] == ')' ) {

        return EXPSTR_SYNTAX;

      }
      else if ( ( inString[nIn] != ' ' ) && ( inString[nIn] != '\t' ) ) {

        state = STATE_COPY_MACRO;

        if ( nMacro+1 >= MAX_EXPAND_SIZE ) return EXPSTR_MACOVFL;
        thisMacro[nMacro] = inString[nIn];
        nMacro++;

      }

      break;

    // ====================================================================

    case STATE_COPY_MACRO:

      if ( nIn >= inStringLen ) {

        return EXPSTR_SYNTAX;

      }
      else if ( inString[nIn] == ')' ) {

        state = STATE_COPY_OUT;

        if ( nMacro+1 >= MAX_EXPAND_SIZE ) return EXPSTR_MACOVFL;
        thisMacro[nMacro] = 0;

        foundOne = 0;

        // do substitution
        // fprintf( stderr, "processing [%s]\n", thisMacro );
        for ( i=0; i<numMacros; i++ ) {

          if ( strcmp( thisMacro, macro[i] ) == 0 ) {

            if ( valFromExec( expansion[i], MAX_EXPAND_SIZE,
             &execBufSize, execBuf ) ) {

	      needSpace = 0;
              for ( ii=0; ii<execBufSize; ii++ ) {
                if ( nOut+1 >= MAX_EXPAND_SIZE ) return EXPSTR_OUTOVFL;
                if ( execBuf[ii] == '\n' ) {
                  needSpace = 1;
		}
                else if ( execBuf[ii] == '\t' ) {
                  buf[nOut] = ' ';
                  nOut++;
		}
		else {
                  if ( needSpace ) {
                    needSpace = 0;
                    buf[nOut] = ' ';
                    nOut++;
                    if ( nOut+1 >= MAX_EXPAND_SIZE ) return EXPSTR_OUTOVFL;
                    buf[nOut] = execBuf[ii];
                    nOut++;
		  }
		  else {
                    buf[nOut] = execBuf[ii];
                    nOut++;
		  }
		}
              }

	    }
	    else {

              for ( ii=0; ii<(int) strlen(expansion[i]); ii++ ) {
                if ( nOut+1 >= MAX_EXPAND_SIZE ) return EXPSTR_OUTOVFL;
                buf[nOut] = expansion[i][ii];
                nOut++;
              }

	    }

            (*numSymbolsFound)++;
            foundOne = 1;
            break; // found one, don't continue testing for more

          }

        }

        if ( !foundOne && preserveSymbols ) {
          if ( nOut+1 >= MAX_EXPAND_SIZE ) return EXPSTR_OUTOVFL;
          buf[nOut] = '$';
          nOut++;
          if ( nOut+1 >= MAX_EXPAND_SIZE ) return EXPSTR_OUTOVFL;
          buf[nOut] = '(';
          nOut++;
          for ( i=0; i<(int) strlen(thisMacro); i++ ) {
            if ( nOut+1 >= MAX_EXPAND_SIZE ) return EXPSTR_OUTOVFL;
            buf[nOut] = thisMacro[i];
            nOut++;
          }
          if ( nOut+1 >= MAX_EXPAND_SIZE ) return EXPSTR_OUTOVFL;
          buf[nOut] = ')';
          nOut++;
        }

      }
      else if ( ( inString[nIn] != ' ' ) && ( inString[nIn] != '\t' ) ) {

        if ( nMacro+1 >= MAX_EXPAND_SIZE ) return EXPSTR_MACOVFL;
        thisMacro[nMacro] = inString[nIn];
        nMacro++;

      }

      break;

    } // end switch

    nIn++;

  }

  //fprintf( stderr, "nOut = %-d\n", nOut );
  //fprintf( stderr, "buf = [%s]\n", buf );

  if ( nOut ) {

    if ( *outStringSize < nOut+1 ) {
      if ( *outStringSize ) delete[] *outString;
      *outStringSize = nOut+1;
      *outString = new char[nOut+1];
    }

    *outStringLen = nOut;
//    strcpy( *outString, buf );

    strncpy( *outString, buf, nOut );
    *((*outString)+nOut) = 0;

  }
  else {

    if ( *outStringSize == 0 ) {
      *outStringSize = 1;
      *outString = new char[1];
    }

    if ( outString ) strcpy( *outString, "" );
    *outStringLen = 0;

  }

  return EXPSTR_SUCCESS;

}

expStringClass::expStringClass ( void )
{

  rawString = NULL;
  rawStringSize = 0;
  rawStringLen = 0;

  expandedString1 = NULL;
  expandedString1Size = 0;
  expandedString1Len = 0;
  numPossibleSymbols1 = 0;

  expandedString = NULL;
  expandedStringSize = 0;
  expandedStringLen = 0;
  numPossibleSymbols = 0;

  expandedString2 = NULL;
  expandedString2Size = 0;
  expandedString2Len = 0;
  numPossibleSymbols2 = 0;

}

expStringClass::~expStringClass ( void )
{

  if ( rawStringSize ) {
    if ( rawString ) delete[] rawString;
    rawString = NULL;
    rawStringSize = 0;
    rawStringLen = 0;
  }

  if ( expandedString1Size ) {
    if ( expandedString1 ) delete[] expandedString1;
    expandedString1 = NULL;
    expandedString1Size = 0;
    expandedString1Len = 0;
    numPossibleSymbols1 = 0;
  }

  if ( expandedStringSize ) {
    if ( expandedString ) delete[] expandedString;
    expandedString = NULL;
    expandedStringSize = 0;
    expandedStringLen = 0;
    numPossibleSymbols = 0;
  }

  if ( expandedString2Size ) {
    if ( expandedString2 ) delete[] expandedString2;
    expandedString2 = NULL;
    expandedString2Size = 0;
    expandedString2Len = 0;
    numPossibleSymbols2 = 0;
  }

}

void expStringClass::copy ( const expStringClass &source )
{

  if ( source.rawStringSize ) {
    rawString = new char[source.rawStringSize];
    rawStringSize = source.rawStringSize;

//      strcpy( rawString, source.rawString );
//      rawStringLen = strlen(rawString);

    strncpy( rawString, source.rawString, rawStringSize-1 );
    rawString[rawStringSize-1] = 0;
    rawStringLen = strlen(rawString);

    numPossibleSymbols1 = source.numPossibleSymbols1;
  }
  else {
    rawString = NULL;
    rawStringSize = 0;
    rawStringLen = 0;
    numPossibleSymbols1 = 0;
  }

  if ( source.expandedString1Size ) {
    expandedString1 = new char[source.expandedString1Size];
    expandedString1Size = source.expandedString1Size;

//      strcpy( expandedString1, source.expandedString1 );
//      expandedString1Len = strlen(expandedString1);

    strncpy( expandedString1, source.expandedString1, expandedString1Size-1 );
    expandedString1[expandedString1Size-1] = 0;
    expandedString1Len = strlen(expandedString1);

    numPossibleSymbols2 = source.numPossibleSymbols2;
  }
  else {
    expandedString1 = NULL;
    expandedString1Size = 0;
    expandedString1Len = 0;
    numPossibleSymbols2 = 0;
  }

  if ( source.expandedStringSize ) {
    expandedString = new char[source.expandedStringSize];
    expandedStringSize = source.expandedStringSize;

//      strcpy( expandedString, source.expandedString );
//      expandedStringLen = strlen(expandedString);

    strncpy( expandedString, source.expandedString, expandedStringSize-1 );
    expandedString[expandedStringSize-1] = 0;
    expandedStringLen = strlen(expandedString);

    numPossibleSymbols = source.numPossibleSymbols;
  }
  else {
    expandedString = NULL;
    expandedStringSize = 0;
    expandedStringLen = 0;
    numPossibleSymbols = 0;
  }

  if ( source.expandedString2Size ) {
    expandedString2 = new char[source.expandedString2Size];
    expandedString2Size = source.expandedString2Size;

//      strcpy( expandedString2, source.expandedString2 );
//      expandedString2Len = strlen(expandedString2);

    strncpy( expandedString2, source.expandedString2, expandedString2Size-1 );
    expandedString2[expandedString2Size-1] = 0;
    expandedString2Len = strlen(expandedString2);

  }
  else {
    expandedString2 = NULL;
    expandedString2Size = 0;
    expandedString2Len = 0;
  }

}

char *expStringClass::getRaw ( void ) {

  if ( !rawString ) {
    return g_expStrBlank;
  }
  else {
    return rawString;
  }

}

int expStringClass::setRaw (
  char *str )
{

char *subStr;

  if ( expandedStringSize ) {
    delete[] expandedString;
    expandedString = NULL;
  }
  expandedStringSize = 0;
  expandedStringLen = 0;

  if ( expandedString1Size ) {
    delete[] expandedString1;
    expandedString1 = NULL;
  }
  expandedString1Size = 0;
  expandedString1Len = 0;
  numPossibleSymbols2 = 0;

  if ( expandedString2Size ) {
    delete[] expandedString2;
    expandedString2 = NULL;
  }
  expandedString2Size = 0;
  expandedString2Len = 0;

  if ( !str ) {

    if ( !rawString ) return EXPSTR_SUCCESS;

    strcpy( rawString, "" );
    rawStringLen = 0;
    numPossibleSymbols1 = 0;

    return EXPSTR_SUCCESS;

  }

 if ( rawStringSize < (int) strlen(str)+1 ) {
    if ( rawStringSize ) delete[] rawString;
    rawStringSize = strlen(str)+1;
    rawString = new char[rawStringSize];
  }

  strncpy( rawString, str, rawStringSize-1 );
  rawString[rawStringSize-1] = 0;
  rawStringLen = strlen(rawString);

  numPossibleSymbols1 = 0;
  subStr = strstr( rawString, "$(" );
  while ( subStr ) {
    numPossibleSymbols1++;
    numPossibleSymbols2++;
    subStr++;
    subStr = strstr( subStr, "$(" );
  }

    return EXPSTR_SUCCESS;

}

char *expStringClass::getExpanded ( void ) {

  if ( !expandedString2 ) {

    if ( !expandedString1 ) {

      if ( !rawString ) {
        return g_expStrBlank;
      }
      else {
        return rawString;
      }

    }
    else {

      //if ( strcmp( expandedString1, "" ) == 0 ) return rawString;
      if ( strcmp( expandedString1, "" ) == 0 ) return g_expStrBlank;

      return expandedString1;

    }

  }
  else {

    // if ( strcmp( expandedString2, "" ) == 0 ) return rawString;
    if ( strcmp( expandedString2, "" ) == 0 ) return g_expStrBlank;

    return expandedString2;

  }

}

int expStringClass::expand1st (
  int numMacros,
  char *macro[],
  char *expansion[] ) {

int stat;
int numSymbolsFound;

  stat = expand( numMacros, macro, expansion,
   1, // preserve symbols
   rawString, rawStringSize, rawStringLen,
   &expandedString1, &expandedString1Size, &expandedString1Len,
   numPossibleSymbols1, &numSymbolsFound );

  if ( stat & 1 ) {
    if ( expandedString1Size > expandedStringSize ) {
      if ( expandedString ) delete[] expandedString;
      expandedString = new char[expandedString1Size];
      expandedStringSize = expandedString1Size;
    }

    if ( expandedString1Size > 0 ) {
      strncpy( expandedString, expandedString1, expandedString1Len );
      expandedString[expandedString1Len] = 0;
      expandedStringLen = expandedString1Len;
    }
    else {
      if ( expandedString ) strcpy( expandedString, "" );
      expandedStringLen = 0;
    }

    numPossibleSymbols = numPossibleSymbols1 - numSymbolsFound;
    numPossibleSymbols2 = numPossibleSymbols;
  }

  return stat;

}

void expStringClass::reset ( void ) {

  if ( expandedString1Size > expandedStringSize ) {
    if ( expandedString ) delete[] expandedString;
    expandedString = new char[expandedString1Size];
    expandedStringSize = expandedString1Size;
  }

  if ( expandedString1Size > 0 ) {
    strncpy( expandedString, expandedString1, expandedString1Len );
    expandedString[expandedString1Len] = 0;
    expandedStringLen = expandedString1Len;
  }
  else {
    if ( expandedString ) strcpy( expandedString, "" );
    expandedStringLen = 0;
  }

  numPossibleSymbols2 = numPossibleSymbols;

}

int expStringClass::expand2nd (
  int numMacros,
  char *macro[],
  char *expansion[] ) {

int stat;
int numSymbolsFound;

  stat = expand( numMacros, macro, expansion,
   1, // preserve symbols
   expandedString, expandedStringSize, expandedStringLen,
   &expandedString2, &expandedString2Size, &expandedString2Len,
   numPossibleSymbols2, &numSymbolsFound );

  if ( stat & 1 ) {

    if ( numPossibleSymbols2 > 0 ) {

      if ( expandedString2Size > expandedStringSize ) {
        if ( expandedString ) delete[] expandedString;
        expandedString = new char[expandedString2Size];
        expandedStringSize = expandedString2Size;
      }

      if ( expandedString2Size > 0 ) {
        strncpy( expandedString, expandedString2, expandedString2Len );
        expandedString[expandedString2Len] = 0;
        expandedStringLen = expandedString2Len;
      }
      else {
        if ( expandedString ) strcpy( expandedString, "" );
        expandedStringLen = 0;
      }

    }

    numPossibleSymbols2 = numPossibleSymbols2 - numSymbolsFound;

  }

  return stat;

}

int expStringClass::expandFinal ( void ) {

int stat;
int numSymbolsFound;
char *macro[] = { "!", "!" };
char *expansion[] = { "!", "!" };

  stat = expand( 2, macro, expansion,
   0, // do not preserve symbols
   expandedString, expandedStringSize, expandedStringLen,
   &expandedString2, &expandedString2Size, &expandedString2Len,
   numPossibleSymbols2, &numSymbolsFound );

  return stat;

}
