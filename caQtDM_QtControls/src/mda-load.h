/*************************************************************************\
* Copyright (c) 2014 UChicago Argonne, LLC,
*               as Operator of Argonne National Laboratory.
* This file is distributed subject to a Software License Agreement
* found in file mdaLICENSE that is included with this distribution. 
\*************************************************************************/


/*

  Written by Dohn A. Arms, Argonne National Laboratory
  Send comments to dohnarms@anl.gov
  
  0.1   -- July 2005
  0.1.1 -- December 2006
           Added support for files that have more than 32k points
  0.2.0 -- October 2007
           Added several new functions for accessing scans or the extra PV's
                 without loading the entire file.
  0.2.1 -- March 2009
           Removed several memory leaks.
  1.0   -- October 2009
           Renamed structures.
  1.1   -- November 2010
  1.1.1 -- December 2010
  1.2   -- March 2011
           Fixed integer issues by tying short to int16_t, long to int32_t,
           and char to int8_t
  1.2.1 -- January 2012
  1.2.2 -- June 2012
           Added preprocessor commands for c++ compatibility
  1.3.0 -- February 2013
  1.3.1 -- February 2014
 */


/******************  mda_load.h  **************/

#ifndef MDA_LOAD_H
#define MDA_LOAD_H 1

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdio.h>
#include <stdint.h>

struct mda_header
{
  float    version;
  int32_t  scan_number;
  int16_t  data_rank;
  int32_t *dimensions;
  int16_t  regular;
  int32_t  extra_pvs_offset;
};


struct mda_positioner
{
  int16_t number;
  char   *name;
  char   *description;
  char   *step_mode;
  char   *unit;
  char   *readback_name;
  char   *readback_description;
  char   *readback_unit;
};


struct mda_detector
{
  int16_t number;
  char   *name;
  char   *description;
  char   *unit;
};


struct mda_trigger
{
  int16_t number;
  char   *name;
  float   command;
};


struct mda_scan
{
  int16_t  scan_rank;
  int32_t  requested_points;
  int32_t  last_point;
  int32_t *offsets;
  char    *name;
  char    *time;
  int16_t  number_positioners;
  int16_t  number_detectors;
  int16_t  number_triggers;

  struct mda_positioner **positioners;  
  struct mda_detector   **detectors;
  struct mda_trigger    **triggers;
  double **positioners_data;
  float  **detectors_data;

  struct mda_scan **sub_scans;
};

enum PV_TYPES { EXTRA_PV_STRING=0, EXTRA_PV_INT8=32,  EXTRA_PV_INT16=29, 
		EXTRA_PV_INT32=33, EXTRA_PV_FLOAT=30, EXTRA_PV_DOUBLE=34 };

struct mda_pv
{
  char   *name;
  char   *description;
  int16_t type;
  int16_t count;
  char   *unit;
  char   *values;
};


struct mda_extra
{
  int16_t number_pvs;
  struct mda_pv **pvs;
};


struct mda_file
{
  struct mda_header *header;
  struct mda_scan   *scan;
  struct mda_extra  *extra;
};


/*****************************************************/

struct mda_scaninfo
{
  int16_t scan_rank;          /* redundant */
  int32_t requested_points;   /* redundant */
  char   *name;
  int16_t number_positioners;
  int16_t number_detectors;
  int16_t number_triggers;

  struct mda_positioner **positioners;  
  struct mda_detector   **detectors;
  struct mda_trigger    **triggers;
};

struct mda_fileinfo
{
  float    version;
  int32_t  scan_number;
  int16_t  data_rank;
  int32_t *dimensions;
  int16_t  regular;
  int32_t  last_topdim_point;
  char  *time;
  struct mda_scaninfo **scaninfos;
};

/******************************************************/

struct mda_file *mda_load( FILE *fptr);
struct mda_header *mda_header_load( FILE *fptr);
struct mda_scan *mda_scan_load( FILE *fptr);
struct mda_scan *mda_subscan_load( FILE *fptr, int depth, int *indices, 
				      int recursive);
struct mda_extra *mda_extra_load( FILE *fptr);


void mda_unload( struct mda_file *mda);
void mda_header_unload( struct mda_header *header);
void mda_scan_unload( struct mda_scan *scan);
void mda_extra_unload( struct mda_extra *extra);


struct mda_fileinfo *mda_info_load( FILE *fptr);
void mda_info_unload( struct mda_fileinfo *fileinfo);


#ifdef __cplusplus
}
#endif 

#endif /* MDA_LOAD_H */
