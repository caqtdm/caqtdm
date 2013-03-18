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

#ifndef ALARMS_H
#define ALARMS_H

enum Alarms {NO_ALARM=0, MINOR_ALARM, MAJOR_ALARM, INVALID_ALARM, NOTCONNECTED=99};

char AlarmStatusStrings[][30] = {"OK", "READ_ALARM", "WRITE_ALARM", "HIHI_ALARM", "HIGH_ALARM", "LOLO_ALARM", "LOW_ALARM", "STATE_ALARM", "COS_ALARM",
                          "COMM_ALARM", "TIMEOUT_ALARM", "HW_LIMIT_ALARM", "CALC_ALARM", "SCAN_ALARM", "LINK_ALARM", "SOFT_ALARM", "BAD_SUB_ALARM",
                          "UDF_ALARM", "DISABLE_ALARM", "SIMM_ALARM", "READ_ACCESS_ALARM", "WRITE_ACCESS_ALARM", "ALARM_NSTATUS"};

#endif
