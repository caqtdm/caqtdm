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


int EpicsSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType);
int EpicsSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess);
int EpicsGetTimeStamp(char *pv, char *timestamp);
int EpicsGetDescription(char *pv, char *description);
void clearEvent(void * ptr);
void addEvent(void * ptr);
void EpicsReconnect(knobData *kData);
void EpicsDisconnect(knobData *kData);
void EpicsFlushIO();
void DestroyContext();
void PrepareDeviceIO();
void TerminateDeviceIO();
void InitializeContextMutex();
int  CreateAndConnect(int index, knobData *kData, int rate, int skip);
void ClearMonitor(knobData *kData);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif //EPICSEXTERNALS_H
