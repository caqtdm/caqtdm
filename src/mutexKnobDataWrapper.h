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

#include <caQtDM_Lib_global.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MutexKnobData MutexKnobData;
#if defined(_MSC_VER)
   extern CAQTDM_LIBSHARED_EXPORT void MutexKnobDataWrapperInit(MutexKnobData*);
#else
   extern void MutexKnobDataWrapperInit(MutexKnobData*);
#endif
extern MutexKnobData* C_SetMutexKnobData(MutexKnobData* p, int indx, knobData data);
extern MutexKnobData* C_GetMutexKnobData(MutexKnobData* p, int indx, knobData *data);
extern MutexKnobData* C_SetMutexKnobDataConnected(MutexKnobData* p, int indx, int connected);
extern MutexKnobData* C_SetMutexKnobDataReceived(MutexKnobData* p, knobData *kData);
extern MutexKnobData* C_UpdateTextLine(MutexKnobData* p, char *message, char *name);
extern MutexKnobData* C_DataLock(MutexKnobData* p, knobData *kData);
extern MutexKnobData* C_DataUnlock(MutexKnobData* p, knobData *kData);


#ifdef __cplusplus
}
#endif

#endif // WRAPPER_H
