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

#ifdef _MSC_VER
    #include <windows.h>
  #if (_MSC_VER > 1700)
    #include <stdbool.h>
  #endif
#else
    #include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#define epicsAlarmGLOBAL
#include <cadef.h>
struct ca_client_context *dbCaClientContext;

#include <alarm.h>
#include <epicsExport.h>
#include "tsDefs.h"

#undef epicsAlarmGLOBAL

#define CA_PRIORITY 50          /* CA priority */
#define CA_TIMEOUT   2          /* CA timeout 2.0 seconds */
#define Min(x,y)   (((x) < (y)) ? (x) : (y))

#include "knobDefines.h"

#include "knobData.h"
#include "mutexKnobDataWrapper.h"
#include "messageWindowWrapper.h"
#include "vaPrintf.h"

#ifdef ACS
#include "acsSubs.h"
#endif

extern MutexKnobData* KnobDataPtr;

typedef struct _connectInfo {
    int cs;          // 0= epics, 1 =acs
    int connected;
    int index;
    int event;
    pv_string pv;    // channel name
    chid ch;         // read channel
    evid evID;       // epics event id
    int  evAdded;    // epics event added yes/no
} connectInfo;

char* myLimitedString (char * strng) {
    static char aux[128] = {0};
    int i, len = -1;
    for(i=0; i< (int) sizeof(dbr_string_t); i++) {
        aux[i] = strng[i];
        if(strng[i] == '\0') {
            len = i;
            break;
        }
    }
    if(len == -1) {
        aux[sizeof(dbr_string_t)-1] = '\0';
        //printf("no null character found in string <%s>\n", aux);
    }
    return aux;
}

// due to gateway, protect copy of units
#define AssignEpicsData {\
    strcpy(kData.edata.units, myLimitedString(stsF->units)); \
    kData.edata.lower_disp_limit = (double) stsF->lower_disp_limit; \
    kData.edata.upper_disp_limit = (double) stsF->upper_disp_limit; \
    kData.edata.lower_alarm_limit = (double) stsF->lower_alarm_limit; \
    kData.edata.upper_alarm_limit = (double) stsF->upper_alarm_limit; \
    kData.edata.lower_ctrl_limit = (double) stsF->lower_ctrl_limit;   \
    kData.edata.upper_ctrl_limit = (double) stsF->upper_ctrl_limit;  \
    kData.edata.lower_warning_limit = (double) stsF->lower_warning_limit; \
    kData.edata.upper_warning_limit = (double) stsF->upper_warning_limit; }

#define AssignEpicsValue(valx, vali, countx) { \
    kData.edata.actTime = now; \
    kData.edata.rvalue = valx; \
    kData.edata.ivalue = vali; \
    kData.edata.severity = stsF->severity; \
    kData.edata.status = stsF->status; \
    kData.edata.accessW = ca_write_access(args.chid); \
    kData.edata.accessR = ca_read_access(args.chid); \
    kData.edata.valueCount = countx; \
    strcpy(kData.edata.fec, myLimitedString((char*) ca_host_name(args.chid))); \
    kData.edata.monitorCount = info->event; }

/**
 * general print routine with timestamp
 */

void Exceptionhandler(struct exception_handler_args args)
{
    char *pName;
    if(args.chid) {
        pName = (char *) ca_name(args.chid);
    } else {
        pName = "?";
    }
    C_postMsgEvent(messageWindow, 1, vaPrintf("Channel Access Exception %s on %s (op=%ld data_type=%s count=%ld)\n",
                                            args.ctx, pName, args.op, dbr_type_to_text(args.type), args.count));
}

/**
 * define what we need for network IO
 */
