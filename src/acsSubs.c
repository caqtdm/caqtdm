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
#include <time.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/timeb.h>

#include <fortypes.h>
#include <userstruct.h>
#include <host2vax.h>
#include <piosubs.h>
#include <coredb.h>

#include <utils/mystring.h>
#include <utils/gendefs.h>
#include <utils/myftime.h>

#include "medmpio.h"
#include "dbrString.h"
#include "knobDefines.h"
#include "knobData.h"
#include "mutexKnobDataWrapper.h"
#include "acsSubs.h"
#include "messageWindowWrapper.h"
#include "vaPrintf.h"

extern MutexKnobData* KnobDataPtr;

typedef char string40[40];

#define MAX_DEVS                                2500        /* maximum number of devices */
#define MAX_FECS                                50          /* maximum number of fecs */
#define MAX_LISTS                              200          /* maximum number of lists */
#define MAX_CELLS                               55          /* maximum number of cells per message */
#define MAX_RECORDS                             32
#define ERRMSGLEN 50

#define Boolean                                 int

extern void     opncom();
extern void     set_usr$process_name(DSC *name);
extern void     ADCOMP(DSC *nodeD, AddProVaxType *addpro, int *error);
extern void     GET_ETHER_ADDPROT(AddProVaxType *addpro);

int BlockserverData(pioDevices *cells, int *totalval, float **Xbuf, float **Ybuf, char *errorMsg);

typedef struct {
    char            fec[6];
} fec6;

/* process & communication parameters and variables */

static          pioDevices ValueCells[MAX_DEVS];
static          pioDevices ActivCells[1];
static void     etherReceive();

static          fec6 fecNames[MAX_FECS][MAX_LISTS];
static DSC      fecNamesD = {6, 14, 4, 0};
static int      listnumber[MAX_LISTS];
static INT4     *flags = (INT4 *) 0;
static INT4     *tims = (INT4 *) 0;
static fecInfo  *infolist  = (fecInfo *) 0;

static int      Repetition[MAX_DEVS];
static int      firstTime[MAX_DEVS];
static int      NumCells;
static struct timeb lastTime[MAX_DEVS];
static int      extent[MAX_DEVS];

#define MaxCellsPerFEC    55
#define MEM_MALLOC(x)     malloc((x))
#define MEM_FREE(x)       free((x))

static void     UnpackDevice(char strng[], char devS[9], char attS[5], int *convS, char *aux, int *bit, Boolean *Err);
static int      AddCell(char *name, pioDevices Cells[], int index, char *aux);
static int      RemoveCell(pioDevices Cells[], int index);
static void     medmAcquisition();
static int      ExecuteValueCells();
static void     SetRepetition(int num, int rep);
static void     CompleteCell(pioDevices Cells[], int index);
extern int      getFec();
static short    my_protocol = 0x0000;
static char     prcName[] = {"CSHELL"};
static DSC      uusr$process = {6, 0, 0, prcName};

/*****************************************************************************/
/* compare function for quicksort                                            */
/*****************************************************************************/

static int PIOcmpF(pioDevices *p1, pioDevices *p2)
{
    return strncmp(p1->fec, p2->fec, 6);
}

/*****************************************************************************/
/* thread performing the data acquisition                                    */
/*****************************************************************************/

static void *Thread()
{
    while (True) {
        //printf("execute thread with %d cells\n", NumCells);
        MONITOR_ENTER(serializeAccess, "thread");
        if (NumCells > 0) {
            ExecuteValueCells();
        }
        MONITOR_EXIT(serializeAccess, "thread");

        usleep(333000);
    }
}
/****************************************************************************/
/* data acquisition initialisation                                          */
/****************************************************************************/

void StartPIOserver()
{
    int             status, error;
    pthread_t       th;
    pthread_attr_t  attr;
    short           lun[2], evt[2];
    short           my_protocol = 0x0000;
    CSTRING         bopp;
    int             TIMEOUT = 1;

    NumCells = 0;

    flags   =(INT4 *)      malloc(MAX_FECS * MAX_LISTS * sizeof(INT4));
    tims    =(INT4 *)      malloc(MAX_FECS * MAX_LISTS * sizeof(INT4) * 2);
    infolist=(fecInfo *)   malloc(MAX_LISTS * sizeof(fecInfo));

    memset(infolist, 0,             MAX_LISTS * sizeof(fecInfo));
    memset(flags,    0,  MAX_FECS * MAX_LISTS * sizeof(LOGICAL));
    memset(tims,     0,  MAX_FECS * MAX_LISTS * sizeof(INT4) * 2);
    memset(firstTime,0,  MAX_DEVS * sizeof(int));

    device_timeout(&TIMEOUT);
    set_usr$process_name(&uusr$process);
    fecNamesD.str = (char *) fecNames;
    opncom(&lun, &evt, &bopp, &my_protocol, etherReceive, &error);
    pthread_attr_init(&attr);
    MONITOR_CREATE(&serializeAccess);
    status = pthread_create(&th, &attr, Thread, (void *)0);
    if (status != 0)
        printf("MEDM -- create thread status= %d\n", status);

    return;
}

