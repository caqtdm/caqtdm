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

bool HTTPCONFIGURATOR = false;

#if defined(_MSC_VER)
  #define NOMINMAX
  #include <windows.h>
  #include <Psapi.h>
  #pragma comment (lib, "Psapi.lib")
#endif
#include "searchfile.h"

#include <QtGui>

#include "pathdefinitions.h"
#include "fileopenwindow.h"
#include "specialFunctions.h"
#include "fileFunctions.h"
#include "loadPlugins.h"

#ifdef MOBILE
  #include "fingerswipegesture.h"
#endif

#include <iostream>
#include <fstream>
#include <string>

#include <QFileDialog>
#include <QString>
#include "messagebox.h"
#include "configDialog.h"
#include "caQtDM_Lib_global.h"

#ifdef linux
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
   #define CAQTDM_X11 Q_WS_X11
#else
   #ifndef MOBILE_ANDROID
      #define CAQTDM_X11 Q_OS_UNIX
   #endif
#endif
#endif

#ifdef CAQTDM_X11
        #include <QX11Info>
        #include <X11/Xutil.h>
        #include <X11/Xlib.h>
        #include <X11/Xatom.h>

        #define MESSAGE_SOURCE_OLD            0
        #define MESSAGE_SOURCE_APPLICATION    1
        #define MESSAGE_SOURCE_PAGER          2
#endif //CAQTDM_X11

#if defined(_MSC_VER)
int setenv(const char *name, const char *value, int overwrite)
{
    int errcode = 0;
    if(!overwrite) {
        size_t envsize = 0;
        errcode = getenv_s(&envsize, Q_NULLPTR, 0, name);
        if(errcode || envsize) return errcode;
    }
    return _putenv_s(name, value);
}
#ifndef snprintf
 #define snprintf _snprintf
#endif

#endif

#if defined(__OSX__)|| defined(__APPLE__)
  #include <mach/mach.h>
  #include <mach/host_info.h>
  #include <mach/mach_init.h>
  #include <mach/vm_statistics.h>
#endif




#if QT_VERSION > 0x050000
void FileOpenWindow::onApplicationStateChange(Qt::ApplicationState state)
{

#ifdef MOBILE
    int pendio;
    switch (state) {
         case Qt::ApplicationSuspended:
             qDebug() << "application state changed to suspended";
             break;
         case Qt::ApplicationHidden:
             qDebug() << "application state changed to hidden";
             break;
         case Qt::ApplicationInactive:
             qDebug() << "application state changed to inactive";

             pendio = false;
             if (mutexKnobData != (MutexKnobData *) Q_NULLPTR) {
                 for (int i=0; i < mutexKnobData->GetMutexKnobDataSize(); i++) {
                     knobData *kPtr = mutexKnobData->GetMutexKnobDataPtr(i);
                     if(kPtr->index != -1)  {
                       //qDebug() << "should disconnect" << kPtr->pv;
                       ControlsInterface * plugininterface = (ControlsInterface *) kPtr->pluginInterface;
                       if(plugininterface != (ControlsInterface *) Q_NULLPTR) plugininterface->pvDisconnect(kPtr);
                       mutexKnobData->SetMutexKnobData(i, *kPtr);
                       pendio = true;
                     }
                 }
             }
             TerminateAllInterfaces();

             break;
         case Qt::ApplicationActive:
             qDebug() << "application state changed to active";
             pendio = false;
              if (mutexKnobData != (MutexKnobData *) Q_NULLPTR) {
                  for (int i=0; i < mutexKnobData->GetMutexKnobDataSize(); i++) {
                      knobData *kPtr = mutexKnobData->GetMutexKnobDataPtr(i);
                      if(kPtr->index != -1) {
                        ControlsInterface * plugininterface = (ControlsInterface *) kPtr->pluginInterface;
                        if(plugininterface != (ControlsInterface *) Q_NULLPTR) plugininterface->pvReconnect(kPtr);
                        pendio = true;
                      }
                  }
                  if(pendio)  {
                      FlushAllInterfaces();
                  }
              }

             break;
         }
#else
         Q_UNUSED(state);
#endif
}
#endif

void FileOpenWindow::FlushAllInterfaces()
{
    // flush all plugins
    if(!interfaces.isEmpty()) {
        QMapIterator<QString, ControlsInterface *> i(interfaces);
        while (i.hasNext()) {
            i.next();
            ControlsInterface *plugininterface = i.value();
            if(plugininterface != (ControlsInterface *) Q_NULLPTR) plugininterface->FlushIO();
        }
    }
}

void FileOpenWindow::TerminateAllInterfaces()
{
    // flush all plugins
    if(!interfaces.isEmpty()) {
        QMapIterator<QString, ControlsInterface *> i(interfaces);
        while (i.hasNext()) {
            i.next();
            ControlsInterface *plugininterface = i.value();
            if(plugininterface != (ControlsInterface *) Q_NULLPTR) plugininterface->TerminateIO();
        }
    }
}

/**
 * our main window (form) constructor
 */
