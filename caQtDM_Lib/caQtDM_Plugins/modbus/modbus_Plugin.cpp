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
#include <QDebug>
#include <QApplication>
#include <QVariant>
#include <QModbusTcpClient>
#include <QSettings>
#include "modbus_plugin.h"
#include "searchfile.h"
#include "fileFunctions.h"

// as defined in knobDefines.h
//caType {caSTRING	= 0, caINT = 1, caFLOAT = 2, caENUM = 3, caCHAR = 4, caLONG = 5, caDOUBLE = 6};


// gives the plugin name back
QString modbusPlugin::pluginName()
{
    return "modbus";
}

// constructor
modbusPlugin::modbusPlugin()
{
    qDebug() << "modbusPlugin: Create";

    mutexknobdataP = Q_NULLPTR;
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(closeEvent()));


}
modbusPlugin:: ~modbusPlugin()
{


}


// initialize our communicationlayer with everything you need
int modbusPlugin::initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow,QMap<QString, QString> options)
{
    qDebug() << "modbusPlugin: InitCommunicationLayer with options" << options;

    mutexknobdataP = data;
    messagewindowP = messageWindow;
    optionsP=options;

    QStringList modbus_database_files;

    QString url = (QString)  qgetenv("CAQTDM_URL_DISPLAY_PATH");
    QString database_file = (QString)  qgetenv("CAQTDM_MODBUS_DATABASE");

    modbus_database_files.append(database_file.split(","));

    if (!optionsP.value("MODBUS_DATABASE","").isEmpty())
        modbus_database_files.append(optionsP.value("MODBUS_DATABASE",""));
    fileFunctions filefunction;
    //qDebug() <<"+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
    foreach (QString modbus_database_file,modbus_database_files) {

        if (!url.isEmpty()){
            //qDebug() << "TestDownlaod:" <<modbus_database_file;
            filefunction.checkFileAndDownload(modbus_database_file, url);
        }
        searchFile *s = new searchFile(modbus_database_file);
        QString fileNameFound = s->findFile();
        //qDebug() << "fileNameFound:" <<fileNameFound;
        delete s;
        if (!fileNameFound.isEmpty()){
            QFile file(fileNameFound);
            if(file.open(QIODevice::ReadOnly)) {
                QString msg="modbus translation found : ";
                msg.append(fileNameFound);
                if(messagewindowP != Q_NULLPTR) messagewindowP->postMsgEvent(QtDebugMsg,(char*) msg.toLatin1().constData());

                QTextStream in(&file);

                while(!in.atEnd()) {
                    QString line = in.readLine();
                    if (!line.trimmed().startsWith("#")){
                        QStringList list = line.split("=");
                        if (list.length()>1){
                            modbus_translation_map.insert(list.at(0).trimmed(),list.at(1).trimmed());
                        }
                    }
                }

                file.close();
            }
            //qDebug() <<"+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
        }
    }




    return true;
}

QString modbusPlugin::removeEPICSExtensions(QString pv)
{
   QString chan_desc=pv;
   chan_desc=chan_desc.remove(".FTVL",Qt::CaseInsensitive);
   chan_desc=chan_desc.remove(".EGU",Qt::CaseInsensitive);
   chan_desc=chan_desc.remove(".NELM",Qt::CaseInsensitive);
   chan_desc=chan_desc.remove(".NORD",Qt::CaseInsensitive);

   return chan_desc;
}


// caQtDM_Lib will call this routine for defining a monitor
//modbus://129.129.130.73:502{type:"coils",addr:0,count:1,datatype:"float",rcalc:"A*10",wcalc:"A/10",egu:"Bla",cycle:10}
//r,wcalc : read/write calc to register
//DiscreteInputs,D
//Coils,C
//InputRegisters,I
//HoldingRegisters,H

