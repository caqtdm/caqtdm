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
 *  Copyright (c) 2010 - 2015
 *
 *  Author:
 *    Helge Brands
 *  Contact details:
 *    helge.brands@psi.ch
 */
#ifndef BSREAD_DECODE_H
#define BSREAD_DECODE_H

#include <QThread>
#include <QThreadPool>
#include <QList>
#include <QAtomicInt>
#include "knobData.h"
#include "mutexKnobData.h"
#include "bsread_channeldata.h"
#include "bsread_wfhandling.h"

class bsread_Decode : public QObject
{
    Q_OBJECT
public:
    bsread_Decode(void * Context,QString ConnectionPoint);
    bsread_Decode(void * Context,QString ConnectionPoint,QString ConnectionType);
    ~bsread_Decode();

    void *getZmqsocket() const;

    QString getConnectionPoint() const;

    bool getRunning_decode() const;

    MutexKnobData *getKnobData() const;
    void setKnobData(MutexKnobData *value);
    size_t getMessage_size() const;

    QString getMainHeader() const;
    bool setMainHeader(char *value, size_t size);
    void setHeader(char *value, size_t size);
    bool bsread_DataMonitorConnection(QString channel,int index);
    bool bsread_DataMonitorConnection(knobData *kData);
    bool bsread_DataMonitorUnConnect(knobData *kData);
    void setTerminate();
    void bsread_createConnection(int rc);
    QString getStreamConnectionPoint() const;

public slots:
    void process();
signals:
    void finished();
private:
    QMutex mutex;
    void * context;
    void * zmqsocket;
    QString StreamConnectionPoint;
    QString StreamConnectionType;
    bool running_decode;
    QString ConnectionPoint;
    MutexKnobData * bsread_KnobDataP;
    size_t message_size;
    QString MainHeader;
    long global_timestamp_epoch;
    long global_timestamp_ns;
    long global_timestamp_sec;
    long global_timestamp_ns_offset;
    double pulse_id;
    QString main_htype;
    QString main_reconnect_adress;
    QString data_htype;
    QString hash;
    QString ChannelHeader;
    int channelcounter;
    QList<bsread_channeldata*> Channels;
    QMap<QString,bsread_channeldata*> ChannelSearch;

    QThreadPool* UpdaterPool;
    QThreadPool* BlockPool;

    QList<QThread> WfDataHandlerHandler;
    QList<bsread_wfhandling*> WfDataHandlerQueue;
    QMutex WfDataHandlerLocker;

    QList<int> listOfIndexes;
    QList<QString> listOfRequestedChannels;
    void bsread_SetChannelData(void *message, size_t size);
    void bsread_SetChannelTimeStamp(void * timestamp);
    void bsread_InitHeaderChannels();
    void bsread_TransferHeaderData();
    void bsread_EndofData();
    bool terminate;


    void bsread_DataTimeOut();
    void bsread_Delay();
    void bsread_SetData(bsread_channeldata *Data, void *message, size_t size);
    void WaveformManagment(knobData *kData, bsread_channeldata *bsreadPV);
    void bsdata_assign_single(void *message, bsread_channeldata* Data);
    void bsdata_assign_single(bsread_channeldata* Data, void *message, int *datatypesize);
};

#endif // BSREAD_DECODE_H
