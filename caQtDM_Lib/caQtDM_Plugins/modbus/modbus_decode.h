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
 *  Copyright (c) 2010 - 2020
 *
 *  Author:
 *    Helge Brands
 *  Contact details:
 *    helge.brands@psi.ch
 */
#ifndef MODBUS_DECODE_H
#define MODBUS_DECODE_H

#include <QtCore>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QModbusTcpClient>
#include "knobData.h"
#include "mutexKnobData.h"
#include "modbus_channeldata.h"

#define MODBUS_ERROR -1
#define MODBUS_OK 0

#define MODBUS_MAX_SEGMENT_SIZE 123

enum modbus_calc_direction {modbus_INVALID = 0, modbus_READ = 1, modbus_WRITE = 2};

class modbus_decode : public QObject
{
    Q_OBJECT
public:
    explicit modbus_decode(QObject *parent = Q_NULLPTR);
    MutexKnobData *getKnobData() const{return mutexknobdataP;}
    void setKnobData(MutexKnobData *value){mutexknobdataP=value;}
    void setMessageWindow(MessageWindow *value){messagewindowP=value;}
    QString removeHost(QString pv);
    QString removeEPICSExtensions(QString pv);
    QString getEPICSExtensions(QString pv);

    QModbusDataUnit* generateDataUnit(QString pv);
    caType generatecaDataType(QString pv);


    int getModbusretries() const;
    void setModbusretries(int value);

    int getModbustimeout() const;
    void setModbustimeout(int value);

    QUrl getModbustarget() const;
    void setModbustarget(const QUrl &value);

    int pvAddMonitor(int index, knobData *kData);
    int pvDisconnect(knobData *kData);
    bool pvClearMonitor(knobData *kData);


    void setTerminate();
    int pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType);
    int pvGetTimeStamp(char *pv, char *timestamp);

    void setModbus_translation_map(const QMap<QString, QString> &value);
    int  do_the_calculation(QString modbus_connection,QModbusDataUnit* unit,knobData *kData,modbus_calc_direction direction);

public slots:
    void process();
    void devicestate_changed(QModbusDevice::State state);
    void trigger_modbusrequest();
    void device_reply_data();
    void handle_TerminateIO();
    void handle_pvReconnect(knobData *kData);
    void handle_createTimer(int modbus_cycle);

signals:
    void finished();
    void TerminateIO();
    void pvReconnect(knobData *kData);
    void create_Timer(int modbus_cycle);

private:
    QMutex mutex;
    QMutex writeData_mutex;
    QEventLoop* loop;
    MessageWindow *messagewindowP;
    MutexKnobData *mutexknobdataP;
    QMap<QString, QString> modbus_translation_map;
    QUrl modbustargetP;
    QModbusTcpClient* device;
    QModbusDevice::State device_state;
    QMap<QString,modbus_channeldata*> readData;
    QMap<QString,modbus_channeldata*> readData_disabledMonitor;
    QMap<int,QTimer*> running_Timer;

    ///QList<modbus_channeldata*> writeData;
    QList<QPair<QString, QModbusDataUnit*>> writeData;

    int modbustimeout;
    int modbusretries;
    bool modbus_disabled;
    bool modbus_terminate;
};
enum Alarms {NO_ALARM=0, MINOR_ALARM, MAJOR_ALARM, INVALID_ALARM, NOTCONNECTED=99};

#endif // MODBUS_DECODE_H
