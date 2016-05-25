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

// utility functions

#include "utility.h"

char *Strncat(char *dest, char *src, int max )
{

    int l, newMax;
    char *s;

    l = strlen( dest );
    newMax = max - l;
    if ( newMax < 0 ) {
        dest[max] = 0;
        return dest;
    }

    s = strncat( dest, src, newMax );
    dest[max] = 0;

    return s;

}

int blank (char *string )
{

    unsigned int i, l;

    l = strlen(string);
    if ( !l ) return 1;

    for ( i=0; i<l; i++ ) {
        if ( !isspace( (int) string[i] ) ) return 0;
    }

    return 1;

}

void trimWhiteSpace (char *str )
{

    int first, last, i, ii, l;

    l = strlen(str);

    i = ii = 0;

    while ( ( i < l ) && isspace( str[i] ) ) {
        i++;
    }

    first = i;

    i = l-1;
    while ( ( i >= first ) && isspace( str[i] ) ) i--;

    last = i;

    if ( first > 0 ) {

        for ( i=first; i<=last; i++ ) {
            str[ii] = str[i];
            ii++;
        }
        str[ii] = 0;

    } else if ( last < l-1 ) {
        str[last+1] = 0;
    }

}

void readStringFromFile ( char *str, int maxChars, FILE *f )
{
    char *ptr;
    int i, j, l, max, first, escape;
    char buf[10000+1];

    if ( maxChars < 1 ) return;

    if ( maxChars > 10000 )
        max = 10000;
    else
        max = maxChars-1;

    ptr = fgets( buf, 10000, f );
    if ( !ptr ) {
        strcpy( str, "4 0 0" );
        return;
    }
    buf[10000] = 0;

    l = strlen(buf);

    buf[l-1] = 0;
    if ( l > max ) l = max;

    if ( strcmp( buf, "<<<empty>>>" ) == 0 ) {

        strcpy( str, "" );
        return;

    }
    else if ( strncmp( buf, "<<<blank>>>", 11 ) == 0 ) {

        first = 11;

    }
    else {

        first = 0;

    }

    escape = 0;
    for ( i=first, j=0; i<l; i++ ) {

        if ( escape ) {

            if ( buf[i] == '\\' ) {
                str[j] = buf[i];
                if ( j < max ) j++;
            }
            else if ( buf[i] == 'n' ) {
                str[j] = 10;
                if ( j < max ) j++;
            }
            else {
                str[j] = buf[i];
                if ( j < max ) j++;
            }

            escape = 0;

        }
        else {

            if ( buf[i] == '\\' ) {
                escape = 1;
            }
            else if ( buf[i] == 1 ) {
                str[j] = 10;
                if ( j < max ) j++;
            }
            else {
                str[j] = buf[i];
                if ( j < max ) j++;
            }

        }

    }

    str[j] = 0;
}