FileOpenWindow::FileOpenWindow(QMainWindow* parent,  QString filename, QString macroString,
                               bool attach, bool minimize, QString geometry, bool printscreen, bool resizing,
                               QMap<QString, QString> options): QMainWindow(parent)
{
    // definitions for last opened file
    debugWindow = true;
    fromIOS = false;
    lastWindow = (QMainWindow*) Q_NULLPTR;
    lastMacro ="";
    lastFile = "";
    if(resizing) lastResizing="true";
    else lastResizing="false";
    reloadList.clear();

    lastGeometry = geometry;
    userClose = false;
    printandexit = printscreen;
    minimizeMessageWindow = minimize;
    activWindow = 0;
    Specials specials;
    OptionList = options;

    caQtDM_TimeOutEnabled = false;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    qDebug() <<  "caQtDM -- desktop size:" << qApp->desktop()->size();
#else
     qDebug() <<  "caQtDM -- desktop size:" <<  QGuiApplication::primaryScreen()->size();
#endif
    // Set Window Title without the whole path
    QString title("caQtDM ");
    title.append(BUILDVERSION);
    title.append(" Build=");
    title.append(__DATE__);
    title.append(" ");
    title.append(BUILDTIME);

    // set for epics longer waveforms
    QString maxBytes = (QString)  qgetenv("EPICS_CA_MAX_ARRAY_BYTES");
    if(maxBytes.size() == 0) setenv("EPICS_CA_MAX_ARRAY_BYTES", "150000000", 1);

    // in case of tablets, use static plugins linked in
#ifdef MOBILE
    Q_IMPORT_PLUGIN(CustomWidgetCollectionInterface_Controllers);
    Q_IMPORT_PLUGIN(CustomWidgetCollectionInterface_Monitors);
    Q_IMPORT_PLUGIN(CustomWidgetCollectionInterface_Graphics);
    Q_IMPORT_PLUGIN(CustomWidgetCollectionInterface_Utilities);
    Q_IMPORT_PLUGIN(DemoPlugin);
    Q_IMPORT_PLUGIN(Epics3Plugin);
    Q_IMPORT_PLUGIN(environmentPlugin);
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    #ifdef CAQTDM_MODBUS
        Q_IMPORT_PLUGIN(modbusPlugin);
    #endif
    #ifdef CAQTDM_GPS
        Q_IMPORT_PLUGIN(gpsPlugin);
    #endif
#endif
//*************************************
#ifdef EPICS4
    Q_IMPORT_PLUGIN(Epics4Plugin);
#endif
#ifdef ARCHIVESF
    Q_IMPORT_PLUGIN(ArchiveSF_Plugin);
#endif
#ifdef ARCHIVEHIPA
    Q_IMPORT_PLUGIN(ArchiveHIPA_Plugin);
#endif
#ifdef ARCHIVEPRO
    Q_IMPORT_PLUGIN(ArchivePRO_Plugin);
#endif
//*************************************
    Q_INIT_RESOURCE(qtcontrolsplugin);  // load resources from resource file
#endif

    // message window used by library and here
    messageWindow = new MessageWindow();

    // create a class for exchanging data
    mutexKnobData = new MutexKnobData();

    // create form
    ui.setupUi(this);
    setGeometry(0,0, 300, 150);
    this->statusBar()->show();

    // connect action buttons
    connect( this->ui.fileAction, SIGNAL( triggered() ), this, SLOT(Callback_OpenButton()) );
    connect( this->ui.aboutAction, SIGNAL( triggered() ), this, SLOT(Callback_ActionAbout()) );
    connect( this->ui.exitAction, SIGNAL( triggered() ), this, SLOT(Callback_ActionExit()) );
    connect( this->ui.reloadAction, SIGNAL( triggered() ), this, SLOT(Callback_ActionReload()) );
    connect( this->ui.unconnectedAction, SIGNAL( triggered() ), this, SLOT(Callback_ActionUnconnected()) );
    connect( this->ui.timedAction, SIGNAL( triggered() ), this, SLOT(Callback_ActionTimed()) );
    connect( this->ui.directAction, SIGNAL( triggered() ), this, SLOT(Callback_ActionDirect()) );
    connect( this->ui.helpAction, SIGNAL( triggered() ), this, SLOT(Callback_ActionHelp()) );
    connect( this->ui.emptycacheAction, SIGNAL( triggered() ), this, SLOT(Callback_EmptyCache()) );
    this->ui.timedAction->setChecked(true);

    setWindowTitle(title);

    // is updatetype specified on the command line as option, then set action direct and remove it
    QMap<QString, QString>::const_iterator i = OptionList.find("updatetype");
    while (i != OptionList.end() && i.key() == "updatetype") {
        QString value = i.value();
        if(value.toLower() == "direct")  emit Callback_ActionDirect();
        else if(value.toLower() == "timed")  emit Callback_ActionTimed();
        ++i;
    }
    OptionList.remove("updatetype");

#ifdef MOBILE
    specials.setNewStyleSheet(messageWindow, qApp->primaryScreen()->size(), 16, 10);
#endif
    messageWindow->setAllowedAreas(Qt::TopDockWidgetArea);
    QGridLayout *gridLayoutCentral = new QGridLayout(this->ui.centralwidget);
    QGridLayout *gridLayout = new QGridLayout();
    gridLayoutCentral->addLayout(gridLayout, 0, 0, 1, 1);
    gridLayout->addWidget(messageWindow, 0, 0, 1, 1);
    messageWindow->show();

#ifndef MOBILE
    QString uniqueKey = QString("caQtDM shared memory:") ;
    bool memoryAttached = false;
    #ifdef CAQTDM_X11
        #if QT_VERSION > QT_VERSION_CHECK(5,0,0)
        if (qApp->platformName()== QLatin1String("xcb")){
        #endif

        uniqueKey.append(DisplayString(QX11Info::display()));
        #if QT_VERSION > QT_VERSION_CHECK(5,0,0)

        }else{
            QString uids = QString::number(getuid());
            uniqueKey.append(":"+ uids);
        }
        #endif

    #endif

    #ifdef linux
        QString uids = QString::number(getuid());
        uniqueKey.append(":"+ uids);
    #endif

    qDebug() << "caQtDM -- shared memory key" << uniqueKey;
    sharedMemory.setKey (uniqueKey);

    // in case that one wants to attach to an instance that is actually creating, wait until we can attach
    if(attach) {
        for(int j=0; j<10; j++) {
            Sleep::msleep(150);
            if (sharedMemory.attach()) {
                memoryAttached = true;
                break;
            }
        }
    }

    // memory attached
    if (memoryAttached) {
        _isRunning = true;
        if(attach) {
            qDebug() << "caQtDM -- another instance of caQtDM detected with size"  << sharedMemory.size() << "==> attach to it (" << uniqueKey <<")" ;
            QString message(filename);
            message.append(";");
            message.append(macroString);
            message.append(";");
            message.append(geometry);
            message.append(";");
            message.append(lastResizing);
            qDebug() << "send a message with file, macro and geometry to it and exit "<< message;
            sendMessage(message);
            sharedMemory.detach();
            qApp->exit(0);  // does not work here
            exit(0);
        } else {
            qDebug() << "caQtDM -- another instance of caQtDM detected, but no attach specified ==> standalone";
        }
    // memory to be created
    } else {
        _isRunning = false;
        // create shared memory with a default value to note that no message is available.
        if (!sharedMemory.create(BlopSize * RingSize + 2 * sizeof(uint))) {
            qDebug() << "caQtDM -- Unable to create shared memory:" << sharedMemory.errorString();
        } else {
            int size =  BlopSize * RingSize + 2 * sizeof(uint);
            QByteArray byteArray(size, '\0');
            qDebug() << "caQtDM -- created shared memory with" << BlopSize * RingSize + 2 * sizeof(uint) << "bytes";
            sharedMemory.lock();
            char *to = (char*)sharedMemory.data();
            const char *from = byteArray.data();
            memcpy(to, from, qMin(sharedMemory.size(), byteArray.size()));
            MSQ_init();
            sharedMemory.unlock();
            // start checking for messages of other instances.
            timer = new QTimer(this);
            connect(timer, SIGNAL(timeout()), this, SLOT(checkForMessage()));
            timer->start(1000);
        }
    }
#else
    Q_UNUSED(attach);
#endif
    // when file was specified, open it
    // when called from here on Windows, the actual size of the window
    // is not found => defer opening
    mustOpenFile = false;
    if(filename.size() > 0) {
        lastMacro = macroString;
        lastFile = filename;
        lastGeometry = geometry;
        mustOpenFile = true;
    }

    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

    // start a timer
    startTimer(1000);

    pvWindow = (QMainWindow*) Q_NULLPTR;
    pvTable = (QTableWidget*) Q_NULLPTR;

//************************************************************************************************************************************************
    if(HTTPCONFIGURATOR) {
    // test reading a local configuration file in order to start caQtDM for ios (read caQTDM_IOS_Config.xml, display its data, choose configuration,
    // then get from the choosen website and choosen config file the epics configuration and ui file to launch

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QSize desktopSize = qApp->desktop()->size();
#else
    QSize desktopSize = QGuiApplication::primaryScreen()->size();
#endif


    //qDebug() <<  "desktop size in millimer" << qApp->desktop()->widthMM() << qApp->desktop()->heightMM();
 again:
    QList<QString> urls;
    QList<QString> files;
    QString url, file;
    QString stdpathdoc = specials.getStdPath();
    debugWindow = false;

    // parse the config file for urls and files
    stdpathdoc.append("/caQtDM_IOS_Config.xml");

    //qDebug() << stdpathdoc;

    QFileInfo fi(stdpathdoc);
    if(fi.exists()) {
       parseConfigFile(stdpathdoc, urls, files);
    } else{
        QString defpathdoc;
#ifdef MOBILE_ANDROID
        defpathdoc ="assets:/caQtDM_IOS_Config.xml";
#else
   #ifdef MOBILE_IOS
        defpathdoc ="caQtDM_IOS_Config.xml";
   #else
        // first create direcory in temp path.
        QDir dir;
        dir.mkpath(specials.getStdPath());
        // create file
        QList<QString> httpstring;
        QList<QString> configfile;
        // write a default configuration file while no file found
        httpstring << "http://epics.web.psi.ch/software/caqtdm/qtDir" << "http://epics.web.psi.ch/software/caqtdm/qtDir";
        configfile << "Proscan_MA85.config" << "HIPA_MA85.config";
        saveConfigFile(stdpathdoc, httpstring, configfile);
        defpathdoc = stdpathdoc;
    #endif
#endif
        parseConfigFile(defpathdoc, urls, files);
    }

    //qDebug() << "urls" << urls;

    // display the results and get the user choices

    configDialog dialog(debugWindow, urls, files, desktopSize, this);
    dialog.exec();
    if(dialog.isEscapeButtonClicked()) exit(0);
    if(!dialog.isStartButtonClicked() && !dialog.isClearConfig())  exit(0);

    // when clear config files is used, then reload dialog from original
    if(dialog.isClearConfig()) {
        dialog.close();
        goto again;
    }
    dialog.getChoice(url, file, urls, files, debugWindow);

    // and save the changes
    saveConfigFile(stdpathdoc, urls, files);

    fileFunctions filefunction;

    // download the choosen configurations file from the choosen url, but find it first locally
    setenv("CAQTDM_DISPLAY_PATH", qasc(specials.getStdPath()), 1);
    int success = filefunction.checkFileAndDownload(file, url);
    if(!success) {
        QMessageBox::critical(Q_NULLPTR, tr("caQtDM"), tr("could not download file %1 from %2").arg(file).arg(url));
        exit(0);
    }

    //set all the environment variables that we need
    setAllEnvironmentVariables(file);

    // now check if file exists and download it. (file is specified by the environment variables CAQTDM_LAUNCHFILE and CAQTDM_URL_DISPLAY)
    QString launchFile = (QString)  qgetenv("CAQTDM_LAUNCHFILE");
    filefunction.checkFileAndDownload(launchFile);

    // lauch the display with the file
    if(launchFile.size() > 0) {
        lastMacro = "";
        lastFile = launchFile;
        lastGeometry = "";
        mustOpenFile = true;
    }

    // we do not want to reload, while when closing all files, the application will also exit
    this->ui.reloadAction->setEnabled(false);
    }

    // Inform user about CAQTDM_REPLACE_UNITS replacements.
    if(messageWindow != (MessageWindow *) Q_NULLPTR) {
    bool doDefaultUnitReplacements = !(qgetenv("CAQTDM_DEFAULT_UNIT_REPLACEMENTS").toLower().replace("\"","") == "false");
        if (doDefaultUnitReplacements)  messageWindow->postMsgEvent(QtInfoMsg, (char*) qasc(QString("Info: Default unit replacements are taking place, you can disable them by setting the environment variable \"CAQTDM_DEFAULT_UNIT_REPLACEMENTS\" to false.")));
        else messageWindow->postMsgEvent(QtInfoMsg, (char*) qasc(QString("Info: Default unit replacements are disabled by user, you can enable them by unsetting the environment variable \"CAQTDM_DEFAULT_UNIT_REPLACEMENTS\" or setting it to true.")));
        QString replaceUnits = QString(qgetenv("CAQTDM_CUSTOM_UNIT_REPLACEMENTS"));
        if(replaceUnits.trimmed().length() > 0) messageWindow->postMsgEvent(QtInfoMsg, (char*) qasc(QString("Info: Environment variable \"CAQTDM_CUSTOM_UNIT_REPLACEMENTS\" is defined.")));
        else messageWindow->postMsgEvent(QtInfoMsg, (char*) qasc(QString("Info: Environment variable \"CAQTDM_CUSTOM_UNIT_REPLACEMENTS\" is not defined, standard unit replacements are taking place. You can define \"CAQTDM_CUSTOM_UNIT_REPLACEMENTS\" to replace characters within or whole units.")));
    }

    // load the control plugins (must be done after setting the environment)
    loadPlugins loadplugins;
    if (!loadplugins.loadAll(interfaces, mutexKnobData, messageWindow, OptionList )) {
        QMessageBox::critical(this, "Error", "Could not load any plugin");
    } else {
        if(!interfaces.isEmpty()) {
            QMapIterator<QString, ControlsInterface *> i(interfaces);
            while (i.hasNext()) {
                char asc[MAX_STRING_LENGTH];
                i.next();
                snprintf(asc, MAX_STRING_LENGTH, "Info: plugin %s loaded", qasc(i.key()));
                messageWindow->postMsgEvent(QtInfoMsg, asc);
            }
        }
    }

    // in case of http support, we add the temporary directory name to the CAQTDM_DISPLAY_PATH if not already set
    // only in case of non mobile platforms
#ifndef MOBILE
    QString displayPath = (QString)  qgetenv("CAQTDM_DISPLAY_PATH");
    if(!displayPath.contains(specials.getStdPath())) {
       displayPath.append(pathSeparator);
       displayPath.append(specials.getStdPath());
       setenv("CAQTDM_DISPLAY_PATH", (char*) qasc(displayPath), 1);
    }
#endif

    // show url in menu when defined
    QString urlpath = (QString)  qgetenv("CAQTDM_URL_DISPLAY_PATH");
    if(urlpath.length() > 0) {
      QString displayPath="url from environment=";
      displayPath.append( (QString)  qgetenv("CAQTDM_URL_DISPLAY_PATH"));
      this->ui.displayUrl->setText(displayPath);
    } else {
        this->ui.menuHttp->setEnabled(false);
    }

//************************************************************************************************************************************************

#ifdef MOBILE
    // add fingerswipe gesture
    QGestureRecognizer* pRecognizer = new FingerSwipeGestureRecognizer();
    fingerSwipeGestureType = QGestureRecognizer::registerRecognizer(pRecognizer);
#endif

    // application state handler
#if QT_VERSION > 0x050000
    connect(qApp, SIGNAL(applicationStateChanged(Qt::ApplicationState)), this, SLOT(onApplicationStateChange(Qt::ApplicationState)));
#endif

    // we want to be able to exit caQtDM after some amount of time (defined by an environment variable), while many displays are normally started at PSI on a central computer
    // and never terminated by the used (comes mainly from NX where a session can be closed without closing the applications)
    QString timeoutHours = (QString)  qgetenv("CAQTDM_TIMEOUT_HOURS");
    if(timeoutHours.length() > 0) {
        bool ok;
        QString displayTimeOut="Info: timeout of caQtDM defined in hours: ";
        displayTimeOut.append( (QString)  qgetenv("CAQTDM_TIMEOUT_HOURS"));

        qApp->installEventFilter(this);  // move in windows should reset our timeout counter
        caQtDM_TimeOut = caQtDM_TimeLeft = timeoutHours.trimmed().toDouble(&ok);
        if(ok && (caQtDM_TimeOut>0.021)) {
            caQtDM_TimeOutEnabled = true;
            displayTimeOut.append(" will be enabled");
        } else {
            displayTimeOut.append(" can not be enabled");
            if (ok && (caQtDM_TimeOut<=0.021))  displayTimeOut.append(",because the Switch is not mean't for jokes!!!");

        }
        messageWindow->postMsgEvent(QtWarningMsg, (char*) qasc(displayTimeOut));
    } else {
        QString displayTimeOut="environment variable CAQTDM_TIMEOUT_HOURS could be set for quitting caQtDM automatically after some time";
        messageWindow->postMsgEvent(QtInfoMsg, (char*) qasc(displayTimeOut));
    }

    // available memory in KiB
    long long availableMemory = getAvailableMemory();

    // Check for available memory and warn the user if memory is not sufficient
    if (availableMemory < 300000) {
        messageWindow->postMsgEvent(QtWarningMsg, (char*) qasc(QString("Available system memory is less than 300MB, this could lead to a crash during operation or while opening new panels.")));
    }
    // Print out available memory in all cases
    messageWindow->postMsgEvent(QtInfoMsg, (char*) qasc(QString("Available system memory: " + QString::number(availableMemory / 1000) + "MB")));
}

