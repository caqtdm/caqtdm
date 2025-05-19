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
    #define EPICS_CALL_DLL
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
#include <ctype.h>
#include <shareLib.h>
#define epicsAlarmGLOBAL
#include <cadef.h>
#include "caQtDM_Lib_global.h"
#include "epicsExternals.h"
#include <alarm.h>
#include <epicsExport.h>
#include "epicsTime.h"

#undef epicsAlarmGLOBAL

#define CA_PRIORITY 50          /* CA priority */
#define CA_TIMEOUT   2          /* CA timeout 2.0 seconds */

//#define PRINT(a) a

#include "knobDefines.h"

#include "knobData.h"
#include "mutexKnobDataWrapper.h"
#include "messageWindowWrapper.h"
#include "vaPrintf.h"

#include <epicsMutex.h>
static epicsMutexId lockEpics = (epicsMutexId) 0;
static int optimizeConnections = false;

// global variables defined in epics3_plugin for access through c routines
extern MutexKnobData* mutexKnobdataPtr;
extern MessageWindow *messageWindowPtr;

static int firstTime = true;

typedef struct _connectInfo {
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

#define EpicsPut_ErrorMessage_ClearChannel_Return  \
    C_postMsgEvent(messageWindowPtr, 1, vaPrintf("put pv (%s) %s\n", pv, ca_message (status))); \
    connectInfo *info = (connectInfo *) ca_puser(ch);\
    info->connected=false;\
    C_SetMutexKnobDataConnected(mutexKnobdataPtr, info->index, info->connected);\
    info->evAdded = false;\
    if (ca_state(info->ch) == cs_conn) {\
        status = ca_clear_event(info->evID);\
        if (status != ECA_NORMAL) {\
            PRINT(printf("ca_clear_event:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]));\
        }\
    }\
    info->evID = 0;\
    info->event = 0;\
    if (info->connected){\
        ca_clear_channel(ch);\
    }\
    return status

#define EpicsGet_ErrorMessage_ClearChannel_Return  \
    C_postMsgEvent(messageWindowPtr, 1, vaPrintf("get pv (%s) %s\n", pv, ca_message (status))); \
    if (ca_state(info->ch) == cs_conn) {\
        status = ca_clear_event(info->evID);\
        if (status != ECA_NORMAL) {\
            PRINT(printf("ca_clear_event:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]));\
        }\
    }\
    if(info->ch != (chid) 0) {\
        ca_clear_channel(ch);\
    }\
    info->connected=false;\
    info->evAdded = false;\
    info->evID = 0;\
    info->event = 0;\
    C_SetMutexKnobDataConnected(mutexKnobdataPtr, info->index, info->connected);\
    knobData kData;\
    C_GetMutexKnobData(mutexKnobdataPtr, info->index, &kData);\
    EpicsReconnect(&kData);\
    return status


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
    C_postMsgEvent(messageWindowPtr, 2, vaPrintf("Channel Access Exception %s on %s (op=%ld data_type=%s count=%ld)\n",
                                            args.ctx, pName, args.op, dbr_type_to_text(args.type), args.count));
}

/**
 * setup of our mutex
 */
void InitializeContextMutex()
{
    lockEpics = epicsMutexCreate();
}

/**
 * define what we need for network IO
 */
void PrepareDeviceIO(void)
{
    //printf("preparedeviceio\n");
    int status;
    char *optimize = Q_NULLPTR;
    char *s;

    if(lockEpics == (epicsMutexId) 0) InitializeContextMutex();

    epicsMutexLock(lockEpics);

    if(!ca_current_context()) {
        //printf("create context\n");
        status = ca_context_create(ca_enable_preemptive_callback);
        if (status != ECA_NORMAL) {
            printf("ca_context_create:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]);
            exit(1);
        }
        ca_add_exception_event(Exceptionhandler, 0);

        optimize = (char*) getenv("CAQTDM_OPTIMIZE_EPICS3CONNECTIONS");
        if (optimize != Q_NULLPTR) {
            s = optimize; while (*s) {*s = toupper((unsigned char) *s); s++;}
            if(strcmp(optimize, "TRUE") == 0) {
                optimizeConnections = true;
                if(firstTime) {
                    C_postMsgEvent(messageWindowPtr, 1, vaPrintf("caQtDM will close epics connections for data in invisible tabs while CAQTDM_OPTIMIZE_EPICS3CONNECTIONS is set to TRUE\n"));
                    printf("caQtDM -- Close epics connections for data in invisible tabs while CAQTDM_OPTIMIZE_EPICS3CONNECTIONS is TRUE\n");
                }
            }
        }
        if(!optimizeConnections) {
            if(firstTime) {
                C_postMsgEvent(messageWindowPtr, 1, vaPrintf("caQtDM will suspend epics connections for data in invisible tabs while CAQTDM_OPTIMIZE_EPICS3CONNECTIONS not set to TRUE\n"));
                printf("caQtDM -- Suspend epics connections for data in invisible tabs while CAQTDM_OPTIMIZE_EPICS3CONNECTIONS not set to TRUE\n");
            }
        }
        firstTime = false;
    } else {
        //printf("context exists\n");
        status = ca_attach_context(ca_current_context());
    }
    epicsMutexUnlock(lockEpics);
}

static void access_rights_handler(struct access_rights_handler_args args)
{
    knobData kData;
    connectInfo *info;
    PrepareDeviceIO();

    info = (connectInfo *) ca_puser(args.chid);
    C_GetMutexKnobData(mutexKnobdataPtr, info->index, &kData);
    kData.edata.accessW = ca_write_access(args.chid);
    kData.edata.accessR = ca_read_access(args.chid);
    kData.edata.monitorCount = info->event;
    C_SetMutexKnobDataReceived(mutexKnobdataPtr, &kData);
    //printf("access rights callback %d %d %d\n",  kData.edata.accessW, kData.edata.accessR, kData.edata.monitorCount);
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

    C_GetMutexKnobData(mutexKnobdataPtr, info->index, &kData);
    if(kData.index == -1) return;

    if (args.status != ECA_NORMAL) {
        PRINT(printf("dataCallback:  get: %s for %s\n", ca_name(args.chid), ca_message_text[CA_EXTRACT_MSG_NO(args.status)]));
    } else {
        kData.edata.monitorCount = info->event;
        kData.edata.connected = info->connected;
        kData.edata.fieldtype = ca_field_type(args.chid);
        ftime(&now);

        C_DataLock(mutexKnobdataPtr, &kData);

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
               if(kData.edata.dataB != (void*) Q_NULLPTR) free(kData.edata.dataB);
                kData.edata.dataB = (void*) malloc((size_t) dataSize);
                kData.edata.dataSize = dataSize;
            }

            ptr = (char*) kData.edata.dataB;
            memcpy(ptr, val_ptr, args.count *sizeof(char));
            ptr[args.count] = '\0';

            AssignEpicsValue((double) stsF->value, (long) stsF->value, args.count);

            C_SetMutexKnobDataReceived(mutexKnobdataPtr, &kData);
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
            // the reason for this here are crashes mainly on reloads of the panels
            if (dataSize < db_strval_dim) dataSize=db_strval_dim;
            if(dataSize != kData.edata.dataSize) {
                if(kData.edata.dataB != (void*) Q_NULLPTR) free(kData.edata.dataB);
                kData.edata.dataB = (void*) malloc((size_t) dataSize);
                kData.edata.dataSize = dataSize;
            }

            ptr = (char*) kData.edata.dataB;
            ptr[0] = '\0';
            len = 0;
            strncpy(ptr, myLimitedString(val_ptr[0]),kData.edata.dataSize);
            for (i = 1; i < args.count; i++) {
                len = len+ (int) strlen(myLimitedString(val_ptr[i-1]));
                strcat(&ptr[len++], "\033");
                strcat(&ptr[len], myLimitedString(val_ptr[i]));
            }

            AssignEpicsValue((double) 0, (long) stsF->value, args.count);

            C_SetMutexKnobDataReceived(mutexKnobdataPtr, &kData);
        }
        break;

        case DBF_ENUM:
        {
            struct dbr_sts_enum *stsF = (struct dbr_sts_enum *) args.dbr;

            PRINT(printf("dataCallback enum  %s %d <%d> %d <%s> status=%d count=%d size=%d\n", ca_name(args.chid), (int) args.chid,
                         stsF->value, info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, dbr_size_n(args.type, args.count)));

            AssignEpicsValue((double) stsF->value, (long) stsF->value, args.count);

            C_SetMutexKnobDataReceived(mutexKnobdataPtr, &kData);
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
                    if(kData.edata.dataB != (void*) Q_NULLPTR) free(kData.edata.dataB);
                    kData.edata.dataB = (void*) malloc(args.count * sizeof(int16_t));
                    kData.edata.dataSize = args.count * (int) sizeof(int16_t);
                }
                memcpy(kData.edata.dataB, &stsF->value, args.count * sizeof(int16_t));
            }

            C_SetMutexKnobDataReceived(mutexKnobdataPtr, &kData);
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
                    if(kData.edata.dataB != (void*) Q_NULLPTR) free(kData.edata.dataB);
                    kData.edata.dataB = (void*) malloc(args.count * sizeof(int32_t));
                    kData.edata.dataSize = args.count * (int) sizeof(int32_t);
                }
                memcpy(kData.edata.dataB, &stsF->value, args.count * sizeof(int32_t));
            }

            C_SetMutexKnobDataReceived(mutexKnobdataPtr, &kData);
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
                    if(kData.edata.dataB != (void*) Q_NULLPTR) free(kData.edata.dataB);
                    kData.edata.dataB = (void*) malloc(args.count * sizeof(float));
                    kData.edata.dataSize = args.count * (int) sizeof(float);
                }
                memcpy(kData.edata.dataB, &stsF->value, args.count * sizeof(float));
            }
            C_SetMutexKnobDataReceived(mutexKnobdataPtr, &kData);
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
                    if(kData.edata.dataB != (void*) Q_NULLPTR) free(kData.edata.dataB);
                    kData.edata.dataB = (void*) malloc(args.count * sizeof(double));
                    memcpy(kData.edata.dataB, &stsF->value, args.count * sizeof(double));
                    kData.edata.dataSize = args.count * (int) sizeof(double);
                }
                memcpy(kData.edata.dataB, &stsF->value, args.count * sizeof(double));
            }
            C_SetMutexKnobDataReceived(mutexKnobdataPtr, &kData);

        }
        break;

            default:
                C_postMsgEvent(messageWindowPtr, 2, vaPrintf("unhandled epics type (%d) in datacallback\n", ca_field_type(args.chid)));

        } // end switch

        C_DataUnlock(mutexKnobdataPtr, &kData);
        info->event++;
    }
}

