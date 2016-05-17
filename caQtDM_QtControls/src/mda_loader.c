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
           Added support for files that have more than 32k points.
  0.2.0 -- October 2007
           Added several new functions for accessing scans or the extra PV's
           without loading the entire file (never publically released).
  0.2.1 -- March 2009
           Removed several memory leaks. Made trace structure consistent.
  1.0.0 -- October 2009
           Release with vastly changed mda-utils.
           Renamed structures to give them "mda_" prefix.
  1.0.1 -- June 2010
           Added checking in header code to make sure there were no screwy
           dimensions, by looking for 0xFFFFFFFF in one of them.
  1.0.2 -- August 2010
           Use offsets to load scans, as bad files sometimes have them out 
           of order.
  1.1   -- November 2010
  1.1.1 -- February 2011
           Realloc memory in screwy xdr counted strings.
           Added <stdio.h> to remove MacOS warning. (From J. Lewis Muir)
  1.2   -- March 2011
           Fixed integer issues by tying short to int16_t, long to int32_t,
           and char to int8_t.
           Renamed DBR_* type variables to EXTRA_PV_* type variables, to
           make explicit the type of integer used (which broke compatibility
           with the EPICS DBR type names).
  1.2.1 -- January 2012
  1.2.2 -- June 2012
           Fixed major bug with INT8 Extra PVs.
  1.3.0 -- February 2013
           Don't load files that have nonsensical scan dimensions.
  1.3.1 -- February 2014
           Added a check for bad zero values in the scan offsets.
           Added support for XDR hack code.
 */





/****************  mda_loader.c  **********************/


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "mda-load.h"


#ifdef XDR_HACK
  #include "xdr_hack.h"
#else
  #include <rpc/types.h>
  #include <rpc/xdr.h>

  static bool_t xdr_counted_string( XDR *xdrs, char **p)
  {
    int mode;
    int32_t length;

    bool_t ret_bool;
 
    mode = (xdrs->x_op == XDR_DECODE);

    /* If writing, obtain the length */
    if( !mode) 
      length = strlen(*p);
 
    /* Transfer the string length */
    // had to change this to int32_t, as values would sometimes turn negative
    // resizing is done below
    if( !xdr_int32_t(xdrs, &length)) 
      return 0;

    /* If reading, obtain room for the string */
    if (mode)
      {
        *p = (char *) malloc( (length + 1) * sizeof(char) );
        (*p)[length] = '\0'; /* Null termination */
      }

    /* If the string length is nonzero, transfer it */
    if( length)
      ret_bool = xdr_string(xdrs, p, length);
    else
      ret_bool = 1;

    // this fix is for when the size is not set correctly,
    // reducing the memory allocated; it only checks if size is large;
    // sometimes 4MB is allocated for an 8 character string....
    if( mode && (length > 4095))
      { 
        int32_t l;

        l = strlen(*p);
        if( l != length)
          *p = realloc( *p, l+1);
      }

    return ret_bool;
  }
#endif


static struct mda_header *header_read( XDR *xdrs)
{
  struct mda_header *header;

  header = (struct mda_header *) 
    malloc( sizeof(struct mda_header));

  if( !xdr_float(xdrs, &(header->version) ))
    return NULL;

  if( !xdr_int32_t(xdrs, &(header->scan_number) ))
    return NULL;

  if( !xdr_int16_t(xdrs, &(header->data_rank) ))
    return NULL;

  header->dimensions = (int32_t *) 
    malloc( header->data_rank * sizeof(int32_t));
  if( !xdr_vector( xdrs, (char *) header->dimensions, header->data_rank, 
		   sizeof( int32_t), (xdrproc_t) xdr_int32_t))
    return NULL;

  if( !xdr_int16_t(xdrs, &(header->regular) ))
    return NULL;

  if( !xdr_int32_t(xdrs, &(header->extra_pvs_offset) )) 
    return NULL;

  {  // need to do error checking on dimensions!
    int i;

    for( i = 0; i < header->data_rank; i++)
      // -1 is it was int16_t, not int32_t
      if(  header->dimensions[i] == 0xFFFFFFFF) 
        {
          free( header);
          free( header->dimensions);
          return NULL;
        }
  }