/****************************************************************************/
static int ExecuteValueCells()
{
    knobData        kData;
    int             Error, i;
    int             devIOF;
    int             listNo;
    int             NumberOfLists;
    int             repeat = 0;
    int             tffunc;
    pioDevices      ProCells[MAX_DEVS];
    pioDevices      PioCells[MAX_DEVS];
    int             rep[MAX_DEVS];
    int             ProNumcell, PioNumcell;
    double          diff;
    struct timeb    time2;
    int             oneCell = 1;


    ProNumcell = 0;
    PioNumcell = 0;
    /* get another array for the block server devices, keep however the same array for all the devices */
    for (i=0; i<NumCells;i++) {
        if(strncmp(ValueCells[i].att, "PROF", 4) == 0) {
            memcpy(&ProCells[ProNumcell], &ValueCells[i], sizeof(pioDevices));
            rep[ProNumcell]= Repetition[i];
            ProNumcell++;
        }
        memcpy(&PioCells[PioNumcell], &ValueCells[i], sizeof(pioDevices));
        PioNumcell++;
    }

    //printf("%d proCells, %d pioCells\n", ProNumcell, PioNumcell);

    if(PioNumcell > 0) {
        tffunc = 1;
        listNo = 1;
        devIOF = 7;
        medmAcquisition(&my_protocol, &devIOF, PioCells, &PioNumcell,
                        &repeat, &listNo, &NumberOfLists, &Error, tffunc);

        if (Error < 1) {
            if(Error != -4) C_postMsgEvent(messageWindow, 1, vaPrintf("acs io-error %d\n", Error));

            // do we have timeouts somewhere ?

            if(Error == -4) {
                for (i=0; i<PioNumcell;i++) {
                    if(PioCells[i].error_code == -4) {
                        int indx = PioCells[i].ident;
                        C_GetMutexKnobData(KnobDataPtr, indx, &kData);
                        kData.edata.connected = False;
                        C_SetMutexKnobDataReceived(KnobDataPtr, &kData);
                    }
                }
            }
        }
    }

    /* if any block server cells, execute them one after the other, we will only acquire every 5 seconds */
    /* we will have to pack them, to avoid too many long buffers (acs gives x and y for a profile and the interface  */
    /* request once for x and once for y */

    if(ProNumcell > 0) {
        char name1[9], name2[9];
        char attr1[5], attr2[5];
        int  execute[MAX_DEVS];
        tffunc = 2;
        devIOF = 1;
        myftime(&time2);
        name1[0] = '\0';
        attr1[0] = '\0';

        // get the profiles to execute
        for (i = ProNumcell-1; i >= 0; i--) {
            extent[i] = -1;
            strncpy(name1, ProCells[i].name, 8); name1[8] = '\0';
            strncpy(attr1, ProCells[i].att, 4); attr1[4] = '\0';

            if((i-1) >= 0) {
                strncpy(name2, ProCells[i-1].name, 8); name2[8] = '\0';
                strncpy(attr2, ProCells[i-1].att, 4); attr2[4] = '\0';
            } else {
                name2[0] = '\0';
                attr2[0] = '\0';
            }

            if (IsInside(name2, name1) && IsInside(attr1, attr2)) {
                execute[i] = False;
            } else {
                execute[i] = True;
                if(i!=ProNumcell-1) extent[ProCells[i].ident] = ProCells[i+1].ident;
            }
        }
        // execute the profiles
        for (i = 0; i < ProNumcell; i++) {
            if(execute[i]) {
                diff = ((double) time2.time + (double) time2.millitm / (double)1000) -
                        ((double) lastTime[i].time + (double) lastTime[i].millitm / (double)1000);
                if(diff >= rep[i] || firstTime[i]) {
                    firstTime[i] = False;
                    myftime(&lastTime[i]);
                    listNo = MAX_LISTS/2 + i;

                    medmAcquisition(&my_protocol, &devIOF, &ProCells[i], &oneCell,
                                    &repeat, &listNo, &NumberOfLists, &Error, tffunc);
                    if (Error < 1) {
                        if(Error != -4) C_postMsgEvent(messageWindow, 1, vaPrintf("acs io-error %d\n", Error));
                    }
                }
            }
        }
    }

    return Error;
}

/*****************************************************************/
/* add/remove a cell to an array of piocells and return an index */
/*****************************************************************/

int AddValueCell(char *name, int indx, char *aux)
{
    MONITOR_ENTER(serializeAccess, "thread");
    int status = AddCell(name, ValueCells, indx, aux);
    MONITOR_EXIT(serializeAccess, "thread");
    return status;
}

int RemoveValueCell(int indx)
{
    MONITOR_ENTER(serializeAccess, "thread");
    int status = RemoveCell(ValueCells, indx);
    MONITOR_EXIT(serializeAccess, "thread");
    return status;
}

