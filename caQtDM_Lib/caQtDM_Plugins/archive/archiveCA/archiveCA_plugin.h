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
#ifndef ArchiveCA_Plugin_H
#define ArchiveCA_Plugin_H

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QList>
#include <QTimer>
#include <QThread>
#include <qwt.h>
#include "cacartesianplot.h"
#include "controlsinterface.h"
#include "archiveCA_plugin.h"
#include "archiverCommon.h"

// Base
#include <epicsVersion.h>
// Tools
#include <AutoPtr.h>
#include <BinaryTree.h>
#include <RegularExpression.h>
#include <epicsTimeHelper.h>
#include <ArgParser.h>
// Storage
#include <SpreadsheetReader.h>
#include <AutoIndex.h>

#include <time.h>
#include <sys/timeb.h>

static int precision;
static RawValue::NumberFormat format = RawValue::DEFAULT;

class Q_DECL_EXPORT WorkerCA : public QObject
{
    Q_OBJECT

public:
    WorkerCA() {
        qRegisterMetaType<indexes>("indexes");
        qRegisterMetaType<stdString>("stdString");
        qRegisterMetaType<QVector<double> >("QVector<double>");
    }

private:
    QVector<double>  TimerN, YValsN;

    void format_time(const epicsTime &time, stdString &text,  time_t &timeStamp)
    {
        epicsTimeStamp stamp = time;
        stamp.nsec = ((stamp.nsec + 500000) / 1000000) * 1000000;
        epicsTimeToTime_t(&timeStamp, &stamp);
        epicsTime2string(epicsTime(stamp), text);
        text = text.substr(0, 23);
        return;
    }

public slots:

    void workerFinish() {
        //qDebug() << "worker finish";
        deleteLater();
    }

    void getFromArchive(QWidget *w, indexes indexNew,  stdString index_name) {

        time_t timeStamp;
        struct timeb now;
        struct tm  *timess_end, *timess_start;
        char  startTime[200], endTime[200];

        format = RawValue::DECIMAL;
        time_t endSeconds, startSeconds;
        MessageWindow *messagewindowP = (MessageWindow *) w;

        QMutex *mutex = indexNew.mutexP;
        mutex->lock();

        ftime(&now);
        endSeconds = (time_t) ((double) now.time + (double) now.millitm / (double)1000);
        startSeconds = (time_t) (endSeconds - indexNew.secondsPast);

        timess_end = localtime(&endSeconds);
        sprintf(endTime,   "%02d/%02d/%04d %02d:%02d:%02d ", timess_end->tm_mon+1, timess_end->tm_mday, timess_end->tm_year+1900,
                timess_end->tm_hour, timess_end->tm_min, timess_end->tm_sec);
        timess_start = localtime(&startSeconds);
        sprintf(startTime,   "%02d/%02d/%04d %02d:%02d:%02d ", timess_start->tm_mon+1, timess_start->tm_mday, timess_start->tm_year+1900,
                timess_start->tm_hour, timess_start->tm_min, timess_start->tm_sec);


        AutoPtr<epicsTime> start, end;
        end = new epicsTime();
        start = new epicsTime();

        string2epicsTime(endTime, *end);
        string2epicsTime(startTime, *start);

        // Channel names
        stdVector<stdString> names;
        names.push_back(qasc(indexNew.pv));

        // How?
        ReaderFactory::How how = ReaderFactory::Raw;
        double delta = 0.0;

        // Open index
        AutoIndex index;
        char error[1000];
        if(!index.open(index_name.c_str(), error, true)) {
            if(messagewindowP != (MessageWindow *) 0) messagewindowP->postMsgEvent(QtCriticalMsg, error);
            emit resultReady(indexNew, 0, TimerN, YValsN , "");
        }

        // get data
        SpreadsheetReader sheet(index, how, delta);
        bool ok = sheet.find(names, start);
        stdString time, stat, val;
        const RawValue::Data *value;

        // resize arrays
        TimerN.clear();
        YValsN.clear();
        TimerN.resize(10*indexNew.secondsPast);
        YValsN.resize(10*indexNew.secondsPast);

        int nbVal = 0;
        while (ok) {
            if (end && sheet.getTime() >= *end) break;
            format_time(sheet.getTime(), time, timeStamp);

            value = sheet.get(0);
            if (value) {
                RawValue::getStatus(value, stat);
                if (RawValue::isInfo(value)) {
                    //qDebug() << "no data";
                } else {
                    RawValue::getValueString( val, sheet.getType(0), sheet.getCount(0), value, &sheet.getInfo(0), format, precision);
                    QString value(val.c_str());
                    if((timeStamp - endSeconds) >= -indexNew.secondsPast) {
                        TimerN[nbVal] = (timeStamp - endSeconds) / 3600.0;
                        YValsN[nbVal] = value.toDouble();
                        nbVal++;
                    }
                }
            } else {
                //qDebug() << "no data";
            }
            ok = sheet.next();
        }
        emit resultReady(indexNew, nbVal, TimerN, YValsN, "");

        mutex->unlock();
    }

signals:
    void resultReady(indexes indexNew, int nbVal, QVector<double> TimerN, QVector<double> YValsN, QString backend);

public:

};

class Q_DECL_EXPORT ArchiveCA_Plugin : public QObject, ControlsInterface
{
    Q_OBJECT

    Q_INTERFACES(ControlsInterface)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "ch.psi.caqtdm.Plugin.ControlsInterface/1.0.democontrols")
#endif

public:
    QString pluginName();
    ArchiveCA_Plugin();

    int initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow, QMap<QString, QString> options);
    int pvAddMonitor(int index, knobData *kData, int rate, int skip);
    int pvClearMonitor(knobData *kData);
    int pvFreeAllocatedData(knobData *kData);
    int pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType);
    int pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess);
    int pvGetTimeStamp(char *pv, char *timestamp);
    int pvGetDescription(char *pv, char *description);
    int pvClearEvent(void * ptr);
    int pvAddEvent(void * ptr);
    int pvReconnect(knobData *kData);
    int pvDisconnect(knobData *kData);
    int FlushIO();
    int TerminateIO();

protected:

private slots:
    void updateValues();
    void Callback_UpdateInterface( QMap<QString, indexes> listOfIndexes);

public slots:
    void handleResults(indexes, int, QVector<double>, QVector<double>, QString backend);
    void closeEvent();

signals:
    void operate(QWidget*, const indexes, const stdString);
    void Signal_StopUpdateInterface();

private:
    QMutex mutex;
    MutexKnobData *mutexknobdataP;
    MessageWindow *messagewindowP;
    ArchiverCommon *archiverCommon;
    QMap<QString, QThread*> listOfThreads;
};

#endif
