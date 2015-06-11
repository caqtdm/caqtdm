#ifdef XDR_HACK
#if defined XDR_LE || defined XDR_BE

#include <stdio.h>
#include <stdint.h>

#define XDR FILE
#define xdr_getpos(a) ftell(a)
#define xdr_setpos(a,b) !fseek(a,b,SEEK_SET)
#define xdrproc_t int (*)(FILE *, void *)

int xdr_int8_t( FILE *fptr, int8_t *my_int8);
int xdr_int16_t( FILE *fptr, int16_t *my_int16);
int xdr_int32_t( FILE *fptr, int32_t *my_int32);
int xdr_float( FILE *fptr, float *my_float);
int xdr_double( FILE *fptr, double *my_double);
int xdr_vector( FILE *fptr, char *buffer, uint16_t size,
                uint16_t type_size, int (*proc)(FILE *fptr, void *data) );
int xdr_counted_string( FILE *fptr, char **p);

#endif
#endif