static int AddCell(char *name, pioDevices Cells[], int indx, char *aux)
{
    char            devS[40];
    char            attS[40];
    char            fecS[7];
    int             convS;
    int             bit;
    Boolean         Err;
    int             pnt, j;

    memset(devS, '\0', 9);
    memset(attS, '\0', 5);
    aux[0] = '\0';

    if(strlen(name) > 39) return (int)(- 1);

    UnpackDevice(name, devS, attS, &convS, aux, &bit, &Err);
    devS[8] = '\0';
    attS[4] = '\0';

    if (getFec(devS, attS, fecS) != 1) {
        //printf("Non-existent device <%s> <%.8s>\n", name, devS);
        return (int)(- 1);
    } else {
        fecS[6] = '\0';
        if(strstr(fecS, "ACSCA1") != (char*)0) {
            //printf("do not use acs devices over gateway device <%s>\n", name);
            //return (int)(- 1);
        }

        strncpy(Cells[NumCells].name, devS, 8);
        strncpy(Cells[NumCells].att, attS, 4);
        Cells[NumCells].conv = convS;
        Cells[NumCells].ident = indx;

        // complete the cell
        CompleteCell(Cells, NumCells);

        // set correct repetion rate for blockserver io
        if(Cells[NumCells].name[2] == 'P' || ValueCells[NumCells].name[2] == 'L') {
            SetRepetition(NumCells, 10);  // moving motor
        } else {
            SetRepetition(NumCells, 1);   // no moving motor
        }

        pnt = NumCells;
        j = NumCells;
        if (j++ > MAX_DEVS) printf("Too many devices %d\n", j--);

        //printf("add cell <%s> %.8s.%.4s index=%d aux=%s\n", name, devS, attS, indx, aux);
        NumCells = j;
        return (int) pnt;
    }
}

static int RemoveCell(pioDevices Cells[], int pnt)
{
    int             i, j;

    j = 0;
    for (i = 0; i < NumCells; i++) {
        if (pnt != Cells[i].ident) {
            memcpy(&Cells[j++], &Cells[i], sizeof(pioDevices));
        } else {
            //printf("remove cell %d %d<%.8s> <%.4s> %d\n", pnt, Cells[i].ident, Cells[i].name, Cells[i].att, Cells[i].conv);
        }
    }
    NumCells--;
    if (NumCells < 0) NumCells = 0;
    return (int) NumCells;

}

/***************************************************************************/
/* complete the cells we are going to send for the dataacquisition         */
/***************************************************************************/

static void SetRepetition(int num, int rep)
{
    Repetition[num] = rep;
    if(Repetition[num] < 1) Repetition[num]=5;  /* default when nothing specified or too small is 5 seconds */
}

static void CompleteCell(pioDevices Cells[], int indx)
{
    char            devS[9];
    char            attS[5];
    char            fecS[7];
    int             j;

    for (j = 0; j < 8; j++) if (Cells[indx].name[j] < ' ') Cells[indx].name[j] = ' ';

    Cells[indx].io_func = CELL_READ;
    Cells[indx].ub.int4val = 0;
    strncpy(devS, Cells[indx].name, 8);
    devS[8] = '\0';
    strncpy(attS, Cells[indx].att, 4);
    attS[4] = '\0';
    if (getFec(devS, attS, fecS) != 1) {
        //printf("device not found <%.8s.%.4s>\n", devS, attS);
    } else {
        strncpy(Cells[indx].fec, fecS, 6);
    }

}

/***************************************************/
/* get device name, attribute and conversion level */
/***************************************************/

static int parseDelimited(char *s, string40 items[], int nbItems, char token)
{
    int i, cnt;
    char * pch;
    char ctoken[2];
    memcpy(ctoken, &token, 1);
    ctoken[1] ='\0';
    for (i=0; i< (int) strlen(s); i++) if(s[i] < ' ') s[i] = '\0';
    cnt = 0;
    pch = strtok (s, ctoken);
    while (pch != NULL)
    {
        strcpy(items[cnt], pch);
        pch = strtok (NULL, ctoken);
        if(cnt++ >= nbItems) break;
    }
    return cnt;
}

static void UnpackDevice(char strng[], char devS[9], char attS[5], int *convS, char *aux, int *bit, Boolean *Err)
{
    int             nbItems1, nbItems2;
    char            cnvS[2];
    string40        items1[10], items2[10], devtot;

    *Err = True;
    *bit = 0;
    memset(devtot, '\0', 40);
    strcpy(devS, "        ");
    strcpy(attS, "    ");
    strcpy(devtot, strng);
    strUpr(devtot);

    // decompose string
    nbItems1 = parseDelimited(devtot, items1, 10, ':');
    if(nbItems1 != 3) return;

    // we should have a device and an attribute
    memcpy(devS, items1[0], strlen(items1[0]));
    memcpy(attS, items1[1], strlen(items1[1]));

    // decompose attribute, conversion + eventually .something
    nbItems2 = parseDelimited(items1[2], items2, 10, '.');

    // we have a dot
    if(nbItems2 == 2) {
        strncpy(cnvS, items2[0], 1);
        strcpy(aux, items2[1]);
    } else {
        strncpy(cnvS, items1[2], 1);
    }

    if (cnvS[0] == '1')
        *convS = 1;
    else if (cnvS[0] == '2')
        *convS = 2;
    else if (cnvS[0] == '3')
        *convS = 3;
    else
        *convS = 0;

    *Err = False;
}

/***************************************************************************/
/* setup a cell for activ device io                                        */
/***************************************************************************/

int SetActivCell(char *name)
{
    char            devS[9], Cname[9];
    char            attS[5], Cattr[9], aux[10];
    char            fecS[7];
    int             convS;
    int             bit;
    Boolean         Err;

    memset(Cname, '\0', 9);
    memset(devS, '\0', 9);
    memset(Cattr, '\0', 5);
    memset(attS, '\0', 5);
    UnpackDevice(name, devS, attS, &convS, aux, &bit, &Err);
    if (getFec(devS, attS, fecS) != 1) {
        //printf("Non-existent device <%s>\n", name);
        return (int)(- 1);
    } else {
        strncpy(ActivCells[0].name, devS, 8);
        strncpy(ActivCells[0].att, attS, 4);
        strncpy(ActivCells[0].fec, fecS, 6);
        ActivCells[0].conv = convS;
    }
    return (int)(0);
}

