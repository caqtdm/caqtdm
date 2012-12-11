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
#include <math.h>
#include <string.h>
#include <unistd.h>

#define epicsAlarmGLOBAL
#include <alarm.h>
#undef epicsAlarmGLOBAL

#define CA_PRIORITY 50          /* CA priority */
#define CA_TIMEOUT   2          /* CA timeout 2.0 seconds */

#include "knobDefines.h"
#include <cadef.h>
struct ca_client_context *dbCaClientContext;

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
} connectInfo;

#define AssignEpicsData \
    strcpy(kData.edata.units, stsF->units); \
    kData.edata.lower_disp_limit = stsF->lower_disp_limit; \
    kData.edata.upper_disp_limit = stsF->upper_disp_limit; \
    kData.edata.lower_alarm_limit = stsF->lower_alarm_limit; \
    kData.edata.upper_alarm_limit = stsF->upper_alarm_limit; \
    kData.edata.lower_ctrl_limit = stsF->lower_ctrl_limit;   \
    kData.edata.upper_ctrl_limit = stsF->upper_ctrl_limit;  \
    kData.edata.lower_warning_limit = stsF->lower_warning_limit; \
    kData.edata.upper_warning_limit = stsF->upper_warning_limit;

#define AssignEpicsValue(valx, vali, countx) \
    kData.edata.rvalue = valx; \
    kData.edata.ivalue = vali; \
    kData.edata.severity = stsF->severity; \
    kData.edata.accessW = ca_write_access(args.chid); \
    kData.edata.accessR = ca_read_access(args.chid); \
    kData.edata.valueCount = countx; \
    strcpy(kData.edata.fec, ca_host_name(args.chid)); \
    kData.edata.monitorCount = info->event;

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
void PrepareDeviceIO()
{
    PRINT(printf("create epics context\n"));
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

    connectInfo *info = (connectInfo *) ca_puser(args.chid);
    if(info == (connectInfo *) 0) return;

    C_GetMutexKnobData(KnobDataPtr, info->index, &kData);
    if(kData.index == -1) return;

    if (args.status != ECA_NORMAL) {
        PRINT(printf("dataCallback:\n""  get: %s\n", ca_message_text[CA_EXTRACT_MSG_NO(args.status)]));
    } else {
        kData.edata.monitorCount = info->event;
        kData.edata.connected = info->connected;
        kData.edata.fieldtype = ca_field_type(args.chid);

        switch (ca_field_type(args.chid)) {

        case DBF_CHAR:
        {
            int dataSize;
            struct dbr_ctrl_char *stsF = (struct dbr_ctrl_char *) args.dbr;
            dbr_char_t *val_ptr = dbr_value_ptr(args.dbr, DBR_CTRL_CHAR);

            PRINT(printf("dataCallback char %s %d %d <%s> status=%d count=%d nBytes=%d\n", ca_name(args.chid), (int) args.chid,
                         info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, dbr_size_n(args.type, args.count)));

            AssignEpicsValue((double) 0.0, (long) 0, args.count);
            kData.edata.precision = 0;
            kData.edata.units[0] = '\0';

            dataSize = dbr_size_n(args.type, args.count) + sizeof(char);
            if(dataSize != kData.edata.dataSize) {
                free(kData.edata.dataB);
                kData.edata.dataB = (void*) malloc(dataSize);
                kData.edata.dataSize = dataSize;
            }

            char *ptr = (char*) kData.edata.dataB;
            memcpy(ptr, val_ptr, args.count *sizeof(char));
            ptr[args.count] = '\0';

            C_SetMutexKnobDataReceived(KnobDataPtr, &kData);
        }
        break;

        case DBF_STRING:
        {
            int dataSize;
            int i;
            struct dbr_sts_string *stsF = (struct dbr_sts_string *) args.dbr;
            dbr_string_t *val_ptr = dbr_value_ptr(args.dbr, DBR_STS_STRING);

            PRINT(printf("dataCallback string %s %d <%s> %d <%s> status=%d count=%d nBytes=%d\n", ca_name(args.chid), (int) args.chid,
                         stsF->value, info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, dbr_size_n(args.type, args.count)));

            AssignEpicsValue((double) 0.0, (long) 0, args.count);
            kData.edata.precision = 0;
            kData.edata.units[0] = '\0';

            // concatenate strings separated with ';'
            dataSize = dbr_size_n(args.type, args.count) + (args.count+1) * sizeof(char);
            if(dataSize != kData.edata.dataSize) {
                free(kData.edata.dataB);
                kData.edata.dataB = (void*) malloc(dataSize);
                kData.edata.dataSize = dataSize;
            }

            char *ptr = (char*) kData.edata.dataB;
            ptr[0] = '\0';
            strcpy(ptr, val_ptr[0]);
            for (i = 1; i < args.count; i++) {
                sprintf(ptr, "%s;%s", ptr, val_ptr[i]);
            }

            C_SetMutexKnobDataReceived(KnobDataPtr, &kData);
        }
        break;

        case DBF_ENUM:
        {
            int dataSize;
            int i;
            struct dbr_ctrl_enum *stsF = (struct dbr_ctrl_enum *) args.dbr;
            PRINT(printf("dataCallback:\n""  get: %s\n", ca_message_text[CA_EXTRACT_MSG_NO(args.status)]));
            PRINT(printf("dataCallback enum  %s %d <%d> %d <%s> status=%d count=%d enum no_str=%d size=%d\n", ca_name(args.chid), (int) args.chid,
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

                char *ptr = (char*) kData.edata.dataB;
                ptr[0] = '\0';
                strcpy(ptr, stsF->strs[0]);
                for (i = 1; i < stsF->no_str; i++) {
                    sprintf(ptr, "%s;%s", ptr, stsF->strs[i]);
                }
                C_SetMutexKnobDataReceived(KnobDataPtr, &kData);
            }
        }
        break;

        case DBF_INT:
        {
            struct dbr_ctrl_int *stsF = (struct dbr_ctrl_int *) args.dbr;

            PRINT(printf("dataCallback int values %s %d %d <%s> %d <%s> status=%d count=%d size=%d\n", ca_name(args.chid), (int) args.chid,
                         stsF->value, stsF->units, info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, dbr_size_n(args.type, args.count)));

            if(info->event == 1) AssignEpicsData;
            AssignEpicsValue((double) stsF->value, (long) stsF->value, args.count);
            kData.edata.precision = 0;

            if(args.count > 1) {
                if((int) (args.count * sizeof(int)) != kData.edata.dataSize) {
                    free(kData.edata.dataB);
                    kData.edata.dataB = (void*) malloc(args.count * sizeof(int));
                    kData.edata.dataSize = args.count * sizeof(int);
                }
                memcpy(kData.edata.dataB, &stsF->value, args.count * sizeof(int));
            }

            C_SetMutexKnobDataReceived(KnobDataPtr, &kData);
        }
        break;

        case DBF_LONG:
        {
            struct dbr_ctrl_long *stsF = (struct dbr_ctrl_long *) args.dbr;

            PRINT(printf("dataCallback long values %s %d %lx <%s> %d <%s> status=%d count=%d size=%d\n", ca_name(args.chid), (int) args.chid,
                         stsF->value, stsF->units, info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, dbr_size_n(args.type, args.count)));

            if(info->event == 1) AssignEpicsData;
            AssignEpicsValue((double) stsF->value, (long) stsF->value, args.count);
            kData.edata.precision = 0;

            if(args.count > 1) {
                if((int) (args.count * sizeof(long)) != kData.edata.dataSize) {
                    free(kData.edata.dataB);
                    kData.edata.dataB = (void*) malloc(args.count * sizeof(long));
                    kData.edata.dataSize = args.count * sizeof(long);
                }
                memcpy(kData.edata.dataB, &stsF->value, args.count * sizeof(long));
            }

            C_SetMutexKnobDataReceived(KnobDataPtr, &kData);
        }
        break;

        case DBF_FLOAT:
        {
            struct dbr_ctrl_float *stsF = (struct dbr_ctrl_float *) args.dbr;

            PRINT(printf("dataCallback float values %s %d %f <%s> %d <%s> status=%d count=%d size=%d\n", ca_name(args.chid), (int) args.chid,
                         stsF->value, stsF->units, info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, dbr_size_n(args.type, args.count)));

            if(info->event == 1) {
                kData.edata.precision = stsF->precision;
                AssignEpicsData;
            }
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
            struct dbr_ctrl_double *stsF = (struct dbr_ctrl_double *) args.dbr;

            PRINT(printf("dataCallback double values %s %d %f <%s> %d <%s> status=%d count=%d size=%d\n", ca_name(args.chid), (int) args.chid,
                         stsF->value, stsF->units, info->index, ca_host_name(args.chid),
                         stsF->status, (int) args.count, dbr_size_n(args.type, args.count)));

            if(info->event == 1) {
                kData.edata.precision = stsF->precision;
                AssignEpicsData;
            }
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

        info->event++;

    }
}

/**
 * epics connect callback
 */
void connectCallback(struct connection_handler_args args)
{
    int status;

    connectInfo *info = (connectInfo *) ca_puser(args.chid);

    //printf("connectInfo pv=<%s>\n", info->pv);

    switch (ca_state(args.chid)) {

    case cs_never_conn:
        PRINT(printf("%s was never connected\n", ca_name(args.chid)));
        info->connected = false;
        break;
    case cs_prev_conn:
        PRINT(printf("%d, %s has just disconnected\n", args.chid, ca_name(args.chid)));
        info->connected = false;
        break;
    case cs_conn:
        PRINT(printf("%s has just connected with channel id=%d count=%d native type=%s\n", ca_name(args.chid), (int) args.chid, ca_element_count(args.chid), dbf_type_to_text(ca_field_type(args.chid))));
        info->connected = true;
        if (info->event == 0) {
            info->event++;
            status = ca_add_array_event( dbf_type_to_DBR_CTRL(ca_field_type(args.chid)), ca_element_count(args.chid),
                                         args.chid, dataCallback, info, 0.0,0.0,0.0, NULL);

            if (status != ECA_NORMAL) {
                PRINT(printf("ca_add_array_event:\n"" %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)]));
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
int CreateAndConnect(int index, knobData *kData, int rate)
{
    int status;
#ifdef ACS
    char aux[10];
    static int first = true;
#endif

    /* initialize channels */

    kData->index = index;

    connectInfo *tmp = (connectInfo *) 0;

    //printf("create channel for button=%d <%s>\n", index, kData->pv);

    kData->edata.monitorCount = 0;
    kData->edata.displayCount = 0;
    kData->edata.precision = 0; //default
    kData->edata.units[0] = '\0';
    kData->edata.info = (connectInfo *) malloc(sizeof (connectInfo));
    kData->edata.dataB =(void*) 0;
    kData->edata.dataSize = 0;
    kData->edata.initialize = true;

    kData->edata.repRate = rate;   // default 5 Hz, will anyhow not be higher due to scan loop

    tmp = (connectInfo *) kData->edata.info;
    strcpy(tmp->pv, kData->pv);

    tmp->connected = false;
    tmp->index = index;
    tmp->event = 0;
    tmp->ch = 0;
    tmp->cs = 0;

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

        //printf("we added an acs device <%s> %d aux=<%s>\n", kData->pv,  index, aux);
        tmp->cs = 1;                 // acs controlsystem
        // update knobdata
        C_SetMutexKnobData(KnobDataPtr, index, *kData);
        return index;
    }
#endif
    //printf("we have to add an epics device <%s>\n", kData->pv);
    status = ca_attach_context(dbCaClientContext);
    status = ca_create_channel(kData->pv,
                               (void(*)())connectCallback,
                               tmp,
                               CA_PRIORITY_DEFAULT,
                               &tmp->ch);
    if(status != ECA_NORMAL) {
        printf("ca_create_channel:\n"" %s for %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)], kData->pv);
    }

    status = ca_pend_io(CA_TIMEOUT);

    //printf("channel created for button=%d <%s> chid=%d\n", index, kData->pv, tmp->ch);

    return index;
}

/**
 * unhook the device that has been hooked here on the gui
 */
void ClearMonitor(knobData *kData)
{
    int status;

    status = ca_attach_context(dbCaClientContext);

    if (kData->index == -1) return;

    kData->index = -1;
    kData->pv[0] = '\0';

    connectInfo *tmp = (connectInfo *) kData->edata.info;
    if (tmp != (connectInfo *) 0) {
        if(tmp->cs == 0) { // epics
            if(tmp->ch != (chid) 0) {
                status = ca_clear_channel(tmp->ch);
                if(status != ECA_NORMAL) {
                    printf("ca_clear_channel:\n"" %s %s\n", ca_message_text[CA_EXTRACT_MSG_NO(status)], tmp->pv);
                }
            }
        } else {
#ifdef ACS
            PRINT(printf("delete acs channel %s index=%d\n", tmp->pv, kData->index));
            RemoveValueCell(kData->index);
#endif
        }
    }
    status = ca_pend_io(CA_TIMEOUT);
}


int EpicsSetValue(char *pv, float rdata, long idata, char *sdata, char *object, char *errmess)
{
    chid     ch;
    chtype   chType;
    struct dbr_ctrl_float ctrlR;
    struct dbr_sts_string ctrlS;

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
    int status = ca_create_channel(pv, NULL, 0, CA_PRIORITY, &ch);
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
    case DBF_STRING:
    case DBF_ENUM:
        //printf("Epicsput string for <%s> with data=%s\n", pv, sdata);
        status = ca_put(DBR_STRING, ch, sdata);
        if (status != ECA_NORMAL) {
            C_postMsgEvent(messageWindow, 1, vaPrintf("put pv (%s) %s\n", pv, ca_message (status)));
            return status;
        }
        break;

    case DBF_INT:
        //printf("Epicsput int for <%s> with data=%d\n", pv, (int) idata);
        status = ca_put(DBR_INT, ch, &idata);
        if (status != ECA_NORMAL) {
            C_postMsgEvent(messageWindow, 1, vaPrintf("put pv (%s) %s\n", pv, ca_message (status)));
            return status;
        }
        break;

    case DBF_LONG:
        //printf("Epicsput long for <%s> with data=%d\n", pv, (int) idata);
        status = ca_put(DBR_LONG, ch, &idata);
        if (status != ECA_NORMAL) {
            C_postMsgEvent(messageWindow, 1, vaPrintf("put pv (%s) %s\n", pv, ca_message (status)));
            return status;
        }
        break;

    case DBF_DOUBLE:
    case DBF_FLOAT:

        //printf("put double/float for <%s> with data=%f\n", pv, rdata);
        status = ca_put(DBR_FLOAT, ch, &rdata);
        if (status != ECA_NORMAL) {
            C_postMsgEvent(messageWindow, 1, vaPrintf("put pv (%s) %s)\n", pv, ca_message (status)));
            return status;
        }
        break;

    case DBF_CHAR:

        //printf("put char array for <%s> with <%s>\n", pv, sdata);
        status = ca_array_put(DBR_CHAR, strlen(sdata), ch, sdata);
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

    switch (chType) {

    case DBF_ENUM:
        break;

    case DBF_DOUBLE:
    case DBF_FLOAT:
        status = ca_get(DBR_CTRL_FLOAT, ch, &ctrlR);
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

void TerminateDeviceIO()
{
    ca_flush_io();
    ca_context_destroy();
    ca_task_exit();
}

/**
 * exit handler, stop data acquisition
 */
void ExitHandler(int sig)
{
    printf("Exiting with signal=%d\n", sig);
}