int modbusPlugin::pvAddMonitor(int index, knobData *kData, int rate, int skip) {
    Q_UNUSED(index)
    Q_UNUSED(rate)
    Q_UNUSED(skip)

    int pos;

    QMutexLocker locker(&mutex);
    //qDebug() << "modbusPlugin:pvAddMonitor" << kData->pv << kData->index << kData;




    QString target=kData->pv;
    QString replace=modbus_translation_map.value(removeEPICSExtensions(target),"");
    if (!replace.isEmpty()){
        target=target.replace(removeEPICSExtensions(target),modbus_translation_map.value(removeEPICSExtensions(target)));
    }




    pos=target.indexOf("{");
    if(pos != -1) {
     target.truncate(target.indexOf("{"));
    }

    modbus_decode* connector=Q_NULLPTR;
    QMap<QString, QPointer<modbus_decode> >::iterator i = modbusconnections.find(target);
    while (i !=modbusconnections.end() && i.key() == target) {
        connector = i.value();
        break;
    }
    if (!connector){
        qDebug() << "create new decode" << target;
       connector = new modbus_decode();
       //qDebug() << "Target" << target << QUrl::fromUserInput(target);
       connector->setModbustarget(QUrl::fromUserInput(target));


       modbusconnections.insert(target,connector);



       modbusThreads.append(new QThread(this));
       connector->setKnobData(mutexknobdataP);
       connector->setMessageWindow(messagewindowP);
       connector->setModbus_translation_map(modbus_translation_map);
       connector->moveToThread(modbusThreads.last());
       connect(modbusThreads.last(), SIGNAL(started()), connector, SLOT(process()));
       connect(connector, SIGNAL(finished()), modbusThreads.last(), SLOT(quit()));
       connect(modbusThreads.last(), SIGNAL(finished()), modbusThreads.last(), SLOT(deleteLater()));
       connect(connector, SIGNAL(finished()),connector, SLOT(deleteLater()));
       modbusThreads.last()->start();
       QString msg="modbus connection started: ";
       disconnect(connector);
       msg.append(target);
       if(messagewindowP != Q_NULLPTR) messagewindowP->postMsgEvent(QtDebugMsg,(char*) msg.toLatin1().constData());
    }else{
      //qDebug() << "reuse decode" << target;
    }


    return connector->pvAddMonitor(index, kData);

}

// caQtDM_Lib will call this routine for getting rid of a monitor
int modbusPlugin::pvClearMonitor(knobData *kData) {


    QMutexLocker locker(&mutex);
    QList<QString> usedkeys=modbusconnections.keys();
    foreach (QString index,usedkeys){
        QMap<QString,QPointer<modbus_decode>>::iterator i = modbusconnections.find(index);
        while (i !=modbusconnections.end() && i.key() == index) {
            modbus_decode* connector=i.value();
            connector->pvClearMonitor(kData);
            ++i;
        }
    }

    return true;
}
int modbusPlugin::pvFreeAllocatedData(knobData *kData)
{
    Q_UNUSED(kData)
    //qDebug() << "DemoPlugin:pvFreeAllocatedData";
    //if (kData->edata.info != (void *) Q_NULLPTR) {
        //free(kData->edata.info);
     //   kData->edata.info = (void*) Q_NULLPTR;
    //}
   // if(kData->edata.dataB != (void*) Q_NULLPTR) {
        //free(kData->edata.dataB);
  //      kData->edata.dataB = (void*) Q_NULLPTR;
  //  }

    return true;
}

// caQtDM_Lib will call this routine for setting data (see for more detail the epics3 plugin)
int modbusPlugin::pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType) {
    Q_UNUSED(forceType);
    Q_UNUSED(errmess);
    Q_UNUSED(object);
    QMutexLocker locker(&mutex);
    QList<QPointer<modbus_decode>> connectors=modbusconnections.values();
    foreach(QPointer<modbus_decode> connector, connectors) {
       connector->pvSetValue(pv,rdata,idata,sdata,object,errmess,forceType);
    }
    //qDebug() << "modbusPlugin:pvSetValue" << pv << rdata << idata << sdata;
    return 1;
}