FileOpenWindow::~FileOpenWindow()
{
    delete messageWindow;
}

void FileOpenWindow::parseConfigFile(const QString &filename, QList<QString> &urls, QList<QString> &files)
{
    QFile* file = new QFile(filename);

    /* can not open file */
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(Q_NULLPTR, tr("caQtDM"), tr("could not open configuration file: %1").arg(filename));
        exit(0);
    }

    QXmlStreamReader xml(file);

    /* parse the XML file*/
    while(!xml.atEnd() && !xml.hasError()) {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = xml.readNext();
        /* StartDocument, follow up*/
        if(token == QXmlStreamReader::StartDocument)  continue;

        /* If token is StartElement, we'll see if we can read it.*/
        if(token == QXmlStreamReader::StartElement) {
            if(xml.name() == QString("configuration")) continue;

            if(xml.name() == QString("url")) {
                QXmlStreamAttributes attributes = xml.attributes();
                if(attributes.hasAttribute("value")) urls.append(attributes.value("value").toString());
            }

            if(xml.name() == QString("config")) {
                QXmlStreamAttributes attributes = xml.attributes();
                if(attributes.hasAttribute("value")) files.append(attributes.value("value").toString());
            }
        }
    }
    /* Error handling. */
    if(xml.hasError()) {
        QMessageBox::warning(this, tr("caQtDM"), tr("could not parse configuation file: error=%1").arg( xml.errorString()));
    }

    xml.clear();

    file->close();
}

void FileOpenWindow::saveConfigFile(const QString &filename, QList<QString> &urls, QList<QString> &files)
{
    QFile *file = new QFile(filename);

    /* can not open file */
    if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(Q_NULLPTR, tr("caQtDM"), tr("could not open configuration file: %1").arg(filename));
        exit(0);
    }

    QXmlStreamWriter xmlWriter(file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();

    xmlWriter.writeStartElement("configuration");

    for(int i=0; i<urls.length(); i++) {
        xmlWriter.writeStartElement("url");
        xmlWriter.writeAttribute("value", urls.at(i));
        xmlWriter.writeEndElement();
    }
    for(int i=0; i<files.length(); i++) {
        xmlWriter.writeStartElement("config");
        xmlWriter.writeAttribute("value", files.at(i));
        xmlWriter.writeEndElement();
    }

    xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();

    file->close();
}


