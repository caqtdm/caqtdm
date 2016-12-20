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
#include <QDebug>
#include <QThread>
#include "archiveCA_plugin.h"
#include "archiverCommon.h"

static int precision;
static RawValue::NumberFormat format = RawValue::DEFAULT;

#define qasc(x) x.toLatin1().constData()

// gives the plugin name back
QString ArchiveCA_Plugin::pluginName()
{
    return "archiveCA";
}

// constructor
ArchiveCA_Plugin::ArchiveCA_Plugin()
{
    qDebug() << "ArchiveCA_Plugin: Create (epics channel archiver retrieval)";
    archiverCommon = new ArchiverCommon();

    connect(archiverCommon, SIGNAL(Signal_UpdateInterface(QMap<QString, indexes>)), this,SLOT(Callback_UpdateInterface(QMap<QString, indexes>)));
}

// init communication
int ArchiveCA_Plugin::initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow, QMap<QString, QString> options)
{
    mutexknobdataP = data;
    messagewindowP = messageWindow;
    return archiverCommon->initCommunicationLayer(data, messageWindow, options);
}

void ArchiveCA_Plugin::format_time(const epicsTime &time, stdString &text,  time_t &timeStamp)
{
    epicsTimeStamp stamp = time;
    stamp.nsec = ((stamp.nsec + 500000) / 1000000) * 1000000;
    epicsTimeToTime_t(&timeStamp, &stamp);
    epicsTime2string(epicsTime(stamp), text);
    text = text.substr(0, 23);
    return;
}

// this routine will be called now every 10 seconds to update the cartesianplot
void ArchiveCA_Plugin::Callback_UpdateInterface( QMap<QString, indexes> listOfIndexes)
{
    struct timeb now;
    QMutexLocker locker(&mutex);
    struct tm  *timess_end, *timess_start;
    time_t timeStamp;
    char  startTime[200], endTime[200];
    AutoPtr<epicsTime> start, end;
    precision = 5;
    format = RawValue::DECIMAL;

    //qDebug() << "====================== ArchiveCA_Plugin::Callback_UpdateInterface";

    QMap<QString, indexes>::const_iterator i = listOfIndexes.constBegin();

    while (i != listOfIndexes.constEnd()) {
        QVector<double> TimerN;
        QVector<double> YValsN;

        indexes indexNew = i.value();
        //qDebug() << i.key() << ": " << indexNew.indexX << indexNew.indexY << indexNew.pv << endl;

        int nbVal = 0;

        ftime(&now);
        //qDebug() << "get from sf archive" << key;
        time_t endSeconds = (time_t) ((double) now.time + (double) now.millitm / (double)1000);
        time_t startSeconds = (time_t) (endSeconds - indexNew.secondsPast);

        timess_end = localtime(&endSeconds);
        sprintf(endTime,   "%02d/%02d/%04d %02d:%02d:%02d ", timess_end->tm_mon+1, timess_end->tm_mday, timess_end->tm_year+1900,
                timess_end->tm_hour, timess_end->tm_min, timess_end->tm_sec);
        timess_start = localtime(&startSeconds);
        sprintf(startTime,   "%02d/%02d/%04d %02d:%02d:%02d ", timess_start->tm_mon+1, timess_start->tm_mday, timess_start->tm_year+1900,
                timess_start->tm_hour, timess_start->tm_min, timess_start->tm_sec);
        //qDebug() << startTime << endTime;

        end = new epicsTime();
        start = new epicsTime();
        string2epicsTime(endTime, *end);
        string2epicsTime(startTime, *start);

        // Index name
        stdString index_name =  "/gfa/archiver-data/archive_PRO_ST/index";

        if(caCartesianPlot* w = qobject_cast<caCartesianPlot *>((QWidget*) indexNew.w)) {
            QVariant var = w->property("archiverIndex");
            if(!var.isNull()) {
                QString indexName = var.toString();
                index_name = qasc(indexName);
            } else {
                QString mess("Archive plugin -- no archiverIndex defined as dynamic property in widget, defaulting to /gfa/archiver-data/archive_PRO_ST/index");
                if(messagewindowP != (MessageWindow *) 0) messagewindowP->postMsgEvent(QtWarningMsg, (char*) qasc(mess));
            }
        }

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
           return;
        }

        // get data
        SpreadsheetReader sheet(index, how, delta);
        bool ok = sheet.find(names, start);
        stdString time, stat, val;
        const RawValue::Data *value;

        // resize arrays
        TimerN.resize(10*indexNew.secondsPast);
        YValsN.resize(10*indexNew.secondsPast);

        nbVal = 0;
        while (ok) {
            if (end && sheet.getTime() >= *end) break;
            format_time(sheet.getTime(), time, timeStamp);

            value = sheet.get(0);
            if (value) {
                RawValue::getStatus(value, stat);
                if (RawValue::isInfo(value)) {
                    qDebug() << "no data";
                } else {
                    RawValue::getValueString( val, sheet.getType(0), sheet.getCount(0), value, &sheet.getInfo(0), format, precision);
                    QString value(val.c_str());
                    if((timeStamp - endSeconds) >= -indexNew.secondsPast) {
                       TimerN[nbVal] = (timeStamp - endSeconds) / 3600.0;
                       YValsN[nbVal++] = value.toDouble();
                    }
                }
            } else {
                qDebug() << "no data";
            }
            ok = sheet.next();
        }

        //close index
        index.close();
        if(nbVal > 0) archiverCommon->updateCartesian(nbVal, indexNew, TimerN, YValsN);
        ++i;
    }
}


