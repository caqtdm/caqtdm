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

#ifndef EPICSEXTERNALS_H
#define EPICSEXTERNALS_H

#include "caQtDM_Lib_global.h"
#include <stdint.h>
#include "knobDefines.h"
#include "knobData.h"



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


CAQTDM_LIBSHARED_EXPORT int EpicsSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType);
CAQTDM_LIBSHARED_EXPORT int EpicsSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess);
CAQTDM_LIBSHARED_EXPORT int EpicsGetTimeStamp(char *pv, char *timestamp);
CAQTDM_LIBSHARED_EXPORT int EpicsGetDescription(char *pv, char *description);
CAQTDM_LIBSHARED_EXPORT void clearEvent(void * ptr);
CAQTDM_LIBSHARED_EXPORT void addEvent(void * ptr);
CAQTDM_LIBSHARED_EXPORT void EpicsReconnect(knobData *kData);
CAQTDM_LIBSHARED_EXPORT void EpicsDisconnect(knobData *kData);
CAQTDM_LIBSHARED_EXPORT void EpicsFlushIO();
CAQTDM_LIBSHARED_EXPORT void DestroyContext();
CAQTDM_LIBSHARED_EXPORT void PrepareDeviceIO();
CAQTDM_LIBSHARED_EXPORT void TerminateDeviceIO();
CAQTDM_LIBSHARED_EXPORT void InitializeContextMutex();
CAQTDM_LIBSHARED_EXPORT int  CreateAndConnect(int index, knobData *kData, int rate, int skip);
CAQTDM_LIBSHARED_EXPORT void ClearMonitor(knobData *kData);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif //EPICSEXTERNALS_H