  return header;
}


static struct mda_positioner *positioner_read(XDR *xdrs)
{
  struct mda_positioner *positioner;


  positioner = (struct mda_positioner *) 
    malloc( sizeof(struct mda_positioner));

  if( !xdr_int16_t(xdrs, &(positioner->number) ))
    return NULL;

  if( !xdr_counted_string( xdrs, &(positioner->name) ) )
    return NULL;
  if( !xdr_counted_string( xdrs, &(positioner->description) ) )
    return NULL;
  if( !xdr_counted_string( xdrs, &(positioner->step_mode) ) )
    return NULL;
  if( !xdr_counted_string( xdrs, &(positioner->unit) ) )
    return NULL;
  if( !xdr_counted_string( xdrs, &(positioner->readback_name) ) )
    return NULL;
  if( !xdr_counted_string( xdrs, &(positioner->readback_description) ) )
    return NULL;
  if( !xdr_counted_string( xdrs, &(positioner->readback_unit) ) )
    return NULL;

  return positioner;
}



static struct mda_detector *detector_read(XDR *xdrs)
{
  struct mda_detector *detector;


  detector = (struct mda_detector *) 
    malloc( sizeof(struct mda_detector));

  if( !xdr_int16_t(xdrs, &(detector->number) ))
    return NULL;

  if( !xdr_counted_string( xdrs, &(detector->name) ) )
    return NULL;
  if( !xdr_counted_string( xdrs, &(detector->description) ) )
    return NULL;
  if( !xdr_counted_string( xdrs, &(detector->unit) ) )
    return NULL;

  return detector;
}




static struct mda_trigger *trigger_read(XDR *xdrs)
{
  struct mda_trigger *trigger;


  trigger = (struct mda_trigger *) 
    malloc( sizeof(struct mda_trigger));

  if( !xdr_int16_t(xdrs, &(trigger->number) ))
    return NULL;
  if( !xdr_counted_string( xdrs, &(trigger->name ) ))
    return NULL;
  if( !xdr_float(xdrs, &(trigger->command) ))
    return NULL;

  return trigger;
}



/* this function is recursive, due to the nature of the file format */
/* it can be turned off by making recursive 0 */
static struct mda_scan *scan_read(XDR *xdrs, int recursive)
{
  struct mda_scan *scan;
 
  int i;

  scan = (struct mda_scan *) malloc( sizeof(struct mda_scan));


  if( !xdr_int16_t(xdrs, &(scan->scan_rank) ))
    return NULL;
  if( !xdr_int32_t(xdrs, &(scan->requested_points) ))
    return NULL;
  if( !xdr_int32_t(xdrs, &(scan->last_point) ))
    return NULL;

  // this happens in corrupt files sometimes.
  if( scan->scan_rank < 1)
    return NULL;

  if( scan->scan_rank > 1)
    {
      scan->offsets = (int32_t *) 
        malloc( scan->requested_points * sizeof(int32_t));
      if( !xdr_vector( xdrs, (char *) scan->offsets, scan->requested_points, 
		       sizeof( int32_t), (xdrproc_t) xdr_int32_t))
	return NULL;

      // there can be no zero offsets for the first "last_point" values
      for( i = 0; i < scan->last_point; i++)
        if( scan->offsets[i] == 0)
          return NULL;
    }
  else
    scan->offsets = NULL;

  if( !xdr_counted_string( xdrs, &(scan->name) ))
    return NULL;
  if( !xdr_counted_string( xdrs, &(scan->time) ))
    return NULL;

  if( !xdr_int16_t(xdrs, &(scan->number_positioners) ))
    return NULL;
  if( !xdr_int16_t(xdrs, &(scan->number_detectors) ))
    return NULL;
  if( !xdr_int16_t(xdrs, &(scan->number_triggers) ))
    return NULL;



  scan->positioners = (struct mda_positioner **) 
    malloc( scan->number_positioners * sizeof(struct mda_positioner *));
  for( i = 0; i < scan->number_positioners; i++)
    {
      if( (scan->positioners[i] = positioner_read( xdrs)) == NULL )
	return NULL;
    }