static int medmSingleIO(int command, float *value, int *ival, short *ecode, int *ucode)
{
    int             listNoRet, numlist, error;
    int             repeat = 0;
    int             numdev = 1;
    int             maxfecs = MAX_FECS;
    int             maxlists = MAX_LISTS;
    int             listNo = MAX_LISTS - 2;

    //printf("%.8s.%.4s %d\n", ActivCells[0].name, ActivCells[0].att, ActivCells[0].conv);

    pioreq_execute(&maxfecs, &maxlists, &command, &repeat, &listNo, &listNoRet, &numdev,
                   (allDevices *) ActivCells, &numlist,  infolist, &fecNamesD, &error);
    if (error > 0) {
        int             tmg = 100;
        int             tnt = 1;
        int             loop = 0;

        error = 0;
        while (error < 1) {
            pioreq_sync(&tmg, &tnt, &maxfecs, &maxlists, infolist, &listNoRet, &listNoRet,
                        flags, tims, &error);
            if (loop++ > 3) break;
        }
        if (error != 1) {
            return (int) error;
        } else {
            if (ActivCells[0].unit_code > 3) {
                *value = ActivCells[0].ub.value;
                *ival = ActivCells[0].ub.int4val;
            } else {
                *ival = ActivCells[0].ub.int4val;
            }
            *ecode = ActivCells[0].error_code;
            *ucode = ActivCells[0].unit_code;
            return (int) error;
        }
    } else {
        return (int) error;
    }
}

int SetDeviceFloatValue(char *name, float *value)
{
    int             command, j;
    int             ival, ucode, error;
    short           ecode;
    char            mess[ERRMSGLEN + 1];
    FORSTR messD = {ERRMSGLEN, 0, 0, mess};

    MONITOR_ENTER(serializeAccess, "thread");

    int iofunc = 1;

    if (SetActivCell(name) > - 1) {
        command = PIO_DEFINE + PIO_EXECUTE + PIO_DELETE;
        for (j = 0; j < 8; j++) if (ActivCells[0].name[j] < ' ') ActivCells[0].name[j] = ' ';
        for (j = 0; j < 4; j++) if (ActivCells[0].att[j]  < ' ') ActivCells[0].att[j] = ' ';

        ActivCells[0].io_func = iofunc;
        ActivCells[0].ub.value = *value;
        Host2VaxF(&ActivCells[0].ub.value, ActivCells[0].ub.value);
        error = medmSingleIO(command, value, &ival, &ecode, &ucode);
        if(ecode != 1) {
            getErrMsgStr(&ecode, &messD);
            mess[ERRMSGLEN] = '\0';
            C_postMsgEvent(messageWindow, 1, vaPrintf("device <%s> has io-error %d %s\n", name, ecode, mess));
        }
        MONITOR_EXIT(serializeAccess, "thread");
        return error;
    } else {
        C_postMsgEvent(messageWindow, 1, vaPrintf("device <%s> fec not found\n", name));
        MONITOR_EXIT(serializeAccess, "thread");
        return 0;
    }
}

int SetDeviceStringValue(char *name, char *data)
{
    int             command, i, j;
    char            asc[80];
    int             ival, ucode, error;
    short           ecode;
    float           value;
    char            mess[ERRMSGLEN + 1];
    FORSTR messD = {ERRMSGLEN, 0, 0, mess};
    char            devS[9], attS[5];
    DSC             devD = {8, 0, 0, devS};
    DSC             attD = {4, 0, 0, attS};
    F_CMDT_RCRD     bitmapping[MAX_RECORDS];
    int             dim = 16, num;
    int             found = False;

    MONITOR_ENTER(serializeAccess, "thread");

    //printf("SendString %s %s\n", name, data);

    if (SetActivCell(name) > - 1) {
        command = PIO_DEFINE + PIO_EXECUTE + PIO_DELETE;
        for (j = 0; j < 8; j++) if (ActivCells[0].name[j] < ' ') ActivCells[0].name[j] = ' ';
        for (j = 0; j < 4; j++) if (ActivCells[0].att[j] < ' ') ActivCells[0].att[j] = ' ';

        memcpy(devS, ActivCells[0].name, 8);
        memcpy(attS, ActivCells[0].att, 4);
        num = getCmdNames(&devD, &attD, &dim, bitmapping);

        for(i = 0; i < num; i++) {
            char aux[9];
            strncpy(aux, bitmapping[i].bitName, 8);
            aux[8] = '\0';
            //printf("%s %s\n", aux, data);
            if(IsInside(aux, data)) {
                found = True;
                //printf("found number=%d bitmapping is %x %x\n", i, bitmapping[i].smask, bitmapping[i].cmask);

                ActivCells[0].conv = 1;
                ActivCells[0].io_func = CELL_DIGITAL;
                ActivCells[0].ub.int2val[0] = (unsigned short) bitmapping[i].smask;
                ActivCells[0].ub.int2val[1] = (unsigned short) bitmapping[i].cmask;
                sprintf(asc, "Digital Action on %.8s.%.4s;%d - Data %04x %04x", ActivCells[0].name,
                        ActivCells[0].att,
                        ActivCells[0].conv,
                        (unsigned short) ActivCells[0].ub.int2val[0],
                        (unsigned short) ActivCells[0].ub.int2val[1]);

                C_postMsgEvent(messageWindow, 1, asc);

                error = medmSingleIO(command, &value, &ival, &ecode, &ucode);
                if(ecode != 1) {
                    getErrMsgStr(&ecode, &messD);
                    mess[ERRMSGLEN] = '\0';
                    C_postMsgEvent(messageWindow, 1, vaPrintf("device <%.8s.%.4s;%d> has io-error %d %s\n", ActivCells[0].name, ActivCells[0].att, ActivCells[0].conv, ecode, mess));
                }
                MONITOR_EXIT(serializeAccess, "thread");
                return error;
            }
        }
        if(!found) {
            sprintf(asc, "device  <%.8s.%.4s;%d> no bitpattern found", ActivCells[0].name, ActivCells[0].att, ActivCells[0].conv);
            C_postMsgEvent(messageWindow, 1, asc);
            MONITOR_EXIT(serializeAccess, "thread");
            return 0;
        }
    } else {
        C_postMsgEvent(messageWindow, 1, vaPrintf("device <%s> fec not found\n", name));
        MONITOR_EXIT(serializeAccess, "thread");
        return 0;
    }

    MONITOR_EXIT(serializeAccess, "thread");
    return 0;
}

