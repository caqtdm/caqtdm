/*************************************************************************\
* Copyright (c) 2013 UChicago Argonne, LLC,
*               as Operator of Argonne National Laboratory.
* This file is distributed subject to a Software License Agreement
* found in file mdaLICENSE that is included with this distribution. 
\*************************************************************************/
#include <stdlib.h>
#include "mda-load.h"
#include "mdaReader.h"
#include "qtdefinitions.h"

#define PRINT(x)

void mdaReaderThread(const char *dataFile, int y_cpy);
struct mda_file *mdaData = NULL;

void mdaReader_RegisterPV(QString pvName) {
    PRINT(printf("Somebody registered %s\n", qasc(pvName)));
	return;
}

int mdaReader_gimmeYerData(QString QS_dataFile, QString QS_pvName, float *data, int nx, int ny, int y_cpt) {
	int i, j, index, detNum=0, ycpt, xcpt;
	char pvName[60] = "";
	struct mda_scan *thisScan = NULL;

    strcpy(pvName, qasc(QS_pvName));
	detNum = atol(&(pvName[strlen(pvName)-4]));
	detNum--; // convert from pvName number 01..70 to array index 0..69
	if (detNum < 0) {
		PRINT(printf("mdaReader_gimmeYerData: '%s' not found in pv name\n", pvName));
		return(-1);
	}
	PRINT(printf("Somebody requested data for %s (detNum=%d), y_cpt=%d\n", pvName, detNum, y_cpt));
	
	// This should be a message to a reader thread.  For now, we just read the whole file.
    mdaReaderThread(qasc(QS_dataFile), y_cpt);
	if (!mdaData) {
		return(-1);
	}

	PRINT(printf("top-level scan name %s\n", mdaData->scan->name));
	if (strncmp(mdaData->scan->name, pvName, strlen(mdaData->scan->name))) {
		if (mdaData->scan->scan_rank < 2) {
			return(-1);
		}
		thisScan = mdaData->scan->sub_scans[0]; // 2D data is one rank down in file
		// Find detector index in file that corresponds to detNum
		for (index=0; index < thisScan->number_detectors; index++) {
			if (detNum == thisScan->detectors[index]->number) break;
		}
		if (index == thisScan->number_detectors) {
			PRINT(printf("detNum %d does not occur in data file\n", detNum));
			for (i=0; i<ny; i++) {
				for (j=0; j<nx; j++) data[i*nx+j] = 0.;
			}
			return(-1);
		}
		PRINT(printf("2D data from %s\n", mdaData->scan->sub_scans[0]->name));
		// last_point is the number of data points acquired
		ycpt = mdaData->scan->last_point;
		if (mdaData->scan->sub_scans == NULL) {
			PRINT(printf("Expected 2D data not found\n"));
			return(-1);
		}
		for (i=0; i<ycpt; i++) {
			thisScan = mdaData->scan->sub_scans[i];
			if (thisScan == NULL) {
				PRINT(printf("Expected 2D data (sub_scans[%d]) not found\n", i));
				for (j=0; j<nx; j++) data[i*nx+j] = 0.;
				continue;
			}
			xcpt = thisScan->last_point;
			if (thisScan->detectors_data[index] == NULL) {
				PRINT(printf("Expected 2D data (detectors_data[%d]) not found\n", index));
				for (j=0; j<nx; j++) data[i*nx+j] = 0.;
				continue;
			}
			for (j=0; j<xcpt; j++) {
				data[i*nx+j] = thisScan->detectors_data[index][j];
			}
			for(; j<nx; j++) {
				data[i*nx+j] = 0.;
			}
		}
		for (; i<ny; i++) {
			for (j=0; j<nx; j++) {
				data[i*nx+j] = 0.;
			}
		}
	}
	
	return(0);
};


// Should have a thread to read the data file.  For now, just do it in the GUI thread.

#define NAMELEN 100
void mdaReaderThread(const char *dataFile, int y_cpt) {
	FILE *fp;
	static char saved_dataFile[NAMELEN] = "";
	static int saved_ycpt = 0;
	char fname[100] = "";

	if ((strncmp(dataFile, saved_dataFile, NAMELEN) == 0) && (y_cpt <= saved_ycpt)) {
		return;
	}

	strncpy(saved_dataFile, dataFile, NAMELEN);
	saved_ycpt = y_cpt;

	// for now, because mda_load doesn't know how to add to data structure
	if (mdaData) mda_unload(mdaData);

	if (strncmp(dataFile, "//", strlen("//")) == 0) {
		// a vxWorks IOC has a filepath specification that's different from that of
		// a linux soft ioc.  The form is "//server/dir1/dir2/file", and I assume
		// a valid path to file is "/net/server/dir1/dir2/file".  I don't know how
		// portable this is, but sysadmins here suggest it's common for an automounter.
		// I'm out of my league here.
		strcpy(fname, "/net");
		strcat(fname, dataFile);
	} else {
		strcat(fname, dataFile);
	}
	fp = fopen(fname, "rb");
	if (fp) mdaData = mda_load(fp);
	PRINT(printf("mdaReaderThread: read '%s'\n", fname));
}