  scan->detectors = (struct mda_detector **) 
    malloc( scan->number_detectors * sizeof(struct mda_detector *));
  for( i = 0; i < scan->number_detectors; i++)
    {
      if( (scan->detectors[i] = detector_read( xdrs)) == NULL )
	return NULL;
    }

  scan->triggers = (struct mda_trigger **) 
    malloc( scan->number_triggers * sizeof(struct mda_trigger *));
  for( i = 0; i < scan->number_triggers; i++)
    {
      if( (scan->triggers[i] = trigger_read( xdrs)) == NULL )
	return NULL;
    }

  scan->positioners_data = (double **) 
    malloc( scan->number_positioners * sizeof(double *));
  for( i = 0 ; i < scan->number_positioners; i++)
    {
      scan->positioners_data[i] = (double *) 
	malloc( scan->requested_points * sizeof(double));
      if( !xdr_vector( xdrs, (char *) scan->positioners_data[i], 
		       scan->requested_points, 
		       sizeof( double), (xdrproc_t) xdr_double))
	return NULL;
    }

  scan->detectors_data = (float **) 
    malloc( scan->number_detectors * sizeof(float *));
  for( i = 0 ; i < scan->number_detectors; i++)
    {
      scan->detectors_data[i] = (float *) 
	malloc( scan->requested_points * sizeof(float));
      if( !xdr_vector( xdrs, (char *) scan->detectors_data[i], 
		       scan->requested_points, 
		       sizeof( float), (xdrproc_t) xdr_float))
	return NULL;
    }

  if( (scan->scan_rank > 1) && recursive)
    {
      scan->sub_scans = (struct mda_scan **)
	malloc( scan->requested_points * sizeof( struct mda_scan *) );
      for( i = 0; i < scan->requested_points; i++)
	scan->sub_scans[i] = NULL;
      for( i = 0 ; (i < scan->requested_points) && 
	     (scan->offsets[i] != 0); i++)
	/* it recurses here */
        {
          if( xdr_getpos( xdrs) != scan->offsets[i])
            {
              if( !xdr_setpos( xdrs,scan->offsets[i] ) )
                return NULL;
            }
          scan->sub_scans[i] = scan_read(xdrs, recursive);
        }
    }
  else
    scan->sub_scans = NULL;


  return scan;
}




static struct mda_pv *pv_read(XDR *xdrs)
{
  struct mda_pv *pv;

/*   unsigned int   byte_count; */

  pv = (struct mda_pv *) malloc( sizeof(struct mda_pv));


  if( !xdr_counted_string( xdrs, &(pv->name) ))
    return NULL;
  if( !xdr_counted_string( xdrs, &(pv->description) ))
    return NULL;
  if( !xdr_int16_t(xdrs, &(pv->type) ))
    return NULL;

  if( pv->type != EXTRA_PV_STRING)
    {
      if( !xdr_int16_t(xdrs, &(pv->count) ))
	return NULL;
      if( !xdr_counted_string( xdrs, &(pv->unit) ))
	return NULL;
    }
  else
    {
      pv->count = 0;
      pv->unit = NULL;
    }

