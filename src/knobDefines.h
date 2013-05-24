//******************************************************************************
// Copyright (c) 2012 Paul Scherrer Institut PSI), Villigen, Switzerland
// Disclaimer: neither  PSI, nor any of their employees makes any warranty
// or assumes any legal liability or responsibility for the use of this software
//******************************************************************************
//******************************************************************************
//
//     Author : Anton Chr. Mezger
//
//******************************************************************************

#ifndef __KNOBDEFINES_INC__
#define __KNOBDEFINES_INC__

#define MAX_CALC_INPUTS 12

#define PRINT(x)

#define boolean int 
#define true 1==1
#define false !true

#define MAXFILELEN 60
#define MAXPVLEN 120
#define MAXDISPLEN 20
#define NBSPECS 5

#define UNUSED(x) (void)(x)

typedef char pv_string[MAXPVLEN];  // temporary definition, I should allocate what we need

/* we wanted to really separate epics from the display part, but we still need this information */

enum caType {caSTRING	= 0, caINT = 1, caSHORT = 1, caFLOAT = 2, caENUM = 3, caCHAR = 4, caLONG = 5, caDOUBLE = 6};

typedef struct _connectInfoShort {
    int cs;          // 0= epics, 1 =acs
    int connected;
} connectInfoShort;

#endif