void FileOpenWindow::setAllEnvironmentVariables(const QString &fileName)
{
    char asc[MAX_STRING_LENGTH];
    Specials specials;
    QString stdpathdoc =  specials.getStdPath();

    QString EnvFile=stdpathdoc;
    EnvFile.append("/");
    EnvFile.append(fileName);
    QFile file(EnvFile);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(Q_NULLPTR, "open file error setAllEnviromentVariables", file.errorString());
        return;
    }

    QTextStream in(&file);

    while(!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(" ");
        if(fields.count() > 1) {
            QString envString = "";
            for(int i=1; i<fields.count(); i++) {
                envString.append(fields.at(i));
                if(i<fields.count() -1) envString.append(" ");
            }
            setenv(qasc(fields.at(0)), qasc(envString), 1);
            //messageWindow->postMsgEvent(QtDebugMsg, (char*) qasc(envString));
        } else if(line.size() > 0) {
            snprintf(asc, MAX_STRING_LENGTH, "environment variable could not be set from %s", qasc(line));
            messageWindow->postMsgEvent(QtWarningMsg, asc);
        }
    }
    //Replacement for standard writable directory
    setenv("CAQTDM_DISPLAY_PATH", qasc(stdpathdoc), 1);

    snprintf(asc, MAX_STRING_LENGTH, "epics configuration file loaded: %s", qasc(fileName));
    messageWindow->postMsgEvent(QtDebugMsg, asc);
    file.close();
}

// runs one per second
void FileOpenWindow::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    char asc[MAX_STRING_LENGTH];
    int countPV=0;
    int countNotConnected=0;
    float highCount = 0.0;
    QString highPV;
    int countDisplayed = 0;
    static int printIt = 0;
    static int timeout = 0;

    // when timeout, quit
    if(caQtDM_TimeOutEnabled) {
        caQtDM_TimeLeft -= 1.0/3600.0;
        if(caQtDM_TimeLeft <= 0) {
            QList<CaQtDM_Lib *> all = this->findChildren<CaQtDM_Lib *>();
            foreach(QWidget* widget, all) widget->close();
            if (sharedMemory.isAttached()) sharedMemory.detach();
            qApp->exit(0);
        }
    }

    if(mustOpenFile) {
        mustOpenFile = false;
        Callback_OpenNewFile(lastFile, lastMacro, lastGeometry, lastResizing);
    }

    if(minimizeMessageWindow) {
        minimizeMessageWindow = false;
        showMinimized ();
    }

    asc[0] = '\0';

#ifdef linux
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    snprintf(asc, MAX_STRING_LENGTH, "memory: %ld kB,", usage.ru_maxrss);
#endif
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX procmem;
    if (GetProcessMemoryInfo(GetCurrentProcess(),(PPROCESS_MEMORY_COUNTERS)&procmem,sizeof(procmem))) {
      snprintf(asc, MAX_STRING_LENGTH,"memory: %ld kB", (procmem.PrivateUsage / (1024)));
    } else {
      snprintf(asc, MAX_STRING_LENGTH, "memory: no RAM");
    }
#endif

    // any non connected pv's to display ?
    if (mutexKnobData != (MutexKnobData *) Q_NULLPTR) {
        char msg[MAX_STRING_LENGTH];
        msg[0] = '\0';

        for (int i=0; i < mutexKnobData->GetMutexKnobDataSize(); i++) {
            knobData *kPtr = mutexKnobData->GetMutexKnobDataPtr(i);
            if(kPtr->index != -1) {
                if(!kPtr->edata.connected) {
                    countNotConnected++;
                } else {
                    if(kPtr->edata.displayCount > 0) countDisplayed++;
                }
                countPV++;
            }
        }

        if(caQtDM_TimeOutEnabled) {
            char asc1[50];
            if (caQtDM_TimeLeft<0.02){
                sprintf(asc1, ", T/O=%.0fsec ", caQtDM_TimeLeft*60*60);
            }else{
                sprintf(asc1, ", T/O=%.2lfh ", caQtDM_TimeLeft);
            }
            strcat(asc, asc1);
        }

        highCount = mutexKnobData->getHighestCountPV(highPV);
        if(highCount != 0.0) {
            snprintf(msg, MAX_STRING_LENGTH, "%s - PV=%d (%d NC), %d Monitors/s, %d Displays/s, highest=%s with %.1f Monitors/s ", asc, countPV, countNotConnected,
                      mutexKnobData->getMonitorsPerSecond(), mutexKnobData->getDisplaysPerSecond(), qasc(highPV), highCount);
        } else {
            strcpy(msg, asc);
        }
        statusBar()->showMessage(msg);
    }
    QString filename_save=qgetenv("CAQTDM_SCREENSHOT_NAME");
    // we wanted a print, do it when acquired, then exit
    if(printandexit) {
        if(countPV > 0 && countNotConnected == 0) {
            if(this->findChildren<CaQtDM_Lib *>().count() == 1) {
                CaQtDM_Lib * widget = this->findChild<CaQtDM_Lib *>();
                if(countDisplayed > 0 && countDisplayed == countPV) {
                    printIt++;
                    if(printIt > 2) {
                        widget->printPS("caQtDM.ps");
                        qDebug() << "caQtDM -- file has been printed to caQtDM.ps";
                        qApp->exit(1);
                        exit(1);
                    }
                }
            }
        }
        if(timeout++ > 4) {    // seems we did not get everything
            CaQtDM_Lib * widget = this->findChild<CaQtDM_Lib *>();
            widget->printPS("caQtDM.ps");
            qDebug() << "caQtDM -- file has been printed to caQtDM.ps, probably with errors";
            qApp->exit(1);
            exit(1);
        }
    }
    QVariant var = this->property("savetoimage");
    if(!var.isNull()) {
        bool savetoimage = var.toBool();
        if (savetoimage){
            QString name="caQtDM";
            if (!filename_save.isEmpty()){
                name=filename_save;
            }
            name=name.append(".png");
            if(countPV > 0 && countNotConnected == 0) {
                if(this->findChildren<CaQtDM_Lib *>().count() == 1) {
                    CaQtDM_Lib * widget = this->findChild<CaQtDM_Lib *>();
                    if(countDisplayed > 0 && countDisplayed == countPV) {
                        printIt++;
                        if(printIt > 2) {

                            widget->save_graphics(name);
                            qDebug() << "caQtDM -- file has been printed to "<< name;
                            qApp->exit(1);
                            exit(1);
                        }
                    }
                }
            }
            if(timeout++ > 4) {    // seems we did not get everything
                CaQtDM_Lib * widget = this->findChild<CaQtDM_Lib *>();
                widget->save_graphics(name);
                qDebug() << "caQtDM -- file has been printed to " << name << ", probably with errors";
                qApp->exit(1);
                exit(1);
            }

        }
    }


    // reload windows that were closed to be reloaded (had to be deferrred, due to memory problems)
    if(!reloadList.isEmpty()) {
        foreach( Row row, reloadList ) {
           qDebug() << "caQtDM -- reload file" << row.file << "with macro" << row.macro << "can be resized=" << row.resize;
           QPoint position = row.position;
           QString fileS = row.file;
           QString macroS = row.macro;
           QString resizeS = row.resize;

           loadMainWindow(position, fileS, macroS, resizeS, false, true, false);
        }
        reloadList.clear();
    }

    // any open windows ?
    // we want to ask with timeout if the application has to be closed. 23-jan-2013 no yust exit (in case of tablet do not exit)
#ifndef MOBILE
    if(this->findChildren<CaQtDM_Lib *>().count() <= 0 && userClose) {
        if (sharedMemory.isAttached()) sharedMemory.detach();
        qApp->exit(0);
    } else if(this->findChildren<CaQtDM_Lib *>().count() > 0) {
        userClose = true;
    }
#endif
}

/**
 * Functin to reset the UpdateType back to direct after resetting it to timed for the startup
 * Is called via no argument slot because the needed lambda isn't supported under Qt4.
 */