  switch( pv->type)
    {
    case EXTRA_PV_STRING:
      if( !xdr_counted_string( xdrs, &(pv->values) ))
	return NULL;
      break;
    case EXTRA_PV_INT8:
      pv->values = (char *) malloc( pv->count * sizeof(int8_t));
#ifndef DARWIN
      if( !xdr_vector( xdrs, pv->values, pv->count, 
		       sizeof( int8_t), (xdrproc_t) xdr_int8_t))
	return NULL;
#else
        // MacOS Darwin is missing xdr_int8_t, have to fake it with xdr_char
      if( !xdr_vector( xdrs, pv->values, pv->count, 
		       sizeof( int8_t), (xdrproc_t) xdr_char))
	return NULL;
#endif
      break;
    case EXTRA_PV_INT16:
      pv->values = (char *) malloc( pv->count * sizeof(int16_t));
      if( !xdr_vector( xdrs, pv->values, pv->count, 
		       sizeof( int16_t), (xdrproc_t) xdr_int16_t))
	return NULL;
      break;
    case EXTRA_PV_INT32:
      pv->values = (char *) malloc( pv->count * sizeof(int32_t));
      if( !xdr_vector( xdrs, pv->values, pv->count, 
		       sizeof( int32_t), (xdrproc_t) xdr_int32_t))
	return NULL;
      break;
    case EXTRA_PV_FLOAT:
      pv->values = (char *) malloc( pv->count * sizeof(float));
      if( !xdr_vector( xdrs, pv->values, pv->count, 
		       sizeof( float), (xdrproc_t) xdr_float))
	return NULL;
      break;
    case EXTRA_PV_DOUBLE:
      pv->values = (char *) malloc( pv->count * sizeof(double));
      if( !xdr_vector( xdrs, pv->values, pv->count, 
		       sizeof( double), (xdrproc_t) xdr_double))
	return NULL;
      break;
    }

  return pv;
}




static struct mda_extra *extra_read(XDR *xdrs)
{
  struct mda_extra *extra;

  int i;

  extra = (struct mda_extra *) malloc( sizeof(struct mda_extra));


  if( !xdr_int16_t(xdrs, &(extra->number_pvs) ))
    return NULL;

  extra->pvs = (struct mda_pv **) 
    malloc( extra->number_pvs * sizeof( struct mda_pv *) );

   for( i = 0 ; i < extra->number_pvs; i++)
     { 
       if( (extra->pvs[i] = pv_read(xdrs)) == NULL )
	 return NULL;
     } 

  return extra;
}


/////////////////////////////////////////////////////////////




struct mda_file *mda_load( FILE *fptr)
{
#ifndef XDR_HACK
  XDR xdrs;
#endif
  XDR *xdrstream;

  struct mda_file *mda;

  rewind( fptr);

#ifdef XDR_HACK
  xdrstream = fptr;
#else
  xdrstream = &xdrs;
  xdrstdio_create(xdrstream, fptr, XDR_DECODE);
#endif

  mda = (struct mda_file *) malloc( sizeof(struct mda_file));

  if( (mda->header = header_read( xdrstream)) == NULL)
    return NULL;
  if( (mda->scan = scan_read( xdrstream, 1)) == NULL)
    return NULL;
  if( mda->header->extra_pvs_offset)
    {
      if( !xdr_setpos( xdrstream, mda->header->extra_pvs_offset ))
        return NULL;
      if( (mda->extra = extra_read( xdrstream)) == NULL)
	return NULL;
    }
  else
    mda->extra = NULL;
  
#ifndef XDR_HACK
  xdr_destroy( xdrstream);
#endif

  return mda;
}


struct mda_header *mda_header_load( FILE *fptr)
{
#ifndef XDR_HACK
  XDR xdrs;
#endif
  XDR *xdrstream;

  struct mda_header *header;

  rewind( fptr);

#ifdef XDR_HACK
  xdrstream = fptr;
#else
  xdrstream = &xdrs;
  xdrstdio_create(xdrstream, fptr, XDR_DECODE);
#endif

  if( (header = header_read( xdrstream)) == NULL)
    return NULL;

#ifndef XDR_HACK
  xdr_destroy( xdrstream);
#endif
  
  return header;
}



struct mda_scan *mda_scan_load( FILE *fptr)
{
  return mda_subscan_load( fptr, 0, NULL, 1);
}


