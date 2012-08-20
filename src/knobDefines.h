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
#define MAXPVLEN 80
#define MAXDISPLEN 20
#define NBSPECS 5

typedef char pv_string[MAXPVLEN];  // temporary definition, I should allocate what we need

#endif