void FileOpenWindow::setDirectUpdateTypeOnRestart(const QDateTime reloadTime){

    if (reloadTime < lastReloadTime) {
        // In that case the received signal is not up to date and another timer has been started in the meantime, which has priority.
        return;
    }
    mutexKnobData->UpdateMechanism(MutexKnobData::UpdateDirect);
    qDebug() << "UpdateType reset to direct";
    if(messageWindow != (MessageWindow *) Q_NULLPTR) {
        messageWindow->postMsgEvent(QtWarningMsg, (char*) qasc(QString("UpdateType reset to direct")));
    }
}

/**
 * Slot to reset the UpdateType back to direct after resetting it to timed for the startup
 * Should not be called for any other purpose  --> is a private slot
 */
void FileOpenWindow::onReloadTimeout()
{
    // Get time when the timeout started, this doesn't need to be exact because we compare using less than.
    QDateTime startTime = QDateTime::currentDateTime().addSecs(-10);
    setDirectUpdateTypeOnRestart(startTime);
}

/**
 * here we will load our display window
 */
QMainWindow *FileOpenWindow::loadMainWindow(const QPoint &position, const QString &fileS, const QString &macroS, const QString &resizeS,
                                    const bool &printexit, const bool &moveit, const bool &centerwindow)
{
    char *asc;
    bool willprint = printexit;
    QString suppressUpdates = qgetenv("CAQTDM_SUPPRESS_UPDATES_ONLOAD");
    if (suppressUpdates.toLower() == "true") {
        mutexKnobData->setSuppressUpdates(true);
    }
    QElapsedTimer timer;
    timer.start();
    CaQtDM_Lib *newWindow =  new CaQtDM_Lib(this, fileS, macroS, mutexKnobData, interfaces, messageWindow, willprint, Q_NULLPTR, OptionList);
    QString message = "Loading of window took: " + QString::number(timer.elapsed()) + " milliseconds";
    messageWindow->postMsgEvent(QtInfoMsg, (char*)qasc(message));

    // prc files are not allowed to be resized, or when resizing is prohibited by the command line
    if (fileS.contains("prc")) {
        newWindow->allowResizing(false);
    } else {
        if(resizeS.contains("false")) {
            newWindow->allowResizing(false);
        } else {
            newWindow->allowResizing(true);
        }
        if(resizeS.contains("Window")||
           resizeS.contains("ToolTip")||
           resizeS.contains("SplashScreen")) {
            Qt::WindowFlags flags;
            if(resizeS.contains("Window")){
              flags = Qt::Window;
            }
            if(resizeS.contains("ToolTip")){
              flags = Qt::ToolTip;
            }
            if(resizeS.contains("SplashScreen")){
              flags = Qt::SplashScreen;
            }
            if(resizeS.contains("FramelessWindowHint")){
              flags |= Qt::FramelessWindowHint;
            }
            if(resizeS.contains("WindowStaysOnTopHint")){
              flags |= Qt::WindowStaysOnTopHint;
            }
            newWindow->setWindowFlags(flags);
        }
        if(resizeS.contains("PopUpWindow")){
            newWindow->setProperty("open_as_popupwindow",true);
        }


    }

#ifdef MOBILE
    newWindow->grabSwipeGesture(fingerSwipeGestureType);
    newWindow->setAttribute(Qt::WA_ContentsMarginsRespectsSafeArea,false);
#endif

    QMainWindow *mainWindow = newWindow;
    lastWindow = mainWindow;

    // center the window if requested (only for windows)
#if defined(WIN32) && !defined(__GNUC__)
     if(centerwindow)  {
        #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QDesktopWidget * Desktop = QApplication::desktop();
            QRect defscreengeo = Desktop->availableGeometry(-1);
        #else
            QRect defscreengeo =  QGuiApplication::primaryScreen()->availableGeometry();
        #endif
            int mainw_width = mainWindow->width();
            int mainw_height = mainWindow->height();
            int movx = (defscreengeo.width() / 2) - (mainw_width / 2);
            int movy = (defscreengeo.height() / 2) - (mainw_height / 2);
            mainWindow->move(movx, movy);
      }
#else
    Q_UNUSED(centerwindow);
#endif

    // do we want to set the position of the window
    if(moveit) {
        mainWindow->move(position);
    }

    // show and set sizes
    if(printexit) {
        mainWindow->showMinimized();
    } else {
        mainWindow->show();
    }

    mainWindow->raise();
    mainWindow->setMinimumSize(mainWindow->size()/4);
    mainWindow->setMaximumSize(16777215, 16777215);
    mainWindow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainWindow->setWindowFlags( mainWindow->windowFlags() );

    // set some user properties
    mainWindow->setProperty("fileString", fileS);
    mainWindow->setProperty("macroString", macroS);
    mainWindow->setProperty("resizeString", resizeS);

    // resize the window to minimal size for .prc files
    if (fileS.contains("prc")) {
        mainWindow->resize(mainWindow->minimumSizeHint());
    }
    activWindow = 0;
#ifdef MOBILE_IOS
    // this is needed for the status bar
    mainWindow->setAttribute(Qt::WA_ContentsMarginsRespectsSafeArea,false);
    QRect availscreengeo = qApp->primaryScreen()->availableGeometry();
    //QRect screengeo = QApplication::desktop()->geometry();
    //qDebug() << "IOS screen" << screengeo<< availscreengeo;
    QSize winsize=mainWindow->size();
    winsize.setHeight( winsize.rheight() -availscreengeo.top());
    mainWindow->resize(winsize);

#endif
    //qDebug() << "allocate" << fileS.size()+macroS.size()+100;
    asc = (char*) malloc((fileS.size()+macroS.size()+100) * sizeof(char));
    if(macroS.size() > 0) {
      sprintf(asc, "last file: %s, macro: %s", qasc(fileS), qasc(macroS));
    } else {
      sprintf(asc, "last file: %s", qasc(fileS));
    }
    mutexKnobData->setSuppressUpdates(false);
    messageWindow->postMsgEvent(QtDebugMsg, asc);
    free(asc);
    return mainWindow;
}

/**
 * slot for emptying our local http cache
 */
void FileOpenWindow::Callback_EmptyCache()
{
#ifndef MOBILE
    Specials specials;
    QString path =  specials.getStdPath();
    fileFunctions filefunction;
    path.append("/");
    filefunction.removeFilesInTree(path);
#endif
}

/**
 * slot for opening file by button
 */
void FileOpenWindow::Callback_OpenButton()
{
    //get a filename to open
    QString path = (QString)  qgetenv("CAQTDM_DISPLAY_PATH");

    if(path.size() == 0 && lastFilePath.size()==0) path.append(".");
    else path = lastFilePath;
#ifdef ADL_EDL_FILES
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Display file"), path, tr("ui/prc Files (*.ui *.prc);;MEDM Files (*.adl);;EDM Files (*.edl);;ALL Files (*.*)"));
#else
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Display file"), path, tr("ui/prc Files (*.ui *.prc);;ALL Files (*.*)"));
#endif
    //std::cout << "Got filename: " << fileName.toStdString() << std::endl;

    if(!fileName.isNull()) {
        QFileInfo fi(fileName);
        lastFilePath = fi.absolutePath();
        if(fi.exists()) {
            QPoint position(0,0);
            loadMainWindow(position, fileName, "", lastResizing, false, false, true);
        } else {
            QTDMMessageBox(QMessageBox::Warning, "file open error", "does not exist", ":/caQtDM-logos.png", QMessageBox::Close, this, Qt::Popup, true);
        }
    }
}

/**
 * slot for going to next window (for tablet)
 */
void FileOpenWindow::cycleWindows()
{
    QList<CaQtDM_Lib *> all = this->findChildren<CaQtDM_Lib *>();
    if(all.count() == 0) return;
    if(activWindow > all.count()-1) activWindow = 0;

    QWidget *w = all.at(activWindow);
    w->activateWindow();
    w->raise();
    w->setFocus();
    activWindow++;
    return;
}

void FileOpenWindow::nextWindow()
{
    cycleWindows();
}

/**
 * slot for opening file by signal
 */