int GetDeviceValue(char *name, float *value, int *ival, short *ecode, int *ucode)
{
    int             command, j;
    int             error;

    if (SetActivCell(name) > - 1) {
        command = PIO_DEFINE + PIO_EXECUTE + PIO_DELETE;
        for (j = 0; j < 8; j++)
            if (ActivCells[0].name[j] < ' ')
                ActivCells[0].name[j] = ' ';

        ActivCells[0].io_func = CELL_READ;
        error = 1;
        return medmSingleIO(command, value, ival, ecode, ucode);
    } else {
        C_postMsgEvent(messageWindow, 1, vaPrintf("device <%s> fec not found\n", name));
        return 0;
    }
}

/*******************************************************************************/
/* we build optimized lists (network optimized) for our devices. if the	       */
/* list requested can not contain everything, we build new lists with a higher */
/* list number                                                                 */
/*******************************************************************************/

static void medmAcquisition(short *own_prot, int *devIOFunc, pioDevices *Cells, int *Nbcells, int *rep,
                            int *ReqNo, int *NumberOfLists, int *listError, int tffunc)
{
    int             i, k, error;
    int             finished;
    int             ident_number;
    int             nlists;
    int             repetition;
    int             listNo;
    int             lisReqNo;
    int             listDevices;
    char            actFec[7], prvFec[7];
    int             indFec;
    int             total[MAX_FECS];
    static int      prvNumcells = 0;
    int             numlist = 0;
    static pioDevices     *SortCells = (pioDevices *)0;
    static pioDevices     *SendCells = (pioDevices *)0;
    static int            *CellTaken = (int *)0;

    UNUSED(own_prot);
    repetition = *rep;
    ident_number = 1;
    lisReqNo = *ReqNo;
    nlists = 1;
    *NumberOfLists = 0;

    /* allocate memory */

    if (*Nbcells != prvNumcells) {
        if (SortCells != (pioDevices *)0)
            MEM_FREE(SortCells);

        if (SendCells != (pioDevices *)0)
            MEM_FREE(SendCells);

        if (CellTaken != (int *)0)
            MEM_FREE(CellTaken);

        SortCells = (pioDevices *) MEM_MALLOC(*Nbcells *sizeof(pioDevices));
        SendCells = (pioDevices *) MEM_MALLOC(*Nbcells *sizeof(pioDevices));
        CellTaken = (int *) MEM_MALLOC(*Nbcells *sizeof(int));
        prvNumcells = *Nbcells;
    }

    /* copy all cells into "sort" cells */

    memcpy(SortCells, Cells, *Nbcells *sizeof(pioDevices));

    for (i = 0; i < *Nbcells; i++) CellTaken[i] = False;

    /* qsort the cells according to the ioc */

    qsort(SortCells, *Nbcells, sizeof(pioDevices), (int (*)(const void *, const void *))PIOcmpF);

    strcpy(prvFec, " ");
    prvFec[6] = '\0';
    actFec[6] = '\0';
    indFec = 0;
    total[0] = 0;
    finished = False;
    while (!finished) {
        k = 0;
        for (i = 0; i < *Nbcells; i++) {
            if (!CellTaken[i] && total[indFec] < MaxCellsPerFEC - 3) {
                CellTaken[i] = True;
                memcpy(&SendCells[k++], &SortCells[i], sizeof(pioDevices));
                total[indFec]++;
            }
            memcpy(actFec, SortCells[i].fec, 6);
            if (i > 0 && PtrNull(strstr(prvFec, actFec))) {
                indFec++;
                total[indFec] = 0;
                // 16-2-2009 added
                if (!CellTaken[i]) {
                    //printf("piosubs -- fec change <%.6s> <%.6s> copy cell %d\n", prvFec, actFec, i);
                    CellTaken[i] = True;
                    memcpy(&SendCells[k++], &SortCells[i], sizeof (pioDevices));
                    total[indFec]++;
                }
            }
            memcpy(prvFec, SortCells[i].fec, 6);
        }
        listDevices = k;
        listNo = lisReqNo;
        *listError = 1;
        if (listDevices > 0) {
            int             maxfecs = MAX_FECS;
            int             maxlists = MAX_LISTS;
            int             repeat = 0;

            indFec++; /* one more fec when devices present, 20-11-2004 */
            pioreq_execute_extended(&maxfecs, &maxlists, devIOFunc, &repeat, &listNo, &listnumber[indFec - 1],
                                    &listDevices, (allDevices *) SendCells, &numlist, infolist, &fecNamesD, &error,
                                    &ident_number, &tffunc, &nlists);

            *listError = error;

            /* fatal list error */

            if (*listError < 1) {
                printf("medmAcquisition device io-error=%d on list=%d\n", *listError, listNo);
                return;
            } else {
                *NumberOfLists = *NumberOfLists + 1;
                lisReqNo++;
            }
        }

        /* are we through ? */

        finished = True;
        for (i = 0; i < *Nbcells; i++) {
            if (!CellTaken[i]) {
                finished = False;
                break;
            }
        }
        indFec = 0;
        total[0] = 0;
    }

}