// define data to be called
int ArchiveCA_Plugin::pvAddMonitor(int index, knobData *kData, int rate, int skip) {
    return archiverCommon->pvAddMonitor(index, kData, rate, skip);
}
// clear routines
int ArchiveCA_Plugin::pvClearMonitor(knobData *kData) {
    return archiverCommon->pvClearMonitor(kData);
}
int ArchiveCA_Plugin::pvFreeAllocatedData(knobData *kData) {
    return archiverCommon->pvFreeAllocatedData(kData);
}
int ArchiveCA_Plugin::TerminateIO() {
    return archiverCommon->TerminateIO();
}

// =======================================================================================================================================================
int ArchiveCA_Plugin::pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType) {
    Q_UNUSED(pv); Q_UNUSED(rdata); Q_UNUSED(idata); Q_UNUSED(sdata); Q_UNUSED(object); Q_UNUSED(errmess); Q_UNUSED(forceType);
    return true;
}
int ArchiveCA_Plugin::pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess) {
    Q_UNUSED(pv); Q_UNUSED(fdata); Q_UNUSED(ddata); Q_UNUSED(data16); Q_UNUSED(data32); Q_UNUSED(sdata); Q_UNUSED(nelm); Q_UNUSED(object); Q_UNUSED(errmess);
    return true;
}
int ArchiveCA_Plugin::pvGetTimeStamp(char *pv, char *timestamp) {
    Q_UNUSED(pv); Q_UNUSED(timestamp);
    return true;
}
int ArchiveCA_Plugin::pvGetDescription(char *pv, char *description) {
    Q_UNUSED(pv); Q_UNUSED(description);
    return true;
}
int ArchiveCA_Plugin::pvClearEvent(void * ptr) {
    Q_UNUSED(ptr);
    return true;
}
int ArchiveCA_Plugin::pvAddEvent(void * ptr) {
    Q_UNUSED(ptr);
    return true;
}
int ArchiveCA_Plugin::pvReconnect(knobData *kData) {
    Q_UNUSED(kData);
    return true;
}
int ArchiveCA_Plugin::pvDisconnect(knobData *kData) {
    Q_UNUSED(kData);
    return true;
}
int ArchiveCA_Plugin::FlushIO() {
    return true;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#else
Q_EXPORT_PLUGIN2(ArchiveCA_Plugin, ArchiveCA_Plugin)
#endif