void FileOpenWindow::Callback_OpenNewFile(const QString& inputFile, const QString& macroString, const QString& geometry, const QString& resizeString)
{
    //qDebug() << "*************************************************************************";
    //qDebug() << "callback open new file" << inputFile << "with macro string" << macroString;

#ifdef ADL_EDL_FILES
    const int extensions=4;
    const QString valid_extensions[extensions] = {".ui", ".prc", ".adl", ".edl"};
#else
    const int extensions=2;
    const QString valid_extensions[extensions] = {".ui", ".prc"};
#endif
    QString FileName;
    int ext_found=-1;
    FileName=inputFile;


    int counter=0;
    bool check_extension = false;
    while (counter<extensions){
        check_extension = check_extension || inputFile.endsWith(valid_extensions[counter]);
        if (check_extension) break;
        counter++;
    }
    ext_found=counter;
    counter=0;
    if (!check_extension) {
        while (counter<extensions){
            FileName=inputFile;
            // remove any extension
            FileName=FileName.split(".",SKIP_EMPTY_PARTS).at(0);

            FileName=FileName.append(valid_extensions[counter]);
            //qDebug() << "check " << FileName << valid_extensions[counter] ;
            // this will check for file existence and when an url is defined, download the file from a http server
            fileFunctions filefunction;
            // dont show error because we try!
            filefunction.checkFileAndDownload(FileName);

            searchFile *filecheck = new searchFile(FileName);
            FileName=filecheck->findFile();
            delete filecheck;
            if (!FileName.isNull()) break;
            counter++;
        }
#ifndef ADL_EDL_FILES
        if (counter==extensions)
            messageWindow->postMsgEvent(QtWarningMsg, (char*) qasc(tr("file %1 is not parsed (caQtDM built without ADL_EDL_FILES)").arg(inputFile)));
#endif
    }else{
        //check if file is there else check other extensions
        searchFile *filecheck = new searchFile(FileName);
        FileName=filecheck->findFile();
        delete filecheck;
        if (FileName.isNull()){
            //remove the extension
            FileName=inputFile;
            counter=0;
            while (counter<extensions){
                if (counter!=ext_found){
                    FileName=inputFile;
                    FileName=FileName.remove(valid_extensions[ext_found]);
                    FileName=FileName.append(valid_extensions[counter]);
                    //qDebug() << "check " << FileName << valid_extensions[counter] ;
                    // this will check for file existence and when an url is defined, download the file from a http server
                    fileFunctions filefunction;
                    // dont show error because we try!
                    filefunction.checkFileAndDownload(FileName);

                    searchFile *filecheck = new searchFile(FileName);
                    FileName=filecheck->findFile();
                    //qDebug() << "check " << FileName << valid_extensions[counter] ;
                    delete filecheck;
                    if (!FileName.isNull()) break;
                }
                counter++;
            }
            //if this I give up nothing is found
            if (counter==extensions) FileName=inputFile;


        }

    }
    //qDebug() << "try to open file" << FileName;

    // go through the children of this main window and find out if new or already present
    QList<QWidget *> all = this->findChildren<QWidget *>();
    foreach(QWidget* widget, all) {
        if(QMainWindow* w = qobject_cast<QMainWindow *>(widget)) {
            QString WindowProperty = "";
            // if already exists then yust pop it up
            QFile *file = new QFile;
            file->setFileName(FileName);
            QString title(file->fileName().section('/',-1));
            title.append("&");
            title.append(macroString);
            delete file;

            QVariant fileName = w->property("fileString");
            QVariant macroString = w->property("macroString");
            //qDebug() << "existing filename with macro =" <<  w->windowTitle() << "=" <<  w->property("fileString") << macroString;
            if(!fileName.isNull()) {
                WindowProperty = fileName.toString();
                WindowProperty= WindowProperty.section('/',-1);
            }
            WindowProperty.append("&");
            if(!macroString.isNull()) {
                WindowProperty.append(macroString.toString());
            }
            //qDebug() << "title=" << title << " windowproperty=" << WindowProperty;
            if(QString::compare(WindowProperty, title) == 0) {
                w->activateWindow();
                w->raise();
#ifndef MOBILE
                w->showNormal();
#endif
                w->setFocus();
// all these past commands will only give you a notification in the taskbar
// in case of x windows, we will pop the window really up
#ifdef CAQTDM_X11
        #if QT_VERSION > QT_VERSION_CHECK(5,0,0)
        if (qApp->platformName()== QLatin1String("xcb")){
        #endif

                static Atom  NET_ACTIVE_WINDOW = 0;
                XClientMessageEvent xev;
                if (NET_ACTIVE_WINDOW == 0) {
                    Display *dpy      = QX11Info::display();
                    NET_ACTIVE_WINDOW = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
                }
                xev.serial       = 0;
                xev.send_event   = True;
                xev.type         = ClientMessage;
                xev.window       = w->winId();
                xev.message_type = NET_ACTIVE_WINDOW;
                xev.format       = 32;

                xev.data.l[0]    = MESSAGE_SOURCE_PAGER;
                xev.data.l[1] = xev.data.l[2] = xev.data.l[3] = xev.data.l[4] = 0;
                XSendEvent(QX11Info::display(), QX11Info::appRootWindow(), False, SubstructureNotifyMask | SubstructureRedirectMask, (XEvent*)&xev);
        #if QT_VERSION > QT_VERSION_CHECK(5,0,0)
        }
        #endif

#endif //CAQTDM_X11

                return;
            }
        }
    }

    // this will check for file existence and when an url is defined, download the file from a http server
    fileFunctions filefunction;
    filefunction.checkFileAndDownload(FileName);
    if(filefunction.lastInfo().length() > 0) messageWindow->postMsgEvent(QtInfoMsg, (char*) qasc(filefunction.lastInfo()));
    if(filefunction.lastError().length() > 0)  messageWindow->postMsgEvent(QtCriticalMsg, (char*) qasc(filefunction.lastError()));

    // open file
    searchFile *s = new searchFile(FileName);
    QString fileNameFound = s->findFile();
    if(fileNameFound.isNull()) {
        QString message = inputFile;
        message.append(" does not exist");
        QTDMMessageBox *m = new QTDMMessageBox(QMessageBox::Warning, "file open error", message, ":/caQtDM-logos.png", QMessageBox::Close, this, Qt::Dialog| Qt::Popup, true);
        m->show();
    } else {
        //qDebug() << "file" << fileNameFound << "will be loaded" << "macro=" << macroString;
        QPoint position(0,0);
        QMainWindow *mainWindow = loadMainWindow(position, fileNameFound, macroString, resizeString, printandexit, false, (geometry == ""));//if geometry is empty center window (for windows)
        if(geometry != "") {
            parse_and_set_Geometry(mainWindow, geometry);
        }
    }
    delete s;
}

/**
 * slot for icon signal
 */
void FileOpenWindow::Callback_ActionAbout()
{
    QString message = QString("Qt-based Epics Display Manager Version %1 using Qt %2 and %3 with data from %4 developed at Paul Scherrer Institut, by Anton Mezger\nPlatform support is supported by H.Brands\n");
    message = message.arg(BUILDVERSION, QT_VERSION_STR, BUILDARCH, SUPPORT);
    QTDMMessageBox *m = new QTDMMessageBox(QMessageBox::Information, "About", message, ":/caQtDM-logospsi.png", QMessageBox::Close, this, Qt::Dialog| Qt::Popup, true);
    m->show();
}

/**
 * slot for help signal
 */
void FileOpenWindow::Callback_ActionHelp()
{
    shellCommand("assistant");
}

/**
 * slots for exit signal
 */
void FileOpenWindow::Callback_IosExit()
{
    if(HTTPCONFIGURATOR) {
        fromIOS = true;
        Callback_ActionExit();
        fromIOS = false;
    }
}

void FileOpenWindow::Callback_ReloadWindow(QWidget *w)
{
    reload(w);
}

void FileOpenWindow::Callback_ReloadAllWindows()
{
    Callback_ActionReload();
}

void FileOpenWindow::Callback_ActionExit()
{
    int selected;

    // launch window close
    if(!fromIOS) {
        QString message = QString("Are you sure to want to exit?");
        QTDMMessageBox *m = new QTDMMessageBox(QMessageBox::Warning, "Exit", message, ":/caQtDM-logos.png", QMessageBox::Yes | QMessageBox::No, this, Qt::Dialog, false);
        m->show();
        selected = m->exec();
    // normal close
    } else {
        if(debugWindow) {selected = QMessageBox::No;
    } else selected = QMessageBox::Yes;
    }

    if(selected == QMessageBox::Yes) {

// we are first going to close all open process windows
        // go through the children of the main window and find out if that window still exists
        QList<QWidget *> all = this->findChildren<QWidget *>();
        foreach(QWidget* widget, all) {
            if(QMainWindow* w = qobject_cast<QMainWindow *>(widget)) {
#ifndef MOBILE
                if(processWindow *w1 =  qobject_cast<processWindow *>(widget)) {
                    Q_UNUSED(w1);
                   // do not close processes
                } else {
                  //qDebug() << "close window" << w;
                  w->close();
                }
#else
                w->close();
#endif
            }
        }

// detach shared memory, delete pv container
        if (sharedMemory.isAttached()) sharedMemory.detach();
        qApp->exit(0);
        //exit(0);
    }
}