/*****************************************************************************/
/* here we receive our data from the front ends                              */
/*****************************************************************************/

static void etherReceive(pioStatusB *iosb, pio_ethbuf *irbb)

{
    int             i, stat;
    int             listNumber, devIOF, repTick, numDevs, maxDevices;
    int             error, errort, ident, totaldev, totalfec, tffunc;
    AddProVaxType   AddPro;
    char            mess[70];
    FORSTR messD = {70, 0, 0, mess};
    char            trtnod[7];
    DSC trtnodD  = { 6, 0, 0, trtnod};
    pioDevices      ReceiveCells[MAX_DEVS];
    int             aLen, dLen;
    char            auxAtt[5], auxDev[9];

    maxDevices = MAX_DEVS;
    error = 1;
    pioreq_analyze_extended(&iosb->trans, &irbb->head.bytecount, &maxDevices, &listNumber,
                            &devIOF, &repTick, &numDevs, ReceiveCells, &error, &ident,
                            &totaldev, &totalfec, &tffunc);
    if (error != 1) {
        GET_ETHER_ADDPROT(&AddPro);
        ADCOMP(&trtnodD, &AddPro, &errort);
        printf("caQtDM -- PIOREQ_ANALYZE error=%d list=%d numdevs=%d with TF=%d from <%.6s>\n", error, listNumber, numDevs,
               tffunc, trtnod);
        stat = getPioErrMsgStr(&error, &messD);
        mess[69] = '\0';
        printf("            : %s\n", mess);
        return;
    }

    GET_ETHER_ADDPROT(&AddPro);
    ADCOMP(&trtnodD, &AddPro, &errort);
    trtnod[6] = '\0';

    //printf("caQtDM -- PIOREQ_ANALYZE error=%d list=%d numdevs=%d with TF=%d from <%.6s>\n", error, listNumber, numDevs, tffunc, trtnod);

    /* copy data back to our cells for activ list */

    if (listNumber == MAX_LISTS - 2) {
        if (ReceiveCells[0].unit_code > 3) {
            ActivCells[0].ub.value = (float) Vax2HostF(&ReceiveCells[0].ub.value);
        } else {
            ActivCells[0].ub.int4val = ReceiveCells[0].ub.int4val;
        }
        ActivCells[0].error_code = ReceiveCells[0].error_code;
        ActivCells[0].unit_code = ReceiveCells[0].unit_code;

        /* copy data back to our cells for blockserver data */

    } else if (listNumber >= MAX_LISTS/2) {
        {
            knobData kData, kDataExt;
            int   status;
            float *xbuf, *ybuf;
            char  errorMsg[255];
            int dataSize;
            int totalVal, j;

            // number of devices is always 1 for profiles

            int ident =  ReceiveCells[0].ident;

            C_GetMutexKnobData(KnobDataPtr, ident, &kData);

            if(kData.index == -1) return;

            time_t                time_val;
            struct tm             *times;
            struct timeb          timer;

            kData.edata.connected = true;
            kData.edata.accessW = true;
            kData.edata.accessR = true;
            kData.edata.fieldtype = caFLOAT;
            strcpy(kData.edata.fec, trtnod);
            kData.edata.monitorCount++;

            ftime(&timer);
            time_val = timer.time;
            times = localtime(&time_val);

            status = BlockserverData(&ReceiveCells[0], &totalVal,  &xbuf, &ybuf,  errorMsg);

            if(status) {

                dataSize = totalVal * sizeof(float);

                // we have to fill also the secondary x or y datablock
                if(extent[ident] != -1) {
                    int l =  extent[ident];
                    C_GetMutexKnobData(KnobDataPtr, l, &kDataExt);

                    kDataExt.edata.connected = true;
                    kDataExt.edata.accessW = true;
                    kDataExt.edata.accessR = true;
                    kDataExt.edata.fieldtype = caFLOAT;
                    kDataExt.edata.monitorCount++;

                    if(dataSize != kDataExt.edata.dataSize) {
                        free(kDataExt.edata.dataB);
                        kDataExt.edata.dataB = (void*) malloc(dataSize);
                        kDataExt.edata.dataSize = dataSize;
                    }
                    kDataExt.edata.valueCount = totalVal;
                    float *ptr = (float*) kDataExt.edata.dataB;

                    if(kDataExt.specData[2] == 0) {
                        for(j=0; j < totalVal; j++) {
                            ptr[j] = xbuf[j];
                        }
                    } else {
                        for(j=0; j < totalVal; j++) {
                            ptr[j] = ybuf[j];
                        }
                    }
                }

                // fill datablock for this device

                if(dataSize != kData.edata.dataSize) {
                    free(kData.edata.dataB);
                    kData.edata.dataB = (void*) malloc(dataSize);
                    kData.edata.dataSize = dataSize;
                }
                kData.edata.valueCount = totalVal;
                float *ptr = (float*) kData.edata.dataB;
                if(kData.specData[2] == 0) {
                    for(j=0; j < totalVal; j++) {
                        ptr[j] = xbuf[j];
                    }
                } else {
                    for(j=0; j < totalVal; j++) {
                        ptr[j] = ybuf[j];
                    }
                }

                C_SetMutexKnobDataReceived(KnobDataPtr, &kData);
                if(extent[ident] != -1) C_SetMutexKnobDataReceived(KnobDataPtr, &kDataExt);
                free(xbuf);
                free(ybuf);
            }
        }

        /* copy data back to our cells for read lists */

    } else {
        knobData kData;
        char            unit[9];
        FORSTR          unitD = {8, 0, 0, unit};
        static char     level2[16][21];
        FORSTRARRAY     level2D = {21, 14, 4, 0};
        static char     image2[16][21];
        FORSTRARRAY     image2D = {21, 14, 4, 0};
        int             offset[16];
        char            devS[9], attS[5];
        DSC             devD = {8, 0, 0, devS};
        DSC             attD = {4, 0, 0, attS};
        int             dim = 16;
        int             len = 0;

        for (i = 0; i < numDevs; i++) {
            int dataSize = 0;
            int num = 0;

            if (ReceiveCells[i].error_code == -9) goto skip;   // prof cell coming from pioser taken out

            C_GetMutexKnobData(KnobDataPtr, ReceiveCells[i].ident, &kData);

            if(kData.index != -1) {

                F_CMDT_RCRD  bitmapping[MAX_RECORDS];
                F_SBNT_RCRD result [16];
                kData.edata.monitorCount++;
                kData.edata.connected = true;

                kData.edata.accessW = true;
                kData.edata.accessR = true;
                kData.edata.valueCount = 1;
                strcpy(kData.edata.fec, trtnod);

                memcpy(devS, ReceiveCells[i].name, 8);
                memcpy(attS, ReceiveCells[i].att, 4);
                memset(bitmapping, 0, sizeof(bitmapping));

                attS[4] = '\0';
                devS[8] = '\0';
                aLen = real_len(attS);
                dLen = real_len(devS);
                strcpy(auxAtt, attS);
                strcpy(auxDev, devS);
                auxAtt[aLen] = '\0';
                auxDev[dLen] = '\0';

                if( (strstr(kData.pv, auxAtt) == (char*) 0) ||
                        (strstr(kData.pv, auxDev) == (char*) 0)) {
                    //printf("unexpected device %d %s <%.8s> <%.4s>\n", kData.index, kData.pv, auxDev, auxAtt);
                    goto skip;
                }

                // in case of commands we do not have to consider an error code
                if(strncmp(attS, "C", 1) == 0) {
                    if(kData.edata.dataSize > 0) {
                        kData.edata.monitorCount--;
                        goto skip1;  // cmdnames should not change
                    }
                    num = getCmdNames(&devD, &attD, &dim, bitmapping);
                    if(num > 0) {
                        int j;
                        dataSize = 8 * sizeof(char) * num + (num) * sizeof(char);
                        char aux[9];
                        //printf("%.8s %.4s %d\n", devS, attS, num);

                        kData.edata.fieldtype = caENUM;
                        if(dataSize != kData.edata.dataSize) {
                            free(kData.edata.dataB);
                            kData.edata.dataB = (void*) malloc(dataSize);
                            kData.edata.dataSize = dataSize;
                        }

                        char *ptr = (char*) kData.edata.dataB;
                        ptr[0] = '\0';
                        strncpy(aux, bitmapping[0].bitName, 8);
                        aux[8] = '\0';
                        len = Min(8, real_len(aux));
                        aux[len] = '\0';
                        strcpy(ptr, aux);
                        for (j = 1; j < num; j++) {
                            strncpy(aux, bitmapping[j].bitName, 8);
                            aux[8] = '\0';
                            len = Min(8, real_len(aux));
                            aux[len] = '\0';
                            sprintf(ptr, "%s;%s", ptr, aux);
                        }
                        kData.edata.valueCount=num;
                        // in case of hipa, commands should have no value
                        kData.edata.ivalue = num+1;
                    }

                    // everything except command attribute

                } else {

                    if (ReceiveCells[i].error_code == 1) {

                        kData.edata.severity = 0;

                        // floats
                        if (ReceiveCells[i].unit_code > 3) {

                            float value = (float) Vax2HostF(&ReceiveCells[i].ub.value);

                            // new value ?
                            //if((fabs(kData.edata.rvalue - value) <= 1.e-7)  && (kData.edata.monitorCount > 1)) {
                            //    kData.edata.monitorCount--;
                            //    goto skip1;
                            //}
                            kData.edata.rvalue = (float) Vax2HostF(&ReceiveCells[i].ub.value);
                            kData.edata.fieldtype = caFLOAT;
                            kData.edata.precision = 3;

                            // integers (bitfields)
                        } else {
                            int status;
                            char level2Data[21];
                            level2Data[0] = '\0';

                            int value = ReceiveCells[i].ub.int4val;
                            kData.edata.fieldtype = caINT;  // default type

                            // new value ?
                            //if(strstr(kData.pv, "VHD0") != (char*) 0) printf("pv=<%s> old=%x new=%x mC=%d dC=%d\n",kData.pv, kData.edata.ivalue, value , kData.edata.monitorCount, kData.edata.displayCount);

                            //if(kData.edata.ivalue == value && kData.edata.monitorCount > 1 && kData.edata.displayCount != 0) {
                            //    kData.edata.monitorCount--;
                                //if(strstr(kData.pv, "VHD0") != (char*) 0) printf("skipped\n");
                            //    goto skip1;
                            //}

                            //if(strstr(kData.pv, "VHD0") != (char*) 0)printf("not skipped\n");

                            kData.edata.ivalue = ReceiveCells[i].ub.int4val;
                            kData.edata.rvalue = (float) ReceiveCells[i].ub.int4val;

                            level2D.strPtr = (char *) level2;
                            image2D.strPtr = (char *) image2;

                            // in case of ca_choice and cabitnames get also bitnames
                            if((strstr(kData.clasName, "choice") != (char *) 0) ||
                                    (strstr(kData.clasName, "bitnames") != (char *) 0)) {
                                if(kData.edata.dataSize > 0) {
                                     if(kData.edata.displayCount > 0)  goto skip1;  // bitnames should not change, but value yes
                                }
                                memset(result, 0, sizeof(result));
                                status = getBitNames(&devD, &attD, result);
                                if(status == 1) {
                                    int j;
                                    num = 0;
                                    for(j=0; j < 16; j++) {
                                        strncpy(level2[j], result[j].bitCmt, 20);
                                        level2[j][20] = '\0';
                                        num++;
                                    }
                                    dataSize = 21 * sizeof(char) * num + (num) * sizeof(char);
                                    // in case of hipa, bitnames should have no value
                                    kData.edata.ivalue = num+1;
                                }

                            } else if((strstr(kData.clasName, "lineedit") != (char *) 0) || (strstr(kData.clasName, "image") != (char *) 0)) {
                                // get digital level 2
                                int j;
                                num = getDigLvl2ext(&devD, &attD, &ReceiveCells[i].ub.int4val, &dim,  &level2D, &image2D, offset);
                                dataSize = 21 * sizeof(char) * num + (num) * sizeof(char);
                                for(j = 0; j < num; j++) {
                                    level2[j][20] = '\0';
                                    // if we have an image, compute the correct frame and keep also the string
                                    if(strlen(kData.edata.aux) > 0) {
                                        char levData[21], aux[21];
                                        strcpy(levData, level2[j]);
                                        strcpy(aux, kData.edata.aux); strcat(aux, "_");
                                        if(IsInside(strUpr(levData), aux)) {
                                            kData.edata.rvalue = offset[j];
                                            strcpy(level2Data, level2[j]);
                                            dataSize = 21 * sizeof(char);
                                        }
                                    }
                                }
                            }

                            if(num > 0) {
                                if((strstr(kData.clasName, "choice") != (char *) 0) || (strstr(kData.clasName, "bitnames") != (char *) 0)) {
                                    kData.edata.fieldtype = caENUM;
                                } else {
                                    kData.edata.fieldtype = caSTRING;
                                }
                                if(dataSize != kData.edata.dataSize) {
                                    free(kData.edata.dataB);
                                    kData.edata.dataB = (void*) malloc(dataSize);
                                    kData.edata.dataSize = dataSize;
                                }

                                char *ptr = (char*) kData.edata.dataB;

                                if(strlen(kData.edata.aux) > 0) {
                                    ptr[0] = '\0';
                                    strcpy(ptr, level2Data);
                                    kData.edata.valueCount = 1;
                                    kData.edata.enumCount=0;

                                } else {
                                    int j;
                                    char levData[21];
                                    ptr[0] = '\0';
                                    strcpy(ptr, level2[0]);
                                    for (j = 1; j < num; j++) {
                                        strncpy(levData, level2[j], 20);
                                        levData[20] = '\0';
                                        len = Min(20, real_len(levData));
                                        levData[len] = '\0';
                                        sprintf(ptr, "%s;%s", ptr, levData);
                                    }
                                    kData.edata.valueCount=num;
                                    kData.edata.enumCount=num;
                                }
                            }

                        }
                        fillUnit(&ReceiveCells[i].unit_code, &unitD);
                        unit[8] = '\0';
                        unit[real_len(unit)] = '\0';
                        strcpy(kData.edata.units, unit);
                    } else {
                        kData.edata.severity = ReceiveCells[i].error_code;
                        if (ReceiveCells[i].unit_code > 3) {
                            kData.edata.rvalue = 0.0;
                            kData.edata.fieldtype = caFLOAT;
                            kData.edata.precision = 3;
                        }
                    }
                }
skip1:;
                C_SetMutexKnobDataReceived(KnobDataPtr, &kData);
skip:;
            }
        }
    }
}


