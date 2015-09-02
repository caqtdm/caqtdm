/*
 *  This file is part of the caQtDM Framework, developed at the Paul Scherrer Institut,
 *  Villigen, Switzerland
 *
 *  The caQtDM Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The caQtDM Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the caQtDM Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2010 - 2014
 *
 *  Author:
 *    Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch
 */

#ifndef MUTEXKNOBDATAWRAPPER_H
#define MUTEXKNOBDATAWRAPPER_H

#include "caQtDM_Lib_global.h"

#ifdef __cplusplus
extern "C" {
#endif

extern CAQTDM_LIBSHARED_EXPORT void MutexKnobDataWrapperInit(MutexKnobData*);
extern CAQTDM_LIBSHARED_EXPORT MutexKnobData* C_SetMutexKnobData(MutexKnobData* p, int indx, knobData data);
extern CAQTDM_LIBSHARED_EXPORT MutexKnobData* C_GetMutexKnobData(MutexKnobData* p, int indx, knobData *data);
extern CAQTDM_LIBSHARED_EXPORT MutexKnobData* C_SetMutexKnobDataConnected(MutexKnobData* p, int indx, int connected);
extern CAQTDM_LIBSHARED_EXPORT MutexKnobData* C_SetMutexKnobDataReceived(MutexKnobData* p, knobData *kData);
extern CAQTDM_LIBSHARED_EXPORT MutexKnobData* C_UpdateTextLine(MutexKnobData* p, char *message, char *name);
extern CAQTDM_LIBSHARED_EXPORT MutexKnobData* C_DataLock(MutexKnobData* p, knobData *kData);
extern CAQTDM_LIBSHARED_EXPORT MutexKnobData* C_DataUnlock(MutexKnobData* p, knobData *kData);

#ifdef __cplusplus
}
#endif

#endif // WRAPPER_H