void FileOpenWindow::reload(QWidget *w)
{
    QPoint position;
    QString macroS = "";
    QString resizeS = "true";

    QVariant fileName = w->property("fileString");
    QVariant macroString = w->property("macroString");
    if(!macroString.isNull()) {
        macroS = macroString.toString();
    }
    QVariant resizeString= w->property("resizeString");
    if(!resizeString.isNull()) {
        resizeS = resizeString.toString();
    }

    // get rif of old window
    if(!w->isVisible()) return; // add this, while a close will quit when window is not loaded and visible
    position = w->pos();
    w->close();
    w->disconnect();
    w->deleteLater();

    if(!fileName.isNull()) {

        QString FileName = fileName.toString();

        // this will check for file existence and when an url is defined, download the file from a http server
        QFileInfo fi(FileName);
        fileFunctions filefunction;
        filefunction.checkFileAndDownload(fi.fileName());
        if(filefunction.lastInfo().length() > 0) messageWindow->postMsgEvent(QtInfoMsg, (char*) qasc(filefunction.lastInfo()));
        if(filefunction.lastError().length() > 0) messageWindow->postMsgEvent(QtCriticalMsg, (char*) qasc(filefunction.lastError()));

        // we were loading here before a new instance of caQtDM_Lib,; however the deferred delete of the previous instance
        // seemed to have a major memory leak, so that now we queue the reloading
        searchFile *s = new searchFile(FileName);
        QString fileNameFound = s->findFile();
        Row row;
        row.position = position;
        row.file = fileNameFound;
        row.macro = macroS;
        row.resize = resizeS;
        reloadList.append(row);
        // When reloading a file, set UpdateType to timed, else caQtDM might crash if too much data is processed in the beginning.
        if (this->ui.directAction->isChecked()) {
            mutexKnobData->UpdateMechanism(MutexKnobData::UpdateTimed);
            qDebug() << "Setting UpdateType to timed for 10 Seconds, can't start up with UpdateType = direct";
            if(messageWindow != (MessageWindow *) Q_NULLPTR) {
                messageWindow->postMsgEvent(QtWarningMsg, (char*) qasc(QString("Setting UpdateType to timed for 10 Seconds, can't start up with UpdateType = direct")));
            }
            // Make sure that if the timer triggers while another reload is taking place, the updateType is not reset in that case.
            // This is done by saving the last reloadTime in a member variable and making the slot that handles the timer compare the last reload time and the time the timer was started at.
            // The slot can then check if the received signal is the up to date because if another timer has been triggered in the meantime, the member variable holds a later time than when the timer was triggered.
            // If the signal is not up to date it is simply ignored as not to reset the updateType while a widget is currently reloading, this might happen e.g. when the user reloads multiple panels after each other.
            lastReloadTime = QDateTime::currentDateTime();
            QTimer::singleShot(10000, this, SLOT(onReloadTimeout()));
        }
        s->deleteLater();
    }
}

/// Provides the available RAM memory in Kibibytes (1 KiB = 1024 B)
long long FileOpenWindow::getAvailableMemory()
{
    long long memAvailable = -1;

#ifdef linux // From https://stackoverflow.com/a/70766868
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    while (std::getline(meminfo, line))
    {
        if (line.find("MemAvailable:") != std::string::npos)
        {
            const std::size_t firstWhiteSpacePos = line.find_first_of(' ');
            const std::size_t firstNonWhiteSpaceChar = line.find_first_not_of(' ', firstWhiteSpacePos);
            const std::size_t nextWhiteSpace = line.find_first_of(' ', firstNonWhiteSpaceChar);
            const std::size_t numChars = nextWhiteSpace - firstNonWhiteSpaceChar;
            std::string memAvailableStdStr = line.substr(firstNonWhiteSpaceChar, numChars);
            QString memAvailableStr = QString::fromStdString(memAvailableStdStr);
            memAvailable = memAvailableStr.toLongLong();
            break;
        }
    }
#elif defined(_MSC_VER)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        memAvailable = status.ullAvailPhys / 1024; // Value returned is in Bytes, convert to KiB
    }
#elif defined(__OSX__)|| defined(__APPLE__)
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    mach_port_t host_port = mach_host_self();
    vm_statistics64_data_t vm_stat;
    vm_size_t page_size;
    host_page_size(host_port, &page_size);

    if(KERN_SUCCESS != host_statistics64(host_port, HOST_VM_INFO, (host_info64_t)&vm_stat, &count)) {
        // An error occurred
        memAvailable = -1;
    }else{
        memAvailable = vm_stat.free_count * page_size / 1024;
    }


#endif
    return memAvailable;
}

void FileOpenWindow::Callback_ActionReload()
{
    // block reload button
   this->ui.reloadAction->blockSignals(true);

    Callback_ActionTimed();
    // block processing during reload
    mutexKnobData->setSuppressUpdates(true);

    // go through all windows, close them and reload them from files
    QList<QWidget *> all = this->findChildren<QWidget *>();
    foreach(QWidget* widget, all) {
        if(CaQtDM_Lib* w = qobject_cast<CaQtDM_Lib *>(widget)) {
           reload(w);
        }
    }

    mutexKnobData->setSuppressUpdates(false);

    this->ui.reloadAction->blockSignals(false);
}

void FileOpenWindow::Callback_ActionTimed() {
    this->ui.timedAction->setChecked(true);
    this->ui.directAction->setChecked(false);
    mutexKnobData->UpdateMechanism(MutexKnobData::UpdateTimed);
}

void FileOpenWindow::Callback_ActionDirect() {
    this->ui.timedAction->setChecked(false);
    this->ui.directAction->setChecked(true);
    mutexKnobData->UpdateMechanism(MutexKnobData::UpdateDirect);
}

void FileOpenWindow::checkForMessage()
{
     _blop element;

    // check and remove message in shared memory
    sharedMemory.lock();
    element = MSQ_deQueue();
    sharedMemory.unlock();
    if(element.blop[0] == '\0') {
        //qDebug() << "queue was empty, so do nothing";
        return;  // no message, quit
    }
    QString message = QString::fromUtf8(element.blop); // get and split message
    QStringList vars = message.split(";");

    //qDebug() << "received message=" << message;
    //qDebug() << "vars" << vars.count() <<  vars;

    if(vars.count() == 4) emit Callback_OpenNewFile(vars.at(0), vars.at(1), vars.at(2), vars.at(3));
}

bool FileOpenWindow::isRunning()
{
    return _isRunning;
}

bool FileOpenWindow::sendMessage(const QString &message)
{
    _blop element;
    if (!_isRunning) return false;
    QByteArray byteArray(message.toUtf8());
    byteArray.append('\0');
    const char *from = byteArray.data();
    memcpy(element.blop, from, byteArray.size());
    sharedMemory.lock();
    MSQ_enQueue(element);
    sharedMemory.unlock();
    return true;
}

QString FileOpenWindow::getStatusBarContents()
{
    QString statusBarContents = statusBar()->currentMessage();

    return statusBarContents;
}

QString FileOpenWindow::getLogFilePath()
{
    QString logFilePath;
    if (messageWindow != Q_NULLPTR) {
        logFilePath = messageWindow->getLogFilePath();
    }

    return logFilePath;
}

/**
 * slot for unconnected channels button
 */
