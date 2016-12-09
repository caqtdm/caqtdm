#ifndef HIPARETRIEVAL_H
#define HIPARETRIEVAL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>

#include "logtimes.h"

#define boolean     int

#define MAXDEV 1
#define MAXDEVLEN 40
#define True        1
#define False       0
#define epsilon 1.e-9

typedef struct  {
    char Dev[MAXDEVLEN];
} device;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
 int GetLogShift(int startSeconds, char *dev, int *nbVal, float *Timer, float *Yvals);
 int gmv(char *Node, int Numdev, int *Numval, char *logid, short annee,
                     int retr_time, device Devices[], float *Valdev, int *Timef,
                     short *Valerr, int maxdev, int MAXVAL, int *Tau);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
