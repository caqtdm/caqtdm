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

#ifndef MUTEXKNOBDATAWRAPPER_H
#define MUTEXKNOBDATAWRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MutexKnobData MutexKnobData;
extern void MutexKnobDataWrapperInit(MutexKnobData*);
extern MutexKnobData* C_SetMutexKnobData(MutexKnobData* p, int indx, knobData data);
extern MutexKnobData* C_GetMutexKnobData(MutexKnobData* p, int indx, knobData *data);
extern MutexKnobData* C_SetMutexKnobDataConnected(MutexKnobData* p, int indx, int connected);
extern MutexKnobData* C_SetMutexKnobDataReceived(MutexKnobData* p, knobData *kData);
extern MutexKnobData* C_UpdateTextLine(MutexKnobData* p, char *message, char *name);

#ifdef __cplusplus
}
#endif

#endif // WRAPPER_H