void FileOpenWindow::Callback_ActionUnconnected()
{
    int countPV=0;
    int countNotConnected=0;
    int countDisplayed = 0;

    if(pvWindow != (QMainWindow*) Q_NULLPTR) {
        pvWindow->show();
        return;
    }
    pvWindow = new QMainWindow();
    pvWindow->setWindowTitle(QString::fromUtf8("unconnected PV's"));
    pvWindow->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

    QVBoxLayout *l = new QVBoxLayout();

    pvWindow->resize(400, 250);

    pvTable = new QTableWidget();

    QPushButton *pushbutton = new QPushButton("close");
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(Callback_PVwindowExit()));

    l->addWidget(pvTable);
    l->addWidget(pushbutton);

    QWidget* widg = new QWidget();
    widg->setLayout(l);

    fillPVtable(countPV, countNotConnected, countDisplayed);

    pvWindow->setCentralWidget(widg);
    pvWindow->show();

    // set width of window
    int w = 0;
    int count = pvTable->columnCount();
    for (int i = 0; i < count; i++) w += pvTable->columnWidth(i);
    int maxW = (w + count + pvTable->verticalHeader()->width() + pvTable->verticalScrollBar()->width());
    pvWindow->setMinimumWidth(maxW+25);
}

void FileOpenWindow::Callback_PVwindowExit()
{
    pvWindow->hide();
}

void FileOpenWindow::fillPVtable(int &countPV, int &countNotConnected, int &countDisplayed)
{
    int count = 0;
    if(pvTable != (QTableWidget*) Q_NULLPTR) {
        pvTable->clear();
        pvTable->setColumnCount(4);
        pvTable->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        pvTable->setHorizontalHeaderLabels(QString("unconnected PV;object;plugin;filename").split(";"));
        pvTable->setAlternatingRowColors(true);
    }

    for (int i=0; i < mutexKnobData->GetMutexKnobDataSize(); i++) {
        knobData *kPtr = mutexKnobData->GetMutexKnobDataPtr(i);
        if(kPtr->index != -1) {
            if(!kPtr->edata.connected) {
                countNotConnected++;
            } else {
                if(kPtr->edata.displayCount > 0) countDisplayed++;
            }
            countPV++;
        }
    }

    if(pvTable != (QTableWidget*) Q_NULLPTR) {
        pvTable->setRowCount(countNotConnected);
        count = 0;
        for (int i=0; i < mutexKnobData->GetMutexKnobDataSize(); i++) {
            knobData *kPtr = mutexKnobData->GetMutexKnobDataPtr(i);
            if(kPtr->index != -1) {
                if(!kPtr->edata.connected) {
                    pvTable->setItem(count,0, new QTableWidgetItem(kPtr->pv));
                    pvTable->setItem(count,1, new QTableWidgetItem(kPtr->dispName));
                    pvTable->setItem(count,2, new QTableWidgetItem(kPtr->pluginName));
                    pvTable->setItem(count,3, new QTableWidgetItem(kPtr->fileName));
                    count++;
                }
            }
        }
        pvTable->resizeColumnsToContents();
        pvTable->horizontalHeader()->setStretchLastSection(true);
    }
}

/**
 *   in medm geometry is passed through XParseGeometry:
 *    XParseGeometry parses strings of the form
 *   "=<width>x<height>{+-}<xoffset>{+-}<yoffset>", where
 *   width, height, xoffset, and yoffset are unsigned integers.
 *   Example:  "=80x24+300-49"
 *   The equal sign is optional.
 *   It returns a bitmask that indicates which of the four values
 *   were actually found in the string. For each value found,
 *   the corresponding argument is updated;  for each value
 *   not found, the corresponding argument is left unchanged.
 *
 *   here we implement the following routines to implement the
 *   same behaviour
 */

int FileOpenWindow::ReadInteger(char *string, char **NextString)
{
    int Result = 0;
    int Sign = 1;
    if (*string == '+') string++;
    else if (*string == '-') {
        string++;
        Sign = -1;
    }

    for (; (*string >= '0') && (*string <= '9'); string++) {
        Result = (Result * 10) + (*string - '0');
    }

    *NextString = string;

    if (Sign >= 0) return Result;
    else return -Result;
}

int FileOpenWindow::parseGeometry(const char* string, int* x, int* y, int* width, int* height)
{
    int mask = NoValue;
    char *strind;
    unsigned int tempWidth=0, tempHeight=0;
    int tempX=0, tempY=0;
    char *nextCharacter;
    if (!string || (*string == '\0')) return mask;
    if (*string == '=') string++;  /* ignore possible '=' at beg of geometry spec */
    strind = const_cast<char *>(string);
    if (*strind != '+' && *strind != '-' && *strind != 'x') {
        tempWidth = ReadInteger(strind, &nextCharacter);
        if (strind == nextCharacter)
            return 0;
        strind = nextCharacter;
        mask |= WidthValue;
    }

    if (*strind == 'x' || *strind == 'X') {
        strind++;
        tempHeight = ReadInteger(strind, &nextCharacter);
        if (strind == nextCharacter) return 0;
        strind = nextCharacter;
        mask |= HeightValue;
    }

    if ((*strind == '+') || (*strind == '-')) {
        if (*strind == '-') {
            strind++;
            tempX = -ReadInteger(strind, &nextCharacter);
            if (strind == nextCharacter) return 0;
            strind = nextCharacter;
            mask |= XNegative;
        } else {
            strind++;
            tempX = ReadInteger(strind, &nextCharacter);
            if (strind == nextCharacter) return 0;
            strind = nextCharacter;
        }
        mask |= XValue;

        if ((*strind == '+') || (*strind == '-')) {
            if (*strind == '-') {
                strind++;
                tempY = -ReadInteger(strind, &nextCharacter);
                if (strind == nextCharacter) return 0;
                strind = nextCharacter;
                mask |= YNegative;
            } else {
                strind++;
                tempY = ReadInteger(strind, &nextCharacter);
                if (strind == nextCharacter) return 0;
                strind = nextCharacter;
            }
            mask |= YValue;
        }
    }

    /* If strind isn't at the end of the string then it's an invalid geometry specification. */

    if (*strind != '\0') return 0;
    if (mask & XValue) *x = tempX;
    if (mask & YValue) *y = tempY;
    if (mask & WidthValue)  *width = tempWidth;
    if (mask & HeightValue) *height = tempHeight;

    return mask;
}

void FileOpenWindow::parse_and_set_Geometry(QMainWindow *widget, QString parsestring) {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    int m = parseGeometry(parsestring.toLatin1().data(), &x, &y, &w, &h);

    QSize minSize = widget->minimumSize();
    QSize maxSize = widget->maximumSize();
    if ((m & XValue) == 0)
        x = widget->geometry().x();

    if ((m & YValue) == 0)
        y = widget->geometry().y();

    if ((m & WidthValue) == 0)
        w = widget->width();

    if ((m & HeightValue) == 0)
        h = widget->height();

    w = qMin(w,maxSize.width());
    h = qMin(h,maxSize.height());
    w = qMax(w,minSize.width());
    h = qMax(h,minSize.height());

    if ((m & XNegative)) {

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        x = qApp->desktop()->width()  + x - w;
#else
        x = QGuiApplication::primaryScreen()->availableGeometry().width()  + x - w;
#endif
        x -= (widget->frameGeometry().width() - widget->width()) / 2;
    } else {
        x += (widget->geometry().x() - widget->x());
    }

    if ((m & YNegative)) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        y = qApp->desktop()->height() + y - h;
#else
        y = QGuiApplication::primaryScreen()->availableGeometry().height() + y - h;
#endif
    } else {
        y += (widget->geometry().y() - widget->y());
    }
    //qDebug() << "set window" << w << "to" << x << y << w << h;
    widget->setGeometry(x, y, w, h);
}

void FileOpenWindow::shellCommand(QString command) {
#ifndef MOBILE
    command = command.trimmed();
    myQProcess *proc = new myQProcess( this);
    proc->start(command.trimmed(), QIODevice::ReadWrite);
#else
    Q_UNUSED(command);
#endif
}

void FileOpenWindow::closeEvent(QCloseEvent* ce)
{
    Q_UNUSED(ce);
    fromIOS = false;
    Callback_ActionExit();
    ce->ignore();
}
#ifdef MOBILE
bool FileOpenWindow::event(QEvent *e)
{
    if (e->type() == QEvent::Show) {
        qDebug()<<"QEvent::Show!";
        // Qt 6.5.2 ShowMinimized=crash in QWidget::event better solution = setVisible(false)
        if (!debugWindow) this->setVisible(false);
    }
    return QWidget::event(e);
}
#endif

bool FileOpenWindow::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    if (event->type() == QEvent::MouseMove) caQtDM_TimeLeft = caQtDM_TimeOut;
    return false;
}