void PrepareDeviceIO(void)
{
    //PRINT(printf("create epics context\n"));
    int status = ca_context_create(ca_enable_preemptive_callback);
    if (status != ECA_NORMAL) {
        printf("ca_context_create:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]);
        exit(1);
    }
    dbCaClientContext = ca_current_context();

    ca_add_exception_event(Exceptionhandler, 0);
}

static void access_rights_handler(struct access_rights_handler_args args)
{
    knobData kData;

    connectInfo *info = (connectInfo *) ca_puser(args.chid);
    C_GetMutexKnobData(KnobDataPtr, info->index, &kData);
    kData.edata.accessW = ca_write_access(args.chid);
    kData.edata.accessR = ca_read_access(args.chid);
    C_SetMutexKnobDataReceived(KnobDataPtr, &kData);

    return;
}

/**
 * initiate data acquisition
 */

static void dataCallback(struct event_handler_args args)
{
    knobData kData;
    struct timeb now;

    connectInfo *info = (connectInfo *) ca_puser(args.chid);
    if(info == (connectInfo *) 0) return;

    C_GetMutexKnobData(KnobDataPtr, info->index, &kData);
    if(kData.index == -1) return;

    if (args.status != ECA_NORMAL) {
        PRINT(printf("dataCallback:  get: %s for %s\n", ca_name(args.chid), ca_message_text[CA_EXTRACT_MSG_NO(args.status)]));
    } else {
        kData.edata.monitorCount = info->event;
        kData.edata.connected = info->connected;
        kData.edata.fieldtype = ca_field_type(args.chid);
        ftime(&now);

        C_DataLock(KnobDataPtr, &kData);

        switch (ca_field_type(args.chid)) {

        case DBF_CHAR:
        {
            int dataSize;
            char* ptr;
            struct dbr_sts_char *stsF = (struct dbr_sts_char *) args.dbr;
            dbr_char_t *val_ptr = dbr_value_ptr(args.dbr, DBR_STS_CHAR);

            PRINT(printf("dataCallback char %s %d %d <%s> status=%d count=%d nBytes=%d\n", ca_name(args.chid), (int) args.chid,
                         info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, dbr_size_n(args.type, args.count)));

            dataSize = dbr_size_n(args.type, args.count) + sizeof(char);
            if(dataSize != kData.edata.dataSize) {
                free(kData.edata.dataB);
                kData.edata.dataB = (void*) malloc(dataSize);
                kData.edata.dataSize = dataSize;
            }

            ptr = (char*) kData.edata.dataB;
            memcpy(ptr, val_ptr, args.count *sizeof(char));
            ptr[args.count] = '\0';

            kData.edata.valueCount =  args.count;

            C_SetMutexKnobDataReceived(KnobDataPtr, &kData);
        }
        break;

        case DBF_STRING:
        {
            int dataSize, len;
            int i;
            char *ptr;
            struct dbr_sts_string *stsF = (struct dbr_sts_string *) args.dbr;
            dbr_string_t *val_ptr = dbr_value_ptr(args.dbr, DBR_STS_STRING);

            PRINT(printf("dataCallback string %s %d <%s> %d <%s> status=%d count=%d nBytes=%d\n", ca_name(args.chid), (int) args.chid,
                         stsF->value, info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, dbr_size_n(args.type, args.count)));


            // concatenate strings separated with ';'
            dataSize = dbr_size_n(args.type, args.count) + (args.count+1) * sizeof(char);
            if(dataSize != kData.edata.dataSize) {
                free(kData.edata.dataB);
                kData.edata.dataB = (void*) malloc(dataSize);
                kData.edata.dataSize = dataSize;
            }

            ptr = (char*) kData.edata.dataB;
            ptr[0] = '\0';
            len = 0;
            strcpy(ptr, myLimitedString(val_ptr[0]));
            for (i = 1; i < args.count; i++) {
                len = len+strlen(myLimitedString(val_ptr[i-1]));
                strcat(&ptr[len++], ";");
                strcat(&ptr[len], myLimitedString(val_ptr[i]));
            }

            C_SetMutexKnobDataReceived(KnobDataPtr, &kData);
        }
        break;

        case DBF_ENUM:
        {
            struct dbr_sts_enum *stsF = (struct dbr_sts_enum *) args.dbr;
            PRINT(printf("dataCallback enum  %s %d <%d> %d <%s> status=%d count=%d size=%d\n", ca_name(args.chid), (int) args.chid,
                         stsF->value, info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, dbr_size_n(args.type, args.count)));

            AssignEpicsValue((double) stsF->value, (long) stsF->value, args.count);

            C_SetMutexKnobDataReceived(KnobDataPtr, &kData);
        }
        break;

        case DBF_INT:
        {
            struct dbr_sts_int *stsF = (struct dbr_sts_int *) args.dbr;

            PRINT(printf("dataCallback int values %s %d %d %d <%s> status=%d count=%d size=%d\n", ca_name(args.chid), (int) args.chid,
                         stsF->value, info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, dbr_size_n(args.type, args.count)));


            AssignEpicsValue((double) stsF->value, (long) stsF->value, args.count);

            if(args.count > 1) {
                if((int) (args.count * sizeof(int16_t)) != kData.edata.dataSize) {
                    free(kData.edata.dataB);
                    kData.edata.dataB = (void*) malloc(args.count * sizeof(int16_t));
                    kData.edata.dataSize = args.count * sizeof(int16_t);
                }
                memcpy(kData.edata.dataB, &stsF->value, args.count * sizeof(int16_t));
            }

            C_SetMutexKnobDataReceived(KnobDataPtr, &kData);
        }
        break;

        case DBF_LONG:
        {
            struct dbr_sts_long *stsF = (struct dbr_sts_long *) args.dbr;

            PRINT(printf("dataCallback long values %s %d %lx %d <%s> status=%d count=%d size=%d\n", ca_name(args.chid), (int) args.chid,
                         stsF->value, info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, dbr_size_n(args.type, args.count)));


            AssignEpicsValue((double) stsF->value, (long) stsF->value, args.count);


            if(args.count > 1) {
                if((int) (args.count * sizeof(int32_t)) != kData.edata.dataSize) {
                    free(kData.edata.dataB);
                    kData.edata.dataB = (void*) malloc(args.count * sizeof(int32_t));
                    kData.edata.dataSize = args.count * sizeof(int32_t);
                }
                memcpy(kData.edata.dataB, &stsF->value, args.count * sizeof(int32_t));
            }

            C_SetMutexKnobDataReceived(KnobDataPtr, &kData);
        }
        break;

        case DBF_FLOAT:
        {
            struct dbr_sts_float *stsF = (struct dbr_sts_float *) args.dbr;

            PRINT(printf("dataCallback float values %s %d %f %d <%s> status=%d count=%d size=%d\n", ca_name(args.chid), (int) args.chid,
                         stsF->value, info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, dbr_size_n(args.type, args.count)));

            AssignEpicsValue((double) stsF->value, (long) stsF->value, args.count);

            if(args.count > 1) {
                if((int) (args.count * sizeof(float)) != kData.edata.dataSize) {
                    free(kData.edata.dataB);
                    kData.edata.dataB = (void*) malloc(args.count * sizeof(float));
                    kData.edata.dataSize = args.count * sizeof(float);
                }
                memcpy(kData.edata.dataB, &stsF->value, args.count * sizeof(float));
            }
            C_SetMutexKnobDataReceived(KnobDataPtr, &kData);
        }
        break;

        case DBF_DOUBLE:
        {
            struct dbr_sts_double *stsF = (struct dbr_sts_double *) args.dbr;

            PRINT(printf("dataCallback double values %s %d %f %d <%s> status=%d count=%d size=%d\n", ca_name(args.chid), (int) args.chid,
                         stsF->value, info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, dbr_size_n(args.type, args.count)));


            AssignEpicsValue((double) stsF->value, (long) stsF->value, args.count);

            if(args.count > 1) {
                if((int) (args.count * sizeof(double)) != kData.edata.dataSize) {
                    free(kData.edata.dataB);
                    kData.edata.dataB = (void*) malloc(args.count * sizeof(double));
                    memcpy(kData.edata.dataB, &stsF->value, args.count * sizeof(double));
                    kData.edata.dataSize = args.count * sizeof(double);
                }
                memcpy(kData.edata.dataB, &stsF->value, args.count * sizeof(double));
            }
            C_SetMutexKnobDataReceived(KnobDataPtr, &kData);

        }
        break;

        } // end switch

        C_DataUnlock(KnobDataPtr, &kData);
        info->event++;
    }
}

static void displayCallback(struct event_handler_args args) {
    knobData kData;
    struct timeb now;
    int status;

    connectInfo *info = (connectInfo *) ca_puser(args.chid);
    if(info == (connectInfo *) 0) return;

    C_GetMutexKnobData(KnobDataPtr, info->index, &kData);
    if(kData.index == -1) return;

    if (args.status != ECA_NORMAL) {
        PRINT(printf("displayCallback:\n""  get: %s for %s\n", ca_name(args.chid), ca_message_text[CA_EXTRACT_MSG_NO(args.status)]));
    } else {
        kData.edata.initialize = true;
        kData.edata.monitorCount = kData.edata.displayCount = info->event;
        kData.edata.connected = info->connected;
        kData.edata.fieldtype = ca_field_type(args.chid);
        ftime(&now);

        C_DataLock(KnobDataPtr, &kData);

        switch (ca_field_type(args.chid)) {

        case DBF_CHAR:
        {
            struct dbr_ctrl_char *stsF = (struct dbr_ctrl_char *) args.dbr;

            PRINT(printf("displayCallback char %s %d %d <%s> status=%d count=%d nBytes=%d\n", ca_name(args.chid), (int) args.chid,
                         info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, dbr_size_n(args.type, args.count)));

            AssignEpicsValue((double) 0.0, (long) 0, args.count);
            kData.edata.precision = 0;
            kData.edata.units[0] = '\0';
        }
        break;

        case DBF_STRING:
        {
            struct dbr_sts_string *stsF = (struct dbr_sts_string *) args.dbr;

            PRINT(printf("displayCallback string %s %d <%s> %d <%s> status=%d count=%d nBytes=%d\n", ca_name(args.chid), (int) args.chid,
                         stsF->value, info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, dbr_size_n(args.type, args.count)));

            AssignEpicsValue((double) 0.0, (long) 0, args.count);
            kData.edata.precision = 0;
            kData.edata.units[0] = '\0';
        }
        break;

        case DBF_ENUM:
        {
            int dataSize, len;
            int i;
            char *ptr;
            struct dbr_ctrl_enum *stsF = (struct dbr_ctrl_enum *) args.dbr;
            PRINT(printf("displayCallback enum  %s %d <%d> %d <%s> status=%d count=%d enum no_str=%d size=%d\n", ca_name(args.chid), (int) args.chid,
                         stsF->value, info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, stsF->no_str, dbr_size_n(args.type, args.count)));

            AssignEpicsValue((double) stsF->value, (long) stsF->value, args.count);
            kData.edata.precision = 0;
            kData.edata.units[0] = '\0';
            kData.edata.enumCount = stsF->no_str;

            if(stsF->no_str>0) {
                // concatenate strings separated with ';'
                dataSize = dbr_size_n(args.type, args.count) + stsF->no_str * sizeof(char);
                if(dataSize != kData.edata.dataSize) {
                    free(kData.edata.dataB);
                    kData.edata.dataB = (void*) malloc(dataSize);
                    kData.edata.dataSize = dataSize;
                }

                ptr = (char*) kData.edata.dataB;
                ptr[0] = '\0';
                len = 0;
                strcpy(ptr, myLimitedString(stsF->strs[0]));
                for (i = 1; i < stsF->no_str; i++) {
                    len = len+strlen(myLimitedString(stsF->strs[i-1]));
                    strcat(&ptr[len++], ";");
                    strcat(&ptr[len], myLimitedString(stsF->strs[i]));
                }

            } else if(args.count == 1) {  // no strings, must be a value, convert it to text
                // concatenate strings separated with ';'
                dataSize = 40;
                if(dataSize != kData.edata.dataSize) {
                    free(kData.edata.dataB);
                    kData.edata.dataB = (void*) malloc(dataSize);
                    kData.edata.dataSize = dataSize;
                }
                ptr = (char*) kData.edata.dataB;
                ptr[0] = '\0';
                sprintf(ptr, "%d", stsF->value);
            }
        }
        break;

        case DBF_INT:
        {
            struct dbr_ctrl_int *stsF = (struct dbr_ctrl_int *) args.dbr;

            PRINT(printf("displayCallback int values %s %d %d <%s> %d <%s> status=%d count=%d size=%d\n", ca_name(args.chid), (int) args.chid,
                         stsF->value, stsF->units, info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, dbr_size_n(args.type, args.count)));

            AssignEpicsData;
            AssignEpicsValue((double) stsF->value, (long) stsF->value, args.count);
            kData.edata.precision = 0;
        }
        break;

        case DBF_LONG:
        {
            struct dbr_ctrl_long *stsF = (struct dbr_ctrl_long *) args.dbr;

            PRINT(printf("displayCallback long values %s %d %lx <%s> %d <%s> status=%d count=%d size=%d\n", ca_name(args.chid), (int) args.chid,
                         stsF->value, stsF->units, info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, dbr_size_n(args.type, args.count)));

            AssignEpicsData;
            AssignEpicsValue((double) stsF->value, (long) stsF->value, args.count);
            kData.edata.precision = 0;
        }
        break;

        case DBF_FLOAT:
        {
            struct dbr_ctrl_float *stsF = (struct dbr_ctrl_float *) args.dbr;

            PRINT(printf("displayCallback float values %s %d %f <%s> %d <%s> status=%d count=%d size=%d\n", ca_name(args.chid), (int) args.chid,
                         stsF->value, stsF->units, info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, dbr_size_n(args.type, args.count)));

            AssignEpicsData;
            AssignEpicsValue((double) stsF->value, (long) stsF->value, args.count);
            kData.edata.precision = stsF->precision;
        }
        break;

        case DBF_DOUBLE:
        {
            struct dbr_ctrl_double *stsF = (struct dbr_ctrl_double *) args.dbr;

            PRINT(printf("displayCallback double values %s %d %f <%s> %d <%s> status=%d count=%d size=%d\n", ca_name(args.chid), (int) args.chid,
                         stsF->value, stsF->units, info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, dbr_size_n(args.type, args.count)));

            AssignEpicsData;
            AssignEpicsValue((double) stsF->value, (long) stsF->value, args.count);
            kData.edata.precision = stsF->precision;
        }
        break;

        } // end switch

        // when specifying zero as number of requested elements, we will get variable length arrays (zero lenght is then also considered)
        // probably will not work with older channel access gateways
        status = ca_add_array_event(dbf_type_to_DBR_STS(ca_field_type(args.chid)), 0, //ca_element_count(args.chid),
                                     args.chid, dataCallback, info, 0.0,0.0,0.0, &info->evID);
        info->evAdded = true;
        PRINT(printf("ca_add_array_event added for %s with chid=%d index=%d info->evAdded=%d\n", ca_name(args.chid), args.chid, kData.index, info->evAdded));
        if (status != ECA_NORMAL) {
            PRINT(printf("ca_add_array_event:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]));
        }

        C_SetMutexKnobData(KnobDataPtr, kData.index, kData);

        C_DataUnlock(KnobDataPtr, &kData);
        info->event++;
    }
}

/**
 * routine used to suspend the monitor when requested
 */
void clearEvent(void * ptr)
{
    int status;
    connectInfo *info = (connectInfo *) ptr;
    if(info == (connectInfo *) 0) return;
    if(!info->connected) return;  // must be connected
    if(info->event < 2) return;  // a first normal addevent must be done
    if(info->evAdded) {
      PRINT(printf("clearEvent -- %s %d %d %d %d\n", info->pv, info->evID, info->index, info->connected, info->evAdded));
      info->evAdded = false;
      status = ca_clear_event(info->evID);
      if (status != ECA_NORMAL) {
          PRINT(printf("ca_clear_event:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]));
      }
    }
}

/**
 * routine used to add the monitor again when requested
 */
void addEvent(void * ptr)
{
    connectInfo *info = (connectInfo *) ptr;
    if(info == (connectInfo *) 0) return;
    if(!info->connected) return; // must be connected
    if(info->event < 2) return;  // a first normal addevent must be done
    if(!info->evAdded) {

        knobData kData;
        int status;
        C_GetMutexKnobData(KnobDataPtr, info->index, &kData);
        if(kData.index == -1) return;

        C_DataLock(KnobDataPtr, &kData);
        PRINT(printf("addEvent -- %s %d %d %d %d\n", info->pv, info->evID, info->index, info->connected, info->evAdded));
        status = ca_add_array_event(dbf_type_to_DBR_STS(ca_field_type(info->ch)), 0,
                                        info->ch, dataCallback, info, 0.0,0.0,0.0, &info->evID);
        info->evAdded = true;

        if (status != ECA_NORMAL) {
            PRINT(printf("ca_add_array_event:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]));
        }
        C_SetMutexKnobData(KnobDataPtr, kData.index, kData);

        C_DataUnlock(KnobDataPtr, &kData);
    }
}


/**
 * epics connect callback
 */
void connectCallback(struct connection_handler_args args)
{
    int status;

    connectInfo *info = (connectInfo *) ca_puser(args.chid);

    //printf("connectInfo %p pv=<%s> %d chid=%d\n", info, info->pv, info->evAdded, args.chid);

    switch (ca_state(args.chid)) {

    case cs_never_conn:
        PRINT(printf("%s was never connected\n", ca_name(args.chid)));
        info->connected = false;
        break;
    case cs_prev_conn:
        PRINT(printf("%s with channel %d has just disconnected, evid=%d\n", ca_name(args.chid), args.chid, info->evID));
        if(info->evAdded) {
            status = ca_clear_event(info->evID);
            if (status != ECA_NORMAL) {
               PRINT(printf("ca_clear_event:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]));
            }
        }
        info->connected = false;
        info->event = 0;
        info->evAdded = false;
        break;
    case cs_conn:
        PRINT(printf("%s has just connected with channel id=%d count=%d native type=%s\n", ca_name(args.chid), (int) args.chid, ca_element_count(args.chid), dbf_type_to_text(ca_field_type(args.chid))));
        info->connected = true;
        info->evAdded = false;
        if (info->event == 0) {
            info->event++;
            status = ca_array_get_callback(dbf_type_to_DBR_CTRL(ca_field_type(args.chid)), 1, args.chid, displayCallback, NULL);
            if (status != ECA_NORMAL) {
                PRINT(printf("ca_array_get_callback:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]));
            }

            /* install access rights monitor */
            status = ca_replace_access_rights_event(args.chid, access_rights_handler);
            if (status != ECA_NORMAL) {
                PRINT(printf("ca_replace_access_rights_event:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]));
            }
        }

        break;
    case cs_closed:
        info->connected = false;
        PRINT(printf("connectCallback invalid channel\n"));
        break;

    default:
        PRINT(printf("connectCallback ???????????\n"));
        break;
    }

    // update knobdata connection
    C_SetMutexKnobDataConnected(KnobDataPtr, info->index, info->connected);
}

/**
 * define device io and hook info to gui
 */
int CreateAndConnect(int index, knobData *kData, int rate, int skip)
{
    int status;
    connectInfo *info = (connectInfo *) 0;
    struct timeb now;
    ftime(&now);
#ifdef ACS
    char aux[10];
    static int first = true;
#endif

    /* initialize channels */
    PRINT(printf("create channel index=%d <%s> rate=%d\n", index, kData->pv, rate));

    kData->index = index;
    kData->edata.monitorCount = 0;
    kData->edata.displayCount = 0;
    kData->edata.precision = 0; //default
    kData->edata.units[0] = '\0';
    kData->edata.info = (connectInfo *) malloc(sizeof (connectInfo));
    kData->edata.dataB =(void*) 0;
    kData->edata.dataSize = 0;
    kData->edata.initialize = true;
    kData->edata.lastTime = now;
    kData->edata.repRate = rate;   // default 5 Hz

    info = (connectInfo *) kData->edata.info;

    strcpy(info->pv, kData->pv);
    info->connected = false;
    info->index = index;
    info->event = 0;
    info->evAdded = false;
    info->ch = 0;
    info->cs = 0;

    // update knobdata
    C_SetMutexKnobData(KnobDataPtr, index, *kData);

    // in case of a soft channel there is nothing to do
    if(kData->soft) return index;

    // otherwise define data acquisition
#ifdef ACS
    if(first) {
        StartPIOserver();
        first = false;
    }
    kData->edata.severity = -4;
    if(AddValueCell(kData->pv, index, aux) != -1) {
        strcpy(kData->edata.aux, aux);

        //printf("we added an acs device <%s> %d %d aux=<%s>\n", kData->pv,   kData->index, index, aux);
        info->cs = 1;                 // acs controlsystem
        // update knobdata
        C_SetMutexKnobData(KnobDataPtr, index, *kData);
        return index;
    }
#endif

    // epics4
    if(skip) {
        info->cs = 2;
        return index;
    }

    //printf("we have to add an epics device <%s>\n", kData->pv);
    status = ca_attach_context(dbCaClientContext);
    status = ca_create_channel(kData->pv,
                               (void(*)())connectCallback,
                               info,
                               CA_PRIORITY_DEFAULT,
                               &info->ch);
    if(status != ECA_NORMAL) {
        printf("ca_create_channel:\n"" %s for %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)], kData->pv);
    }

    status = ca_pend_io(CA_TIMEOUT);
    if(status != ECA_NORMAL) {
        printf("ca_pend_io:\n"" %s for %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)], kData->pv);
    }

    //printf("channel created for button=%d <%s> info=%p, chid=%d\n", index, kData->pv, info, info->ch);

    return index;
}

void EpicsReconnect(knobData *kData)
{
    int status;
    connectInfo *info;
    // in case of a soft channel there is nothing to do
    if(kData->soft) return;

    info = (connectInfo *) kData->edata.info;

    PRINT(printf("create channel for an epics device <%s>\n", kData->pv));

    if (info != (connectInfo *) 0) {
        if(info->cs == 0) { // epics
            status = ca_attach_context(dbCaClientContext);
            status = ca_create_channel(kData->pv,
                                       (void(*)())connectCallback,
                                       info,
                                       CA_PRIORITY_DEFAULT,
                                       &info->ch);
            if(status != ECA_NORMAL) {
                printf("ca_create_channel:\n"" %s for %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)], kData->pv);
            }
        }
    }
}

void EpicsDisconnect(knobData *kData)
{
    int status;
    connectInfo *info;

    status = ca_attach_context(dbCaClientContext);

    if (kData->index == -1) return;

    printf("clear channel %s index=%d\n", kData->pv, kData->index);
    info = (connectInfo *) kData->edata.info;
    if (info != (connectInfo *) 0) {
        if(info->cs == 0) { // epics
            if(info->ch != (chid) 0) {
                printf("event added=%d\n", info->evAdded);
                if(info->evAdded) {
                    info->evAdded = false;
                    printf("ca_clear_event: %s index=%d\n", info->pv, kData->index);
                    status = ca_clear_event(info->evID);
                    if (status != ECA_NORMAL) {
                        PRINT(printf("ca_clear_event:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]));
                    }
                }
                if(info->connected) {
                printf("ca_clear_channel: %s chid=%d, index=%d\n", info->pv, info->ch, kData->index);
                status = ca_clear_channel(info->ch);
                info->connected = false;
                info->event = 0;
                info->ch = 0;
                if(status != ECA_NORMAL) {
                    printf("ca_clear_channel: %s %s index=%d\n", ca_message_text[CA_EXTRACT_MSG_NO(status)], info->pv, kData->index);
                }
                }
            }
        }
    }
}


/**
 * unhook the device that has been hooked here on the gui
 */
void ClearMonitor(knobData *kData)
{
    int status, aux;
    connectInfo *info;

    status = ca_attach_context(dbCaClientContext);

    if (kData->index == -1) return;

    PRINT(printf("ClearMonitor -- clear channel %s index=%d\n", kData->pv, kData->index));

    aux =  kData->index;
    kData->index = -1;
    kData->pv[0] = '\0';

    info = (connectInfo *) kData->edata.info;
    if (info != (connectInfo *) 0) {
        if(info->cs == 0) { // epics
            if(info->ch != (chid) 0) {
                if(info->evAdded) {
                    info->evAdded = false;
                    PRINT(printf("ca_clear_event: %s index=%d\n", info->pv, aux));
                    status = ca_clear_event(info->evID);
                    if (status != ECA_NORMAL) {
                        PRINT(printf("ca_clear_event:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]));
                    }
                }
                status = ca_clear_channel(info->ch);
                PRINT(printf("ca_clear_channel: %s chid=%d, index=%d\n", info->pv, info->ch, aux));
                info->connected = false;
                info->event = 0;
                info->ch = 0;
                if(status != ECA_NORMAL) {
                    printf("ca_clear_channel: %s %s index=%d\n", ca_message_text[CA_EXTRACT_MSG_NO(status)], info->pv, aux);
                }
                info->pv[0] = '\0';

            }
        } else if(info->cs == 2) { // epics4

        } else {
#ifdef ACS
            //printf("delete acs channel %s index=%d\n", info->pv, aux);
            RemoveValueCell(aux);
#endif
        }
    }

    status = ca_pend_io(CA_TIMEOUT);
}

void DestroyContext()
{
    int status = ca_pend_io(CA_TIMEOUT);
    printf("destroycontext\n");
    ca_context_destroy();
}

int EpicsSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType)
{
    chid     ch;
    chtype   chType;
    int status;
    struct dbr_ctrl_double ctrlR;
    struct dbr_sts_string ctrlS;

    UNUSED(errmess);
    UNUSED(object);

    if(strlen(pv) < 1)  {
            C_postMsgEvent(messageWindow, 1, vaPrintf("pv with length=0 (not translated for macro?)\n"));
            return !ECA_NORMAL;
    }

#ifdef ACS
    if(SetActivCell(pv) != -1) {
        PRINT(printf("we have to set a value to an acs device <%s> %f %ld <%s> object=%s\n", pv, rdata, idata, sdata, object));
        if(strstr(object, "numeric") != (char*) 0) {
            SetDeviceFloatValue(pv, &rdata);
        } else if(strstr(object, "slider") != (char*) 0) {
            SetDeviceFloatValue(pv, &rdata);
        } else if(strstr(object, "choice") != (char*) 0) {
            SetDeviceStringValue(pv, sdata);
        } else if(strstr(object, "menu") != (char*) 0) {
            SetDeviceStringValue(pv, sdata);
        } else if(strstr(object, "message") != (char*) 0) {
            SetDeviceStringValue(pv, sdata);
        } else if(strstr(object, "textEntry") != (char*) 0) {
            SetDeviceStringValue(pv, sdata);
        }
        return 0;
    }
#endif

    // set epics value
    PRINT(printf(" we have to set a value to an epics device <%s> %f %ld <%s>\n", pv, rdata, idata, sdata));
    status = ca_create_channel(pv, NULL, 0, CA_PRIORITY, &ch);
    if (ch == (chid) 0) {
        return !ECA_NORMAL;
    }
    status = ca_pend_io(CA_TIMEOUT);

    if (ca_state(ch) != cs_conn) {
        C_postMsgEvent(messageWindow, 1, vaPrintf("pv (%s) is not connected\n", pv));
        return status;
    }

    chType = ca_field_type(ch);

    if(forceType == 1) chType = DBF_DOUBLE;
    else if(forceType == 2) chType = DBF_INT;

    switch (chType) {
    case DBF_STRING:
    case DBF_ENUM:
        PRINT(printf("Epicsput string for <%s> with data=%s\n", pv, sdata));
        status = ca_put(DBR_STRING, ch, sdata);
        if (status != ECA_NORMAL) {
            C_postMsgEvent(messageWindow, 1, vaPrintf("put pv (%s) %s\n", pv, ca_message (status)));
            return status;
        }
        break;

    case DBF_INT:
        PRINT(printf("Epicsput int for <%s> with data=%d\n", pv, (int) idata));
        status = ca_put(DBR_INT, ch, &idata);
        if (status != ECA_NORMAL) {
            C_postMsgEvent(messageWindow, 1, vaPrintf("put pv (%s) %s\n", pv, ca_message (status)));
            return status;
        }
        break;

    case DBF_LONG:
        PRINT(printf("Epicsput long for <%s> with data=%d\n", pv, (int) idata));
        status = ca_put(DBR_LONG, ch, &idata);
        if (status != ECA_NORMAL) {
            C_postMsgEvent(messageWindow, 1, vaPrintf("put pv (%s) %s\n", pv, ca_message (status)));
            return status;
        }
        break;

    case DBF_DOUBLE:
    case DBF_FLOAT:
        PRINT(printf("put double/float for <%s> with data=%f chid=%p\n", pv, rdata, ch));
        status = ca_put(DBR_DOUBLE, ch, &rdata);
        if (status != ECA_NORMAL) {
            C_postMsgEvent(messageWindow, 1, vaPrintf("put pv (%s) %s)\n", pv, ca_message (status)));
            return status;
        }
        break;

    case DBF_CHAR:
        PRINT(printf("put char array for <%s> with <%s>\n", pv, sdata));
        status = ca_array_put(DBR_CHAR, strlen(sdata)+1, ch, sdata);
        if (status != ECA_NORMAL) {
            C_postMsgEvent(messageWindow, 1, vaPrintf("put pv (%s) %s\n", pv, ca_message (status)));
            return status;
        }
        break;

    }

    status = ca_pend_io(CA_TIMEOUT);
    if (status != ECA_NORMAL) {
        C_postMsgEvent(messageWindow, 1, vaPrintf("put pv (%s) %s\n", pv, ca_message (status)));
        return status;
    }

    // something was written, so check status

    //printf("check status after write\n");

    switch (chType) {

    case DBF_ENUM:
        break;

    case DBF_DOUBLE:
    case DBF_FLOAT:
        status = ca_get(DBR_CTRL_DOUBLE, ch, &ctrlR);
        status = ca_pend_io(CA_TIMEOUT);
        if (status != ECA_NORMAL) {
            C_postMsgEvent(messageWindow, 1, vaPrintf("get pv (%s) %s\n", pv, ca_message (status)));
            return status;
        }
        status = ctrlR.status;
        break;

    case DBF_INT:
    case DBF_LONG:
        break;

    case DBF_STRING:
        status = ca_get(DBR_STRING, ch, &ctrlS);
        if (status != ECA_NORMAL) {
            C_postMsgEvent(messageWindow, 1, vaPrintf("get pv (%s) %s\n", pv, ca_message (status)));
            return status;
        }
        status = ctrlS.status;
        break;

    case DBF_CHAR:
        break;
    }

    return ECA_NORMAL;

}

int EpicsSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess)
{
    chid     ch;
    chtype   chType;
    int status;

    UNUSED(errmess);
    UNUSED(object);

    if(strlen(pv) < 1)  {
            C_postMsgEvent(messageWindow, 1, vaPrintf("pv with length=0 (not translated for macro?)\n"));
            return !ECA_NORMAL;
    }


    status = ca_create_channel(pv, NULL, 0, CA_PRIORITY, &ch);
    if (ch == (chid) 0) {
        return !ECA_NORMAL;
    }
    status = ca_pend_io(CA_TIMEOUT);

    if (ca_state(ch) != cs_conn) {
        C_postMsgEvent(messageWindow, 1, vaPrintf("pv (%s) is not connected\n", pv));
        return status;
    }

    chType = ca_field_type(ch);


    switch (chType) {

    case DBF_DOUBLE:
        status = ca_array_put (DBR_DOUBLE, nelm, ch, ddata);
        if (status != ECA_NORMAL) {
            C_postMsgEvent(messageWindow, 1, vaPrintf("put pv (%s) %s\n", pv, ca_message (status)));
            return status;
        }
        break;
    case DBF_FLOAT:
        status = ca_array_put (DBR_FLOAT, nelm, ch, fdata);
        if (status != ECA_NORMAL) {
            C_postMsgEvent(messageWindow, 1, vaPrintf("put pv (%s) %s\n", pv, ca_message (status)));
            return status;
        }
        break;
    case DBF_INT:
        status = ca_array_put (DBR_INT, nelm, ch, data16);
        if (status != ECA_NORMAL) {
            C_postMsgEvent(messageWindow, 1, vaPrintf("put pv (%s) %s\n", pv, ca_message (status)));
            return status;
        }
        break;
    case DBF_LONG:
        status = ca_array_put (DBR_LONG, nelm, ch, data32);
        if (status != ECA_NORMAL) {
            C_postMsgEvent(messageWindow, 1, vaPrintf("put pv (%s) %s\n", pv, ca_message (status)));
            return status;
        }
        break;
    case DBF_CHAR:
        status = ca_array_put (DBR_CHAR, nelm, ch, sdata);
        if (status != ECA_NORMAL) {
            C_postMsgEvent(messageWindow, 1, vaPrintf("put pv (%s) %s\n", pv, ca_message (status)));
            return status;
        }
        break;
    }

    status = ca_pend_io(CA_TIMEOUT);
    if (status != ECA_NORMAL) {
        C_postMsgEvent(messageWindow, 1, vaPrintf("put pv (%s) %s\n", pv, ca_message (status)));
        return status;
    }

    return ECA_NORMAL;
}

void TerminateDeviceIO()
{
    printf("TeminateDeviceIO\n");
    ca_flush_io();
    ca_context_destroy();
    ca_task_exit();
}

void EpicsFlushIO()
{
    ca_flush_io();
}

/**
 * exit handler, stop data acquisition
 */
void ExitHandler(int sig)
{
    printf("Exiting with signal=%d\n", sig);
}

int EpicsGetTimeStamp(char *pv, char *timestamp)
{
    chid     ch;
    int status;
    struct dbr_time_string ctrlS;
    char tsString[32];
    timestamp[0] = '\0';

    if(strlen(pv) < 1)  {
            C_postMsgEvent(messageWindow, 1, vaPrintf("pv with length=0 (not translated for macro?)\n"));
            return !ECA_NORMAL;
    }

    // get epics timestamp
    status = ca_create_channel(pv, NULL, 0, CA_PRIORITY, &ch);
    if (ch == (chid) 0) return !ECA_NORMAL;

    status = ca_pend_io(CA_TIMEOUT/2);

    if (ca_state(ch) != cs_conn) {
        C_postMsgEvent(messageWindow, 1, vaPrintf("pv (%s) is not connected\n", pv));
        return status;
    }

    status = ca_get(DBR_TIME_STRING, ch, &ctrlS);
    if (status != ECA_NORMAL) {
        C_postMsgEvent(messageWindow, 1, vaPrintf("get pv (%s) %s\n", pv, ca_message (status)));
        return status;
    }

    status = ca_pend_io(CA_TIMEOUT/2);

    sprintf(timestamp, "TimeStamp: %s\n", tsStampToText(&ctrlS.stamp,TS_TEXT_MONDDYYYY, tsString));

    return ECA_NORMAL;

}
