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

#include "dbrString.h"
#include "knobDefines.h"

#include "knobData.h"
#include "mutexKnobData.h"
#include "mutexKnobDataWrapper.h"
#include <caQtDM_Lib_global.h>

#ifdef __cplusplus
extern "C" {
#endif

MutexKnobData* KnobDataPtr;

#ifdef __cplusplus
}
#endif

CAQTDM_LIBSHARED_EXPORT void MutexKnobDataWrapperInit(MutexKnobData* KnobData)
{
    KnobDataPtr = KnobData;
}
