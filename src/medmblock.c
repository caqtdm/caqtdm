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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <sys/time.h>

#define NBPOINTS 128	    /* number of vector points */
		
			   
#define True                                    (1==1)
#define False 					 !True

#include <fortypes.h>
#include <userstruct.h>
#include <cssnew.h>
#include <piosubs.h>
#include <host2vax.h>
#include <profileacc.h>
#include <profileacc_harfebydoelling.h>

#include <utils/mystring.h>

#define MIN(a,b)  ((a)<(b))?(a):(b)

extern int getErrMsgStr (short int *code, FORSTR *result);

static char *io_getErrMsgStr(int error_code)
{
  static char errmsg[80+1] = {0};
  static INT2 ecode = -9999;	
  char        *sp;
  FORSTR      errmsgD = {sizeof(errmsg)-1, 0, 0, (char*)errmsg};

  if (!errmsg[0] && (ecode == error_code))
    return (errmsg);

  ecode = error_code;
  getErrMsgStr(&ecode, &errmsgD);
  errmsg[sizeof(errmsg)-1] = '\0';
  
  /* remove space chars */
  for ( sp = &errmsg[sizeof(errmsg)-2];sp>=errmsg;sp--) {
    if (*sp != ' ') break;
    *sp = '\0';	
  }

  return (errmsg);	
} /* io_getErrMsgStr */


int BlockserverData(pioDevices *cells, int *totalval, 
                                       float **Xbuf, float **Ybuf, char *errorMsg)

