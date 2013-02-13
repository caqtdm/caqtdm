#ifndef ALARMS_H
#define ALARMS_H

enum Alarms {NO_ALARM=0, MINOR_ALARM, MAJOR_ALARM, ALARM_INVALID, NOTCONNECTED=99};

#define AL_GREEN QColor(0x00, 0xcd, 0x00)
#define AL_YELLOW QColor(0xff, 0xff, 0x00)
#define AL_RED QColor(0xff, 0x00, 0x00)
#define AL_WHITE QColor(0xff, 0xff, 0xff)
#define AL_DEFAULT QColor(0x88, 0x88, 0x88)

#endif
