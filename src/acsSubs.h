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

#ifndef __ACSSUBS_INC__
#define __ACSSUBS_INC__

void StartPIOserver();
int  AddValueCell(char *name, int indx, char *aux);
int  RemoveValueCell(int indx);
int  SetActivCell(char *name);
int  SetDeviceFloatValue(char *name, float *value);
int  SetDeviceStringValue(char *name, char *data);

#endif