struct mda_scan *mda_subscan_load( FILE *fptr, int depth, int *indices, 
                                          int recursive)
{
  struct mda_scan *scan;

#ifndef XDR_HACK
  XDR xdrs;
#endif
  XDR *xdrstream;

  struct mda_header *header;

  rewind( fptr);

#ifdef XDR_HACK
  xdrstream = fptr;
#else
  xdrstream = &xdrs;
  xdrstdio_create(xdrstream, fptr, XDR_DECODE);
#endif

  if( (header = header_read( xdrstream)) == NULL)
    return NULL;

  if( (depth < 0) || (depth >= header->data_rank ) )
    return NULL;

  if( depth)
    {
      int i;

      int16_t scan_rank;
      int32_t  requested_points;
      int32_t  last_point;
      int32_t *offsets;

      for( i = 0; i < depth; i++)
	{
	  if( indices[i] >= header->dimensions[i])
	    return NULL;
	}

      for( i = 0; i < depth; i++)
	{
	  if( !xdr_int16_t(xdrstream, &scan_rank ))
	    return NULL;
	  if( scan_rank < 1)  // file error
	    return NULL;
	  if( scan_rank == 1)  // this case should not happen
	    return NULL;

	  if( !xdr_int32_t(xdrstream, &requested_points ))
	    return NULL;

	  if( !xdr_int32_t(xdrstream, &last_point ))
	    return NULL;
	  if( indices[i] >= last_point)
	    return NULL;


	  offsets = (int32_t *) malloc( requested_points * sizeof(int32_t));
	  if( !xdr_vector( xdrstream, (char *) offsets, requested_points, 
			   sizeof( int32_t), (xdrproc_t) xdr_int32_t))
	    return NULL;

	  if( offsets[indices[i]])
	    fseek( fptr, offsets[indices[i]], SEEK_SET);
	  else
	    return NULL;

	  free( offsets);
	}
    }

  if( (scan = scan_read( xdrstream, recursive)) == NULL)
    return NULL;

#ifndef XDR_HACK
  xdr_destroy( xdrstream);
#endif

  mda_header_unload(header);

  return scan;
}


// logic here is screwy, as a NULL return could mean there are no extra PV's
struct mda_extra *mda_extra_load( FILE *fptr)
{
  struct mda_extra *extra;

#ifndef XDR_HACK
  XDR xdrs;
#endif
  XDR *xdrstream;

  struct mda_header *header;


  rewind( fptr);

#ifdef XDR_HACK
  xdrstream = fptr;
#else
  xdrstream = &xdrs;
  xdrstdio_create(xdrstream, fptr, XDR_DECODE);
#endif

  if( (header = header_read( xdrstream)) == NULL)
    return NULL;


  if( header->extra_pvs_offset)
    {
      fseek( fptr, header->extra_pvs_offset, SEEK_SET);
      if( (extra = extra_read( xdrstream)) == NULL)
	return NULL;
    }
  else
    extra = NULL;


#ifndef XDR_HACK
  xdr_destroy( xdrstream);
#endif

  mda_header_unload(header);

  return extra;
}


///////////////////////////////////////////////////////////////////
// unloaders


void mda_header_unload(struct mda_header *header)
{
  free( header->dimensions);
  free( header);
}


/* this function is recursive */
void mda_scan_unload( struct mda_scan *scan)
{
  int i;

  if( (scan->scan_rank > 1) && (scan->sub_scans != NULL))
    {
      for( i = 0; (i < scan->requested_points) && (scan->sub_scans[i] != NULL);
           i++)
	mda_scan_unload( scan->sub_scans[i]);
    }
  free( scan->sub_scans);
  
  free( scan->offsets);
  free( scan->name);
  free( scan->time);

  for( i = 0; i < scan->number_positioners; i++)
    {
      free(scan->positioners[i]->name);
      free(scan->positioners[i]->description);
      free(scan->positioners[i]->step_mode);
      free(scan->positioners[i]->unit);
      free(scan->positioners[i]->readback_name);
      free(scan->positioners[i]->readback_description);
      free(scan->positioners[i]->readback_unit);
      free(scan->positioners[i]);
    }
  free( scan->positioners);

  for( i = 0; i < scan->number_triggers; i++)
    {
      free(scan->triggers[i]->name);
      free(scan->triggers[i]);
    }      
  free( scan->triggers);

  for( i = 0; i < scan->number_detectors; i++)
    {
      free(scan->detectors[i]->name);
      free(scan->detectors[i]->description);
      free(scan->detectors[i]->unit);
      free(scan->detectors[i]);
    }
  free( scan->detectors);

  for( i = 0 ; i < scan->number_positioners; i++)
    free( scan->positioners_data[i] );
  free( scan->positioners_data );

  for( i = 0 ; i < scan->number_detectors; i++)
    free( scan->detectors_data[i] );
  free( scan->detectors_data );

  free( scan);
}