static void displayCallback(struct event_handler_args args) {
    knobData kData;
    struct timeb now;
    int status;

    connectInfo *info = (connectInfo *) ca_puser(args.chid);
    if(info == (connectInfo *) 0) return;

    C_GetMutexKnobData(mutexKnobdataPtr, info->index, &kData);
    if(kData.index == -1) return;

    if (args.status != ECA_NORMAL) {
        PRINT(printf("displayCallback:\n""  get: %s for %s\n", ca_name(args.chid), ca_message_text[CA_EXTRACT_MSG_NO(args.status)]));
    } else {
        kData.edata.initialize = true;
        kData.edata.monitorCount = kData.edata.displayCount = info->event;
        kData.edata.connected = info->connected;
        kData.edata.fieldtype = ca_field_type(args.chid);
        kData.edata.nelm = ca_element_count(args.chid);
        ftime(&now);

        C_DataLock(mutexKnobdataPtr, &kData);

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
                    if(kData.edata.dataB != (void*) Q_NULLPTR) free(kData.edata.dataB);
                    kData.edata.dataB = (void*) malloc((size_t) dataSize);
                    kData.edata.dataSize = dataSize;
                }

                ptr = (char*) kData.edata.dataB;
                ptr[0] = '\0';
                len = 0;
                strcpy(ptr, myLimitedString(stsF->strs[0]));
                for (i = 1; i < stsF->no_str; i++) {
                    len = len + (int) strlen(myLimitedString(stsF->strs[i-1]));
                    //strcat(&ptr[len++], ";");
                    strcat(&ptr[len++], "\033");
                    strcat(&ptr[len], myLimitedString(stsF->strs[i]));
                }

            } else if(args.count == 1) {  // no strings, must be a value, convert it to text
                // concatenate strings separated with ';'
                dataSize = 40;
                if(dataSize != kData.edata.dataSize) {
                    if(kData.edata.dataB != (void*) Q_NULLPTR) free(kData.edata.dataB);
                    kData.edata.dataB = (void*) malloc((size_t) dataSize);
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

            default:
                C_postMsgEvent(messageWindowPtr, 2, vaPrintf("unhandled epics type (d) in displaycallback %d\n", ca_field_type(args.chid)));

        } // end switch

        // when specifying zero as number of requested elements, we will get variable length arrays (zero lenght is then also considered)
        // probably will not work with older channel access gateways

#if EPICS_REVISION < 15
        status = ca_add_array_event(dbf_type_to_DBR_STS(ca_field_type(args.chid)), 0, //ca_element_count(args.chid),
                                           args.chid, dataCallback, info, 0.0,0.0,0.0, &info->evID);
        info->evAdded = true;
        PRINT(printf("ca_add_array_event added for %s with chid=%d index=%d info->evAdded=%d\n", ca_name(args.chid), args.chid, kData.index, info->evAdded));
        if (status != ECA_NORMAL) {
            PRINT(printf("ca_add_array_event:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]));
        }
        C_SetMutexKnobData(mutexKnobdataPtr, kData.index, kData);
        info->event++;
#else
        if(!info->evAdded) {
            status = ca_add_array_event(dbf_type_to_DBR_STS(ca_field_type(args.chid)), 0, //ca_element_count(args.chid),
                                               args.chid, dataCallback, info, 0.0,0.0,0.0, &info->evID);
            info->evAdded = true;
            PRINT(printf("ca_add_array_event added for %s with chid=%d index=%d info->evAdded=%d\n", ca_name(args.chid), args.chid, kData.index, info->evAdded));
            if (status != ECA_NORMAL) {
                PRINT(printf("ca_add_array_event:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]));
            }
            C_SetMutexKnobData(mutexKnobdataPtr, kData.index, kData);
            info->event++;
        } else {
            printf("display info event nr=%d\n", info->event);
            kData.edata.displayCount = info->event-2;
            kData.edata.monitorCount = info->event-1;
            C_SetMutexKnobDataReceived(mutexKnobdataPtr, &kData);
        }
#endif
        C_DataUnlock(mutexKnobdataPtr, &kData);
    }
}

/**
 * routine used to suspend the monitor when requested
 */
void clearEvent(void * ptr)
{
    int status;
    connectInfo *info = (connectInfo *) ptr;

    if(optimizeConnections) {

        knobData kData;
        C_GetMutexKnobData(mutexKnobdataPtr, info->index, &kData);
        if(kData.index == -1) return;

        PrepareDeviceIO();

        PRINT(printf("destroyConnection -- %s %d %d %d %d\n", info->pv, info->evID, info->index, info->connected, info->evAdded));
        EpicsDisconnect(&kData);
        C_DataLock(mutexKnobdataPtr, &kData);
        kData.edata.connected = false;
        kData.edata.unconnectCount = 0;
        C_SetMutexKnobData(mutexKnobdataPtr, kData.index, kData);
        C_DataUnlock(mutexKnobdataPtr, &kData);

    } else {
        if(!info->connected) return;  // must be connected
        if(info->event < 2) return;  // a first normal addevent must be done
        if(info->evAdded) {

            PrepareDeviceIO();

            PRINT(printf("clearEvent -- %s %d %d %d %d\n", info->pv, info->evID, info->index, info->connected, info->evAdded));
            info->evAdded = false;
            status = ca_clear_event(info->evID);
            if (status != ECA_NORMAL) {
                PRINT(printf("ca_clear_event:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]));
            }
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

    if(optimizeConnections) {
        knobData kData;
        if(info->connected) return; // already connected ?
        if(info->ch != (chid) 0) return; // already requested

        PrepareDeviceIO();

        C_GetMutexKnobData(mutexKnobdataPtr, info->index, &kData);
        if(kData.index == -1) return;

        PRINT(printf("recreateConnection -- %s %d %d %d %d\n", info->pv, info->evID, info->index, info->connected, info->evAdded));
        EpicsReconnect(&kData);

    } else {

        if(!info->connected) return; // must be connected
        if(info->event < 2) return;  // a first normal addevent must be done
        if(!info->evAdded) {

            knobData kData;
            int status;

            PrepareDeviceIO();

            C_GetMutexKnobData(mutexKnobdataPtr, info->index, &kData);
            if(kData.index == -1) return;

            C_DataLock(mutexKnobdataPtr, &kData);
            PRINT(printf("addEvent -- %s %d %d %d %d\n", info->pv, info->evID, info->index, info->connected, info->evAdded));
            status = ca_add_array_event(dbf_type_to_DBR_STS(ca_field_type(info->ch)), 0,
                                        info->ch, dataCallback, info, 0.0,0.0,0.0, &info->evID);
            info->evAdded = true;

            if (status != ECA_NORMAL) {
                PRINT(printf("ca_add_array_event:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]));
            }
            C_SetMutexKnobData(mutexKnobdataPtr, kData.index, kData);

            C_DataUnlock(mutexKnobdataPtr, &kData);
        }
    }
}


/**
 * epics connect callback
 */
void connectCallback(struct connection_handler_args args)
{
    int status;

    connectInfo *info = (connectInfo *) ca_puser(args.chid);
    if (!info) return;
    PRINT(printf("connectcallback %p pv=<%s> %d chid=%d\n", info, info->pv, info->evAdded, args.chid));

    switch (ca_state(args.chid)) {

    case cs_never_conn:
        PRINT(printf("%s was never connected\n", ca_name(args.chid)));
        info->connected = false;
        break;
    case cs_prev_conn:
        PRINT(printf("%s with channel %d has just disconnected, evid=%d\n", ca_name(args.chid), args.chid, info->evID));
        if(info->evAdded) {
            if (info->connected){
                if (info->evID){
                    status = ca_clear_event(info->evID);
                    if (status != ECA_NORMAL) {
                       PRINT(printf("ca_clear_event:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]));
                    }
                }
            }
        }
        info->connected = false;
        info->event = 0;
        info->evAdded = false;
        info->evID = 0;
        break;
    case cs_conn:
        PRINT(printf("%s has just connected with channel id=%d count=%d native type=%s\n", ca_name(args.chid), (int) args.chid, ca_element_count(args.chid), dbf_type_to_text(ca_field_type(args.chid))));
        info->connected = true;
        info->evAdded = false;
        if (info->event == 0) {
            info->event++;

#if EPICS_REVISION < 15
            status = ca_array_get_callback(dbf_type_to_DBR_CTRL(ca_field_type(args.chid)), 1, args.chid, displayCallback, Q_NULLPTR);
#else
            status = ca_add_masked_array_event(dbf_type_to_DBR_CTRL(ca_field_type(args.chid)), 0, //ca_element_count(args.chid),
                                         args.chid, displayCallback, info, 0.0,0.0,0.0, &info->evID, DBE_PROPERTY);
#endif
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
    C_SetMutexKnobDataConnected(mutexKnobdataPtr, info->index, info->connected);
}

/**
 * define device io and hook info to gui
 */
int CreateAndConnect(int index, knobData *kData, int rate, int skip)
{
    int status;
    connectInfo *info = (connectInfo *) 0;
    UNUSED(skip);
    UNUSED(rate);

    PrepareDeviceIO();

    /* initialize channels */
    PRINT(printf("create channel index=%d <%s> rate=%d\n", index, kData->pv, rate));

    kData->edata.info = (connectInfo *) malloc(sizeof (connectInfo));
    info = (connectInfo *) kData->edata.info;

    strcpy(info->pv, kData->pv);
    info->connected = false;
    info->index = index;
    info->event = 0;
    info->evAdded = false;
    info->ch = 0;

    // update knobdata
    C_SetMutexKnobData(mutexKnobdataPtr, index, *kData);

    //printf("we have to add an epics device <%s>\n", kData->pv);
    status = ca_create_channel(kData->pv,
                               (void(*)())connectCallback,
                               info,
                               CA_PRIORITY_DEFAULT,
                               &info->ch);
    if(status != ECA_NORMAL) {
        printf("ca_create_channel: %s for device -%s-\n", ca_message_text[CA_EXTRACT_MSG_NO(status)], kData->pv);
    }

    status = ca_pend_io(CA_TIMEOUT);
    if(status != ECA_NORMAL) {
        printf("ca_pend_io:\n"" %s for %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)], kData->pv);
    }

    PRINT(printf("channel created for button=%d <%s> info=%p, chid=%d\n", index, kData->pv, info, info->ch));

    return index;
}

void EpicsReconnect(knobData *kData)
{
    int status;
    connectInfo *info;

    // in case of a soft channel there is nothing to do
    if(kData->soft) return;

    PrepareDeviceIO();

    info = (connectInfo *) kData->edata.info;

    PRINT(printf("create channel for an epics device <%s>\n", kData->pv));

    if (info != (connectInfo *) 0) {
        status = ca_create_channel(kData->pv,
                                   (void(*)())connectCallback,
                                   info,
                                   CA_PRIORITY_DEFAULT,
                                   &info->ch);
        if(status != ECA_NORMAL) {
            printf("ca_create_channel:\n"" %s for %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)], kData->pv);
        }
        status = ca_pend_io(CA_TIMEOUT);
        if (status != ECA_NORMAL) {
            printf("ca_pend_io:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]);
        }
    }

}

void EpicsDisconnect(knobData *kData)
{
    int status;
    connectInfo *info;

    if (kData->index == -1) return;

    PrepareDeviceIO();

    info = (connectInfo *) kData->edata.info;
    if (info != (connectInfo *) 0) {
        if(info->ch != (chid) 0) {
            if(info->evAdded && (info->evID != (evid) 0)) {
                info->evAdded = false;
                if (ca_state(info->ch) == cs_conn) {
                    status = ca_clear_event(info->evID);
                    if (status != ECA_NORMAL) {
                        printf("ca_clear_event:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]);
                    }
                }
                info->evAdded = 0;
                info->evID = 0;
            }

            status = ca_clear_channel(info->ch);
            info->connected = false;
            info->event = 0;
            info->ch = 0;

            if(status != ECA_NORMAL) {
                printf("ca_clear_channel: %s %s index=%d\n", ca_message_text[CA_EXTRACT_MSG_NO(status)], info->pv, kData->index);
            }
            status = ca_pend_io(CA_TIMEOUT);
            if (status != ECA_NORMAL) {
                printf("ca_pend_io:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]);
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

    if (kData->index == -1) return;

    PrepareDeviceIO();

    PRINT(printf("ClearMonitor -- clear channel %s index=%d\n", kData->pv, kData->index));

    aux =  kData->index;
    kData->index = -1;
    kData->pv[0] = '\0';

    info = (connectInfo *) kData->edata.info;
    if (info != (connectInfo *) 0) {
        if (info->connected){
            if(info->ch != (chid) 0) {
                if(info->evAdded) {
                    info->evAdded = false;
                    PRINT(printf("ca_clear_event: %s index=%d\n", info->pv, aux));
                    if (ca_state(info->ch) == cs_conn) {
                        status = ca_clear_event(info->evID);
                        if (status != ECA_NORMAL) {
                            PRINT(printf("ca_clear_event:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]));
                        }
                    }
                }
                status = ca_clear_channel(info->ch);
                PRINT(printf("ca_clear_channel: %s chid=%d, index=%d\n", info->pv, info->ch, aux));
                info->connected = false;
                info->event = 0;
                //info->ch = 0;
                if(status != ECA_NORMAL) {
                    printf("ca_clear_channel: %s %s index=%d\n", ca_message_text[CA_EXTRACT_MSG_NO(status)], info->pv, aux);
                }
                info->pv[0] = '\0';
            }
        } else {
            PRINT(printf("ClearMonitor -- %s is not connected index=%d %d\n", info->pv, info->index, info->evID));
            info->connected=false;
            C_SetMutexKnobDataConnected(mutexKnobdataPtr, info->index, info->connected);
            info->evAdded = false;
            info->evID = 0;
            info->event = 0;
            if(info->ch != (chid) 0) {
                ca_clear_channel(info->ch);
            }
            info->ch = 0;
            PRINT(printf("ClearMonitor -- %s should now have been taken out \n", info->pv));
            info->pv[0] = '\0';
        }
    }

    status = ca_pend_io(CA_TIMEOUT);
}

void DestroyContext()
{
    PrepareDeviceIO();
    ca_pend_io(CA_TIMEOUT);
    ca_context_destroy();
}

int EpicsSetValue_Connected(chid ch,char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType)
{
    chtype   chType;
    int status;
    struct dbr_ctrl_double ctrlR;
    struct dbr_sts_string ctrlS;
    connectInfo *info = (connectInfo *) ca_puser(ch);
    if (!info){

        return ECA_DISCONN;
    }
    chType = ca_field_type(ch);
    if (!ca_write_access(ch)){
        C_postMsgEvent(messageWindowPtr, 1, vaPrintf("put pv (%s) no write access\n", pv));
        return ECA_NOWTACCESS;
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
            EpicsPut_ErrorMessage_ClearChannel_Return;
        }
        break;

    case DBF_INT:
        PRINT(printf("Epicsput int for <%s> with data=%d\n", pv, (int) idata));
        status = ca_put(DBR_INT, ch, &idata);
        if (status != ECA_NORMAL) {
             EpicsPut_ErrorMessage_ClearChannel_Return;
        }
        break;

    case DBF_LONG:
        PRINT(printf("Epicsput long for <%s> with data=%d\n", pv, (int) idata));
        status = ca_put(DBR_LONG, ch, &idata);
        if (status != ECA_NORMAL) {
             EpicsPut_ErrorMessage_ClearChannel_Return;
        }
        break;

    case DBF_DOUBLE:
    case DBF_FLOAT:
        PRINT(printf("put double/float for <%s> with data=%f chid=%p\n", pv, rdata, ch));
        status = ca_put(DBR_DOUBLE, ch, &rdata);
        if (status != ECA_NORMAL) {
             EpicsPut_ErrorMessage_ClearChannel_Return;
        }
        break;

    case DBF_CHAR:
        PRINT(printf("put char array for <%s> with <%s>\n", pv, sdata));
        status = ca_array_put(DBR_CHAR, strlen(sdata)+1, ch, sdata);
        if (status != ECA_NORMAL) {
             EpicsPut_ErrorMessage_ClearChannel_Return;
        }
        break;

        default:
            C_postMsgEvent(messageWindowPtr, 2, vaPrintf("unhandled epics type (%d) in epicssetvalue\n", chType));

    }

    status = ca_pend_io(CA_TIMEOUT);
    if (status != ECA_NORMAL) {
         EpicsPut_ErrorMessage_ClearChannel_Return;
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
             //EpicsGet_ErrorMessage_ClearChannel_Return;
            C_postMsgEvent(messageWindowPtr, 1, vaPrintf("get pv (%s) %s\n", pv, ca_message (status)));
            if (ca_state(info->ch) == cs_conn) {
                status = ca_clear_event(info->evID);
                if (status != ECA_NORMAL) {
                    PRINT(printf("ca_clear_event:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]));
                }
            }
            if(info->ch != (chid) 0) {
                ca_clear_channel(ch);
            }
            info->connected=false;
            info->ch=0;
            info->evAdded = false;
            info->evID = 0;
            info->event = 0;
            C_SetMutexKnobDataConnected(mutexKnobdataPtr, info->index, info->connected);
            knobData kData;
            C_GetMutexKnobData(mutexKnobdataPtr, info->index, &kData);\
            EpicsReconnect(&kData);
            return status;

        }
        status = ctrlR.status;
        break;

    case DBF_INT:
    case DBF_LONG:
        break;

    case DBF_STRING:
        status = ca_get(DBR_STRING, ch, &ctrlS);
        status = ca_pend_io(CA_TIMEOUT);
        if (status != ECA_NORMAL) {
             //EpicsGet_ErrorMessage_ClearChannel_Return;
            C_postMsgEvent(messageWindowPtr, 1, vaPrintf("get pv (%s) %s\n", pv, ca_message (status)));
            if (ca_state(info->ch) == cs_conn) {
                status = ca_clear_event(info->evID);
                if (status != ECA_NORMAL) {
                    PRINT(printf("ca_clear_event:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]));
                }
            }
            if(info->ch != (chid) 0) {
                ca_clear_channel(ch);
            }
            info->connected=false;
            info->ch=0;
            info->evAdded = false;
            info->evID = 0;
            info->event = 0;
            C_SetMutexKnobDataConnected(mutexKnobdataPtr, info->index, info->connected);
            knobData kData;
            C_GetMutexKnobData(mutexKnobdataPtr, info->index, &kData);\
            EpicsReconnect(&kData);
            return status;


        }
        status = ctrlS.status;
        break;

    case DBF_CHAR:
        break;

        default:
            C_postMsgEvent(messageWindowPtr, 2, vaPrintf("unhandled epics type (%d) in epicssetvalue\n", chType));
    }

    return ECA_NORMAL;

}
int EpicsSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType)
{
    chid     ch;
    int status;

    UNUSED(errmess);
    UNUSED(object);

    if(strlen(pv) < 1)  {
            C_postMsgEvent(messageWindowPtr, 1, vaPrintf("pv with length=0 (not translated for macro?)\n"));
            return !ECA_NORMAL;
    }

    PrepareDeviceIO();

    // set epics value
    PRINT(printf(" we have to set a value to an epics device <%s> %f %ld <%s>\n", pv, rdata, idata, sdata));
    status = ca_create_channel(pv, Q_NULLPTR, 0, CA_PRIORITY, &ch);
    if (ch == (chid) 0) {
        return !ECA_NORMAL;
    }
    status = ca_pend_io(CA_TIMEOUT);

    if (ca_state(ch) != cs_conn) {
        C_postMsgEvent(messageWindowPtr, 1, vaPrintf("pv (%s) is not connected\n", pv));
        ca_clear_channel(ch);
        return status;
    }

    EpicsSetValue_Connected(ch, pv, rdata, idata, sdata, object, errmess, forceType);

    status = ca_clear_channel(ch);


    return ECA_NORMAL;

}

int EpicsSetWave_Connected(chid ch,char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess)
{
    chtype   chType;
    int status= ECA_NORMAL;

    UNUSED(errmess);
    UNUSED(object);

    if (ca_state(ch) != cs_conn) {
        C_postMsgEvent(messageWindowPtr, 1, vaPrintf("pv (%s) is not connected\n", pv));
        return status;
    }
    if (!ca_write_access(ch)){
        C_postMsgEvent(messageWindowPtr, 1, vaPrintf("put pv (%s) no write access\n", pv));
        return ECA_NOWTACCESS;
    }

    chType = ca_field_type(ch);


    switch (chType) {

    case DBF_DOUBLE:
        status = ca_array_put (DBR_DOUBLE, (unsigned long) nelm, ch, ddata);
        if (status != ECA_NORMAL) {
            EpicsPut_ErrorMessage_ClearChannel_Return;
        }
        break;
    case DBF_FLOAT:
        status = ca_array_put (DBR_FLOAT, (unsigned long) nelm, ch, fdata);
        if (status != ECA_NORMAL) {
            EpicsPut_ErrorMessage_ClearChannel_Return;
        }
        break;
    case DBF_INT:
        status = ca_array_put (DBR_INT, (unsigned long) nelm, ch, data16);
        if (status != ECA_NORMAL) {
            EpicsPut_ErrorMessage_ClearChannel_Return;
        }
        break;
    case DBF_LONG:
        status = ca_array_put (DBR_LONG, (unsigned long) nelm, ch, data32);
        if (status != ECA_NORMAL) {
            EpicsPut_ErrorMessage_ClearChannel_Return;
        }
        break;
    case DBF_CHAR:
        status = ca_array_put (DBR_CHAR, (unsigned long) nelm, ch, sdata);
        if (status != ECA_NORMAL) {
            EpicsPut_ErrorMessage_ClearChannel_Return;
        }
        break;

        default:
            C_postMsgEvent(messageWindowPtr, 2, vaPrintf("unhandled epics type (%d) in epicssetwave\n", chType));
    }

    status = ca_pend_io(CA_TIMEOUT);
    if (status != ECA_NORMAL) {
        C_postMsgEvent(messageWindowPtr, 1, vaPrintf("put pv (%s) %s\n", pv, ca_message (status)));
        return status;
    }

    return ECA_NORMAL;
}







int EpicsSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess)
{
    chid     ch;

    int status;

    PrepareDeviceIO();

    if(strlen(pv) < 1)  {
            C_postMsgEvent(messageWindowPtr, 1, vaPrintf("pv with length=0 (not translated for macro?)\n"));
            return !ECA_NORMAL;
    }

    status = ca_create_channel(pv, Q_NULLPTR, 0, CA_PRIORITY, &ch);
    if (ch == (chid) 0) {
        return !ECA_NORMAL;
    }
    status = ca_pend_io(CA_TIMEOUT);

    EpicsSetWave_Connected(ch,pv, fdata, ddata, data16, data32,sdata,nelm,object,errmess);

    status = ca_clear_channel(ch);

    return ECA_NORMAL;
}

void TerminateDeviceIO()
{
    int status;
    PrepareDeviceIO();

    status = ca_pend_io(CA_TIMEOUT);
    if (status != ECA_NORMAL) {
        C_postMsgEvent(messageWindowPtr, 1, vaPrintf("ca_pend_io (%s)\n", ca_message (status)));
    }

    ca_context_destroy();
}

void EpicsFlushIO()
{

    PrepareDeviceIO();
    ca_flush_io();
}

/**
 * exit handler, stop data acquisition
 */
void ExitHandler(int sig)
{
    printf("Exiting with signal=%d\n", sig);
}
int EpicsGetTimeStamp_Connected(chid ch,char *pv, char *timestamp)
{
    int status= ECA_NORMAL;
    struct dbr_time_string ctrlS;
    char tsString[32];
    timestamp[0] = '\0';

    if (ca_state(ch) != cs_conn) {
        C_postMsgEvent(messageWindowPtr, 1, vaPrintf("pv (%s) is not connected\n", pv));
        return status;
    }

    status = ca_get(DBR_TIME_STRING, ch, &ctrlS);
    if (status != ECA_NORMAL) {
        C_postMsgEvent(messageWindowPtr, 1, vaPrintf("get pv (%s) %s\n", pv, ca_message (status)));
        return status;
    }

    status = ca_pend_io(CA_TIMEOUT/2);
    if (status == ECA_NORMAL) {
        epicsTimeToStrftime(tsString, 32, "%b %d, %Y %H:%M:%S.%09f", &ctrlS.stamp);
        sprintf(timestamp, "TimeStamp: %s\n", tsString);
    } else {
        strcpy(timestamp, "-timestamp timeout-");
    }
    return status;
}


int EpicsGetTimeStamp(char *pv, char *timestamp)
{
    chid     ch;
    int status;

    PrepareDeviceIO();

    if(strlen(pv) < 1)  {
        C_postMsgEvent(messageWindowPtr, 1, vaPrintf("pv with length=0 (not translated for macro?)\n"));
        return !ECA_NORMAL;
    }

    // get epics timestamp
    status = ca_create_channel(pv, Q_NULLPTR, 0, CA_PRIORITY, &ch);
    if (ch == (chid) 0) return !ECA_NORMAL;

    status = ca_pend_io(CA_TIMEOUT/2);

    EpicsGetTimeStamp_Connected(ch, pv, timestamp);

    ca_clear_channel(ch);

    return ECA_NORMAL;
}

int EpicsGetDescription(char *pv, char *description)
{
    chid     ch;
    int status;
    pv_desc pvDesc = {'\0'};
    char * pch;
    dbr_string_t value;
    strcpy(description, "");

    PrepareDeviceIO();

    if(strlen(pv) < 1)  {
        C_postMsgEvent(messageWindowPtr, 1, vaPrintf("pv with length=0 (not translated for macro?)\n"));
        return !ECA_NORMAL;
    }
    // if there is a filter we remove it first

    strcpy(pvDesc,pv);
    pch = strstr (pvDesc,".{");

    if (pch) *pch='\0';

    sprintf(pvDesc, "%s.DESC", pvDesc);

    // get description
    status = ca_create_channel(pvDesc, Q_NULLPTR, 0, CA_PRIORITY, &ch);
    if (ch == (chid) 0) return !ECA_NORMAL;

    status = ca_pend_io(CA_TIMEOUT/2);

    if (ca_state(ch) != cs_conn) {
        C_postMsgEvent(messageWindowPtr, 1, vaPrintf("pv (%s) is not connected\n", pvDesc));
        return status;
    }

    status = ca_get(DBR_STRING, ch, &value);
    if (status != ECA_NORMAL) {
        C_postMsgEvent(messageWindowPtr, 1, vaPrintf("get pv (%s) %s\n", pv, ca_message (status)));
        return status;
    }

    status = ca_pend_io(CA_TIMEOUT/2);
    if (status == ECA_NORMAL) strcpy(description, value); else strcpy(description, "- description timeout-");

    ca_clear_channel(ch);

    return ECA_NORMAL;
}

