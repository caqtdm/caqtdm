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
    int          displayCount;          /* last displayed acquisition counter */
    int          unconnectCount;        /* counter for limiting the disconnected pv's */
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
    void         *dataB;                /* vector data */
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
} knobData;

#ifdef __cplusplus
}
#endif

#endif // KNOBDATA_H