void mda_extra_unload(struct mda_extra *extra)
{
  int i;

  if( extra)
    {
      for( i = 0; i < extra->number_pvs; i++)
	{
	  free( extra->pvs[i]->name);
	  free( extra->pvs[i]->description);
	  if( extra->pvs[i]->unit)
	    free( extra->pvs[i]->unit);
	  free( extra->pvs[i]->values);
	  free( extra->pvs[i]);
	}
      free( extra->pvs);
      free( extra);
    }
}


/*  deallocates all the memory used for the mda file loading  */
void mda_unload( struct mda_file *mda)
{
  mda_header_unload(mda->header);
  mda_scan_unload(mda->scan);
  mda_extra_unload(mda->extra);

  free( mda);
}




//////////////////////////////////////////////////////////////////////////


struct mda_fileinfo *mda_info_load( FILE *fptr)
{
  struct mda_fileinfo *fileinfo;

#ifndef XDR_HACK
  XDR xdrs;
#endif
  XDR *xdrstream;

  int32_t  last_point;
  int32_t *offsets;
  char *time;

  int i, j;

  int32_t t;
  
 
  rewind( fptr);

#ifdef XDR_HACK
  xdrstream = fptr;
#else
  xdrstream = &xdrs;
  xdrstdio_create(xdrstream, fptr, XDR_DECODE);
#endif

  fileinfo = (struct mda_fileinfo *) 
    malloc( sizeof(struct mda_fileinfo));
  
  if( !xdr_float(xdrstream, &(fileinfo->version) ))
    return NULL;

  if( !xdr_int32_t(xdrstream, &(fileinfo->scan_number) ))
    return NULL;

  if( !xdr_int16_t(xdrstream, &(fileinfo->data_rank) ))
    return NULL;

  fileinfo->dimensions = 
    (int32_t *) malloc( fileinfo->data_rank * sizeof(int32_t));
  if( !xdr_vector( xdrstream, (char *) fileinfo->dimensions, 
                   fileinfo->data_rank, 
                   sizeof( int32_t), (xdrproc_t) xdr_int32_t))
    return NULL;

  for( i = 0; i < fileinfo->data_rank; i++)
    // -1 is it was int16_t, not int32_t
    if(  fileinfo->dimensions[i] == 0xFFFFFFFF) 
      {
        free(fileinfo->dimensions);
        free(fileinfo);
        return NULL;
      }

  if( !xdr_int16_t(xdrstream, &(fileinfo->regular) ))
    return NULL;

  // don't need this
  if( !xdr_int32_t(xdrstream, &t )) 
    return NULL;


  // This double pointer business is a bit of overkill, but to be consistent, 
  // I'll do it here as well
  fileinfo->scaninfos = (struct mda_scaninfo **) 
    malloc( fileinfo->data_rank * sizeof(struct mda_scaninfo *));


