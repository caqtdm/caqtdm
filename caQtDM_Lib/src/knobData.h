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

#ifndef KNOBDATA_H
#define KNOBDATA_H

#include <sys/timeb.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _epicsData {
    int          connected;             /* connection flag */
    dbr_string_t fec;                   /* ioc */
    int          monitorCount;          /* acquisition counter */
    int          monitorCountPrev;
    int          displayCount;          /* last displayed acquisition counter */
    int          unconnectCount;        /* counter for limiting the disconnected pv's */
    int          nelm;                  /* number of record elements */
    int          valueCount;            /* number of values */
    int          enumCount;             /* number of values */
    short        fieldtype;             /* fieldtype */
    short	     status;	            /* status of value */
    short	     severity;		        /* severity of alarm */
    short	     precision;		        /* number of decimal places */
    dbr_string_t units;	                /* units of value */
    double	     upper_disp_limit;	    /* upper limit of graph */
    double	     lower_disp_limit;	    /* lower limit of graph */
    double	     upper_alarm_limit;
    double	     upper_warning_limit;
    double	     lower_warning_limit;
    double	     lower_alarm_limit;
    double	     upper_ctrl_limit;	    /* upper control limit */
    double	     lower_ctrl_limit;	    /* lower control limit */
    double	     rvalue;		        /* real value */
    double       oldsoftvalue;          /* for softpv in order to increment monitorcount only when necessary */
    long	     ivalue;		        /* integer value */
    int          accessW;               /* epics access control */
    int          accessR;
    void         *info;                 /* pointer to  epics connection info */
    int          dataSize;              /* size of vector data */
    void         *dataB;                /* vector data, right size will be allocated on data receive and waveform copied into*/
    void         *dataPtr;
    int          initialize;            /* first initialisation */
    char         aux[10];               /* used for acs controlsystem images */
    struct timeb lastTime;              /* last displayed time */
    struct timeb actTime;               /* receive time */
    int          repRate;               /* repetition rate for this channel, default will be 5Hz */
} epicsData;

typedef struct _knobData {
    void *thisW;                        /* mainwindow identifier */
    int  index;                         /* index (-1 for not used) */
    short soft;                         /* soft channel mark */
    pv_string pv;                       /* device process variable name */
    void *dispW;                        /* widget identifier */
    void *mutex;                        /* mutex used for waveforms */
    epicsData edata;                    /* epics data */
    int specData[NBSPECS];              /* some special data, will be replaced by properties later */
    int valPix;                         /* pixel value of caThermo */
    char clasName[MAXDISPLEN];          /* class of widget type */
    char dispName[MAXDISPLEN];          /* name of widget type */
    char fileName[MAXFILELEN];
    void *pluginInterface;                    /* plugin pointer */
    dbr_string_t pluginName;            /* plugin name */
} knobData;

#ifdef __cplusplus
}
#endif

#endif // KNOBDATA_H
