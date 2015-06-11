/*************************************************************************\
* Copyright (c) 2014 UChicago Argonne, LLC,
*               as Operator of Argonne National Laboratory.
* This file is distributed subject to a Software License Agreement
* found in file mdaLICENSE that is included with this distribution. 
\*************************************************************************/


/*

  Written by Dohn A. Arms, Argonne National Laboratory
  Send comments to dohnarms@anl.gov
  
  1.3.1 -- February 2014
           Initial Version
 */

/* 
  This code is an XDR hack!

  It contains routines for decoding XDR data types for systems of
  either little or big endianness.  It is meant for Windows (which is
  little-endian on x86 architectures), which has no native XDR
  library.  If your system has an XDR library, USE IT!

  The only routines that are provided are those needed by the MDA format,
  and they only decode, not encode.
*/

// the hack has to be enabled, and the endianness has to be specified
#ifdef XDR_HACK
#if defined XDR_LE || defined XDR_BE

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int xdr_int8_t( FILE *fptr, int8_t *my_int8)
{
  if( fseek( fptr, 3, SEEK_CUR) )
    return 0;
  if( !fread( my_int8, 1, 1, fptr) )
    return 0;
  return 1;
}

int xdr_int16_t( FILE *fptr, int16_t *my_int16)
{
#ifdef XDR_LE
  int i;
  union
  {
    char b[2];
    int16_t d;
  } data;
#endif

  if( fseek( fptr, 2, SEEK_CUR) )
    return 0;
#ifdef XDR_LE
  for( i = 1; i >= 0; i--)
    if( !fread( data.b + i, 1, 1, fptr) )
      return 0;
  *my_int16 = data.d;
#else
  if( !fread( my_int16, 2, 1, fptr) )
    return 0;
#endif

  return 1;
}

int xdr_int32_t( FILE *fptr, int32_t *my_int32)
{
#ifdef XDR_LE
  union 
  {
    char b[4];
    int32_t d;
  } data;

  int i;

  for( i = 3; i >= 0; i--)
    if( !fread( data.b + i, 1, 1, fptr) )
      return 0;
  *my_int32 = data.d;
#else
  if( !fread( my_int32, 4, 1, fptr) )
    return 0;
#endif

  return 1;
}


int xdr_float( FILE *fptr, float *my_float)
{
#ifdef XDR_LE
  union 
  {
    char b[4];
    float d;
  } data;

  int i;

  for( i = 3; i >= 0; i--)
    if( !fread( data.b + i, 1, 1, fptr) )
      return 0;
  *my_float = data.d;
#else
  if( !fread( my_float, 4, 1, fptr) )
    return 0;
#endif

  return 1;
}

int xdr_double( FILE *fptr, double *my_double)
{
#ifdef XDR_LE
  union 
  {
    char b[8];
    double d;
  } data;

  int i;

  for( i = 7; i >= 0; i--)
    if( !fread( data.b + i, 1, 1, fptr) )
      return 0;
  *my_double = data.d;
#else
  if( !fread( my_double, 8, 1, fptr) )
    return 0;
#endif

  return 1;
}


int xdr_vector( FILE *fptr, char *buffer, uint16_t size,
                   uint16_t type_size, int (*proc)(FILE *fptr, void *data) )
{
  uint16_t i;

  for( i = 0; i < size; i++)
    {
      if( !proc(fptr, &buffer[i*type_size]) )
        return 0;
    }

  return 1;
}


int xdr_counted_string( FILE *fptr, char **p)
{
  int32_t length, rlength;

  if( !xdr_int32_t( fptr, &length) )
    return 0;
  if( !length)
    {
      *p = (char *) malloc( sizeof(char) );
      (*p)[0] = '\0';
      return 1;
    }

  if( !xdr_int32_t( fptr, &length) )
    return 0;

  // round length up to multiple of 4, how data is stored
  rlength = length + ( (length%4) == 0 ? 0 : 4 - (length%4) );

  *p = (char *) malloc( (length + 1) * sizeof(char) );
  if( fread( *p, 1, length, fptr) != length)
    {
      free( *p);
      return 0;
    }
  (*p)[length] = '\0'; /* Null termination using string length */
  
  if( rlength > length)
    if( fseek( fptr, rlength-length, SEEK_CUR) )
      return 0;

  return 1;
}

#endif
#endif