// caQtDM_Lib will call this routine for setting waveforms data (see for more detail the epics3 plugin)
int modbusPlugin::pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess) {
    Q_UNUSED(pv);
    Q_UNUSED(fdata);
    Q_UNUSED(ddata);
    Q_UNUSED(data16);
    Q_UNUSED(data32);
    Q_UNUSED(sdata);
    Q_UNUSED(nelm);
    Q_UNUSED(object);
    Q_UNUSED(errmess);

    QMutexLocker locker(&mutex);
    qDebug() << "modbusPlugin:pvSetWave";
    return false;
}

// caQtDM_Lib will call this routine for getting a description of the monitor
int modbusPlugin::pvGetTimeStamp(char *pv, char *timestamp) {
    QList<QPointer<modbus_decode>> connectors=modbusconnections.values();
    foreach(QPointer<modbus_decode> connector, connectors) {
       connector->pvGetTimeStamp(pv,timestamp);
    }
    return true;
}

// caQtDM_Lib will call this routine for getting the timestamp for this monitor
int modbusPlugin::pvGetDescription(char *pv, char *description) {
    Q_UNUSED(pv);
    qDebug() << "modbusPlugin:pvGetDescription";
    qstrcpy(description, "no Description available for MODBUS data transfer");
    return true;
}

// next routines are used to stop and restart the dataacquisition (used in case of tabWidgets in the display)
int modbusPlugin::pvClearEvent(void * ptr) {
    Q_UNUSED(ptr);
    qDebug() << "modbusPlugin:pvClearEvent";
    return true;
}

int modbusPlugin::pvAddEvent(void * ptr) {
    Q_UNUSED(ptr);
    qDebug() << "modbusPlugin:pvAddEvent";
    return true;
}

// next routines are used to Connect and disconnect monitors
int modbusPlugin::pvReconnect(knobData *kData) {
     Q_UNUSED(kData);
    qDebug() << "modbusPlugin:pvReconnect";
    QMutexLocker locker(&mutex);
    QList<QPointer<modbus_decode>> connectors=modbusconnections.values();
    foreach(QPointer<modbus_decode> connector, connectors) {
       emit connector->pvReconnect(kData);
    }


    return true;
}

int modbusPlugin::pvDisconnect(knobData *kData) {
    qDebug() << "modbusPlugin:pvDisconnect";
    QMutexLocker locker(&mutex);
    QList<QString> usedkeys=modbusconnections.keys();
    foreach (QString index,usedkeys){
        modbus_decode* connector=Q_NULLPTR;
        QMap<QString, QPointer<modbus_decode> >::iterator i = modbusconnections.find(index);
        while (i !=modbusconnections.end() && i.key() == index) {
            connector=i.value();
            connector->pvClearMonitor(kData);
            ++i;
        }
    }

    return true;
}

// flush any io
int modbusPlugin::FlushIO() {
    //qDebug() << "modbusPlugin:FlushIO";
    return true;
}

// termination
int modbusPlugin::TerminateIO() {
    qDebug() << "modbusPlugin:TerminateIO";
    QMutexLocker locker(&mutex);
    QList<QPointer<modbus_decode>> connectors=modbusconnections.values();
    foreach(QPointer<modbus_decode> connector, connectors) {
       emit connector->TerminateIO();
    }
     return true;
}

void modbusPlugin::closeEvent(){
    //qDebug() << "modbusPlugin:closeEvent ";
    QMutexLocker locker(&mutex);
    QList<QPointer<modbus_decode>> connectors=modbusconnections.values();
    foreach(QPointer<modbus_decode> connector, connectors) {
       connector->setTerminate();
    }
    foreach(QThread* modbusthread, modbusThreads) {
        modbusthread->quit();
        modbusthread->wait();
        delete (modbusthread);
    }
    modbusThreads.clear();
    emit closeSignal();

}


#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#else
Q_EXPORT_PLUGIN2(modbusPlugin, modbusPlugin)
#endif