  for( i = 0; i < fileinfo->data_rank; i++)
    {
      fileinfo->scaninfos[i] = (struct mda_scaninfo *) 
        malloc( sizeof(struct mda_scaninfo ));

      if( !xdr_int16_t(xdrstream, &(fileinfo->scaninfos[i]->scan_rank) ))
        return NULL;

      // file error
      if( fileinfo->scaninfos[i]->scan_rank < 1)
        return NULL;

      if( !xdr_int32_t(xdrstream, 
                       &(fileinfo->scaninfos[i]->requested_points)))
	return NULL;
      if( !xdr_int32_t(xdrstream, &last_point ))
	return NULL;

      if( fileinfo->scaninfos[i]->scan_rank > 1)
	{
	  offsets = 
            (int32_t *) malloc( fileinfo->scaninfos[i]->requested_points 
				     * sizeof(int32_t));
	  if( !xdr_vector( xdrstream, (char *) offsets, 
			   fileinfo->scaninfos[i]->requested_points, 
			   sizeof( int32_t), (xdrproc_t) xdr_int32_t))
	    return NULL;
	}
      else
	offsets = NULL;

      if( !xdr_counted_string( xdrstream, &(fileinfo->scaninfos[i]->name) ))
	return NULL;

      if( !xdr_counted_string( xdrstream, &time ))
	return NULL;
      
      // only want this stuff for outer loop
      if( !i)
	{
	  fileinfo->last_topdim_point = last_point;
	  fileinfo->time = time;
	}
      else
	free(time);

      if( !xdr_int16_t(xdrstream, 
                       &(fileinfo->scaninfos[i]->number_positioners)))
	return NULL;
      if( !xdr_int16_t(xdrstream, 
                       &(fileinfo->scaninfos[i]->number_detectors)))
	return NULL;
      if( !xdr_int16_t(xdrstream, &(fileinfo->scaninfos[i]->number_triggers)))
	return NULL;

     
      fileinfo->scaninfos[i]->positioners = (struct mda_positioner **) 
	malloc( fileinfo->scaninfos[i]->number_positioners * 
		sizeof(struct mda_positioner *));
      for( j = 0; j < fileinfo->scaninfos[i]->number_positioners; j++)
	{
	  if( (fileinfo->scaninfos[i]->positioners[j] = 
	       positioner_read(xdrstream)) == NULL )
	    return NULL;
	}

      fileinfo->scaninfos[i]->detectors = (struct mda_detector **) 
	malloc( fileinfo->scaninfos[i]->number_detectors * 
		sizeof(struct mda_detector *));
      for( j = 0; j < fileinfo->scaninfos[i]->number_detectors; j++)
	{
	  if( (fileinfo->scaninfos[i]->detectors[j] = 
	       detector_read( xdrstream)) == NULL )
	    return NULL;
	}

      fileinfo->scaninfos[i]->triggers = (struct mda_trigger **) 
	malloc( fileinfo->scaninfos[i]->number_triggers * 
		sizeof(struct mda_trigger *));
      for( j = 0; j < fileinfo->scaninfos[i]->number_triggers; j++)
	{
	  if( (fileinfo->scaninfos[i]->triggers[j] = 
	       trigger_read( xdrstream)) == NULL )
	    return NULL;
	}

      if( offsets != NULL)
	{
	  fseek( fptr, offsets[0], SEEK_SET);
	  free( offsets);
	}
    }

#ifndef XDR_HACK
  xdr_destroy( xdrstream);
#endif

  return fileinfo;
}


void mda_info_unload( struct mda_fileinfo *fileinfo)
{
  int i, j;

  struct mda_scaninfo *scaninfo;

  free( fileinfo->time);

  for( j = 0; j < fileinfo->data_rank; j++)
  {
    scaninfo = fileinfo->scaninfos[j];

    free( scaninfo->name);

    for( i = 0; i < scaninfo->number_positioners; i++)
      {
	free(scaninfo->positioners[i]->name);
	free(scaninfo->positioners[i]->description);
	free(scaninfo->positioners[i]->step_mode);
	free(scaninfo->positioners[i]->unit);
	free(scaninfo->positioners[i]->readback_name);
	free(scaninfo->positioners[i]->readback_description);
	free(scaninfo->positioners[i]->readback_unit);
	free(scaninfo->positioners[i]);
      }
    free( scaninfo->positioners);
    
    for( i = 0; i < scaninfo->number_triggers; i++)
      {
        free(scaninfo->triggers[i]->name);
        free(scaninfo->triggers[i]);
      }
    free( scaninfo->triggers);
    
    for( i = 0; i < scaninfo->number_detectors; i++)
      {
	free(scaninfo->detectors[i]->name);
	free(scaninfo->detectors[i]->description);
	free(scaninfo->detectors[i]->unit);
        free(scaninfo->detectors[i]);
      }
    free( scaninfo->detectors);

    free( scaninfo);
 }

  free( fileinfo->scaninfos);
  free( fileinfo->dimensions);
  
  free( fileinfo);

}
