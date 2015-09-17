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
#endif
#include "searchfile.h"

#include <QtGui>

#include "fileopenwindow.h"
#include "specialFunctions.h"
#include "fileFunctions.h"
#include "loadPlugins.h"

#ifdef MOBILE
  #include "fingerswipegesture.h"
#endif

#include <iostream>
#include <string>

#include <QFileDialog>
#include <QString>
#include "messagebox.h"
#include "configDialog.h"

#ifdef linux
#include <sys/resource.h>
#include <sys/time.h>
#endif

#ifdef Q_WS_X11
        #include <QX11Info>
        #include <X11/Xutil.h>
        #include <X11/Xlib.h>
        #include <X11/Xatom.h>

        #define MESSAGE_SOURCE_OLD            0
        #define MESSAGE_SOURCE_APPLICATION    1
        #define MESSAGE_SOURCE_PAGER          2
#endif //Q_WS_X11

#if defined(_MSC_VER)
int setenv(const char *name, const char *value, int overwrite)
{
    int errcode = 0;
    if(!overwrite) {
        size_t envsize = 0;
        errcode = getenv_s(&envsize, NULL, 0, name);
        if(errcode || envsize) return errcode;
    }
    return _putenv_s(name, value);
}
#endif


#if QT_VERSION > 0x050000
void FileOpenWindow::onApplicationStateChange(Qt::ApplicationState state)
{

#ifdef MOBILE
    int pendio;
    switch (state) {
         case Qt::ApplicationSuspended:
             qDebug() << "application state changed to suspended";
             //exit(0);
             break;
         case Qt::ApplicationHidden:
             qDebug() << "application state changed to hidden";
             break;
         case Qt::ApplicationInactive:
             qDebug() << "application state changed to inactive";
             pendio = false;
             if (mutexKnobData != (MutexKnobData *) 0) {
                 for (int i=0; i < mutexKnobData->GetMutexKnobDataSize(); i++) {
                     knobData *kPtr = mutexKnobData->GetMutexKnobDataPtr(i);
                     if(kPtr->index != -1)  {
                       //qDebug() << "should disconnect" << kPtr->pv;
                       ControlsInterface * plugininterface = (ControlsInterface *) kPtr->pluginInterface;
                       plugininterface->pvDisconnect(kPtr);
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
              if (mutexKnobData != (MutexKnobData *) 0) {
                  for (int i=0; i < mutexKnobData->GetMutexKnobDataSize(); i++) {
                      knobData *kPtr = mutexKnobData->GetMutexKnobDataPtr(i);
                      if(kPtr->index != -1) {
                        ControlsInterface * plugininterface = (ControlsInterface *) kPtr->pluginInterface;
                        if(plugininterface != (ControlsInterface *) 0) plugininterface->pvReconnect(kPtr);
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
            if(plugininterface != (ControlsInterface *) 0) plugininterface->FlushIO();
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
            if(plugininterface != (ControlsInterface *) 0) plugininterface->TerminateIO();
        }
    }
}

/**
 * our main window (form) constructor
 */
FileOpenWindow::FileOpenWindow(QMainWindow* parent,  QString filename, QString macroString,
                               bool attach, bool minimize, QString geometry, bool printscreen, bool resizing): QMainWindow(parent)
{
    // definitions for last opened file
    debugWindow = true;
    fromIOS = false;
    lastWindow = (QMainWindow*) 0;
    lastMacro ="";
    lastFile = "";
    lastGeometry = geometry;
    userClose = false;
    printandexit = printscreen;
    allowResize = resizing;
    minimizeMessageWindow = minimize;
    activWindow = 0;
    Specials specials;

    qDebug() <<  "caQtDM -- desktop size:" << qApp->desktop()->size();

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
    Q_IMPORT_PLUGIN(DemoPlugin);
    Q_IMPORT_PLUGIN(Epics3Plugin);
    Q_IMPORT_PLUGIN(Epics4Plugin);
#endif

    // message window used by library and here
    QWidget *widget =new QWidget();
    messageWindow = new MessageWindow(widget);

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

#ifdef MOBILE
    specials.setNewStyleSheet(messageWindow, qApp->desktop()->size(), 16, 10);
#endif
    messageWindow->setAllowedAreas(Qt::TopDockWidgetArea);
    QGridLayout *gridLayoutCentral = new QGridLayout(this->ui.centralwidget);
    QGridLayout *gridLayout = new QGridLayout();
    gridLayoutCentral->addLayout(gridLayout, 0, 0, 1, 1);
    gridLayout->addWidget(messageWindow, 0, 0, 1, 1);
    messageWindow->show();

#ifndef MOBILE
#ifdef Q_WS_X11
    QString uniqueKey = QString("caQtDM shared memory:") + DisplayString(QX11Info::display());
    sharedMemory.setKey (uniqueKey);
#else
    QString uniqueKey = QString("caQtDM shared memory");
    sharedMemory.setKey ("caQtDM shared memory");
#endif
    if (sharedMemory.attach()) {
        _isRunning = true;
        if(attach) {
#ifdef Q_WS_X11
            qDebug() << "caQtDM -- another instance of caQtDM detected ==> attach to it (" << DisplayString(QX11Info::display()) <<")" ;
#else
            qDebug() << "caQtDM -- another instance of caQtDM detected ==> attach to it";
#endif
            QString message(filename);
            message.append(";");
            message.append(macroString);
            message.append(";");
            message.append(geometry);
            //qDebug() << "send a message with file, macro and geometry to it and exit "<< message;
            sendMessage(message);
            sharedMemory.detach();
            qApp->exit(0);  // does not work here
            exit(0);
        } else {
            qDebug() << "caQtDM -- another instance of caQtDM detected, but no attach specified ==> standalone";
        }
    } else {
        QByteArray byteArray("0");
        _isRunning = false;
        // create shared memory with a default value to note that no message is available.
        if (!sharedMemory.create(255)) {
            qDebug("caQtDM -- Unable to create single instance.");
        } else {
            qDebug() << "caQtDM -- created shared memory";
            sharedMemory.lock();
            char *to = (char*)sharedMemory.data();
            const char *from = byteArray.data();
            memcpy(to, from, qMin(sharedMemory.size(), byteArray.size()));
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

    pvWindow = (QMainWindow*) 0;
    pvTable = (QTableWidget*) 0;

//************************************************************************************************************************************************
    if(HTTPCONFIGURATOR) {
    // test reading a local configuration file in order to start caQtDM for ios (read caQTDM_IOS_Config.xml, display its data, choose configuration,
    // then get from the choosen website and choosen config file the epics configuration and ui file to launch
    QSize desktopSize = qApp->desktop()->size();
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

    // download the choosen configurations file from the choosen url
    int success = filefunction.checkFileAndDownload(file, url);
    if(!success) {
        QMessageBox::critical(0, tr("caQtDM"), tr("could not download file %1 from %2").arg(file).arg(url));
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

    // load the control plugins (must be done after setting the environment)
    loadPlugins loadplugins;
    if (!loadplugins.loadAll(interfaces, mutexKnobData, messageWindow )) {
        QMessageBox::critical(this, "Error", "Could not load any plugin");
    } else {
        if(!interfaces.isEmpty()) {
            QMapIterator<QString, ControlsInterface *> i(interfaces);
            while (i.hasNext()) {
                char asc[256];
                i.next();
                sprintf(asc, "Info: plugin %s loaded", i.key().toLatin1().constData());
                messageWindow->postMsgEvent(QtWarningMsg, asc);
            }
        }
    }

    // in case of http support, we add the temporary directory name to the CAQTDM_DISPLAY_PATH if not already set
    // only in case of non mobile plattforms
#ifndef MOBILE
    QString displayPath = (QString)  qgetenv("CAQTDM_DISPLAY_PATH");
    if(!displayPath.contains(specials.getStdPath())) {
       displayPath.append(":"); displayPath.append(specials.getStdPath());
       setenv("CAQTDM_DISPLAY_PATH", (char*) displayPath.toLatin1().constData(), 1);
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
}


void FileOpenWindow::parseConfigFile(const QString &filename, QList<QString> &urls, QList<QString> &files)
{
    QFile* file = new QFile(filename);

    qDebug() << filename;

    /* can not open file */
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(0, tr("caQtDM"), tr("could not open configuration file: %1").arg(filename));
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
            if(xml.name() == "configuration") continue;

            if(xml.name() == "url") {
                QXmlStreamAttributes attributes = xml.attributes();
                if(attributes.hasAttribute("value")) urls.append(attributes.value("value").toString());
            }

            if(xml.name() == "config") {
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
        QMessageBox::critical(0, tr("caQtDM"), tr("could not open configuration file: %1").arg(filename));
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
    char asc[2048];
    Specials specials;
    QString stdpathdoc =  specials.getStdPath();

    QString EnvFile=stdpathdoc;
    EnvFile.append("/");
    EnvFile.append(fileName);
    QFile file(EnvFile);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "open file error setAllEnviromentVariables", file.errorString());
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
            setenv(fields.at(0).toLatin1().constData(), envString.toLatin1().constData(), 1);
            //messageWindow->postMsgEvent(QtDebugMsg, (char*)envString.toLatin1().constData());
        } else if(line.size() > 0) {
            sprintf(asc, "environment variable could not be set from %s", line.toLatin1().constData());
            messageWindow->postMsgEvent(QtDebugMsg, asc);
        }
    }
    //Replacement for standard writable directory
    setenv("CAQTDM_DISPLAY_PATH",stdpathdoc.toLatin1().constData(),1);

    sprintf(asc, "epics configuration file loaded: %s", fileName.toLatin1().constData());
    messageWindow->postMsgEvent(QtDebugMsg, asc);
    file.close();
}

void FileOpenWindow::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    char asc[255];
    int countPV=0;
    int countNotConnected=0;
    float highCount = 0.0;
    QString highPV;
    int countDisplayed = 0;
    static int printIt = 0;
    static int timeout = 0;

    if(mustOpenFile) {
        mustOpenFile = false;
        Callback_OpenNewFile(lastFile, lastMacro, lastGeometry);
    }

    if(minimizeMessageWindow) {
        minimizeMessageWindow = false;
        showMinimized ();
    }

    asc[0] = '\0';

#ifdef linux
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    sprintf(asc, "memory: %ld kB", usage.ru_maxrss);
#endif

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

    // any non connected pv's to display ?

    if (mutexKnobData != (MutexKnobData *) 0) {
        char msg[255];
        msg[0] = '\0';
        fillPVtable(countPV, countNotConnected, countDisplayed);
        highCount = mutexKnobData->getHighestCountPV(highPV);
        if(highCount != 0.0) {
            sprintf(msg, "%s - PV=%d (%d NC), %d Monitors/s, %d Displays/s, highest=%s with %.1f Monitors/s ", asc, countPV, countNotConnected,
                      mutexKnobData->getMonitorsPerSecond(), mutexKnobData->getDisplaysPerSecond(), highPV.toLatin1().constData(), highCount);
        } else {
            strcpy(msg, asc);
        }
        statusBar()->showMessage(msg);
    }

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
}

/**
 * slot for emptying our local http cache
 */
void FileOpenWindow::Callback_EmptyCache()
{
#ifndef MOBILE
    Specials specials;
    QString path =  specials.getStdPath();
    path.append("/");
    QDir dir(path);
    dir.setNameFilters(QStringList() << "*.ui" << "*.prc" << "*.gif" << "*.jpg" << "*.png");
    dir.setFilter(QDir::Files);
    foreach(QString dirFile, dir.entryList()) dir.remove(dirFile);
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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open ui or prc file"), path, tr("ui/prc Files (*.ui *.prc)"));
    //std::cout << "Got filename: " << fileName.toStdString() << std::endl;

    if(!fileName.isNull()) {
        char asc[1000];
        QFileInfo fi(fileName);
        lastFilePath = fi.absolutePath();
        if(fi.exists()) {
            CaQtDM_Lib *newWindow = new CaQtDM_Lib(this, fileName, "", mutexKnobData, interfaces, messageWindow);
            if (fileName.contains("prc")) {
                allowResize = false;
            }
            newWindow->allowResizing(allowResize);
 #ifdef MOBILE
            newWindow->grabSwipeGesture(fingerSwipeGestureType);
 #endif

            QMainWindow *mainWindow = newWindow;

#if defined(WIN32) && !defined(__GNUC__)
			QDesktopWidget * Desktop = QApplication::desktop();
			QRect defscreengeo = Desktop->availableGeometry(-1);//Defaultscreen=-1
			int mainw_width = mainWindow->width();
			int mainw_height = mainWindow->height();
			int movx = (defscreengeo.width() / 2) - (mainw_width / 2);
			int movy = (defscreengeo.height() / 2) - (mainw_height / 2);
			mainWindow->move(movx, movy);
#endif

            mainWindow->show();
            mainWindow->raise();
            mainWindow->setMinimumSize(0, 0);
            mainWindow->setMaximumSize(16777215, 16777215);
            mainWindow->setWindowFlags(mainWindow->windowFlags() );
            mainWindow->setProperty("fileString", fileName);
            mainWindow->setProperty("macroString", "");

            sprintf(asc, "last file: %s", fileName.toLatin1().constData());
            messageWindow->postMsgEvent(QtDebugMsg, asc);

            if (fileName.contains("prc")) {
                mainWindow->resize(mainWindow->minimumSizeHint());
            }
            activWindow=0;

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
void FileOpenWindow::Callback_OpenNewFile(const QString& inputFile, const QString& macroString, const QString& geometry)
{
    //qDebug() << "*************************************************************************";
    //qDebug() << "callback open new file" << inputFile << "with macro string" << macroString;

    int found1 = inputFile.lastIndexOf(".ui");
    int found2 = inputFile.lastIndexOf(".adl");
    int found3 = inputFile.lastIndexOf(".prc");
    QString openFile = inputFile;
    if (found1 != -1) {
        openFile = inputFile.mid(0, found1);
    }
    if(found2 != -1) {
        openFile = inputFile.mid(0, found2);
    }

    QString FileName;
    if(found3 == -1)
        FileName = openFile.append(".ui");
    else
        FileName = inputFile;

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
#ifdef Q_WS_X11
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
#endif //Q_WS_X11

                return;
            }
        }
    }

    // this will check for file existence and when an url is defined, download the file from a http server
    fileFunctions filefunction;
    filefunction.checkFileAndDownload(FileName);
    if(filefunction.lastInfo().length() > 0) messageWindow->postMsgEvent(QtWarningMsg, (char*) filefunction.lastInfo().toLatin1().constData());
    if(filefunction.lastError().length() > 0)  messageWindow->postMsgEvent(QtCriticalMsg, (char*)filefunction.lastError().toLatin1().constData());

    // open file
    searchFile *s = new searchFile(FileName);
    QString fileNameFound = s->findFile();
    if(fileNameFound.isNull()) {
        QString message = QString(FileName);
        message.append(" does not exist");
        QTDMMessageBox *m = new QTDMMessageBox(QMessageBox::Warning, "file open error", message, ":/caQtDM-logos.png", QMessageBox::Close, this, Qt::Dialog, true);
        m->show();
    } else {
        char asc[2048];
        bool willPrint = false;
        //qDebug() << "file" << fileNameFound << "will be loaded" << "macro=" << macroString;

        if(printandexit) willPrint = true;
        CaQtDM_Lib *newWindow =  new CaQtDM_Lib(this, fileNameFound, macroString, mutexKnobData, interfaces, messageWindow, willPrint);
#ifdef MOBILE
        newWindow->grabSwipeGesture(fingerSwipeGestureType);
#endif
        if (FileName.contains("prc")) {
           allowResize = false;
        }
        newWindow->allowResizing(allowResize);
        QMainWindow *mainWindow = newWindow;

#if defined(WIN32) && !defined(__GNUC__)
		QDesktopWidget * Desktop = QApplication::desktop();
		QRect defscreengeo = Desktop->availableGeometry(-1);//Defaultscreen=-1
		int mainw_width = mainWindow->width();
		int mainw_height = mainWindow->height();
		int movx = (defscreengeo.width() / 2) - (mainw_width / 2);
		int movy = (defscreengeo.height() / 2) - (mainw_height / 2);
		mainWindow->move(movx, movy);
#else
        Q_UNUSED(geometry);
#endif


        if(printandexit) {
            mainWindow->showMinimized();
        } else {
            mainWindow->show();
        }
        mainWindow->raise();
        mainWindow->setMinimumSize(0, 0);
        mainWindow->setMaximumSize(16777215, 16777215);
        mainWindow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mainWindow->setWindowFlags(mainWindow->windowFlags() );

        // get the filename
        mainWindow->setProperty("fileString", fileNameFound);
        mainWindow->setProperty("macroString", macroString);

        if (FileName.contains("prc")) {
            mainWindow->resize(mainWindow->minimumSizeHint());
        }
        activWindow = 0;

        //qDebug() << "set properties in qmainwindow" << mainWindow << macroString;

        if(geometry != "") {
            parse_and_set_Geometry(mainWindow, geometry);
        }

        // for this kind of file we resize to a minimum while the size is not known
        if (FileName.contains("prc")) {
            mainWindow->resize(mainWindow->minimumSizeHint());
        }

        if(macroString.size() > 0) {
          sprintf(asc, "last file: %s, macro: %s", fileNameFound.toLatin1().constData(), macroString.toLatin1().constData());
        } else {
          sprintf(asc, "last file: %s", fileNameFound.toLatin1().constData());
        }
        messageWindow->postMsgEvent(QtDebugMsg, asc);
    }
    delete s;
}

/**
 * slot for icon signal
 */
void FileOpenWindow::Callback_ActionAbout()
{
    QString message = QString("Qt-based Epics Display Manager Version %1 using Qt %2 and %3 with data from %4 developed at Paul Scherrer Institut, by Anton Mezger\nPlattform support is supported by H.Brands\n");
    message = message.arg(BUILDVERSION, QT_VERSION_STR, BUILDARCH, SUPPORT);
    QTDMMessageBox *m = new QTDMMessageBox(QMessageBox::Information, "About", message, ":/caQtDM-logospsi.png", QMessageBox::Close, this, Qt::Dialog, true);
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
        if(debugWindow) selected = QMessageBox::No;
        else selected = QMessageBox::Yes;
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

void FileOpenWindow::Callback_ActionReload()
{
    QPoint position;
    QString macroS = "";
    QString fileS = "";

    // block reload button
   this->ui.reloadAction->blockSignals(true);

    Callback_ActionTimed();
    // block processing during reload
    mutexKnobData->BlockProcessing(true);

    // go through all windows, close them and reload them from files
    QList<QWidget *> all = this->findChildren<QWidget *>();
    foreach(QWidget* widget, all) {
        if(CaQtDM_Lib* w = qobject_cast<CaQtDM_Lib *>(widget)) {
            QVariant fileName = w->property("fileString");
            QVariant macroString = w->property("macroString");
            if(!macroString.isNull()) {
                macroS = macroString.toString();
            }
            //qDebug() << fileName;

            position = w->pos();

            w->close();
            if(!fileName.isNull()) {

                QString FileName = fileName.toString();  

                // this will check for file existence and when an url is defined, download the file from a http server
                QFileInfo fi(FileName);
                fileFunctions filefunction;
                filefunction.checkFileAndDownload(fi.fileName());
                if(filefunction.lastInfo().length() > 0) messageWindow->postMsgEvent(QtWarningMsg, (char*) filefunction.lastInfo().toLatin1().constData());
                if(filefunction.lastError().length() > 0) messageWindow->postMsgEvent(QtCriticalMsg, (char*)filefunction.lastError().toLatin1().constData());

                searchFile *s = new searchFile(FileName);
                QString fileNameFound = s->findFile();
                fileS = fileNameFound;

                CaQtDM_Lib *newWindow =  new CaQtDM_Lib(this, fileS, macroS, mutexKnobData, interfaces, messageWindow);
                newWindow->allowResizing(allowResize);
#ifdef MOBILE
                newWindow->grabSwipeGesture(fingerSwipeGestureType);
#endif
                QMainWindow *mainWindow = newWindow;

                mainWindow->show();
                mainWindow->move(position);
                mainWindow->raise();
                mainWindow->setMinimumSize(0, 0);
                mainWindow->setMaximumSize(16777215, 16777215);
                mainWindow->setWindowFlags( mainWindow->windowFlags() );
                mainWindow->setProperty("fileString", fileS);
                mainWindow->setProperty("macroString", macroS);
                if (FileName.contains("prc")) {
                    mainWindow->resize(mainWindow->minimumSizeHint());
                }
                activWindow = 0;
            }
        }
    }

    mutexKnobData->BlockProcessing(false);

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
    // check for message in memory
    sharedMemory.lock();
    QByteArray byteArray = QByteArray((char*)sharedMemory.constData(), sharedMemory.size());
    sharedMemory.unlock();

    if (byteArray.left(1) == "0") return;  // no message, quit
    byteArray.remove(0, 1);                // remove first character
    QString message = QString::fromUtf8(byteArray.constData()); // get and split message in filename and macro
    QStringList vars = message.split(";");

    //qDebug() << "received message=" << message;
    //qDebug() << "vars" << vars.count() <<  vars;

    emit Callback_OpenNewFile(vars.at(0), vars.at(1), vars.at(2));

    // remove message from shared memory.
    byteArray = "0";
    sharedMemory.lock();
    char *to = (char*)sharedMemory.data();
    const char *from = byteArray.data();
    memcpy(to, from, qMin(sharedMemory.size(), byteArray.size()));
    sharedMemory.unlock();
}

bool FileOpenWindow::isRunning()
{
    return _isRunning;
}

bool FileOpenWindow::sendMessage(const QString &message)
{
    if (!_isRunning) return false;
    QByteArray byteArray("1");
    byteArray.append(message.toUtf8());
    byteArray.append('\0');
    sharedMemory.lock();
    char *to = (char*)sharedMemory.data();
    const char *from = byteArray.data();
    memcpy(to, from, qMin(sharedMemory.size(), byteArray.size()));
    sharedMemory.unlock();
    return true;
}

/**
 * slot for unconnected channels button
 */
void FileOpenWindow::Callback_ActionUnconnected()
{
    int countPV=0;
    int countNotConnected=0;
    int countDisplayed = 0;

    if(pvWindow != (QMainWindow*) 0) {
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
    if(pvTable != (QTableWidget*) 0) {
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

    if(pvTable != (QTableWidget*) 0) {
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
        x = qApp->desktop()->width()  + x - w;
        x -= (widget->frameGeometry().width() - widget->width()) / 2;
    } else {
        x += (widget->geometry().x() - widget->x());
    }

    if ((m & YNegative)) {
        y = qApp->desktop()->height() + y - h;
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
