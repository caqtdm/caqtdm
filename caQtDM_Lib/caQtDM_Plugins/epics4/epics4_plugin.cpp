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
#include <QString>
#include "epics4_plugin.h"

QString Epics4Plugin::pluginName()
{
    return "epics4";
}

Epics4Plugin::Epics4Plugin()
{
    qDebug() << "Epics4Plugin: Create";
}

int Epics4Plugin::initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow)
{
    mutexknobdataP = data;
    messagewindowP = messageWindow;

    qDebug() << "Epics4Plugin: InitCommunicationLayer";
#ifdef EPICS4
    Epics4 = new epics4Subs(data);
#endif
    return true;
}

int Epics4Plugin::pvAddMonitor(int index, knobData *kData, int rate, int skip) {
    qDebug() << "Epics4Plugin:pvAddMonitor" << kData->pv << kData->index;
#ifdef EPICS4
    Epics4->CreateAndConnect4(index, kData->pv);
#endif
    return true;
}

int Epics4Plugin::pvClearMonitor(knobData *kData) {
    qDebug() << "Epics4Plugin:pvClearMonitor";
    return true;
}

int Epics4Plugin::pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType) {

    qDebug() << "Epics4Plugin:pvSetValue" << pv << rdata << idata << sdata;
    QString from = QString::number(rdata);
#ifdef EPICS4
    Epics4->Epics4SetValue(QString(pv), from);
#endif
    return true;
}

int Epics4Plugin::pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess) {
    qDebug() << "Epics4Plugin:pvSetWave";
    return true;
}

int Epics4Plugin::pvGetTimeStamp(char *pv, char *timestamp) {
    qDebug() << "Epics4Plugin:pvgetTimeStamp";
    return true;
}

int Epics4Plugin::pvGetDescription(char *pv, char *description) {
    qDebug() << "Epics4Plugin:pvGetDescription";
    return true;
}

int Epics4Plugin::pvClearEvent(void * ptr) {
    qDebug() << "Epics4Plugin:pvClearEvent";
    return true;
}

int Epics4Plugin::pvAddEvent(void * ptr) {
    qDebug() << "Epics4Plugin:pvAddEvent";
    return true;
}

int Epics4Plugin::pvReconnect(knobData *kData) {
    qDebug() << "Epics4Plugin:pvReconnect";
    return true;
}

int Epics4Plugin::pvDisconnect(knobData *kData) {
    qDebug() << "Epics4Plugin:pvDisconnect";
    return true;
}

int Epics4Plugin::FlushIO() {
    //qDebug() << "Epics4Plugin:FlushIO";
    return true;
}

int Epics4Plugin::TerminateIO() {
    //qDebug() << "Epics4Plugin:TerminateIO";
#ifdef EPICS4
    delete Epics4;
#endif
    return true;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#else
    Q_EXPORT_PLUGIN2(Epics4Plugin, Epics4Plugin)
#endif

