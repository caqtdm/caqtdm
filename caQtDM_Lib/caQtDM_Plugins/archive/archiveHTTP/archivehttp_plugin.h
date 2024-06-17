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
 *  Copyright (c) 2024
 *
 *  Author:
 *    Erik Schwarz
 *  Contact details:
 *    erik.schwarz@psi.ch
 */

#ifndef ARCHIVEHTTP_PLUGIN_H
#define ARCHIVEHTTP_PLUGIN_H

#include <QList>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QTimer>
#include "archiverGeneral.h"
#include "controlsinterface.h"
#include "workerHttpThread.h"

class Q_DECL_EXPORT ArchiveHTTP_Plugin : public QObject, ControlsInterface
{
    Q_OBJECT

    Q_INTERFACES(ControlsInterface)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "ch.psi.caqtdm.Plugin.ControlsInterface/1.0.democontrols")
#endif

    // Everything in public: is present in ControlsInterface
public:
    ArchiveHTTP_Plugin();
    ~ArchiveHTTP_Plugin();
    QString pluginName();
    int initCommunicationLayer(MutexKnobData *data,
                               MessageWindow *messageWindow,
                               QMap<QString, QString> options);
    /*
     * Makes the plugin start updating this knobData.
     * */
    int pvAddMonitor(int index, knobData *kData, int rate, int skip);

    /*
     * Makes the plugin stop updating this knobData.
     * */
    int pvClearMonitor(knobData *kData);

    /*
     *  Frees knobData pointers allocated by the plugin.
     * */
    int pvFreeAllocatedData(knobData *kData);

    /*
     * Not implemented by this plugin. Returns true.
     * */
    int pvSetValue(char *pv,
                   double rdata,
                   int32_t idata,
                   char *sdata,
                   char *object,
                   char *errmess,
                   int forceType);

    /*
     * Not implemented by this plugin. Returns true.
     * */
    int pvSetWave(char *pv,
                  float *fdata,
                  double *ddata,
                  int16_t *data16,
                  int32_t *data32,
                  char *sdata,
                  int nelm,
                  char *object,
                  char *errmess);

    /*
     * Not implemented by this plugin. Returns true.
     * */
    int pvGetTimeStamp(char *pv, char *timestamp);

    /*
     * Fills the description to the pv containing performance data from the last request.
     * */
    int pvGetDescription(char *pv, char *description);

    /*
     * Not implemented by this plugin. Returns true.
     * */
    int pvClearEvent(void *keyPtr);

    /*
     * Not implemented by this plugin. Returns true.
     * */
    int pvAddEvent(void *keyPtr);

    /*
     * Not implemented by this plugin. Returns true.
     * */
    int pvReconnect(knobData *kData);

    /*
     * Not implemented by this plugin. Returns true.
     * */
    int pvDisconnect(knobData *kData);

    /*
     * Not implemented by this plugin. Returns true.
     * */
    int FlushIO();

    /*
     * Not implemented by this plugin. Returns true.
     * */
    int TerminateIO();

    // From hereon downwards everything is new, so not available through ControlsInterface (without cast)
public slots:
    /*
     * Processes received data, updates the cartesianPlot and destroys the worker & workerThread, if isFinalIteration is true.
     * */
    void handleResults(indexes indexNew, int valueCount, QVector<double> XVals, QVector<double> YVals, QVector<double> YMinVals, QVector<double> YMaxVals, QString backend, bool isFinalIteration);

signals:
    void operate(const indexes, const QString, MessageWindow *, MutexKnobData *, QSharedPointer<HttpPerformanceData>);
    void Signal_StopUpdateInterface();

private slots:
    /*
     * This routine is called everytime new knobData entries are to be updated.
     * For each entry, it makes sure no redundant data is requested, and creates a new worker & workerThread to retrieve the data.
     * */
    void Callback_UpdateInterface(QMap<QString, indexes> listOfIndexes);

    /*
     * Tells all currently processing requests and soon to be processed requests to return early and abort.
     * */
    void Callback_AbortOutstandingRequests(QString key);

    /*
     * Stops any more update from being triggered.
     * */
    void closeEvent();

private:
    /*
     * Updates the cartesianPlot by adding the new values to the already existing values.
     * This also removes any old values that are outdated and not used anymore.
     * */
    void updateCartesianAppended(int numberOfValues,
                         indexes indexNew,
                         QVector<double> XValues,
                         QVector<double> YValues,
                         QString backend);

    QMutex m_globalMutex;
    MutexKnobData *m_mutexKnobDataP;
    MessageWindow *m_messageWindowP;
    ArchiverGeneral *m_archiverGeneral;
    QMap<QString, WorkerHttpThread*> m_listOfThreads;
    QMap<QString, indexes> m_IndexesToUpdate;
    QRegularExpression m_regexStr;
    bool m_IsSuspended;
    QMap<QString, QSharedPointer<HttpPerformanceData> > m_retrievalPerformancePerPV;
    QStringList availableBackends;
};

#endif