{	    

#define INT16 int16_t
#define INT32 int32_t

	INT16      *buffer;
	pioDevices *profcell;
	INT32       profwc;
	INT16      *profbuf;

        INT32      numpointsoll, numpointist, numvalueperpoint, numvalue;
        INT32      resnumvalue, resNumPoints, resNumValuesPerPoint;
        INT32      valuebufsize = 0;
	float      scale_rangelow, scale_rangehigh;
	float     *valbuf;
	float     *xbuf;

        int        n, pmtype, pmstatus, pmrange, scale_unit, scale_direction;
        float      speed;
	float      scale_offset, scale_step;


        /*
	  cells[0].ub.int4val == "actual address of user buffer"
	  
	  user buffer:
	  
	      +------------+
	      | cell       |     /pioDevices
	      +------------+
	      | WC         |     /INT16
	      +------------+
	      | Profile-   |     /WC*INT16
	      | Buffer     |
	      |            |
	      |            |
	      +------------+
	       
        */

	buffer   = (INT16*)cells[0].ub.int4val;
	
	if(cells[0].error_code != 1) {
	  //printf("MEDM -- %.8s %s\n", cells[0].name, io_getErrMsgStr(cells[0].error_code));
	  sprintf(errorMsg, "MEDM -- %s %s",cells[0].name, io_getErrMsgStr(cells[0].error_code)); 
	  return False;
	}
	
	if (buffer==NULL) {
	  //strcpy(errorMsg, "Address of user buffer is NULL");
	  printf("MEDM -- Address of user buffer is NULL/cells[0].ub.int4val == NULL\n"); 
	  return False;	
	}
	
	profcell = (pioDevices*)&buffer[0];
	profwc   = buffer[sizeof(pioDevices)/sizeof(INT16)];
	profbuf  = &buffer[sizeof(pioDevices)/sizeof(INT16) + 1];


	/* convert profile buffer */
	pmVax2Host (profwc, profbuf);

/*
	printf("profwc  =   %8d Word's\n",profwc);
	printf("profbuf = 0x%08x Addr\n",(unsigned int)profbuf);
	printf("\n");
*/


/*

	fprintf(stdout,"\n");
	fprintf(stdout,"Output of pmFPrintRawHeader():\n");	
	pmFPrintRawHeader(profwc, profbuf, stdout);
	
	fprintf(stdout,"\n");
	fprintf(stdout,"Output of pmFPrintInfo():\n");
	pmFPrintInfo(profwc, profbuf, stdout);
*/
/* analyze the data we got */

        {
	     if(!pmValidProfile (profwc, profbuf )) {
	       printf("MEDM -- something wrong\n");
	       strcpy(errorMsg, "not a valid profile");
	       return False;
	    } else {
                pmtype = pmGetType(profwc, profbuf);
                pmstatus = pmGetStatus(profwc, profbuf);
                pmrange = pmGetRange(profwc, profbuf);
                pmGetScale(profwc, profbuf, 
                           &scale_unit, 
                           &scale_direction, 
                           &scale_offset, 
                           &scale_step, 
                           &scale_rangelow, 
                           &scale_rangehigh);
                speed = pmGetSpeed(profwc, profbuf);
/*
                printf("x Profile type  : %d %s\n", pmtype, pmTypeToString(pmtype));
                printf("x         status: 0x%08x\n", pmstatus);
                printf("x         range : %d speed=%9.3f scaleunit=%d scaledirection=%d\n", pmrange, 
                       speed, scale_unit, scale_direction);
                printf("x         scale offset: %9.3f\n", scale_offset);
                printf("x         scale step  : %9.3f\n", scale_step);
                printf("x         scale range : %9.3f - %9.3f\n", scale_rangelow, scale_rangehigh);
*/
                numpointsoll = pmGetNumPointsSoll(profwc, profbuf);
                numpointist = pmGetNumPoints(profwc, profbuf);
                numvalueperpoint = pmGetNumValuesPerPoint(profwc, profbuf);
                numvalue = pmGetNumValues(profwc, profbuf);
/* alloc buffer for values */
                valuebufsize = numvalue;

		valbuf = (float *)  malloc(sizeof(float) * valuebufsize);
		xbuf   =  (float *) malloc(sizeof(float) * valuebufsize);
                resnumvalue = pmGetRealValues(profwc, profbuf, 
                                          0,     	           /* reqStartPointIndex */
                                          numpointist - 1,         /* reqEndPointIndex */
                                          0,                       /* reqValueIndex */
                                          valuebufsize,            /* valuebufsize */
                                          valbuf,                  /* valuebuf */
                                          &resNumPoints,           /* resNumPoints */
                                          &resNumValuesPerPoint);  /* resNumValuesPerPoint */
/* use values */
/*
                printf("        points          : %d (%d)\n", numpointist, numpointsoll);
                printf("        value per point : %d\n", numvalueperpoint);
*/
                *totalval = (resNumPoints * resNumValuesPerPoint);

                if (*totalval <= 0) {
                    strcpy(errorMsg, "no profile data");
                    return False;
                } else {
                    if (resNumValuesPerPoint != 1) {
		       *totalval = resNumValuesPerPoint;

/* harps = special case */

                       switch (pmGetType (profwc, profbuf)) {
		         case pmTYPE_Harfe:
			 case pmTYPE_LOGIV32:
			   {
			    float *res = (float *) pmHarfeByDoellingF(cells[0].name, cells[0].att, valbuf, 1, 
			                                              resNumValuesPerPoint, NBPOINTS, 1, &scale_offset, 
			    					      &scale_step);
			    if(res != (float *) 0)  {
			      *totalval = NBPOINTS;  /* 128 points are enough */
			       free(xbuf);
			       xbuf   =  (float *) malloc(sizeof(float) *  *totalval);
			       for(n=0; n < *totalval; n++) {
	                         xbuf[n] = scale_offset + n * scale_step;
				 if (res[n] < 10.0e-6) {
                                   res[n] = 0.0;
                                 }    
			       }
			       free(valbuf);
			       valbuf = res;
			    } else {
		               scale_step = 1;	     /* best guess at this time */
			       scale_offset = -7.5;
			       for(n=0; n < *totalval; n++) {
	                         xbuf[n] = scale_offset + n * scale_step;
	                       }
			    }
			   }
			   break;
			    
		         default:
			   printf("MEDM -- not implemented\n");
		           break;
		       }
/* normal monitor */
                    } else {
		    	   for(n=0; n < *totalval; n++) {
	                      xbuf[n] = scale_offset + n * scale_step;
	                   }
		    }
		    *Xbuf = (float *) xbuf;
                    *Ybuf = (float *) valbuf;
                }
            }
    	}
	return True;
} 






