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

#ifdef PYTHON
#include <Python.h>
#endif

#include "caQtDM_Lib_global.h"
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

#if QT_VERSION > QT_VERSION_CHECK(5,0,0)
#ifndef MOBILE
#include "hmisharedeventbus.h"
#include "hmisharedconfiglistmanager.h"
#endif

#ifdef WEB
#include "websocketserver.h"
#include "webportpool.h"
#include "weblaunchermanager.h"
#endif
#endif

#include <QObject>

#include <QJsonDocument>
#include <QJsonArray>
#include <QToolBar>
#include <QUuid>
#include <QHostInfo>
#include <QMutableListIterator>

// interfacing widgets, handling their own data acquisition ... (thanks zai)
#include "caWidgetInterface.h"

// we are using for calculations postfix of epics
// therefore we need this include and also link with the epics libraries
// should probably be changed at some point.
#include <postfix.h>
#include <epicsVersion.h>

#ifdef MOBILE_ANDROID
#  include <unistd.h>
#endif

#if defined(linux) || defined(__FreeBSD__)
#  include <sys/wait.h>
#  include <sys/time.h>
#  include <unistd.h>
#endif

#ifdef __APPLE__
#  include <sys/time.h>
#  include <unistd.h>
#endif

#include "caqtdm_lib.h"
#include "uiconverter.h"
#include "fileFunctions.h"

#include "myMessageBox.h"
#include "alarmstrings.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QKeyCombination>
#endif

#ifdef AUSTRALIAN
#include <QEWidget.h>
#endif

#define PRINT(x)
#define min(x,y)   (((x) < (y)) ? (x) : (y))

#define ToolTipPrefix "<p style='background-color:yellow; color:black;'>"
#define ToolTipPostfix "</p>"

#define InfoStyle "style='background-color:lightyellow; color:black; white-space: pre-wrap;'"
#define InfoPrefix "<div " InfoStyle ">"
#define InfoPostfix "</div>"

// context texts
#define GETINFO         "Get Info"
#define SETASIS         "Set Mono/Color as defined"
#define SETGREY         "Set Color to Mono"
#define SETWAVELEN      "Set Mono to Spectrum Wavelength"
#define SETHOT          "Set Mono to Spectrum Hot"
#define SETHEAT         "Set Mono to Spectrum Heat"
#define SETJET          "Set Mono to Spectrum Jet"
#define SETCUSTOM       "Set Mono to Spectrum Custom"
#define KILLPROCESS 	"Kill Process"
#define UNDEFINEDMACROS "Undefined macros"
#define GLOBALSHORTCUTS "Global Shortcuts"
#define PRINTWINDOW 	"Print"
#define RELOADWINDOW 	"Reload"
#define RAISEWINDOW 	"Raise main window"
#define INCLUDES        "Include Files"
#define TOGGLESIZE      "Toggle fit to size"
#define CHANGEVALUE 	"Change Increment/Value"
#define CHANGEAXIS      "Change Axis"
#define RESETZOOM       "Reset Zoom"
#define INPUTDIALOG 	"Input Dialog"
#define FILEDIALOG      "File Dialog"
#define CHANGELIMITS 	"Change Limits/Precision"
#define COPYIMAGE       "Copy Image"
#define COPYDATACSV     "Copy Data as CSV"
#define PASTEDATACSV     "Paste Data as CSV"

#define POPUPDEFENITION "popup.ui"

#ifdef WEB
#define WEB_CARELATED_DISPLAY_ERROR_MSG "Unable to open this caRelatedDisplay, please check your caQtDM Web configuration."
#endif

// used for calculating visibility for several types of widgets
#define ComputeVisibility(x, obj)  {  \
    switch(obj->getVisibility()) { \
    case x::StaticV:\
    return true;\
    break; \
    case x::IfNotZero: \
    visible = (value != 0.0?true:false); \
    break;\
    case x::IfZero:\
    visible = (value == 0.0?true:false);\
    break;\
    case x::Calc:\
    visible = CalcVisibility(obj, result, valid);\
    break;\
    }}

// used for calculating visibility for several types of widgets
#define GetDefinedCalcString(x, obj, strng)  {  \
    switch(obj->getVisibility()) { \
    case x::StaticV:\
    strng = "static"; \
    break; \
    case x::IfNotZero: \
    strng = "IfNotZero"; \
    break;\
    case x::IfZero:\
    strng = "IfZero"; \
    break;\
    case x::Calc:\
    strng = obj->getVisibilityCalc();\
    break;\
    }}


// used for interfacing epics routines with (pv, text, ...)
#define QStringsToChars(x,y,z) \
    char param1[MAXPVLEN], param2[MAX_STRING_LENGTH], param3[80]; \
    QByteArray Parameter_1 = x.toLatin1().constData(); \
    QByteArray Parameter_2;\
    for (int i = 0; i < y.length(); ++i) {\
            QChar ch = y[i];\
            ushort unicode = ch.unicode();\
            if (unicode >= 128 && unicode <= 255) {\
                QByteArray utf8Bytes = QString(ch).toUtf8();\
                for (int j = 0; j < utf8Bytes.length(); ++j) {\
                    Parameter_2.append(static_cast<unsigned char>(utf8Bytes[j]));\
                }\
            } else {\
                Parameter_2.append(ch.toLatin1());\
            }\
        }\
    QByteArray Parameter_3 = z.toLatin1().constData(); \
    strncpy(param1, Parameter_1.constData(), MAXPVLEN-1); \
    strncpy(param2, Parameter_2.constData(), MAX_STRING_LENGTH-1); \
    strncpy(param3, Parameter_3.constData(), 80-1); \

// common code too many widgets; for several reasons we did not try to put similar code in base classes.
// colors back after no connect

#define SetColorsBack(obj)                            \
    if(!obj->property("Connect").value<bool>()) { \
    obj->setNormalColors();                   \
    obj->setProperty("Connect", true);        \
    }

// colors back after no connect
#define SetColorsNotConnected(obj)                    \
    obj->setAlarmColors(NOTCONNECTED);        \
    obj->setProperty("Connect", false);

// replace visibility channels while macro could be used */
#define replaceVisibilityChannels(obj) \
    if(i==0) obj->setChannelA(pv);  \
    if(i==1) obj->setChannelB(pv);  \
    if(i==2) obj->setChannelC(pv);  \
    if(i==3) obj->setChannelD(pv);

// get visibility channels
#define getVisibilityChannels(obj) \
    strng[0] = obj->getChannelA(); \
    strng[1] = obj->getChannelB(); \
    strng[2] = obj->getChannelC(); \
    strng[3] = obj->getChannelD();

// force an ernter event in order to change the cursor when access changes
// done here, while I did not want to reproduce this code in each widget
#define updateAccessCursor(obj)   \
   if(obj->hasFocus()) {          \
      QEvent* evt = new QEvent(QEvent::Enter); \
      QCoreApplication::postEvent(obj,evt);   \
   }

// ui "file" including the requested pep file (.prc), i.e. file with a layout description used at PSI
#define uiIntern "<?xml version=\"1.0\" encoding=\"UTF-8\"?> " \
    "<ui version=\"4.0\"> " \
    "<class>MainWindow</class> " \
    "<widget class=\"QMainWindow\" name=\"MainWindow\"> " \
    "<property name=\"geometry\"> " \
    "<rect> " \
    "<x>0</x> " \
    "<y>0</y> " \
    "<width>823</width> " \
    "<height>911</height> " \
    "</rect> " \
    "</property> " \
    "<property name=\"windowTitle\"> " \
    "<string>MainWindow</string> " \
    "</property> " \
    "<widget class=\"QWidget\" name=\"centralwidget\"> " \
    "<layout class=\"QGridLayout\" name=\"gridLayout\"> " \
    "<item row=\"0\" column=\"0\"> " \
    "<widget class=\"caInclude\" name=\"cainclude\"> " \
    "<property name=\"filename\" stdset=\"0\"> " \
    "<string notr=\"true\">%1</string> " \
    "</property> " \
    "</widget> " \
    "</item> " \
    "</layout> " \
    "</widget> " \
    "</widget> " \
    "<customwidgets> " \
    "<customwidget> " \
    "<class>caInclude</class> " \
    "<extends>QWidget</extends> " \
    "<header>caInclude</header> " \
    "</customwidget> " \
    "</customwidgets> " \
    "<resources/> " \
    "<connections/> " \
    "</ui>"

#define addColorTableActions \
    myMenu.addAction(GETINFO); \
    myMenu.addAction(SETASIS); \
    myMenu.addAction(SETGREY); \
    myMenu.addAction(SETWAVELEN); \
    myMenu.addAction(SETHOT); \
    myMenu.addAction(SETHEAT); \
    myMenu.addAction(SETJET); \
    myMenu.addAction(SETCUSTOM); \

//===============================================================================================
#define calcstring_length 256
#define MIN_FONT_SIZE 3

Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QTabWidget*)
Q_DECLARE_METATYPE(QStackedWidget*)
Q_DECLARE_METATYPE(QtMsgType)

// this sleep will not block the GUI and QThread::msleep is protected in Qt4.8 (so do not use that)
class Sleep
{
public:
    static void msleep(unsigned long msecs)
    {
#ifndef MOBILE_ANDROID
        QMutex mutex;
        mutex.lock();
        QWaitCondition waitCondition;
        waitCondition.wait(&mutex, msecs);
        mutex.unlock();
#else
        // not nice, but the above does not work on android now (does not wait)
        usleep(msecs * 100);
#endif
    }
};

#if !defined(useElapsedTimer)
double CaQtDM_Lib::rTime()
{
    return static_cast<double>(
        QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() * 1000.0
    );
}
#endif

/**
 * helper for fixing file lists to work with relative paths
 */
static bool fileListEntryResolves(const QString &fileName)
{
    if (fileName.isEmpty()) return false;

    QString fileNameUi = fileName;
    if (fileName.endsWith(".edl") || fileName.endsWith(".adl")) {
        fileNameUi.replace(".edl", ".ui").replace(".adl", ".ui");
    } else if (!fileName.endsWith(".ui")) {
        fileNameUi.append(".ui");
    }

    qCDebug(caRelatedDisplayLog) << "fileListEntryResolves: name=" << fileName << "len=" << fileName.size() << "uiVariant=" << fileNameUi;

    if (QFileInfo::exists(fileName) || QFileInfo::exists(fileNameUi)) {
        qCDebug(caRelatedDisplayLog) << "fileListEntryResolves: matched directly (cwd)";
        return true;
    }

    QString displayPath = (QString) qgetenv("CAQTDM_DISPLAY_PATH");
    QStringList paths = displayPath.split(pathSeparator, SKIP_EMPTY_PARTS);

    for (int i=0; i < paths.count(); i++) {
#if defined(_WIN32) || defined(_WIN64)
        paths[i] = paths[i].replace("\"", "");
#endif
        bool hit = QFileInfo::exists(paths[i] + "/" + fileName) || QFileInfo::exists(paths[i] + "/" + fileNameUi);
        qCDebug(caRelatedDisplayLog) << "fileListEntryResolves: candidate[" << i << "]=" << QDir::toNativeSeparators(paths[i] + "/" + fileName) << "hit=" << hit;
        if (hit) {
            return true;
        }
    }

    qCDebug(caRelatedDisplayLog) << "fileListEntryResolves: NOT RESOLVED name=" << fileName;
    return false;
}

bool fixFileListRelative(const QString &cainclude_path, QString *filelist, bool shell=false)
{
    if (cainclude_path.isEmpty()) return false;

    QStringList files = filelist->split(";", SKIP_EMPTY_PARTS);
    bool affected = false;

    for (int i=0; i < files.count(); i++) {
        // Shell files are only treated if they start with "./" or "../", since otherwise they may be on PATH
        // Windows is not treated, as relative files might not have anything to differentiate from files on  and "./" is invalid on windows.
        QString fileName = files.at(i).trimmed();
        if (QFileInfo(fileName).isRelative() && (!shell || fileName.startsWith("./") || fileName.startsWith("../")) && !fileListEntryResolves(fileName)) {
            QFileInfo relativeFile(cainclude_path + fileName);
            QFileInfo relativeFileUi = relativeFile;
            if (relativeFileUi.absoluteFilePath().endsWith(".adl") || relativeFileUi.absoluteFilePath().endsWith(".edl")) {
                relativeFileUi.setFile(relativeFileUi.absoluteFilePath().replace(".adl", ".ui").replace(".edl", ".ui"));
            } else if (!relativeFileUi.absoluteFilePath().endsWith(".ui")) {
                relativeFileUi.setFile(relativeFileUi.absoluteFilePath() + ".ui");
            }

            if (relativeFile.exists() || relativeFileUi.exists()) {
                // We only process the file path here, not the extension conversion.
                // So even if it is found with a different extension, keep the specified one.
                files[i] = relativeFile.absoluteFilePath();
                affected = true;
            }
        }
    }

    if (affected) *filelist = files.join(";");
    return affected;
}

#ifdef WEB
static QString webResolveDisplayPath(const QString &file)
{
    qCDebug(webRelatedDisplay) << "webResolveDisplayPath: input=" << file << "len=" << file.size()
                               << "DISPLAY_PATH=" << (QString) qgetenv("CAQTDM_DISPLAY_PATH");

    fileFunctions filefunction;
    filefunction.checkFileAndDownload(file);

    searchFile filecheck(file);
    QString resolvedPath = filecheck.findFile();
    bool entryResolves = fileListEntryResolves(file);
    qCDebug(webRelatedDisplay) << "webResolveDisplayPath: findFile(input)=" << QDir::toNativeSeparators(resolvedPath)
                               << "fileListEntryResolves=" << entryResolves;
    if (!resolvedPath.isNull() || !entryResolves) {
        qCDebug(webRelatedDisplay) << "webResolveDisplayPath: return(early)=" << QDir::toNativeSeparators(resolvedPath);
        return resolvedPath;
    }

    QString fallback = file;
    if (fallback.endsWith(".edl") || fallback.endsWith(".adl")) {
        fallback.replace(".adl", ".ui").replace(".edl", ".ui");
    } else if (!fallback.endsWith(".ui")) {
        fallback.append(".ui");
    }

    if (fallback == file) {
        qCDebug(webRelatedDisplay) << "webResolveDisplayPath: fallback==input, return=" << QDir::toNativeSeparators(resolvedPath);
        return resolvedPath;
    }

    filefunction.checkFileAndDownload(fallback);
    searchFile fallbackCheck(fallback);
    QString fallbackResolved = fallbackCheck.findFile();
    qCDebug(webRelatedDisplay) << "webResolveDisplayPath: fallback=" << fallback << "findFile(fallback)=" << QDir::toNativeSeparators(fallbackResolved);
    return fallbackResolved;
}

static QString webOpenPathFromDisplayPath(const QString &path)
{
    QFileInfo pathInfo(path);
    if (pathInfo.isRelative()) return path;

    QString displayPath = (QString) qgetenv("CAQTDM_DISPLAY_PATH");
    QStringList paths = displayPath.split(pathSeparator, SKIP_EMPTY_PARTS);
    QString normalizedPath = QDir::cleanPath(path);
    QString bestRelativePath;

    for (int i=0; i < paths.count(); i++) {
#if defined(_WIN32) || defined(_WIN64)
        paths[i] = paths[i].replace("\"", "");
#endif
        QString basePath = QDir::cleanPath(paths[i]);
        if (basePath.isEmpty()) continue;

        QDir baseDir(basePath);
        QString relativePath = baseDir.relativeFilePath(normalizedPath);
        relativePath.replace('\\', '/');
        QFileInfo relativeInfo(relativePath);
        if (!relativeInfo.isAbsolute() && relativePath != ".." && !relativePath.startsWith("../") && fileListEntryResolves(relativePath)) {
            if (bestRelativePath.isEmpty() || relativePath.length() < bestRelativePath.length()) {
                bestRelativePath = relativePath;
            }
        }
    }

    if (!bestRelativePath.isEmpty()) return "./" + bestRelativePath;

    return QFileInfo(path).fileName();
}
#endif

Q_LOGGING_CATEGORY(caQtDMLibLog, "caqtdm.lib.lib")
Q_LOGGING_CATEGORY(fileIOLog, "caqtdm.lib.fileio")
Q_LOGGING_CATEGORY(caHMILog, "caqtdm.lib.cahmi")
Q_LOGGING_CATEGORY(caRelatedDisplayLog, "caqtdm.widgets.carelateddisplay")
Q_LOGGING_CATEGORY(caShellCommandLog, "caqtdm.widgets.cashellcommand")
Q_LOGGING_CATEGORY(caLabelLog, "caqtdm.widgets.calabel")
Q_LOGGING_CATEGORY(caLabelVerticalLog, "caqtdm.widgets.calabelvertical")
Q_LOGGING_CATEGORY(caNumericLog, "caqtdm.widgets.canumeric")
Q_LOGGING_CATEGORY(caSpinboxLog, "caqtdm.widgets.caspinbox")
Q_LOGGING_CATEGORY(caMessageButtonLog, "caqtdm.widgets.camessagebutton")
Q_LOGGING_CATEGORY(caScriptButtonLog, "caqtdm.widgets.cascriptbutton")
Q_LOGGING_CATEGORY(caLedLog, "caqtdm.widgets.caled")
Q_LOGGING_CATEGORY(caBitnamesLog, "caqtdm.widgets.cabitnames")
Q_LOGGING_CATEGORY(caClockLog, "caqtdm.widgets.caclock")
Q_LOGGING_CATEGORY(caLinearGaugeLog, "caqtdm.widgets.calineargauge")
Q_LOGGING_CATEGORY(caCircularGaugeLog, "caqtdm.widgets.cacirculargauge")
Q_LOGGING_CATEGORY(caMeterLog, "caqtdm.widgets.cameter")
Q_LOGGING_CATEGORY(caFrameLog, "caqtdm.widgets.caframe")
Q_LOGGING_CATEGORY(wmSignalRescaleLog, "caqtdm.widgets.wmsignalrescale")

#ifndef MOBILE // Names cannot be duplicated, and on mobile we only have one translatable unit as output, so this would cause duplication
               // For regular builds they need to be defined here, since their other definition in the respective widget source files ends up in another translatable unit.
Q_LOGGING_CATEGORY(caCartesianPlotLog, "caqtdm.widgets.cacartesianplot")
Q_LOGGING_CATEGORY(caImageLog, "caqtdm.widgets.caimage")
Q_LOGGING_CATEGORY(caCalcLog, "caqtdm.widgets.cacalc")
Q_LOGGING_CATEGORY(caMimeDisplayLog, "caqtdm.widgets.camimedisplay")
Q_LOGGING_CATEGORY(caMenuLog, "caqtdm.widgets.camenu")
Q_LOGGING_CATEGORY(caCameraLog, "caqtdm.widgets.cacamera")
Q_LOGGING_CATEGORY(caChoiceLog, "caqtdm.widgets.cachoice")
Q_LOGGING_CATEGORY(caTextEntryLog, "caqtdm.widgets.catextentry")
Q_LOGGING_CATEGORY(caLineEditLog, "caqtdm.widgets.calineedit")
Q_LOGGING_CATEGORY(caMultiLineStringLog, "caqtdm.widgets.camultilinestring")
Q_LOGGING_CATEGORY(caGraphicsLog, "caqtdm.widgets.cagraphics")
Q_LOGGING_CATEGORY(caPolyLineLog, "caqtdm.widgets.capolyline")
Q_LOGGING_CATEGORY(caApplyNumericLog, "caqtdm.widgets.caapplynumeric")
Q_LOGGING_CATEGORY(caToggleButtonLog, "caqtdm.widgets.catogglebutton")
Q_LOGGING_CATEGORY(caSliderLog, "caqtdm.widgets.caslider")
Q_LOGGING_CATEGORY(caThermoLog, "caqtdm.widgets.cathermo")
Q_LOGGING_CATEGORY(caByteLog, "caqtdm.widgets.cabyte")
Q_LOGGING_CATEGORY(caByteControllerLog, "caqtdm.widgets.cabytecontroller")
Q_LOGGING_CATEGORY(caIncludeLog, "caqtdm.widgets.cainclude")
Q_LOGGING_CATEGORY(caWaterfallPlotLog, "caqtdm.widgets.cawaterfallplot")
Q_LOGGING_CATEGORY(caStripPlotLog, "caqtdm.widgets.castripplot")
Q_LOGGING_CATEGORY(caTableLog, "caqtdm.widgets.catable")
Q_LOGGING_CATEGORY(caWaveTableLog, "caqtdm.widgets.cawavetable")
Q_LOGGING_CATEGORY(replaceMacroLog, "caqtdm.widgets.replacemacro")
Q_LOGGING_CATEGORY(caScan2DLog, "caqtdm.widgets.cascan2d")
#ifdef WEB
Q_LOGGING_CATEGORY(webRelatedDisplay, "caqtdm.web.relateddisplay")
#endif
#endif

QList<QSharedPointer<caHMIConfigTransferItem>> CaQtDM_Lib::externalHmiConfigList;
QReadWriteLock CaQtDM_Lib::externalHmiConfigListLock;

QList<caHMIConfigTransferItem*> CaQtDM_Lib::hmiConfigList;
QReadWriteLock CaQtDM_Lib::hmiConfigListLock;

#ifdef WEB
QHash<QString, VncWebChildProcess*> CaQtDM_Lib::webChildProcesses;
QReadWriteLock CaQtDM_Lib::webChildProcessesLock;

quint16 CaQtDM_Lib::vncPortIndex = 1;
quint16 CaQtDM_Lib::vncPort;
quint16 CaQtDM_Lib::webPort;
QString CaQtDM_Lib::webHost;
bool CaQtDM_Lib::slaveServer;
bool CaQtDM_Lib::vncServer;
bool CaQtDM_Lib::noVncPlugin;
bool CaQtDM_Lib::noVncReadonly;
bool CaQtDM_Lib::interactionBasedTimeout;
bool CaQtDM_Lib::webAllowInsecureCaShellCommands;
uint CaQtDM_Lib::webTimeout;
quint16 CaQtDM_Lib::webInstanceLimit;
#endif

/**
 * CaQtDM_Lib destructor
 */
CaQtDM_Lib::~CaQtDM_Lib()
{

    disconnect(mutexKnobDataP,
               SIGNAL(Signal_UpdateWidget(int, QWidget*, const QString&, const QString&, const QString&, knobData)), this,
               SLOT(Callback_UpdateWidget(int, QWidget*, const QString&, const QString&, const QString&, knobData)));

    //if(!fromAS) delete myWidget;
    includeWidgetList.clear();
    topIncludesWidgetList.clear();
    includeFilesList.clear();
    allCalcs_Vectors.clear();
    allTabs.clear();
    allStacks.clear();
}

/**
 * CaQtDM_Lib constructor
 */
CaQtDM_Lib::CaQtDM_Lib(QWidget *parent, QString filename, QString macro, MutexKnobData *mKnobData, QMap<QString,
                       ControlsInterface *> interfaces, MessageWindow *msgWindow, bool pepprint, QWidget *parentAS,
                       QMap<QString,QString> options) : QMainWindow(parent)
{
    QUiLoader loader;
    fromAS = false;
    AllowsUpdate = true;
    mutexKnobDataP = mKnobData;
    messageWindowP = msgWindow;
    controlsInterfaces = interfaces;
    pepPrint = pepprint;
    firstResize = true;
    loopTimer = 0;
    prcFile = false;

    // for cainclude, we need when updating internal positions to know about the resize factors
    this->setProperty("RESIZEX", 1.0);
    this->setProperty("RESIZEY", 1.0);

    // is a default plugin specified (normally nothing means epics3)
    QString option = options["defaultPlugin"];
    if(!option.isEmpty()) {
        if(getControlInterface(option) == (ControlsInterface *) Q_NULLPTR) {
            postMessage(QtCriticalMsg, (char*) qasc(tr("sorry -- specified default plugin %1 is not loaded, fallback to epics3").arg(option)));
            qCInfo(caQtDMLibLog) << "caQtDM -- specified default plugin" << option << "is not loaded, fallback to epics3";
            defaultPlugin = "";
        } else {
            defaultPlugin = option;
        }
    }

    // file watcher for changes
    watcher = new QFileSystemWatcher(this);
    QObject::connect(watcher, SIGNAL(fileChanged(const QString&)), this, SLOT(handleFileChanged(const QString&)));

    if(parentAS != (QWidget*) Q_NULLPTR) {
        fromAS = true;
        myWidget = parentAS;
    }

    qCDebug(fileIOLog) << "open file" << filename << "with macro" << macro;
    setAttribute(Qt::WA_DeleteOnClose);

    // define a layout
    QGridLayout *layout = new QGridLayout;
#ifdef ADL_EDL_FILES
    const bool isMedmFile = filename.endsWith (".adl");
    const bool isEdmFile = filename.endsWith (".edl");
    QFileInfo adledlcheck(filename);
    if(!adledlcheck.exists()){
        // if file is not existing try ui ending
        if (isMedmFile) filename=filename.replace(".adl",".ui");
        if (isEdmFile) filename=filename.replace(".edl",".ui");
    }
#endif
    // define the file to use
    QFile *file = new QFile;
    file->setFileName(filename);

    // treat ui file */
    QFileInfo fi(filename);

    if(!fromAS) {

        if(filename.lastIndexOf(".ui") != -1) {

            //symtomatic AFS check
            if (!(file->open(QFile::ReadOnly) && file->isOpen())){
                postMessage(QtDebugMsg, (char*) qasc(tr("can't open file %1 ").arg(filename)));
            }else{
                if (file->size()==0){
                    postMessage(QtDebugMsg, (char*) qasc(tr("file %1 has size zero ").arg(filename)));
                }else{
                    QBuffer *buffer = new QBuffer();
                    buffer->open(QIODevice::ReadWrite);
                    buffer->write(file->readAll());

                    buffer->seek(0);

                    myWidget = loader.load(buffer, this);
                    delete buffer;
                    qCDebug(fileIOLog) << "load= " << filename;
                }
            }
            if (!myWidget) {
                QMessageBox::warning(this, tr("caQtDM"), tr("Error loading %1. Use designer to find errors").arg(filename));
                file->close();
                delete file;
                this->deleteLater();
                return;
            }
            file->close();

            // treat prc file and load designer description from internal buffer
        } else if(filename.lastIndexOf(".prc") != -1) {

            prcFile = true;
            QString uiString = QString(uiIntern);
            uiString= uiString.arg(filename);
            QByteArray *array= new QByteArray();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            array->append(uiString);
#else
            array->append(uiString.toLocal8Bit());
#endif

            QBuffer *buffer = new QBuffer();
            buffer->open(QIODevice::ReadWrite);
            buffer->write(*array);
            delete array;

            buffer->seek(0);
            myWidget = loader.load(buffer, parent);
            buffer->close();
            delete buffer;

            if (!myWidget) {
                QMessageBox::warning(this, tr("caQtDM"), tr("Error loading %1. Use designer to find errors").arg(filename));
                file->close();
                delete file;
                this->deleteLater();
                return;
            }
#ifdef ADL_EDL_FILES
        } else if(isMedmFile || isEdmFile) {
            UiConverterInterface *otherFile = UiConverterFactory::create(filename);
            if(otherFile != (UiConverterInterface *) Q_NULLPTR) {
                if(otherFile->errorString().length() > 0) postMessage(QtDebugMsg, (char*) qasc(otherFile->errorString()));
                if(otherFile->ok()) {
                    myWidget = otherFile->load(this);
                }
                delete otherFile;
            }
            if (!myWidget) {
                QMessageBox::warning(this, tr("caQtDM"), tr("Error loading %1. Use designer to find errors").arg(filename));
                this->deleteLater();
                return;
            }
#endif
        } else {
            qCCritical(fileIOLog) << "caQtDM -- internal error with fileName= " << filename;
            this->deleteLater();
            return;
        }

        // set window title without the whole path
        QString title(file->fileName().section('/',-1));
        thisFileShort = file->fileName().section('/',-1);
        thisFileFull = fi.absoluteFilePath ();

        // user wants to have its own title
        QVariant myTitle=myWidget->property("Title");
        if(!myTitle.isNull()) {
            QString Title = myTitle.toString();
            QMap<QString, QString> map;
            map = createMap(macro);
            reaffectText(map, &Title, myWidget);
            title = Title;
        }

        setWindowTitle(title);
        setUnifiedTitleAndToolBarOnMac(true);

        delete file;

        // define size of application
        setMinimumSize(myWidget->width(), myWidget->height());
        setMaximumSize(myWidget->width(), myWidget->height());

        // add widget to the gui
        layout->addWidget(myWidget);

        QWidget *centralWidget = new QWidget;
        centralWidget->setLayout(layout);
        centralWidget->layout()->setContentsMargins(0,0,0,0);
        setCentralWidget(centralWidget);

#ifdef MOBILE
        // info can be called with tapandhold
        connect(this, SIGNAL(Signal_NextWindow()), parent, SLOT(nextWindow()));
        installEventFilter(this);
#endif
    }

    // connect all signals of our propagators
    QList<wmSignalPropagator *> allM = this->findChildren<wmSignalPropagator *>();
    foreach(wmSignalPropagator* widget, allM) {
        connect(widget, SIGNAL(wmCloseWindow()), this, SLOT(closeWindow()));
        connect(widget, SIGNAL(wmShowNormal()), this, SLOT(showNormalWindow()));
        connect(widget, SIGNAL(wmShowMaximized()), this, SLOT(showMaxWindow()));
        connect(widget, SIGNAL(wmShowMinimized()), this, SLOT(showMinWindow()));
        connect(widget, SIGNAL(wmShowFullScreen()), this, SLOT(showFullWindow()));
        connect(widget, SIGNAL(wmReloadWindow()), this, SLOT(Callback_ReloadWindowL()));
        connect(widget, SIGNAL(wmPrintWindow()), this, SLOT(Callback_printWindow()));
        connect(widget, SIGNAL(wmResizeMainWindow(QRect&)), this, SLOT(resizeFullWindow(QRect&)));
    }

    // connect close launchfile action to parent
    connect(this, SIGNAL(Signal_IosExit()), parent, SLOT(Callback_IosExit()));

    // connect reload window action to parent
    connect(this, SIGNAL(Signal_ReloadWindowL()), this, SLOT(Callback_ReloadWindowL()));
    connect(this, SIGNAL(Signal_ReloadWindow(QWidget*)), parent, SLOT(Callback_ReloadWindow(QWidget*)));
    connect(this, SIGNAL(Signal_ReloadAllWindows()), parent, SLOT(Callback_ReloadAllWindows()));

    qRegisterMetaType<knobData>("knobData");

    // connect signals to slots for exchanging data
    connect(mutexKnobDataP, SIGNAL(Signal_QLineEdit(const QString&, const QString&)), this,
            SLOT(Callback_UpdateLine(const QString&, const QString&)));

    connect(mutexKnobDataP,
            SIGNAL(Signal_UpdateWidget(int, QWidget*, const QString&, const QString&, const QString&, knobData)), this,
            SLOT(Callback_UpdateWidget(int, QWidget*, const QString&, const QString&, const QString&, knobData)));

    if(!fromAS) {
        connect(this, SIGNAL(Signal_OpenNewWFile(const QString&, const QString&, const QString&, const QString&)), parent,
                SLOT(Callback_OpenNewFile(const QString&, const QString&, const QString&, const QString&)));
    }

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenu(const QPoint&)));
    connect(parent, SIGNAL(themeChanged()), this, SLOT(themeChanged()));
#ifndef MOBILE
    this->globalEventFilter = new HMIApplicationEventFilter(this);

    if (qApp != Q_NULLPTR){
        qCDebug(caHMILog) << "global event filter installed for caHMIConfig";
        qApp->installEventFilter(globalEventFilter);
        connect(this->globalEventFilter, &HMIApplicationEventFilter::keyPressed, this, &CaQtDM_Lib::hmiHandleKeyPressed);
        connect(this->globalEventFilter, &HMIApplicationEventFilter::mouse, this, &CaQtDM_Lib::hmiHandleMouse);
    }

    if (HmiSharedEventBus::instance().isInitialized()) {
        connect(&HmiSharedEventBus::instance(), &HmiSharedEventBus::eventReceived, this, &CaQtDM_Lib::Callback_ExternalHmiEventReceived);
    }
#endif

    level=0;

    // Using just path here since it should work better with URLs than converting to abs path
    //cainclude_path = fi.absolutePath() + "/";
    cainclude_path = fi.path() + "/";

    // say for all widgets that they have to be treated, will be set to true when treated to avoid multiple use
    // by findChildren, and get the list of all the includes at this level

    includeWidgetList.clear();
    includeFilesList.clear();
    topIncludesWidgetList.clear();
    allTabs.clear();
    allStacks.clear();
    allCalcs_Vectors.clear();
    cartesianGroupList.clear();
    cartesianList.clear();
    stripGroupList.clear();
    stripList.clear();
    softvars.clear();

    nbIncludes = 0;
    splashCounter = 1;
    QList<QWidget *> all = myWidget->findChildren<QWidget *>();
    foreach(QWidget* widget, all) {
        widget->setProperty("Taken", false);
        if(caInclude* include = qobject_cast<caInclude *>(widget)){
            Q_UNUSED(include);
            topIncludesWidgetList.append(include);
            nbIncludes++;
        }
    }

    if(nbIncludes > 0 && !thisFileFull.contains(POPUPDEFENITION)) {
#ifdef WEB
        if (vncServer && WebSocketServer::instance().isInitialized()) {
            WebSocketServer::instance().sendProgressInfo(0, nbIncludes);
        } else {
#endif
            splash = new SplashScreen(parent);
            splash->setMaximum(nbIncludes);
            splash->show();
            splash->setProgress(0);
#ifdef WEB
        }
#endif
    }

    savedFile[0] = fi.baseName();
    savedMacro[0] = macro;

    scanWidgets(myWidget->findChildren<QWidget *>(), macro);

    // build a list for getting all soft pv
    mutexKnobDataP->BuildSoftPVList(myWidget);

    // setup changeevent for QTabWidgets
    allTabs = myWidget->findChildren<QTabWidget *>();
    foreach(QTabWidget* widget, allTabs) {
        connect(widget, SIGNAL(currentChanged(int)), this, SLOT(Callback_TabChanged(int)));
    }
    // setup changeevent for QStackedWidgets
    allStacks = myWidget->findChildren<QStackedWidget *>();
    foreach(QStackedWidget* widget, allStacks) {
        connect(widget, SIGNAL(currentChanged(int)), this, SLOT(Callback_TabChanged(int)));
    }
    // setup changeevent for QStackedWidgets
    allCalcs_Vectors.clear();
    QList<caCalc *> allCalcs = myWidget->findChildren<caCalc *>();
    foreach(caCalc* w, allCalcs) {
        // when cacalc is a waveform composed from individual channels
        if(w->getVariableType() == caCalc::vector) {
            allCalcs_Vectors.append(w);
        }
    }



    // scan cartesianplots and find groups != 0; compose a list with the groups, and a map with the group key
    QList<caCartesianPlot *> allCarts = myWidget->findChildren<caCartesianPlot *>();
    if(allCarts.count() > 0) {
        foreach(caCartesianPlot* widget, allCarts) {
            if( widget->getXaxisSyncGroup() != 0) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                cartesianList.insertMulti(widget->getXaxisSyncGroup(), widget);
#else
                cartesianList.insert(widget->getXaxisSyncGroup(), widget);
#endif
                if(!cartesianGroupList.contains(widget->getXaxisSyncGroup())) cartesianGroupList.append(widget->getXaxisSyncGroup());
            }
        }
    }

    // scan stripplots and find groups != 0; compose a list with the groups, and a map with the group key
    QList<caStripPlot *> allStrips = myWidget->findChildren<caStripPlot *>();
    if(allStrips.count() > 0) {
        foreach(caStripPlot* widget, allStrips) {
            if( widget->getXaxisSyncGroup() != 0) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                stripList.insertMulti(widget->getXaxisSyncGroup(), widget);
#else
                stripList.insert(widget->getXaxisSyncGroup(), widget);
#endif
                if(!stripGroupList.contains(widget->getXaxisSyncGroup())) stripGroupList.append(widget->getXaxisSyncGroup());
            }
        }
    }

    setContextMenuPolicy(Qt::CustomContextMenu);

    // start a timer
    loopTimerID = startTimer(1000);

    // all interfaces flush io
    FlushAllInterfaces();

    // due to crash in connection with the splash screen, changed
    // these instructions to the botton of this class
    if(nbIncludes > 0 && !thisFileFull.contains(POPUPDEFENITION) && !vncServer) {
        Sleep::msleep(200);
        // this seems to causes the crash and is not really needed here?
        //splash->finish(this);

        splash->deleteLater();
    }
    // reapply a globally loaded user stylesheet, cainlude seems to disable it
    qCInfo(fileIOLog) << "caQtDM -- user_defined_stylesheet:" << qApp->property("user_defined_stylesheet").toString();
    if (qApp->property("user_defined_stylesheet").isValid() && (!qApp->property("user_defined_stylesheet").toString().isEmpty())){
        QString printdata=qApp->styleSheet();
        QString stylereload = (QString)  qgetenv("CAQTDM_STYLESHEET_RELOAD");
        if (stylereload.contains("file",Qt::CaseInsensitive)){
            qCInfo(fileIOLog) << "caQtDM -- search for:" << qApp->property("user_defined_stylesheet").toString();
            searchFile *searchDefaultStyleSheet = new searchFile(qApp->property("user_defined_stylesheet").toString());
            QString fileNameFound = searchDefaultStyleSheet->findFile();
            qCInfo(fileIOLog) << "caQtDM -- custom stylesheet found:" << fileNameFound;
            if(!fileNameFound.isEmpty()) {
                QFile file(fileNameFound);
                if (file.open(QFile::ReadOnly)) {
                    QString StyleSheet = QLatin1String(file.readAll());
                    printdata=StyleSheet;
                    qCInfo(fileIOLog) << "caQtDM -- custom stylesheet file:" << fileNameFound << "reloaded stylesheet";
                    if (stylereload.contains("later",Qt::CaseInsensitive)){
                        QTimer::singleShot(3000, this, [this,StyleSheet] () {
                                this->setStyleSheet(StyleSheet);
                            });
                    }else setStyleSheet(StyleSheet);
                    file.close();
                }
            }
            delete searchDefaultStyleSheet;
        }
        if (stylereload.contains("apply",Qt::CaseInsensitive)){
            setStyleSheet(qApp->styleSheet());
        }

        if (stylereload.contains("print",Qt::CaseInsensitive)){
            qCInfo(fileIOLog) << "caQtDM -- custom stylesheet file data:" << printdata;
        }
    }

    // add a reload action
    QAction *ReloadWindowAction = new QAction(this);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    ReloadWindowAction->setShortcut(QApplication::translate("MainWindow", "Ctrl+R", 0, QApplication::UnicodeUTF8));
#else
    ReloadWindowAction->setShortcut(QApplication::translate("MainWindow", "Ctrl+R", Q_NULLPTR));
#endif
    connect(ReloadWindowAction, SIGNAL(triggered()), this, SLOT(Callback_ReloadWindowL()));
    this->addAction(ReloadWindowAction);

    // add also a global reload action
    QAction *ReloadAllWindowsAction = new QAction(this);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    ReloadAllWindowsAction->setShortcut(QApplication::translate("MainWindow", "Ctrl+Alt+R", 0, QApplication::UnicodeUTF8));
#else
    ReloadAllWindowsAction->setShortcut(QApplication::translate("MainWindow", "Ctrl+Alt+R", Q_NULLPTR));
#endif
    connect(ReloadAllWindowsAction, SIGNAL(triggered()), this, SLOT(Callback_reloadAllWindows()));
    this->addAction(ReloadAllWindowsAction);

    if (!vncServer) {
        // add a print action
        QAction *PrintWindowAction = new QAction(this);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        PrintWindowAction->setShortcut(QApplication::translate("MainWindow", "Ctrl+P", 0, QApplication::UnicodeUTF8));
#else
        PrintWindowAction->setShortcut(QApplication::translate("MainWindow", "Ctrl+P", Q_NULLPTR));
#endif
        connect(PrintWindowAction, SIGNAL(triggered()), this, SLOT(Callback_printWindow()));
        this->addAction(PrintWindowAction);
    }

    // add a resize action
    QAction *ResizeUpAction = new QAction(this);
    ResizeUpAction->setShortcut(QKeySequence::ZoomIn);
    connect(ResizeUpAction, SIGNAL(triggered()), this, SLOT(Callback_ResizeUp()));
    this->addAction(ResizeUpAction);
    // add a resize action
    QAction *ResizeDownAction = new QAction(this);
    ResizeDownAction->setShortcut(QKeySequence::ZoomOut);
    connect(ResizeDownAction, SIGNAL(triggered()), this, SLOT(Callback_ResizeDown()));
    this->addAction(ResizeDownAction);


    QShortcut *CopyMarking = new QShortcut(tr("Ctrl+C"), this);
    connect(CopyMarking, SIGNAL(activated()), this, SLOT(Callback_CopyMarked()));

    // Ctrl+Alt+D was selected arbitrarily -> "Deselect" everything currently marked
    QShortcut *DeSelect = new QShortcut(tr("Ctrl+Alt+D"), this);
    connect(DeSelect, SIGNAL(activated()), this, SLOT(clearSelection()));

    char asc[MAX_STRING_LENGTH];
    QString path = thisFileFull;
    int pos = path.lastIndexOf("/");
    if((pos > 0) && ((path.length() - pos -1) > 0)) path.chop(path.length() - pos -1);
    snprintf(asc, MAX_STRING_LENGTH, "special macro CAQTDM_INTERNAL_UIPATH set to %s\n", qasc(path));
    postMessage(QtWarningMsg, asc);

    qstrncpy(asc,"unresolved macros present, press context in display to obtain a list",MAX_STRING_LENGTH);
    if(unknownMacrosList.count() > 0) postMessage(QtCriticalMsg, asc);
}

/**
 * resize scrollbars of scrollwidget containing a caInclude
 */
void CaQtDM_Lib::ResizeScrollBars(caInclude * includeWidget, int sizeX, int sizeY)
{
    if(QScrollArea* scrollWidget = qobject_cast<QScrollArea *>(includeWidget->parent()->parent()->parent())) {
        Q_UNUSED(scrollWidget);
        QWidget *contents = (QWidget*) includeWidget->parent();
        contents->setMinimumSize(sizeX, sizeY);
    }
}

/**
 * when a QTabWidget exist, this slot will enable or disable the io for invisible stuff
 */
void CaQtDM_Lib::Callback_TabChanged(int current)
{
    Q_UNUSED(current);
    // Enable & Disable IO when in invisible page of a TabWidget
    EnableDisableIO();
    FlushAllInterfaces();

    // due to some problem with the legend of cartesianplot in tab widgets, we should update its layout
    QTabWidget *tabwidget = qobject_cast<QTabWidget *>(sender());
    if(tabwidget == (QTabWidget*) Q_NULLPTR) return;

    QList<caCartesianPlot*> children = tabwidget->findChildren< caCartesianPlot*>();
    foreach(caCartesianPlot* w1, children) {
        if(w1->getLegendEnabled()) {
            qreal fontsize = w1->property("legendfontsize").value<qreal>();
            QFont f = QFont("Arial");
            f.setPointSizeF(fontsize);
            w1->setLegendAttribute(w1->getScaleColor(), f, caCartesianPlot::FONT);
            w1->updateLayout();
        }
    }

}

/**
 * find for a widget if it is contained in a parent QTabWidget
 */
QWidget* CaQtDM_Lib::getTabParent(QWidget *w1)
{
    QObject *Parent = w1->parent();
    QWidget *tabWidget = (QTabWidget*) Q_NULLPTR;
    qCDebug(caQtDMLibLog) << w1->objectName();

    while(Parent != (QObject*) Q_NULLPTR) {
        qCDebug(caQtDMLibLog) << "parent=" << Parent;
        Parent = Parent->parent();
        if(Parent != (QObject*) Q_NULLPTR) {
            if(QTabWidget* widget = qobject_cast<QTabWidget *>(Parent)) {
                tabWidget = (QWidget*) widget;
                qCDebug(caQtDMLibLog) << "found" << widget;
                return tabWidget;
            }
            if(QStackedWidget* widget = qobject_cast<QStackedWidget *>(Parent)) {
                tabWidget = (QWidget*) widget;
                if(!widget->objectName().contains("qt_tabwidget_stackedwidget")) {
                    qCDebug(caQtDMLibLog) << "found" << widget;
                    return tabWidget;
                }
            }
        } else {
            return (QWidget *) Q_NULLPTR;
        }
    }
    return (QWidget *) Q_NULLPTR;
}

/**
 * this routine will go through all are ca objects, find their nearest QTabWidget or  QStackedWidget if any and decides
 * to enable or disable their monitor
 */
void CaQtDM_Lib::scanChildren(QList<QWidget*> children, QWidget *tab, int indexTab) {

    void *ptr1, *ptr2;
    int currentIndex;

    // go through our ca objects on this page (except for caStripplot and cawaterfallplot, needing history data)
    foreach(QWidget* w1, children) {
        bool treatit = false;
        QString className = w1->metaObject()->className();
        if(className.contains("ca") &&
                !className.contains("caStripPlot") &&
                !className.contains("caWaterfallPlot")) treatit = true;
/* this would enable again all the monitors used by a hidden cacalc with signals and would then inrease the load drastically again
        if(caCalc* calcWidget = qobject_cast<caCalc *>(w1)) {
           if(calcWidget->getEventSignal() != caCalc::Never) treatit = true;
        }
*/
        if(treatit) {
            // nearest parent tab
            QWidget* tabstack = (QWidget*) w1->property("parentTab").value<QWidget*>();

            // no tab
            if(tabstack != (QWidget*) Q_NULLPTR) {

                // the widget to be considered
                if(tabstack == tab) {
                    // get current tabindex
                    if(QTabWidget* nearestTab = qobject_cast<QTabWidget *>(tabstack)) {
                        currentIndex = nearestTab->currentIndex();
                    } else if(QStackedWidget* nearestStack = qobject_cast<QStackedWidget *>(tabstack)) {
                        currentIndex = nearestStack->currentIndex();
                    } else {
                        currentIndex = -1;
                    }
                    bool hidden = false;

                    if(!tabstack->isVisible()) {
                        qCDebug(caQtDMLibLog) << "thus on hidden tab";
                        hidden = true;
                        w1->setProperty("hidden", true);
                    } else if(indexTab == currentIndex) {
                        qCDebug(caQtDMLibLog) << "thus on visible tab";
                        hidden = false;
                        w1->setProperty("hidden", false);
                    } else {
                        qCDebug(caQtDMLibLog) << "thus on hidden tab";
                        hidden = true;
                        w1->setProperty("hidden", true);
                    }

                    qCDebug(caQtDMLibLog) << w1->objectName() << "sitting in " << tabstack << "actual position is" << currentIndex << hidden;

                    // get the associated monitor pointers and add or remove the event
                    QVariant var1=w1->property("InfoList");
                    QVariant var2=w1->property("Interface");
                    QVariantList infoList1 = var1.toList();
                    QVariantList infoList2 = var2.toList();
                    for(int j=0; j< qMin(infoList1.count(), infoList2.count()); j++) {
                        ptr1 = (void*) infoList1.at(j).value<void *>();
                        ptr2 = (void*) infoList2.at(j).value<void *>();
                        if((ptr1 != (void*) Q_NULLPTR) && (ptr2 != (void*) Q_NULLPTR)) {
                            ControlsInterface * plugininterface = (ControlsInterface *) ptr2;
                            if(plugininterface != (ControlsInterface *) Q_NULLPTR) {
                                if(!hidden) {
                                    plugininterface->pvAddEvent(ptr1);
                                } else {
                                    plugininterface->pvClearEvent(ptr1);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/**
 * go through all tab and stacked widgets
 */
void CaQtDM_Lib::EnableDisableIO()
{
#ifdef IO_OPTIMIZED_FOR_TABWIDGETS

    // any tabwidgets in this window ? when not do nothing
    qCDebug(caQtDMLibLog) << allTabs.count() << allStacks.count();

    if((allTabs.count() == 0) && (allStacks.count() == 0)) return;

    // go through are QTabWidgets
    foreach(QTabWidget* widget, allTabs) {
        // go through their pages
        for(int i=0; i<widget->count(); i++) {
            qCDebug(caQtDMLibLog) << widget << widget->currentIndex() << widget->tabText(i);
            QList<QWidget*> children = widget->widget(i)->findChildren<QWidget *>();
            scanChildren(children, widget, i);
        }
    }

    // go through are QStackedWidgets
    foreach(QStackedWidget* widget, allStacks) {
        // go through their pages
        for(int i=0; i<widget->count(); i++) {
            if(!widget->objectName().contains("qt_tabwidget_stackedwidget")) {
                qCDebug(caQtDMLibLog) << widget << widget->currentIndex();
                QList<QWidget*> children = widget->widget(i)->findChildren<QWidget *>();
                scanChildren(children, widget, i);
            }
        }
    }

    FlushAllInterfaces();
#endif
}

//extern uint qGlobalPostedEventsCount(); // from qapplication.cpp
//#include "private/qobject_p.h"
/**
 * timer event
 */
void CaQtDM_Lib::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    // for epics we flush the buffer every second
    FlushAllInterfaces();

    if(loopTimer == 5){
        EnableDisableIO();
        loopTimer = 0;
    }
    loopTimer++;

    //qCDebug(caQtDMLibLog) << qGlobalPostedEventsCount();
}

/**
 * this routine reaffects a text when macro is used
 */
bool CaQtDM_Lib::reaffectText(QMap<QString, QString> map, QString *text, QWidget *w) {
    bool doNothing;
    if(text->size() > 0) {
        if(text->contains("$(") && text->contains(")")) {
            *text =  treatMacro(map, *text, &doNothing, w->objectName());
            return true;
        }
    }
    return false;
}

/**
 * this routine will replace inside a macro string a value for a macro name when found
 */
QString CaQtDM_Lib::actualizeMacroString(QMap<QString, QString> map, QString argument)
{
    QString newMacro = "";
    QMap<QString, QString> mapArgs = createMap(argument);
    if(!mapArgs.isEmpty()) {
        // go through macro string and replace the value when this key equals a key in the map
        QMapIterator<QString, QString> i(mapArgs);
        while (i.hasNext()) {
            i.next();
            QMap<QString, QString>::const_iterator k = map.find(i.key());
            while (k != map.end() && k.key() == i.key()) {

                // only when a replacemacro uses this key with a value > 0
                bool replace = false;
                QList<replaceMacro *> all = myWidget->findChildren<replaceMacro *>();
                foreach(replaceMacro* widget, all) {
                    qCDebug(caQtDMLibLog) << widget;
                    QString key =  widget->getKey();
                    QString value = widget->getNewValue();
                    if(key == i.key() && value != Q_NULLPTR) {
                        replace = true;
                        qCDebug(caQtDMLibLog) << key << value << replace;
                        break;
                    }
                }

                qCDebug(caQtDMLibLog) << "found and replace " << i.key() <<  "with " << k.value() << "when replace=true : " << replace;
                if(replace) mapArgs.insert(i.key(), k.value());
                ++k;
            }
        }
        // flatten the macro (from map to string) macro will be of type A=MMAC3,B=STR,C=RMJ:POSA:2
        QMapIterator<QString, QString> k(mapArgs);
        while (k.hasNext()) {
            k.next();
            newMacro.append(k.key()+"="+k.value()+",");
        }
        newMacro = newMacro.left(newMacro.length() - 1);
        qCDebug(caQtDMLibLog) << "newmacro" << newMacro;
    }
   return newMacro;
}

/**
 * this routine replaces in this macro map when exists, a value from replaceMacro for a specified macro name
 */
QMap<QString, QString> CaQtDM_Lib::actualizeMacroMap()
{
    QMap<QString, QString> map;
    QVariant macroString = this->property("macroString");

    qCDebug(caQtDMLibLog) << "actualizeMacroMap macrostring" << macroString;

    if(!macroString.isNull()) {
        map = createMap(macroString.toString());
        if(!map.isEmpty()) {
            QMapIterator<QString, QString> i(map);
            while (i.hasNext()) {
                i.next();
                QString macroName = i.key();
                qCDebug(caQtDMLibLog) << "macroName" << macroName;

                // go through all the children of type replaceMacro
                QList<replaceMacro *> all = myWidget->findChildren<replaceMacro *>();
                foreach(replaceMacro* widget, all) {
                    if(widget->isEnabled()) {
                        qCDebug(caQtDMLibLog) << widget;
                        QString key =  widget->getKey();
                        QString value = widget->getNewValue();
                        qCDebug(caQtDMLibLog) << widget << key << value << macroName;
                        if(macroName == key && value.length() > 0) {
                            qCDebug(caQtDMLibLog) << i.key() << i.value();
                            map.insert(macroName, value);
                            qCDebug(caQtDMLibLog) << "map replace done for key" << macroName;
                        } else if(!i.key().contains(key) && value.length() && widget->getDefineMacro()) {
                            map.insert(key, value);
                            qCDebug(caQtDMLibLog) << "map insert done for key" << key;
                        }
                    }
                }
            }
        }
    }
    qCDebug(caQtDMLibLog) << "actualizeMacroMap" << map;
    return map;
}

/**
 * this routine will create a macro string from a QMap
 */
QString CaQtDM_Lib::createMacroStringFromMap(QMap<QString, QString> map)
{
    QString newMacro = "";
    // flatten the macro (from map to string) macro will be of type A=MMAC3,B=STR,C=RMJ:POSA:2
    QMapIterator<QString, QString> k(map);
    while (k.hasNext()) {
        k.next();
        newMacro.append(k.key()+"="+k.value()+",");
    }
    newMacro = newMacro.left(newMacro.length() - 1);
    return newMacro;
}

/**
 * this routine creates a QMap from a macro string
 */
QMap<QString, QString> CaQtDM_Lib::createMap(const QString& macro)
{
    qCDebug(caQtDMLibLog) << "treat macro" << macro;
    QMap<QString, QString> map;
    // macro of type A=MMAC3,B=STR,C=RMJ:POSA:2 to be used for replacements of pv in child widgets
    if(macro != Q_NULLPTR) {
        QStringList vars = macro.split(",", SKIP_EMPTY_PARTS);
        for(int i=0; i< vars.count(); i++) {
            int pos = vars.at(i).indexOf("=");
            if(pos != -1) {
                QString key = vars.at(i).mid(0, pos);
                QString value = vars.at(i).mid(pos+1);
                map.insert(key.trimmed(), value);
            } else {
                qCCritical(caQtDMLibLog) <<"macro" <<  macro << "could not be parsed";
            }
        }
    }
    qCDebug(caQtDMLibLog) << "create map from macro:" << macro;
    qCDebug(caQtDMLibLog) << "resulting map=" << map;
    return map;
}

void CaQtDM_Lib::scanWidgets(QList<QWidget*> list, QString macro)
{
    QMutableListIterator<QWidget*> i(list);
    while (i.hasNext()) {
        QString className(i.next()->metaObject()->className());
        if (className.contains("EPushButton")) i.remove();
    }


    // get first all primary softs (inorder that pv working on their own value will always be treated first
    qCDebug(caQtDMLibLog) << " ------------ first pass treat softs being involved in itsself (incrementing)";
    foreach(QWidget *w1, list) {
        HandleWidget(w1, macro, true, true);
    }
    qCDebug(caQtDMLibLog) << " ------------ first pass other softs";
    // other softpvs
    foreach(QWidget *w1, list) {
            HandleWidget(w1, macro, true, false);
    }
    // other pvs
    qCDebug(caQtDMLibLog) << " ------------ no first pass other stuff";
    foreach(QWidget *w1, list) {
            HandleWidget(w1, macro, false, false);
    }
}

/**
 * this routine handles the initialization of all widgets
 */
void CaQtDM_Lib::HandleWidget(QWidget *w1, QString macro, bool firstPass, bool treatPrimary)
{
    QList<QVariant> integerList;
    QMap<QString, QString> map;
    knobData kData;
    int specData[5] = {0,0,0,0,0};
    memset(&kData, 0, sizeof (knobData));
    bool doNothing;
    QString pv;

    QString className(w1->metaObject()->className());
    if(!className.contains("ca") && !className.contains("QTextBrowser") && !className.contains("replaceMacro") &&
            !className.contains("genSoftPV") &&
            !className.contains("QE") && !className.contains("QTabWidget")&& !className.contains("QGroupBox")) return;

    int nbMonitors = 0;

    integerList.clear();

    QVariant test=w1->property("Taken");
    if(!test.isNull()) {
        if(test.toBool()) return;
    }

    qCDebug(caQtDMLibLog) << w1->metaObject()->className() << w1->objectName();

    if(className.contains("ca") || className.contains("QTextBrowser") || className.contains("replaceMacro") || className.contains("genSoftPV") || className.contains("QTabWidget")|| className.contains("QGroupBox")) {
        PRINT(printf("\n%*c %s macro=<%s>", 15 * level, '+', qasc(w1->objectName()), qasc(macro)));
        map = createMap(macro);
        // insert special macro into map
        map.insert("CAQTDM_INTERNAL_UIFULLFILE", thisFileFull);
        QString path = thisFileFull;
        int pos = path.lastIndexOf("/");
        if((pos > 0) && ((path.length() - pos -1) > 0)) path.chop(path.length() - pos -1);
        map.insert("CAQTDM_INTERNAL_UIPATH", path);

        QString filename = thisFileFull.right(thisFileFull.length()-path.length()).trimmed();
        map.insert("CAQTDM_INTERNAL_UIFILENAME", filename);

        map.insert("CAQTDM_INTERNAL_STARTTIME", QTime::currentTime().toString());
        map.insert("CAQTDM_INTERNAL_STARTDATE", QDate::currentDate().toString("dd.MM.yyyy"));

        map.insert("CAQTDM_INTERNAL_VERSION", TARGET_VERSION_STR);

        QString message = QString("%1");
        message = message.arg(QT_VERSION_STR);
        map.insert("CAQTDM_INTERNAL_QTVERSION", message);

        path =qApp->applicationFilePath();
        pos = path.lastIndexOf("/");
        if((pos > 0) && ((path.length() - pos -1) > 0)) path.chop(path.length() - pos -1);
        map.insert("CAQTDM_INTERNAL_EXEPATH", path);

        map.insert("CAQTDM_INTERNAL_PID",QString::number(qApp->applicationPid()));
        map.insert("CAQTDM_INTERNAL_HOSTNAME", QHostInfo::localHostName());


#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

        map.insert("CAQTDM_INTERNAL_SCREENCOUNT",QString::number( qApp->desktop()->screenCount()));
        map.insert("CAQTDM_INTERNAL_DPI",QString::number( qApp->desktop()->physicalDpiX()));
#else
        map.insert("CAQTDM_INTERNAL_SCREENCOUNT",QString::number( qApp->screens().count()));
        map.insert("CAQTDM_INTERNAL_DPI",QString::number(qApp->primaryScreen()->physicalDotsPerInch()));

#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
        map.insert("CAQTDM_INTERNAL_REFRESHRATE",QString::number(qApp->primaryScreen()->refreshRate()));
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        map.insert("CAQTDM_INTERNAL_DESKTOP_WIDTH",QString::number(qApp->desktop()->size().width()));
        map.insert("CAQTDM_INTERNAL_DESKTOP_HEIGHT",QString::number(qApp->desktop()->size().height()));
#else
        map.insert("CAQTDM_INTERNAL_DESKTOP_WIDTH",QString::number(qApp->primaryScreen()->size().width()));
        map.insert("CAQTDM_INTERNAL_DESKTOP_HEIGHT",QString::number(qApp->primaryScreen()->size().height()));
#endif


        map.insert("CAQTDM_INTERNAL_CA_ADDRLIST",qgetenv("EPICS_CA_ADDR_LIST"));
        map.insert("CAQTDM_INTERNAL_BS_ADDRLIST",qgetenv("BSREAD_ZMQ_ADDR_LIST"));
        map.insert("CAQTDM_INTERNAL_BS_DISPATCHER",qgetenv("BSREAD_DISPATCHER"));
        map.insert("CAQTDM_INTERNAL_ARCHIVESF_URL",qgetenv("CAQTDM_ARCHIVERSF_URL"));
    }

    QColor bg = w1->property("background").value<QColor>();
    QColor fg = w1->property("foreground").value<QColor>();
    QColor lg = w1->property("lineColor").value<QColor>();

    // keep original colors
    w1->setProperty("BColor", bg);
    w1->setProperty("FColor", fg);
    w1->setProperty("LColor", lg);

    // say not hideen

    w1->setProperty("hidden", false);

    // when first pass specified, treat only caCalc
    //==================================================================================================================
    if(firstPass) {
        if(caCalc* calcWidget = qobject_cast<caCalc *>(w1)) {

            //bool doit;
            w1->setProperty("ObjectType", caCalc_Widget);
            QWidget *tabWidget = getTabParent(w1);
            w1->setProperty("parentTab",QVariant::fromValue(tabWidget) );

            kData.soft = true;

            QString pv = calcWidget->getVariable();
            if(pv.size() == 0) {
                pv =  QUuid::createUuid().toString();
                pv = pv.replace("{", "");  // otherwise a json string, that would be taken out
                pv = pv.replace("}", "");
            // no plugin can be set, while it is a softvariable; however it will write to the real variable of the same name when changing
            } else if(pv.contains("://")) {
                int pos = pv.indexOf("://");
                if(pos != -1) pv = pv.mid(pos+3);
            }
            reaffectText(map, &pv, w1);
            calcWidget->setVariable(pv);
            calcWidget->setDataCount(0);
            addMonitor(myWidget, &kData, qasc(pv), w1, specData, map, &pv);
            // Special treatment for QRect
            if (calcWidget->getCalc().startsWith("%QRect")){
                double qrectvalues[12];
                QString qrectscan=calcWidget->getCalc();
                qrectscan=qrectscan.right(qrectscan.length()-6);
                if (!qrectscan.isEmpty()){
                    qCDebug(caCalcLog) << "Check QRectString: "<< qrectscan;
                    if (parseForQRectConst(qrectscan,qrectvalues)){
                        for (int i=0;i<4;i++)
                            calcWidget->setQRectParam(i,qrectvalues[i]);
                    }
                }
            }

            // test for multiple occurrences
            if(treatPrimary) {
                bool found = false;
                QString fileFound = "";
                QHash<QString, QString>::const_iterator i = softvars.find(pv);
                while (i != softvars.end() && i.key() == pv) {
                    fileFound = i.value();
                    found = true;
                    break;
                }
                if(!found) {
                    softvars.insert(pv, savedFile[level]);
                } else {
                    postMessage(QtCriticalMsg, (char*) qasc(tr("cacalc softvariable %1 in file %2 already defined in file %3").arg(pv).arg(savedFile[level]).arg(fileFound)));
                }
            }

            qCDebug(caCalcLog) <<  "firstpass" << firstPass <<  "treatPrimary:" << treatPrimary << pv << calcWidget << SoftPVusesItsself(calcWidget, map);

            // when cacalc is a waveform composed from individual channels
            if(calcWidget->getVariableType() == caCalc::vector) {
                // test for waveform
                int num;
                kData.soft = false;
                QString text;
                QList<QVariant> monitorList;
                QList<QVariant> indexList;
                QList<QString> pvList = calcWidget->getPVList();

                // add all channels
                nbMonitors = pvList.count();
                calcWidget->setDataCount(nbMonitors);
                for(int i=0; i<pvList.count(); i++) {
                    specData[0] = i;
                    text = treatMacro(map, pvList[i], &doNothing, w1->objectName());
                    num = addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
                    monitorList.append(num);
                    indexList.append(i);
                }
                monitorList.insert(0, nbMonitors);
                indexList.insert(0, nbMonitors);
                calcWidget->setProperty("MonitorList", monitorList);
                calcWidget->setProperty("IndexList", indexList);
                calcWidget->setValue(calcWidget->getVariable());
            }

            // softchannels calculating with themselves are done first
            else if(SoftPVusesItsself(calcWidget, map) && treatPrimary) {
                //doit=true;
                qCDebug(caCalcLog) << "softchannels calculating with themselves have to be done first: doit";

            // softchannels not using themselves are done second
            } else if(!SoftPVusesItsself(calcWidget, map) && !treatPrimary) {
                //doit=true;
                qCDebug(caCalcLog) << "softchannels not using themselves are done second: doit";

            // softchannels not using themselves, but that just define themselves
            } else {
                qCDebug(caCalcLog) << "softchannels that just define themselves: dont";
                return;
            }
            // when no monitors then inititalize value
            if(nbMonitors == 0) {
                qCDebug(caCalcLog) << "update " << qasc(calcWidget->getVariable()) << "initial value" << calcWidget->getInitialValue();
                calcWidget->setValue(calcWidget->getInitialValue());
                mutexKnobDataP->UpdateSoftPV(calcWidget->getVariable(), calcWidget->getInitialValue(), myWidget, 0, 1);
            }

            // other channels if any
            kData.soft = false;
            // if cacalc is a simple double then calculate its value with the up to 4 channels
            if(calcWidget->getVariableType() == caCalc::scalar) nbMonitors = InitVisibility(w1, &kData, map, specData, qasc(calcWidget->getVariable()));


            w1->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(w1, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenu(const QPoint&)));
            w1->setProperty("Connect", false);


            connect(w1, SIGNAL(changeValue(double)), this, SLOT(Callback_CaCalc(double)));

            calcWidget->setProperty("Taken", true);
        }

        // generic soft pvs for the internal plugin, prioritized like the caCalc softs
        else if(genSoftPV* softpvWidget = qobject_cast<genSoftPV *>(w1)) {
            if(treatPrimary) {
                w1->setProperty("Taken", true);

                QString pv = softpvWidget->getVariable();
                reaffectText(map, &pv, w1);
                if(pv.isEmpty()) {
                    postMessage(QtWarningMsg, (char*) qasc(tr("genSoftPV widget %1 has no variable name, ignored").arg(w1->objectName())));
                    return;
                }
                softpvWidget->setVariable(pv);

                w1->setProperty("ObjectType", caCalc_Widget);
                QWidget *tabWidget = getTabParent(w1);
                w1->setProperty("parentTab", QVariant::fromValue(tabWidget));

                pv = "internal://" + pv;
                addMonitor(myWidget, &kData, qasc(pv), w1, specData, map, &pv);

                w1->setContextMenuPolicy(Qt::CustomContextMenu);
                connect(w1, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenu(const QPoint&)));
                w1->setProperty("Connect", false);
            }
        }

        return;
    }
/*
    if(caScripting* ws = dynamic_cast<caScripting *>(w1)) {
        ws->setParent(myWidget);
        ws->evaluate();
    }
*/
    // any caWidget with caWidgetInterface - actually caInoutDemo (new monitor displaying values/string)
    if(caWidgetInterface* wif = dynamic_cast<caWidgetInterface *>(w1)) {
        wif->caActivate(this, map, &kData, specData, myWidget);
        nbMonitors = 1; // assume at least one monitor;
    }

    // not a ca widget, but offer the possibility to change the tab text by using macros
    //==================================================================================================================
    if(QTabWidget* tabWidget = qobject_cast<QTabWidget *>(w1)) {
        for(int i=0; i< tabWidget->count(); i++) {
            QString text =  tabWidget->tabText(i);
            if(reaffectText(map, &text, w1)) tabWidget->setTabText(i, text);
        }
    // not a ca widget, but offer the possibility to change the title text by using macros
        //==================================================================================================================
      } else if(QGroupBox* groupBoxWidget = qobject_cast<QGroupBox *>(w1)) {
            qCDebug(caQtDMLibLog)<<"groupBoxWidget:"<<groupBoxWidget->objectName();
            QString text =  groupBoxWidget->title();
            if(reaffectText(map, &text, w1)) groupBoxWidget->setTitle(text);
    // not a ca widget, but offer the possibility to load files into the text browser by using macros
    //==================================================================================================================
    } else if(QTextBrowser* browserWidget = qobject_cast<QTextBrowser *>(w1)) {

        qCDebug(caQtDMLibLog) << "create QTextBrowser";

        QString source = browserWidget->source().toString();
        if(reaffectText(map, &source, w1))  browserWidget->setSource(source);
        QString fileName = browserWidget->source().path();

        if(!fileName.isEmpty()) {
            qCInfo(fileIOLog) << "caQtDM -- watch file" << source;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
            bool success = watcher->addPath(fileName);
            if(!success) qCWarning(fileIOLog) << fileName << "can not be watched for changes";
            else qCInfo(fileIOLog) << fileName << "is watched for changes";
#else
            watcher->addPath(fileName);
#endif
        // we could also look at the searchpath, perhaps there is an url we want to get
        } else {

            QList<QString> list = browserWidget->searchPaths();
            //when the file name has a url form then get the file from a website
            if(list.count() > 0 && list.at(0).contains("http")) {
                QUrl url = QUrl::fromUserInput(list.at(0));
                 if (!url.isValid()) {
                      qCWarning(fileIOLog) << QString("Invalid URL: %1").arg(url.toString());
                 // try to load from that url
                 } else {
                     fileFunctions filefunction;
                     QList<QString> elements = url.path().split("/");
                     fileName = elements.last();
                     QString Url = url.toString();
                     Url.remove(fileName);
                     // first try to remove the file from the temporary cache directoy
                     Specials specials;
                     QString path = specials.getStdPath();
                     path.append("/");
                     QDir dir(path);
                     dir.remove(fileName);

                     // try to download the file
                     filefunction.checkFileAndDownload(fileName, Url);
                     if(messageWindowP != (MessageWindow *) Q_NULLPTR) {
                         if(filefunction.lastInfo().length() > 0) messageWindowP->postMsgEvent(QtInfoMsg, (char*) qasc(filefunction.lastInfo()));
                         if(filefunction.lastError().length() > 0)  messageWindowP->postMsgEvent(QtCriticalMsg, (char*) qasc(filefunction.lastError()));
                     }
                     searchFile *s = new searchFile(fileName);
                     QString fileNameFound = s->findFile();
                     if(!fileNameFound.isNull()) {
                         browserWidget->setSource(fileNameFound);
                     }
                 }
            }
        }
        browserWidget->setProperty("Taken", true);

        // the different widgets to be handled
        //==================================================================================================================
    } else if(caImage* imageWidget = qobject_cast<caImage *>(w1)) {

        qCDebug(caImageLog) << "create caImage";
        w1->setProperty("ObjectType", caImage_Widget);

        if(imageWidget->getFileName().size() > 0) {
            QString text = imageWidget->getFileName();
            reaffectText(map, &text, w1);
            fixFileListRelative(cainclude_path, &text);
            // Always call this, since isProvisional = false tells the widget to only now emit errors if still not found, initially errors were suppressed
            imageWidget->setFileName(text, false);
        }

        // any error messages for this object?
        while(imageWidget->anyMessages()) {
            QString message = imageWidget->getMessages();
            if(message.contains("Info:")) postMessage(QtWarningMsg, (char*) qasc(message));
            else postMessage(QtCriticalMsg, (char*) qasc(message));
        }

        nbMonitors = InitVisibility(w1, &kData, map, specData, "");

        // empty calc string, set animation
        if(imageWidget->getImageCalc().size() == 0) {
            qCDebug(caImageLog) <<  "no calc for image";
            imageWidget->setFrame(0);
            imageWidget->startMovie();
        }

        imageWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caRelatedDisplay* relatedWidget = qobject_cast<caRelatedDisplay *>(w1)) {

        qCDebug(caRelatedDisplayLog) << "create caRelatedDisplay" << relatedWidget << relatedWidget->getLabels() << relatedWidget->getArgs() <<  relatedWidget->getFiles();
        w1->setProperty("ObjectType", caRelatedDisplay_Widget);

        QString text;

        text = relatedWidget->getLabels();
        if(reaffectText(map, &text, w1))  relatedWidget->setLabels(text);

        text = relatedWidget->getArgs();
        if(reaffectText(map, &text, w1))  relatedWidget->setArgs(text);

        text = relatedWidget->getFiles();
        qCDebug(caRelatedDisplayLog) << "create caRelatedDisplay files: cainclude_path=" << QDir::toNativeSeparators(cainclude_path) << "before=" << text;
        bool affected = reaffectText(map, &text, w1);
        qCDebug(caRelatedDisplayLog) << "create caRelatedDisplay files: afterReaffect=" << text << "affected=" << affected;
        affected |= fixFileListRelative(cainclude_path, &text);
        qCDebug(caRelatedDisplayLog) << "create caRelatedDisplay files: afterFixRelative=" << text << "affected=" << affected;
        if (affected) relatedWidget->setFiles(text);

        text = relatedWidget->getLabel();
        if(reaffectText(map, &text, w1))  relatedWidget->setLabel(text);

        connect(relatedWidget, SIGNAL(clicked(int)), this, SLOT(Callback_RelatedDisplayClicked(int)));
        connect(relatedWidget, SIGNAL(triggered(int)), this, SLOT(Callback_RelatedDisplayClicked(int)));

        if(relatedWidget->isElevated()) relatedWidget->raise();

        relatedWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caShellCommand* shellWidget = qobject_cast<caShellCommand *>(w1)) {

        qCDebug(caShellCommandLog) << "create caShellCommand";
        w1->setProperty("ObjectType", caShellCommand_Widget);

        QString text;
        text= shellWidget->getLabels();
        if(reaffectText(map, &text, w1))  shellWidget->setLabels(text);

        text = shellWidget->getArgs();
        if(reaffectText(map, &text, w1))  shellWidget->setArgs(text);

        text = shellWidget->getFiles();
        bool affected = reaffectText(map, &text, w1);
        affected |= fixFileListRelative(cainclude_path, &text, true);
        if (affected) shellWidget->setFiles(text);

        text = shellWidget->getLabel();
        if(reaffectText(map, &text, w1))  shellWidget->setLabel(text);

        connect(shellWidget, SIGNAL(clicked(int)), this, SLOT(Callback_ShellCommandClicked(int)));
        connect(shellWidget, SIGNAL(triggered(int)), this, SLOT(Callback_ShellCommandClicked(int)));

        if(shellWidget->isElevated()) shellWidget->raise();

        shellWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caMimeDisplay* mimeWidget = qobject_cast<caMimeDisplay *>(w1)) {

        qCDebug(caMimeDisplayLog) << "create caMimeDisplay";
        w1->setProperty("ObjectType", caMimeDisplay_Widget);

#ifdef WEB
        mimeWidget->setVNCEnabled(vncServer);

        connect(mimeWidget, &caMimeDisplay::triggerURLWeb, this, [](QString url) {
            if (WebSocketServer::instance().isInitialized()) {
                WebSocketServer::instance().sendOpenURLRequest(url);
            }
        });
#endif

        QString text;
        text= mimeWidget->getLabels();
        if(reaffectText(map, &text, w1))  mimeWidget->setLabels(text);

        text = mimeWidget->getArgs();
        if(reaffectText(map, &text, w1))  mimeWidget->setArgs(text);

        text = mimeWidget->getFiles();
        bool affected = reaffectText(map, &text, w1);
        affected |= fixFileListRelative(cainclude_path, &text);
        if (affected) mimeWidget->setFiles(text);

        text = mimeWidget->getLabel();
        if(reaffectText(map, &text, w1))  mimeWidget->setLabel(text);

        if(mimeWidget->isElevated()) mimeWidget->raise();

        mimeWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caMenu* menuWidget = qobject_cast<caMenu *>(w1)) {

        qCDebug(caMenuLog) << "create caMenu";
        w1->setProperty("ObjectType", caMenu_Widget);
        QList<QVariant> integerList;
        QString text = menuWidget->getPV();
        if(text.size() > 0) {
            text =  treatMacro(map, text, &doNothing, w1->objectName());
            specData[0] = 0;
            int num = addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
            integerList.append(num);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            connect(menuWidget, SIGNAL(activated(QString)), this, SLOT(Callback_MenuClicked(QString)));
#else
            connect(menuWidget, SIGNAL(textActivated(QString)), this, SLOT(Callback_MenuClicked(QString)));
#endif
            menuWidget->setPV(pv);
            nbMonitors++;
        }

        text = menuWidget->getMaskPV();
        if(text.size() > 0) {
            text =  treatMacro(map, text, &doNothing, w1->objectName());
            specData[0] = 1;
            int num = addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
            integerList.append(num);
            menuWidget->setMaskPV(pv);
            nbMonitors++;
        }

        if(menuWidget->isElevated()) menuWidget->raise();

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        menuWidget->setProperty("MonitorList", integerList);

        menuWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caCamera* cameraWidget = qobject_cast<caCamera *>(w1)) {

        qCDebug(caCameraLog) << "create caCamera";
        w1->setProperty("ObjectType", caCamera_Widget);

        // if we need to write channels from the camera widget, we do it by timer and slot
        connect(cameraWidget, SIGNAL(WriteDetectedValuesSignal(QWidget*)), this, SLOT(Callback_WriteDetectedValues(QWidget*)));

        // addmonitor normally will add a tooltip to show the pv; however here we have more than one pv
        QString tooltip;
        QString pvs1= "";
        QString pvs2= "";
        tooltip.append(ToolTipPrefix);

        for(int i=0; i< 9; i++) {
            QString text;
            bool alpha = true;
            if(i==0) text = cameraWidget->getPV_Data();
            if(i==1) text = cameraWidget->getPV_Width();
            if(i==2) text = cameraWidget->getPV_Height();
            if(i==3) text = cameraWidget->getPV_ColormodeChannel();
            if(i==4) text = cameraWidget->getPV_PackingmodeChannel();

            // for spectrum pseudo levels
            if(i==5) {
                alpha = cameraWidget->isAlphaMinLevel();
                text = cameraWidget->getMinLevel();
            }
            if(i==6) {
                alpha = cameraWidget->isAlphaMaxLevel();
                text = cameraWidget->getMaxLevel();
            }
            // for dataprocessing data x,y,w,h ROI read and write
            if(i > 6) {
                QStringList thisString;
                if(i==7) thisString = cameraWidget->getROIChannelsRead().split(";");
                if(i==8) thisString = cameraWidget->getROIChannelsWrite().split(";");

                if(thisString.count() >= 4) {
                    if(thisString.at(0).trimmed().length() > 0 && thisString.at(1).trimmed().length() > 0  &&
                       thisString.at(2).trimmed().length() > 0 &&thisString.at(3).trimmed().length() > 0) {

                        for(int j=0; j<4; j++) {
                            text = treatMacro(map, thisString.at(j), &doNothing, w1->objectName());
                            if(i==7) {
                                specData[0] = i+j;   // x,y,w,h
                                int num = addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
                                integerList.append(num);
                                pvs1.append(pv);
                                if( j<3) pvs1.append(";");
                                nbMonitors++;
                            } else if(i==8) {
                                specData[0] = i+j+4; // x,y,w,h
                                int num = addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
                                integerList.append(num);
                                pvs2.append(text);
                                if(j<3) pvs2.append(";");
                                nbMonitors++;
                            }
                        }
                    }
                }
            }

            if(text.size() > 0 && alpha) {
                specData[0] = i;   // pv type
                text =  treatMacro(map, text, &doNothing, w1->objectName());
                if((i!=7) && (i!=8)) {
                    int num = addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
                    integerList.append(num);
                    nbMonitors++;
                }

                if(i==0) cameraWidget->setPV_Data(pv);
                if(i==1) cameraWidget->setPV_Width(pv);
                if(i==2) cameraWidget->setPV_Height(pv);
                if(i==3) cameraWidget->setPV_ColormodeChannel(pv);
                if(i==4) cameraWidget->setPV_PackingmodeChannel(pv);
                if(i==5) cameraWidget->setMinLevel(pv);
                if(i==6) cameraWidget->setMaxLevel(pv);
                if(i==7) cameraWidget->setROIChannelsRead(pvs1);
                if(i==8) cameraWidget->setROIChannelsWrite(pvs2);
                if(i>0) tooltip.append("<br>");
                if(i<=6) tooltip.append(pv);
                else if(i==7) tooltip.append(pvs1);
                else if(i==8) tooltip.append(pvs2);
            }
        }

        QString text = cameraWidget->getPV_Xaverage();
        if(text.size() > 0) {
            text =  treatMacro(map, text, &doNothing, w1->objectName());
            if(text.size() > 0) {
                specData[0] =15;   // pv type. x waveform
                int num = addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
                integerList.append(num);
                nbMonitors++;
            }
        }
        text = cameraWidget->getPV_Yaverage();
        if(text.size() > 0) {
            text =  treatMacro(map, text, &doNothing, w1->objectName());
            if(text.size() > 0) {
                specData[0] = 16;   // pv type. x waveform
                int num = addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
                integerList.append(num);
                nbMonitors++;
            }
        }

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        cameraWidget->setProperty("MonitorList", integerList);

        // finish tooltip
        tooltip.append(ToolTipPostfix);

        if (!vncServer)
            cameraWidget->setToolTip(tooltip);

        cameraWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caChoice* choiceWidget = qobject_cast<caChoice *>(w1)) {

        qCDebug(caChoiceLog) << "create caChoice";
        w1->setProperty("ObjectType", caChoice_Widget);

        QString text = choiceWidget->getPV();
        if(text.size() > 0) {
            text =  treatMacro(map, text, &doNothing, w1->objectName());
            int num = addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
            integerList.append(num);
            connect(choiceWidget, SIGNAL(clicked(QString)), this, SLOT(Callback_ChoiceClicked(QString)));
            choiceWidget->setPV(pv);
            nbMonitors++;
        }

        if(choiceWidget->isElevated()) choiceWidget->raise();

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        choiceWidget->setProperty("MonitorList", integerList);

        choiceWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caLabel* labelWidget = qobject_cast<caLabel *>(w1)) {

        qCDebug(caLabelLog) << "create caLabel";
        w1->setProperty("ObjectType", caLabel_Widget);

        nbMonitors = InitVisibility(w1, &kData, map, specData, "");

        QString text =  treatMacro(map, labelWidget->text(), &doNothing, w1->objectName());
        text.replace(QString::fromWCharArray(L"\u00A6"), " ");    // replace Â¦ with a blanc (was used in macros for creating blancs)
        labelWidget->setText(text);

        labelWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caLabelVertical* labelverticalWidget = qobject_cast<caLabelVertical *>(w1)) {

        qCDebug(caLabelVerticalLog) << "create caLabelVertical";
        w1->setProperty("ObjectType", caLabelVertical_Widget);

        nbMonitors = InitVisibility(w1, &kData, map, specData, "");

        QString text =  treatMacro(map, labelverticalWidget->text(), &doNothing, w1->objectName());
        text.replace(QString::fromWCharArray(L"\u00A6"), " ");    // replace Â¦ with a blanc (was used in macros for creating blancs)
        labelverticalWidget->setText(text);

        labelverticalWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(replaceMacro* replaceMacroWidget = qobject_cast<replaceMacro *>(w1)) {

        qCDebug(replaceMacroLog) << "create replaceMacro";
        w1->setProperty("ObjectType", replaceMacro_Widget);

        if(!map.isEmpty()) {
            QStringList keys;
            QStringList values;
            QMapIterator<QString, QString> i(map);
            while (i.hasNext()) {
                i.next();
                keys.append(i.key());
                values.append(i.value());
            }
            replaceMacroWidget->updateCombo(keys, values);

            // macrovalueslist will be populated by a channel giving a list
            if(replaceMacroWidget->getForm() == replaceMacro::Channel) {
                int num = addMonitor(myWidget, &kData, replaceMacroWidget->getPV(), w1, specData, map, &pv);
                integerList.append(num);
                replaceMacroWidget->setPV(pv);
                nbMonitors++;
            }

            connect(replaceMacroWidget, SIGNAL(reloadDisplay()), this, SLOT(Callback_ReloadWindowL()));
        }

        replaceMacroWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caTextEntry* textentryWidget = qobject_cast<caTextEntry *>(w1)) {

        qCDebug(caTextEntryLog) << "create caTextEntry";
        w1->setProperty("ObjectType", caTextEntry_Widget);

        if(textentryWidget->getPV().size() > 0) {
            textentryWidget->setEnabled(true);
            int num = addMonitor(myWidget, &kData, textentryWidget->getPV(), w1, specData, map, &pv);
            integerList.append(num);
            textentryWidget->setPV(pv);
            connect(textentryWidget, SIGNAL(TextEntryChanged(const QString&)), this, SLOT(Callback_TextEntryChanged(const QString&)));
            nbMonitors++;
        }
        // default format, format from ui file will be used normally except for channel precision
        textentryWidget->setFormat(1);

        QString text =  treatMacro(map, textentryWidget->text(), &doNothing, w1->objectName());
        text.replace(QString::fromWCharArray(L"\u00A6"), " ");    // replace Â¦ with a blanc (was used in macros for creating blancs)
        textentryWidget->setText(text);

        // get focus away
        textentryWidget->clearFocus();
        myWidget->setFocus();

        if(textentryWidget->isElevated()) textentryWidget->raise();

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        textentryWidget->setProperty("MonitorList", integerList);

        textentryWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caLineEdit* lineeditWidget = qobject_cast<caLineEdit *>(w1)) {
        QString text;
        qCDebug(caLineEditLog) << "create caLineEdit";
        w1->setProperty("ObjectType", caLineEdit_Widget);

        if(lineeditWidget->getPV().size() > 0) {
            lineeditWidget->setCursor(QCursor());
            lineeditWidget->setReadOnly(true);
            QString pv = lineeditWidget->getPV();

            lineeditWidget->setAlignment(lineeditWidget->alignment());
            specData[0] = 0;
            int num = addMonitor(myWidget, &kData, lineeditWidget->getPV(), w1, specData, map, &pv);
            integerList.append(num);
            lineeditWidget->setPV(pv);
            nbMonitors++;

            // this is in principle crap, but necessary for psi's bsread, in order to get the unit (.EGU), that was normally
            // transmitted in the epics protocol
            QVariant var=lineeditWidget->property("Interface");
            if(!var.isNull()) {
                QVariantList infoList = var.toList();
                void *ptr = (void*) infoList.at(0).value<void *>();
                if(ptr != (void*) Q_NULLPTR) {
                    ControlsInterface * plugininterface = (ControlsInterface *) ptr;
                    if(plugininterface != (ControlsInterface *) Q_NULLPTR) {
                        if(plugininterface->pluginName().contains("bsread")) {
                            qCInfo(caLineEditLog) << "bsread detected";
                            pv.append(".EGU");
                            specData[0] = 1;
                            if(pv.contains("bsread://")) pv.replace("bsread://", "epics3://");
                            else pv.prepend("epics3://");
                            int num = addMonitor(myWidget, &kData, pv, w1, specData, map, &pv);
                            integerList.append(num);
                            nbMonitors++;
                        }
                    }
                }
            }
        }

        // default format, format from ui file will be used normally except for channel precision
        lineeditWidget->setFormat(1);

        text= lineeditWidget->toolTip();
        if(reaffectText(map, &text, w1))  lineeditWidget->setToolTip(text);
        text= lineeditWidget->statusTip();
        if(reaffectText(map, &text, w1))  lineeditWidget->setStatusTip(text);

        text =  treatMacro(map, lineeditWidget->text(), &doNothing, w1->objectName());
        text.replace(QString::fromWCharArray(L"\u00A6"), " ");    // replace Â¦ with a blanc (was used in macros for creating blancs)
        lineeditWidget->setText(text);


        // insert dataindex list
        integerList.insert(0, nbMonitors);
        lineeditWidget->setProperty("MonitorList", integerList);

        lineeditWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caMultiLineString* multilinestringWidget = qobject_cast<caMultiLineString *>(w1)) {
        QString text;
        qCDebug(caMultiLineStringLog) << "create caMultilineString";
        w1->setProperty("ObjectType", caMultiLineString_Widget);

        if(multilinestringWidget->getPV().size() > 0) {
            multilinestringWidget->setCursor(QCursor());
            multilinestringWidget->setReadOnly(true);

            //multilinestringWidget->setAlignment(lineeditWidget->alignment());
            int num = addMonitor(myWidget, &kData, multilinestringWidget->getPV(), w1, specData, map, &pv);
            integerList.append(num);
            multilinestringWidget->setPV(pv);
            nbMonitors++;
        }
        text= multilinestringWidget->toolTip();
        if(reaffectText(map, &text, w1))  multilinestringWidget->setToolTip(text);
        text= multilinestringWidget->statusTip();
        if(reaffectText(map, &text, w1))  multilinestringWidget->setStatusTip(text);

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        multilinestringWidget->setProperty("MonitorList", integerList);

        multilinestringWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caGraphics* graphicsWidget = qobject_cast<caGraphics *>(w1)) {

        QString text;
        qCDebug(caGraphicsLog) << "create caGraphics";
        w1->setProperty("ObjectType", caGraphics_Widget);

        text= graphicsWidget->toolTip();
        if(reaffectText(map, &text, w1))  graphicsWidget->setToolTip(text);
        text= graphicsWidget->statusTip();
        if(reaffectText(map, &text, w1))  graphicsWidget->setStatusTip(text);


        nbMonitors = InitVisibility(w1, &kData, map, specData, "");
        graphicsWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caPolyLine* polylineWidget = qobject_cast<caPolyLine *>(w1)) {

        QString text;
        qCDebug(caPolyLineLog) << "create caPolyLine";
        w1->setProperty("ObjectType", caPolyLine_Widget);

        text= polylineWidget->toolTip();
        if(reaffectText(map, &text, w1))  polylineWidget->setToolTip(text);
        text= polylineWidget->statusTip();
        if(reaffectText(map, &text, w1))  polylineWidget->setStatusTip(text);

        nbMonitors = InitVisibility(w1, &kData, map, specData, "");
        polylineWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if (caApplyNumeric* applynumericWidget = qobject_cast<caApplyNumeric *>(w1)){

        qCDebug(caApplyNumericLog) << "create caAppyNumeric";
        w1->setProperty("ObjectType", caApplyNumeric_Widget);

        if(applynumericWidget->getPV().size() > 0) {
            int num = addMonitor(myWidget, &kData, applynumericWidget->getPV(), w1, specData, map, &pv);
            integerList.append(num);
            applynumericWidget->setPV(pv);
            connect(applynumericWidget, SIGNAL(clicked(double)), this, SLOT(Callback_EApplyNumeric(double)));
            nbMonitors++;
        }
        if(applynumericWidget->isElevated()) applynumericWidget->raise();

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        applynumericWidget->setProperty("MonitorList", integerList);

        applynumericWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if (caNumeric* numericWidget = qobject_cast<caNumeric *>(w1)){

        qCDebug(caNumericLog) << "create caNumeric";
        w1->setProperty("ObjectType", caNumeric_Widget);

        if(numericWidget->getPV().size() > 0) {
            int num = addMonitor(myWidget, &kData, numericWidget->getPV(), w1, specData, map, &pv);
            integerList.append(num);
            numericWidget->setPV(pv);
            connect(numericWidget, SIGNAL(valueChanged(double)), this, SLOT(Callback_ENumeric(double)));
            nbMonitors++;
        }
        if(numericWidget->isElevated()) numericWidget->raise();

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        numericWidget->setProperty("MonitorList", integerList);

        numericWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if (caSpinbox* spinboxWidget = qobject_cast<caSpinbox *>(w1)){

        qCDebug(caSpinboxLog) << "create caSpinbox";
        w1->setProperty("ObjectType", caSpinbox_Widget);

        if(spinboxWidget->getPV().size() > 0) {
            int num = addMonitor(myWidget, &kData, spinboxWidget->getPV(), w1, specData, map, &pv);
            integerList.append(num);
            spinboxWidget->setPV(pv);
            connect(spinboxWidget, SIGNAL(valueChanged(double)), this, SLOT(Callback_Spinbox(double)));
            nbMonitors++;
        }
        if(spinboxWidget->isElevated()) spinboxWidget->raise();

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        spinboxWidget->setProperty("MonitorList", integerList);

        spinboxWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if (caMessageButton* messagebuttonWidget = qobject_cast<caMessageButton *>(w1)) {

        qCDebug(caMessageButtonLog) << "create caMessageButton" <<  messagebuttonWidget->getPV();
        w1->setProperty("ObjectType", caMessageButton_Widget);

        QString text;

        if(messagebuttonWidget->getPV().size() > 0) {
            specData[0] = 0;
            int num = addMonitor(myWidget, &kData, messagebuttonWidget->getPV(), w1, specData, map, &pv);
            integerList.append(num);
            messagebuttonWidget->setPV(pv);
            nbMonitors++;
        }

        if(messagebuttonWidget->getDisablePV().size() > 0) {
            specData[0] = 1;
            int num = addMonitor(myWidget, &kData, messagebuttonWidget->getDisablePV(), w1, specData, map, &pv);
            integerList.append(num);
            messagebuttonWidget->setDisablePV(pv);
            nbMonitors++;
        }

        connect(messagebuttonWidget, SIGNAL(messageButtonSignal(int)), this, SLOT(Callback_MessageButton(int)));

        text = messagebuttonWidget->getLabel();
        if(reaffectText(map, &text, w1))  messagebuttonWidget->setLabel(text);

        text = messagebuttonWidget->getPressMessage();
        if(reaffectText(map, &text, w1))  messagebuttonWidget->setPressMessage(text);

        text = messagebuttonWidget->getReleaseMessage();
        if(reaffectText(map, &text, w1))  messagebuttonWidget->setReleaseMessage(text);

        text= messagebuttonWidget->toolTip();
        if(reaffectText(map, &text, w1))  messagebuttonWidget->setToolTip(text);

        text= messagebuttonWidget->statusTip();
        if(reaffectText(map, &text, w1))  messagebuttonWidget->setStatusTip(text);


        if(messagebuttonWidget->isElevated()) messagebuttonWidget->raise();

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        messagebuttonWidget->setProperty("MonitorList", integerList);

        messagebuttonWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caToggleButton* togglebuttonWidget = qobject_cast<caToggleButton *>(w1)) {

        qCDebug(caToggleButtonLog) << "create caToggleButton";
        w1->setProperty("ObjectType", caToggleButton_Widget);

        connect(togglebuttonWidget, SIGNAL(toggleButtonSignal(bool)), this, SLOT(Callback_ToggleButton(bool)));

        int num = addMonitor(myWidget, &kData, togglebuttonWidget->getPV(), w1, specData, map, &pv);
        integerList.append(num);
        togglebuttonWidget->setPV(pv);
        nbMonitors++;

        QString text =  treatMacro(map, togglebuttonWidget->text(), &doNothing, w1->objectName());
        text.replace(QString::fromWCharArray(L"\u00A6"), " ");    // replace Â¦ with a blanc (was used in macros for creating blancs)
        togglebuttonWidget->setText(text);

        if(togglebuttonWidget->isElevated()) togglebuttonWidget->raise();

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        togglebuttonWidget->setProperty("MonitorList", integerList);

        togglebuttonWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caScriptButton* scriptbuttonWidget = qobject_cast<caScriptButton *>(w1)) {

        QString text;
        qCDebug(caScriptButtonLog) << "create caScriptButton";
        w1->setProperty("ObjectType", caScriptButton_Widget);

        connect(scriptbuttonWidget, SIGNAL(scriptButtonSignal()), this, SLOT(Callback_ScriptButton()));

        text= scriptbuttonWidget->getScriptCommand();
        if(reaffectText(map, &text, w1))  scriptbuttonWidget->setScriptCommand(text);

        text= scriptbuttonWidget->getScriptParam();
        if(reaffectText(map, &text, w1))  scriptbuttonWidget->setScriptParam(text);
        scriptbuttonWidget->setToolTip("process never started !");
        text= scriptbuttonWidget->statusTip();
        if(reaffectText(map, &text, w1))  scriptbuttonWidget->setStatusTip(text);

        if(scriptbuttonWidget->isElevated()) scriptbuttonWidget->raise();

        scriptbuttonWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caLed* ledWidget = qobject_cast<caLed *>(w1)) {

        QString text;
        qCDebug(caLedLog) << "create caLed";
        w1->setProperty("ObjectType", caLed_Widget);

        if(ledWidget->getPV().size() > 0) {
            int num = addMonitor(myWidget, &kData, ledWidget->getPV(), w1, specData, map, &pv);
            integerList.append(num);
            ledWidget->setPV(pv);
            nbMonitors++;
        }

        text= ledWidget->toolTip();
        if(reaffectText(map, &text, w1))  ledWidget->setToolTip(text);
        text= ledWidget->statusTip();
        if(reaffectText(map, &text, w1))  ledWidget->setStatusTip(text);

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        ledWidget->setProperty("MonitorList", integerList);

        ledWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caBitnames* bitnamesWidget = qobject_cast<caBitnames *>(w1)) {

        qCDebug(caBitnamesLog) << "create caBitnames";
        w1->setProperty("ObjectType", caBitnames_Widget);

        if(bitnamesWidget->getEnumPV().size() > 0 && bitnamesWidget->getValuePV().size() > 0) {
            int num = addMonitor(myWidget, &kData, bitnamesWidget->getEnumPV(), w1, specData, map, &pv);
            integerList.append(num);
            bitnamesWidget->setEnumPV(pv);
            num = addMonitor(myWidget, &kData, bitnamesWidget->getValuePV(), w1, specData, map, &pv);
            integerList.append(num);
            bitnamesWidget->setValuePV(pv);
            nbMonitors = 2;
        }

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        bitnamesWidget->setProperty("MonitorList", integerList);

        bitnamesWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caSlider* sliderWidget = qobject_cast<caSlider *>(w1)) {

        qCDebug(caSliderLog) << "create caSlider";
        w1->setProperty("ObjectType", caSlider_Widget);

        if(sliderWidget->getPV().size() > 0) {
            int num = addMonitor(myWidget, &kData, sliderWidget->getPV(), w1, specData, map, &pv);
            integerList.append(num);
            sliderWidget->setPV(pv);
            connect(sliderWidget, SIGNAL(valueChanged(double)), this, SLOT(Callback_SliderValueChanged(double)));
            nbMonitors++;
        }

        if(sliderWidget->isElevated())sliderWidget->raise();

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        sliderWidget->setProperty("MonitorList", integerList);

        sliderWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caClock* clockWidget = qobject_cast<caClock *>(w1)) {

        qCDebug(caClockLog) << "create caClock";
        w1->setProperty("ObjectType", caClock_Widget);

        if(clockWidget->getPV().size() > 0) {
            int num = addMonitor(myWidget, &kData, clockWidget->getPV(), w1, specData, map, &pv);
            integerList.append(num);
            clockWidget->setPV(pv);
            nbMonitors++;
        }

        clockWidget->setAlarmColors(NO_ALARM, true);

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        clockWidget->setProperty("MonitorList", integerList);

        clockWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caThermo* thermoWidget = qobject_cast<caThermo *>(w1)) {

        qCDebug(caThermoLog) << "create caThermo";
        w1->setProperty("ObjectType", caThermo_Widget);

        if(thermoWidget->getPV().size() > 0) {
            int num = addMonitor(myWidget, &kData, thermoWidget->getPV(), w1, specData, map, &pv);
            integerList.append(num);
            thermoWidget->setPV(pv);
            nbMonitors++;
        }
        // for an opposite direction, invert maximum and minimum

        if(thermoWidget->getDirection() == caThermo::Down || thermoWidget->getDirection() == caThermo::Left) {
            double max = thermoWidget->maxValue();
            double min = thermoWidget->minValue();
            thermoWidget->setMinValue(max);
            thermoWidget->setMaxValue(min);
        }

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        thermoWidget->setProperty("MonitorList", integerList);

        thermoWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caLinearGauge* lineargaugeWidget = qobject_cast<caLinearGauge *>(w1)) {

        qCDebug(caLinearGaugeLog) << "create lineargauge for" << lineargaugeWidget->getPV();
        w1->setProperty("ObjectType", caLinearGauge_Widget);

        if(lineargaugeWidget->getPV().size() > 0) {
            int num = addMonitor(myWidget, &kData, lineargaugeWidget->getPV(), w1, specData, map, &pv);
            integerList.append(num);
            lineargaugeWidget->setPV(pv);
            nbMonitors++;
        }

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        lineargaugeWidget->setProperty("MonitorList", integerList);

        lineargaugeWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caCircularGauge* circulargaugeWidget = qobject_cast<caCircularGauge *>(w1)) {

        qCDebug(caCircularGaugeLog) << "create circulargauge for" << circulargaugeWidget->getPV();
        w1->setProperty("ObjectType", caCircularGauge_Widget);

        if(circulargaugeWidget->getPV().size() > 0) {
            int num = addMonitor(myWidget, &kData, circulargaugeWidget->getPV(), w1, specData, map, &pv);
            integerList.append(num);
            circulargaugeWidget->setPV(pv);
            nbMonitors++;
        }

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        circulargaugeWidget->setProperty("MonitorList", integerList);

        circulargaugeWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caMeter* meterWidget = qobject_cast<caMeter *>(w1)) {

        qCDebug(caMeterLog) << "create circulargauge for" << meterWidget->getPV();
        w1->setProperty("ObjectType", caMeter_Widget);

        if(meterWidget->getPV().size() > 0) {
            int num = addMonitor(myWidget, &kData, meterWidget->getPV(), w1, specData, map, &pv);
            integerList.append(num);
            meterWidget->setPV(pv);
            nbMonitors++;
        }

        meterWidget->setAlarmColors(NO_ALARM, true);

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        meterWidget->setProperty("MonitorList", integerList);

        meterWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caByte* byteWidget = qobject_cast<caByte *>(w1)) {

        qCDebug(caByteLog) << "create caByte" << w1;
        w1->setProperty("ObjectType", caByte_Widget);

        if(byteWidget->getPV().size() > 0) {
            int num = addMonitor(myWidget, &kData, byteWidget->getPV(), w1, specData, map, &pv);
            integerList.append(num);
            byteWidget->setPV(pv);
            nbMonitors++;
        }

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        byteWidget->setProperty("MonitorList", integerList);

        byteWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caByteController* bytecontrollerWidget = qobject_cast<caByteController *>(w1)) {

        qCDebug(caByteControllerLog) << "create caByteController" << w1;
        w1->setProperty("ObjectType", caByteController_Widget);

        if(bytecontrollerWidget->getPV().size() > 0) {
            int num = addMonitor(myWidget, &kData, bytecontrollerWidget->getPV(), w1, specData, map, &pv);
            integerList.append(num);
            bytecontrollerWidget->setPV(pv);
            connect(bytecontrollerWidget, SIGNAL(clicked(int)), this, SLOT(Callback_ByteControllerClicked(int)));
            nbMonitors++;
        }

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        bytecontrollerWidget->setProperty("MonitorList", integerList);

        bytecontrollerWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caInclude* includeWidget = qobject_cast<caInclude *>(w1)) {

        qCDebug(caIncludeLog) << "create caInclude" << w1;
        int maximumX=1;
        int maximumY=1;
        int posX=0;
        int posY = 0;
        int maxRows = 0;
        int row = 0;
        int maxColumns=0;
        int column = 0;
        int spacingHorizontal = includeWidget->getSpacingHorizontal();
        int spacingVertical = includeWidget->getSpacingVertical();
        w1->setProperty("ObjectType", caInclude_Widget);

        QWidget *thisW = (QWidget *) Q_NULLPTR;
        QUiLoader loader;
        bool prcFile = false;

        QHBoxLayout *m_boxLayout = includeWidget->getIncludeboxLayout();//new QHBoxLayout;
        if (m_boxLayout) SETMARGIN_QT456(m_boxLayout,0);
        if (m_boxLayout) m_boxLayout->setSpacing(0);
        QFrame *frame = includeWidget->getIncludeFrame();//new QFrame();
        // define a layout for adding the includes
        QGridLayout *gridLayout =  includeWidget->getIncludegridLayout();//new QGridLayout;
        if(gridLayout){
            gridLayout->setContentsMargins(0,0,0,0);
            SETMARGIN_QT456(gridLayout,0);
            gridLayout->setVerticalSpacing(spacingVertical);
            gridLayout->setHorizontalSpacing(spacingHorizontal);
        }

        QColor thisFrameColor= includeWidget->getFrameColor();
        QColor thisLightColor = thisFrameColor.lighter();
        QColor thisDarkColor = thisFrameColor.darker();

        QPalette thisPalette = includeWidget->palette();
        thisPalette.setColor(QPalette::WindowText, thisFrameColor);
        thisPalette.setColor(QPalette::Light, thisLightColor);
        thisPalette.setColor(QPalette::Dark, thisDarkColor);
        thisPalette.setColor(QPalette::Window, thisFrameColor);
        if (m_boxLayout) includeWidget->setLayout(m_boxLayout);
        if (m_boxLayout) m_boxLayout->addWidget(frame);

        if(gridLayout) frame->setLayout(gridLayout);


        frame->setFrameShadow(includeWidget->getFrameShadow());
        frame->setLineWidth(includeWidget->getFrameLineWidth());
        frame->setPalette(thisPalette);
        frame->setGeometry(includeWidget->geometry());
        frame->move(0,0);




        //  we set the shape and in case of box, we have to set margins correctly
        switch(includeWidget->getFrameShape()) {
            case caInclude::NoFrame:
                  frame->setFrameShape(QFrame::NoFrame);
                  break;
            case caInclude::Box:
                  frame->setFrameShape(QFrame::Box);
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
                  SETMARGIN_QT456(gridLayout,includeWidget->getFrameLineWidth());
#endif
                  break;
            case caInclude::Panel:
                  frame->setFrameShape(QFrame::Panel);
                  break;
            default:
                 frame->setFrameShape(QFrame::Panel);
        }

        nbMonitors = InitVisibility(w1, &kData, map, specData, "");

        includeWidget->setProperty("Taken", true);

        // define the file to use
        QString providedFileName = includeWidget->getFileName().trimmed();
        QString fileName = providedFileName;

        if (QFileInfo(fileName).isRelative()){
          fileName = cainclude_path + fileName;
        }

        reaffectText(map, &fileName, w1);

        QString openFile = "";
        int found = fileName.lastIndexOf(".");
        openFile = fileName;
        if (found != -1) {
            openFile = fileName.mid(0, found);
        }
        qCDebug(caIncludeLog) << "use file" << fileName << openFile;
        UiConverterInterface *parseFile = (UiConverterInterface *) Q_NULLPTR;
#ifdef ADL_EDL_FILES
        bool isMedmFile = fileName.endsWith (".adl");
        bool isEdmFile = fileName.endsWith (".edl");
        searchFile *other_s = new searchFile(fileName);
        QString adledlcheck = other_s->findFile();
        if(adledlcheck.isNull()){
            // if file is not existing try ui ending
            if (isMedmFile){
                fileName=fileName.replace(".adl",".ui");
                isMedmFile = false;
            }
            if (isEdmFile){
                fileName=fileName.replace(".edl",".ui");
                isEdmFile = false;
            }
        }
        delete other_s;
#endif

        // ui file or prc file or other file?
        if((openFile.size() > 1) && fileName.contains(".prc")) {
            qCDebug(caIncludeLog) << "prc file";
            prcFile = true;

#ifdef ADL_EDL_FILES
        } else if(isMedmFile || isEdmFile) {
            qCDebug(caIncludeLog) << "adl file";
#endif
        } else {
            qCDebug(caIncludeLog) << "ui file";
            fileName = openFile.append(".ui");

        }
        qCDebug(caIncludeLog) << "use2 file" << providedFileName << openFile;
        // this will check for file existence and when an url is defined, download the file from a http server
        fileFunctions filefunction;
        filefunction.checkFileAndDownload(providedFileName);
        if(messageWindowP != (MessageWindow *) Q_NULLPTR) {
            if(filefunction.lastInfo().length() > 0) messageWindowP->postMsgEvent(QtWarningMsg, (char*) qasc(filefunction.lastInfo()));
            if(filefunction.lastError().length() > 0)  messageWindowP->postMsgEvent(QtCriticalMsg, (char*) qasc(filefunction.lastError()));
        }

        QApplication::processEvents();

        // First, check if original filename is now available after download
        searchFile *s = new searchFile(providedFileName);
        QString fileNameFound = s->findFile();
        if (fileNameFound.isNull()) {
            delete s;
            s = new searchFile(fileName);
            fileNameFound = s->findFile();
        }
        if (fileNameFound.isNull()) {
            includeData value;
            value.count = 0;
            value.ms = 0;
            value.text="does not exist";
            includeFilesList.insert(providedFileName, value);
        } else {
            qCDebug(caIncludeLog) << "filenameFound" << fileNameFound;
            qCDebug(caIncludeLog) << "use file" << fileName << "for" << includeWidget;
            fileName = fileNameFound;
        }
        delete s;

        // special files exist, then parse first time
        QFileInfo fi(fileName);
        if(fi.exists()) {
            bool useConverter = prcFile;
#ifdef ADL_EDL_FILES
            useConverter = useConverter || isMedmFile || isEdmFile;
#endif
            if(useConverter) parseFile = UiConverterFactory::create(fileName, pepPrint);
            if((parseFile != (UiConverterInterface *) Q_NULLPTR) && (parseFile->errorString().length() > 0))
                postMessage(QtDebugMsg, (char*) qasc(parseFile->errorString()));
        }

        qCDebug(caIncludeLog) << "caInclude Load:" << fileName;
        QString macros = includeWidget->getMacro();
        qCDebug(caIncludeLog) << "macros Load:" << macros;
        //in case the macro $(B) has to be replaced by another macro  (ex: "B=NAME=ARIMA-CV-02ME;NAME=ARIMA-CV-03ME")
        macros = treatMacro(map, macros, &doNothing, w1->objectName());
        qCDebug(caIncludeLog) << "treatMacro:" << macros;
        QStringList macroList = macros.split(";", SKIP_EMPTY_PARTS);

        macroList=treat_read_MacroCommand(macroList);
        qCDebug(caIncludeLog) << macroList;
        int adjustMargin = includeWidget->getMargin();

        // loop on this include with different macro
        for(int j=0; j<qMax(macroList.count(), includeWidget->getItemCount()); j++) {
            QString macroS;
            if(j < macroList.count()) {
                macroS = macroList.at(j);
            } else {
                macroS = "";
            }

            if(macroS.size() < 1) {
                if(level > 0){
                    macroS = savedMacro[level-1];
                } else {
                    macroS = savedMacro[level];
                }
            }

            macroS = treatMacro(map, macroS, &doNothing, w1->objectName());
            savedMacro[level] = macroS;

            // when file exist, then load parsed file
            QFileInfo fi(fileName);
            if(fi.exists()) {
                qint64 diff=0;
                // load converted (prc/adl/edl) or plain ui file
                if(parseFile != (UiConverterInterface *) Q_NULLPTR) {
                    if(parseFile->ok()) thisW = parseFile->load(this);

                } else {
#if !defined(useElapsedTimer)
                    double last = rTime();
#else
                    QElapsedTimer timer;
                    timer.start();
#endif
                    QFile *file = new QFile;
                    // open and load ui file
                    file->setFileName(fileName);
                    //symtomatic AFS check
                    if (!(file->open(QFile::ReadOnly) && file->isOpen())){
                        postMessage(QtDebugMsg, (char*) qasc(tr("can't open file %1 ").arg(providedFileName)));
                    }else{
                        if (file->size()==0){
                            postMessage(QtDebugMsg, (char*) qasc(tr("file %1 has size zero ").arg(providedFileName)));
                        }else{
                            if (level<CAQTDM_MAX_INCLUDE_LEVEL-1){
                                QBuffer *buffer = new QBuffer();
                                buffer->open(QIODevice::ReadWrite);
                                //QByteArray data=file->readAll();
                                buffer->write(file->readAll());

                                //QCryptographicHash md5Gen(QCryptographicHash::Md5);
                                //md5Gen.addData(data);

                                buffer->seek(0);

                                thisW = loader.load(buffer, this);
                                QApplication::processEvents();
                                //qDebug() << "iload= " << fileName << buffer->size() << md5Gen.result().toHex();
                                //qDebug() << thisW->findChildren<QWidget *>();
                                //foreach(QWidget *w1, thisW->findChildren<QWidget *>()) {
                                //  qDebug() << w1->metaObject()->className();
                                //}

                                delete buffer;
                            }
                        }
                        file->close();
                    }

                    delete file;

#if !defined(useElapsedTimer)
                    double now = rTime();
                    diff = qRound ((now - last) /1000.0);
#else
                    diff = timer.elapsed();
#endif
                    if(diff < 1) diff=1; // you really do not believe that smaller is possible, do you?
                }

                QMap<QString, includeData>::const_iterator name = includeFilesList.find(fi.absoluteFilePath());
                if(name != includeFilesList.end()) {
                    includeData value = name.value();
                    value.count++;
                    value.ms = value.ms + ((int) diff - value.ms) / value.count;
                    if(!thisW) value.text = "not loaded"; else value.text="loaded";
                    includeFilesList.insert(fi.absoluteFilePath(), value);
                } else {
                    includeData value;
                    value.count = 1;
                    value.ms = (int) diff;
                    if(!thisW) value.text = "not loaded"; else value.text="loaded";
                    includeFilesList.insert(fi.absoluteFilePath(), value);
                }

                // some error with loading
                if (!thisW) {
                    postMessage(QtDebugMsg, (char*) qasc(tr("could not load include file %1").arg(fileName)));
                    // seems to be ok
                } else {
                    includeWidgetList.append(thisW);
                    includeWidget->appendChildToList(thisW);

                    includeWidget->update_geometrysave();
                    // add includeWidget to the gui
                    if(includeWidget->getStacking() == caInclude::Row) {
                        if(gridLayout) gridLayout->addWidget(thisW, j, 0);
                        row++;
                        maxRows = row;
                        maxColumns = 1;
                    } else if(includeWidget->getStacking() == caInclude::Column) {
                       if(gridLayout) gridLayout->addWidget(thisW, 0, j);
                       column++;
                       maxColumns = column;
                       maxRows = 1;
                    } else if(includeWidget->getStacking() == caInclude::RowColumn) {
                        if(row >= includeWidget->getMaxLines()) {
                            row=0;
                            column++;
                        }
                        if(gridLayout) gridLayout->addWidget(thisW, row, column);
                        row++;
                        if(row > maxRows) maxRows = row;
                        maxColumns = column + 1;
                    } else if(includeWidget->getStacking() == caInclude::ColumnRow) {
                        if(column >= includeWidget->getMaxColumns()) {
                            row++;
                            column=0;
                        }
                        if(gridLayout) gridLayout->addWidget(thisW, row, column);
                        column++;
                        if(column > maxColumns) maxColumns = column;
                        maxRows = row + 1;
                    } else {
                        thisW->setParent(frame);

                        qCDebug(caIncludeLog) << "Frame: "<< frame->children();
                        qCDebug(caIncludeLog) << "thisW: "<< thisW->children();
                        QString pos;
                        posX=0;
                        posY=0;
                        if(!includeWidget->getXposition(j, posX, thisW->width(), pos)) {
                            specData[0] = 1;   // x position
                            specData[1] = j;   // actual position in array;
                            specData[2] = adjustMargin;
                            memcpy(&specData[3], &thisW, sizeof(QWidget*));
                            int num = addMonitor(myWidget, &kData, pos, w1, specData, map, &pv);
                            integerList.append(num);
                            nbMonitors++;
                        }

                        if(!includeWidget->getYposition(j, posY, thisW->height(), pos)) {
                            specData[0] = 2;   // x position
                            specData[1] = j;   // actual position in array;
                            specData[2] = adjustMargin;
                            memcpy(&specData[3], &thisW, sizeof(QWidget*));
                            int num = addMonitor(myWidget, &kData, pos, w1, specData, map, &pv);
                            integerList.append(num);
                            nbMonitors++;
                        }

                        int xpos = qRound((double) posX * includeWidget->getXcorrection());
                        int ypos = qRound((double) posY * includeWidget->getYcorrection());
                        thisW->move(xpos + adjustMargin/2, ypos + adjustMargin/2);

                        int maxX = xpos + thisW->width();
                        int maxY = ypos + thisW->height();
                        if(maxX > maximumX) maximumX = maxX;
                        if(maxY > maximumY) maximumY = maxY;


                        qCDebug(caIncludeLog)<< "caInclude Pos:"<< xpos << ypos<<posX<<includeWidget->getXcorrection()<<pos <<xpos + adjustMargin/2 << includeWidget->width()<<includeWidget->height();

                    }
                    //frame->setLayout(gridLayout);
                    //includeWidget->setLineSize(0);
                    level++;
                    //create a stack of the cainclude_path to work properly with recursion
                    QString cainclude_path_stacked = cainclude_path;

                    // keep actual filename
                    savedFile[level] = fi.baseName();

                    qCDebug(caIncludeLog) << "cainclude ++"<< cainclude_path << level << fi.baseName();

                    // Using just path here since it should work better with URLs than converting to abs path
                    //cainclude_path = fi.absolutePath() + "/";
                    cainclude_path = fi.path() + "/";

                    // recurse (DFS) to lower widgets
                    scanWidgets(thisW->findChildren<QWidget *>(), macroS);

                    cainclude_path = cainclude_path_stacked;
                    level--;
                    qCDebug(caIncludeLog) << "cainclude --"<< cainclude_path << level;
                }

            } else {
                #ifdef ADL_EDL_FILES
                    if (isMedmFile){
                        fileName=fileName.replace(".ui",".adl");
                        postMessage(QtDebugMsg, (char*) qasc(tr("error ADL file conversion")));
                    }
                    if (isEdmFile){
                        fileName=fileName.replace(".ui",".edl");
                        postMessage(QtDebugMsg, (char*) qasc(tr("error EDL file conversion")));
                    }
                    postMessage(QtDebugMsg, (char*) qasc(tr("sorry, could not load include file %1").arg(providedFileName)));
                    qCDebug(caIncludeLog) << "sorry, file" << providedFileName << " does not exist";
                    break;
                #else
                    postMessage(QtDebugMsg, (char*) qasc(tr("sorry, could not load include file %1").arg(providedFileName)));
                    qCDebug(caIncludeLog) << "sorry, file" << providedFileName << " does not exist";
                    break;
                #endif
            }
        } // end for

        if(parseFile != (UiConverterInterface *) Q_NULLPTR ) {
            // #!title of the new prc parser: apply to the top level window
            if(this->prcFile && !parseFile->title().isEmpty()) setWindowTitle(parseFile->title());
            delete parseFile;
            parseFile = (UiConverterInterface *) Q_NULLPTR;
        }

        // resize the include widget
        if((thisW != (QWidget *) Q_NULLPTR ) && (!prcFile) && includeWidget->getAdjustSize() && includeWidget->getStacking() != caInclude::Positions) {
            includeWidget->resize(maxColumns * thisW->width() + (maxColumns-1) * spacingHorizontal + adjustMargin,
                                  maxRows * thisW->height() + (maxRows-1) * spacingVertical + adjustMargin);
            includeWidget->getIncludeFrame()->resize(maxColumns * thisW->width() + (maxColumns-1) * spacingHorizontal + adjustMargin,
                                                     maxRows * thisW->height() + (maxRows-1) * spacingVertical + adjustMargin);

        } else if((thisW != (QWidget *) Q_NULLPTR ) && (!prcFile) && includeWidget->getAdjustSize() && includeWidget->getStacking() == caInclude::Positions) {



            //QRect resizedata=includeWidget->childrenRect();
            //includeWidget->resize(resizedata.width(),resizedata.height());
            //QRect resizedata=thisW->childrenRect();
            //qDebug()<<"childrenRect"<<resizedata;
            //frame->resize(resizedata.width(),resizedata.height());
            //includeWidget->resize(resizedata.width(),resizedata.height());

            //QRect resizedata=includeWidget->scanChildsneededArea();
            //includeWidget->resize(resizedata.width(),resizedata.height());

            includeWidget->resize(maximumX + adjustMargin, maximumY + adjustMargin);
            includeWidget->getIncludeFrame()->resize(maximumX + adjustMargin, maximumY + adjustMargin);
            includeWidget->update_geometrysave();

             }

        // when the include is packed into a scroll area, set the minimumsize too
        if((thisW != (QWidget *) Q_NULLPTR ) && (!prcFile) && includeWidget->getAdjustSize()) {
            if(includeWidget->getStacking() != caInclude::Positions) {
                includeWidget->updateGeometry();
                ResizeScrollBars(includeWidget, maxColumns * thisW->width() + (maxColumns-1) * spacingHorizontal + adjustMargin,
                                 maxRows * thisW->height() + (maxRows-1) * spacingVertical + adjustMargin);
            } else {
                ResizeScrollBars(includeWidget, maximumX + adjustMargin, maximumY + adjustMargin);
            }
        }

        // increment splashcounter when include is in list
        if(nbIncludes > 0 && !thisFileFull.contains(POPUPDEFENITION)) {
            for (int i = topIncludesWidgetList.count()-1; i >= 0; --i) {
                if(w1 ==  topIncludesWidgetList.at(i)) {
#ifdef WEB
                    if (vncServer && WebSocketServer::instance().isInitialized()) {
                        WebSocketServer::instance().sendProgressUpdate(splashCounter++);
                    } else
#endif
                    splash->setProgress(splashCounter++);
                    QApplication::processEvents();
                    break;
                }
            }
        }

        includeWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caFrame* frameWidget = qobject_cast<caFrame *>(w1)) {

        qCDebug(caFrameLog) << "treat caFrame" << w1;
        w1->setProperty("ObjectType", caFrame_Widget);

        nbMonitors = InitVisibility(w1, &kData, map, specData, "");

        frameWidget->setProperty("Taken", true);

        QString macroS = frameWidget->getMacro();

        if(macroS.size() < 1) {
            if(level > 0){
                macroS = savedMacro[level-1];
            } else {
                macroS = macro;
            }
        }

        macroS = treatMacro(map, macroS, &doNothing, w1->objectName());
        savedMacro[level] = macroS;

        level++;

        // get actual filename from previous level
        savedFile[level] = savedFile[level-1];

        scanWidgets(frameWidget->findChildren<QWidget *>(), macroS);

        level--;

        frameWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caCartesianPlot* cartesianplotWidget = qobject_cast<caCartesianPlot *>(w1)) {

        qCDebug(caCartesianPlotLog) << "treat caCartesianPlot" << w1;
        w1->setProperty("ObjectType", caCartesianPlot_Widget);

        QString triggerChannel, countChannel, eraseChannel, title;

        // addmonitor normally will add a tooltip to show the pv; however here we have more than one pv
        QString tooltip;
        tooltip.append(ToolTipPrefix);

        // do this while the order has to be correct
        cartesianplotWidget->setForeground(cartesianplotWidget->getForeground());
        cartesianplotWidget->setBackground(cartesianplotWidget->getBackground());
        cartesianplotWidget->setScaleColor(cartesianplotWidget->getScaleColor());
        cartesianplotWidget->setGridColor(cartesianplotWidget->getGridColor());

        // reaffect special dynamic strings to be used for the archive plots (secondsPast, secondsUpdate, nrOfBins)
        // should have been done with a new method, however stay compatible with existing caQtDM
        QVariant dynVars;
        for(int varLoop=0; varLoop < 3; varLoop++) {
            bool ok = false;
            if(varLoop == 0) dynVars = w1->property("secondsUpdate");
            if(varLoop == 1) dynVars = w1->property("secondsPast");
            if(varLoop == 2) dynVars = w1->property("nrOfBins");
            if(!dynVars.isNull()) {
                dynVars.toInt(&ok);
                if(!ok && dynVars.canConvert<QString>())  {
                    title =  dynVars.toString();
                    if((reaffectText(map, &title, w1)) && (varLoop == 0)) cartesianplotWidget->setProperty("secondsUpdate", title);
                    title =  dynVars.toString();
                    if((reaffectText(map, &title, w1)) && (varLoop == 1)) cartesianplotWidget->setProperty("secondsPast", title);
                    title =  dynVars.toString();
                    if((reaffectText(map, &title, w1)) && (varLoop == 2)) cartesianplotWidget->setProperty("nrOfBins", title);
                }
            }
        }

        // go through all possible curves and add monitor
        for(int i=0; i< caCartesianPlot::curveCount; i++) {
            QString pvs ="";
            QStringList thisString = cartesianplotWidget->getPV(i).split(";");


            cartesianplotWidget->setColor(cartesianplotWidget->getColor(i), i);
            cartesianplotWidget->setStyle(cartesianplotWidget->getStyle(i), i);
            cartesianplotWidget->setSymbol(cartesianplotWidget->getSymbol(i), i);

            if(thisString.count() >= 2 && thisString.at(0).trimmed().length() > 0 && thisString.at(1).trimmed().length() > 0) {
                specData[0] = i; // curve number
                specData[1] = caCartesianPlot::XY_both;
                specData[2] = caCartesianPlot::CH_X; // X
                int num = addMonitor(myWidget, &kData, thisString.at(0), w1, specData, map, &pv);
                integerList.append(num);
                nbMonitors++;
                tooltip.append(pv);
                pvs = pv;
                specData[2] = caCartesianPlot::CH_Y; // Y
                num = addMonitor(myWidget, &kData, thisString.at(1), w1, specData, map, &pv);
                integerList.append(num);
                nbMonitors++;
                tooltip.append(",");
                tooltip.append(pv);
                pvs.append(";");
                pvs.append(pv);
                tooltip.append("<br>");
            } else if(thisString.count() >= 2 && thisString.at(0).trimmed().length() > 0 && thisString.at(1).trimmed().length() == 0) {
                specData[0] = i; // curve number
                specData[1] = caCartesianPlot::X_only;
                specData[2] = caCartesianPlot::CH_X; // X
                int num = addMonitor(myWidget, &kData, thisString.at(0), w1, specData, map, &pv);
                integerList.append(num);
                nbMonitors++;
                pvs.append(pv);
                pvs.append(";");
                tooltip.append(pv);
                tooltip.append("<br>");
            } else if(thisString.count() >= 2 && thisString.at(1).trimmed().length() > 0 && thisString.at(0).trimmed().length() == 0) {
                specData[0] = i; // curve number
                specData[1] = caCartesianPlot::Y_only;
                specData[2] = caCartesianPlot::CH_Y; // Y
                int num = addMonitor(myWidget, &kData, thisString.at(1), w1, specData, map, &pv);
                integerList.append(num);
                nbMonitors++;
                pvs.append(";");
                pvs.append(pv);
                tooltip.append(pv);
                tooltip.append("<br>");
            }
            if (thisString.count() > 2) {
                postMessage(
                    QtWarningMsg,
                    (char *) qasc(
                        tr("caCartesianPlot widget %1 has too many entries (%2) in "
                           "channelList_%3, should be 2.")
                            .arg(cartesianplotWidget->objectName())
                            .arg(thisString.count())
                            .arg(i + 1)));
            }
            cartesianplotWidget->setPV(pvs, i);
        }

        cartesianplotWidget->updateLegendsPV();

        // handle trigger channel if any
        triggerChannel = cartesianplotWidget->getTriggerPV();
        if(triggerChannel.trimmed().length() > 0) {
            specData[2] = caCartesianPlot::CH_Trigger; // Trigger
            int num = addMonitor(myWidget, &kData, triggerChannel, w1, specData, map, &pv);
            integerList.append(num);
            nbMonitors++;
            tooltip.append(pv);
            tooltip.append("<br>");
            cartesianplotWidget->setTriggerPV(pv);
        }

        // handle count channel if any
        int Number;
        if(!cartesianplotWidget->hasCountNumber(&Number)) {
            countChannel = cartesianplotWidget->getCountPV();
            if(countChannel.trimmed().length() > 0) {
                specData[2] = caCartesianPlot::CH_Count; // Count
                int num = addMonitor(myWidget, &kData, countChannel, w1, specData, map, &pv);
                integerList.append(num);
                nbMonitors++;
                tooltip.append(pv);
                tooltip.append("<br>");
                cartesianplotWidget->setCountPV(pv);
            }
        } else {
            qCDebug(caCartesianPlotLog) << "count=" << Number;
        }

        // handle erase channel if any
        eraseChannel = cartesianplotWidget->getErasePV();
        if(eraseChannel.trimmed().length() > 0) {
            specData[2] = caCartesianPlot::CH_Erase; // Count
            int num = addMonitor(myWidget, &kData, eraseChannel, w1, specData, map, &pv);
            integerList.append(num);
            nbMonitors++;
            tooltip.append(pv);
            tooltip.append("<br>");
            cartesianplotWidget->setErasePV(pv);
        }

        // handle user scale
        if(cartesianplotWidget->getXscaling() == caCartesianPlot::User) {
            double xmin, xmax;
            int ok=cartesianplotWidget->getXLimits(xmin, xmax);
            if(ok) cartesianplotWidget->setScaleX(xmin, xmax);
        } else if(cartesianplotWidget->getXscaling() == caCartesianPlot::Auto) {
            cartesianplotWidget->setXscaling(caCartesianPlot::Auto);
            // in case of channel the limits will be defined later by the hopr and lopr
            // however in case of channel, it is possible to get dynamic limits through monitors
        } else if(cartesianplotWidget->getXscaling() == caCartesianPlot::Channel) {
            QString pvs ="";
            QStringList thisStrings = cartesianplotWidget->getXaxisLimits().split(";");
            if(thisStrings.count() == 2 && thisStrings.at(0).trimmed().length() > 0 && thisStrings.at(1).trimmed().length() > 0) {
                double xmin, xmax;
                int ok=cartesianplotWidget->getXLimits(xmin, xmax);
                if(ok) {
                    cartesianplotWidget->setScaleX(xmin, xmax);
                    // when channel was specified for x, but no channel was defined for the plot then set to auto
                    QStringList thisStrngs = cartesianplotWidget->getPV(0).split(";");
                    if(thisStrngs.count() == 2 && thisStrngs.at(0).trimmed().length() == 0)  cartesianplotWidget->setXscaling(caCartesianPlot::Auto);
                } else {
                    specData[0] = 0;
                    specData[2] = caCartesianPlot::CH_Xscale;
                    int num = addMonitor(myWidget, &kData, thisStrings.at(0), w1, specData, map, &pv);
                    integerList.append(num);
                    nbMonitors++;
                    tooltip.append(pv);
                    pvs = pv;
                    specData[0] = 1;
                    specData[2] = caCartesianPlot::CH_Xscale;
                    num = addMonitor(myWidget, &kData, thisStrings.at(1), w1, specData, map, &pv);
                    integerList.append(num);
                    nbMonitors++;
                    tooltip.append(",");
                    tooltip.append(pv);
                    pvs.append(";");
                    pvs.append(pv);
                    tooltip.append("<br>");
                    cartesianplotWidget->setXaxisLimits(pvs);
                }
            }
        }
        if(cartesianplotWidget->getYscaling() == caCartesianPlot::User) {
            double ymin, ymax;
            int ok=cartesianplotWidget->getYLimits(ymin, ymax);
            if(ok) cartesianplotWidget->setScaleY(ymin, ymax);
        } else if(cartesianplotWidget->getYscaling() == caCartesianPlot::Auto){
            cartesianplotWidget->setYscaling(caCartesianPlot::Auto);
            // in case of channel the limits will be defined later by the hopr and lopr
            // however in case of channel, it is possible to get dynamic limits through monitors
        } else if(cartesianplotWidget->getYscaling() == caCartesianPlot::Channel) {
            QString pvs ="";
            QStringList thisStrings = cartesianplotWidget->getYaxisLimits().split(";");
            if(thisStrings.count() == 2 && thisStrings.at(0).trimmed().length() > 0 && thisStrings.at(1).trimmed().length() > 0) {
                double ymin, ymax;
                int ok=cartesianplotWidget->getYLimits(ymin, ymax);
                if(ok) {
                    cartesianplotWidget->setScaleY(ymin, ymax);
                    // when channel was specified for x, but no channel was defined for the plot then set to auto
                    QStringList thisStrngs = cartesianplotWidget->getPV(0).split(";");
                    if(thisStrngs.count() == 2 && thisStrngs.at(1).trimmed().length() == 0)  cartesianplotWidget->setYscaling(caCartesianPlot::Auto);
                } else {
                    specData[0] = 0;
                    specData[2] = caCartesianPlot::CH_Yscale;
                    int num = addMonitor(myWidget, &kData, thisStrings.at(0), w1, specData, map, &pv);
                    integerList.append(num);
                    nbMonitors++;
                    tooltip.append(pv);
                    pvs = pv;

                    specData[0] = 1;
                    specData[2] = caCartesianPlot::CH_Yscale;
                    num = addMonitor(myWidget, &kData, thisStrings.at(1), w1, specData, map, &pv);
                    integerList.append(num);
                    nbMonitors++;
                    tooltip.append(",");
                    tooltip.append(pv);
                    pvs.append(";");
                    pvs.append(pv);
                    tooltip.append("<br>");
                    cartesianplotWidget->setYaxisLimits(pvs);
                }
            }
        }

        // finish tooltip
        tooltip.append(ToolTipPostfix);
        if (!vncServer)
            cartesianplotWidget->setToolTip(tooltip);

        // reaffect titles
        title = cartesianplotWidget->getTitlePlot();
        if(reaffectText(map, &title, w1)) cartesianplotWidget->setTitlePlot(title);
        title = cartesianplotWidget->getTitleX();
        if(reaffectText(map, &title, w1)) cartesianplotWidget->setTitleX(title);
        title = cartesianplotWidget->getTitleY();
        if(reaffectText(map, &title, w1)) cartesianplotWidget->setTitleY(title);

        cartesianplotWidget->setWhiteColors();

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        cartesianplotWidget->setProperty("MonitorList", integerList);

        cartesianplotWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caWaterfallPlot* waterfallplotWidget = qobject_cast<caWaterfallPlot *>(w1)) {

        qCDebug(caWaterfallPlotLog) << "create caWaterfallPlot";
        w1->setProperty("ObjectType", caWaterfallPlot_Widget);

        QString countChannel, waveChannel;

        // addmonitor normally will add a tooltip to show the pv; however here we have more than one pv
        QString tooltip;
        tooltip.append(ToolTipPrefix);

        // handle count channel if any
        int Number;
        specData[1] = 0; // Count must not be waited for
        if(!waterfallplotWidget->hasCountNumber(&Number)) {
            countChannel = waterfallplotWidget->getCountPV();
            if(countChannel.trimmed().length() > 0) {
                specData[0] = 1; // Count
                specData[1] = 1; // Count must be waited for
                int num = addMonitor(myWidget, &kData, countChannel, w1, specData, map, &pv);
                integerList.append(num);
                nbMonitors++;
                tooltip.append(pv);
                tooltip.append("<br>");
                waterfallplotWidget->setCountPV(pv);
            }
        } else {
            qCDebug(caWaterfallPlotLog) << "count=" << Number;
        }

        waveChannel = waterfallplotWidget->getPV();
        specData[0] = 0; // waveform
        int num = addMonitor(myWidget, &kData, waveChannel, w1, specData, map, &pv);
        integerList.append(num);
        nbMonitors++;
        waterfallplotWidget->setPV(pv);
        tooltip.append(pv);
        tooltip.append("<br>");

        // finish tooltip
        tooltip.append(ToolTipPostfix);
        if (!vncServer)
            waterfallplotWidget->setToolTip(tooltip);

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        waterfallplotWidget->setProperty("MonitorList", integerList);

        waterfallplotWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caStripPlot* stripplotWidget = qobject_cast<caStripPlot *>(w1)) {

        qCDebug(caStripPlotLog) << "create caStripPlot";
        w1->setProperty("ObjectType", caStripPlot_Widget);

        QString text, title;
        QStringList extra_legend,legend;

        // addmonitor normally will add a tooltip to show the pv; however here we have more than one pv
        QString tooltip;
        tooltip.append(ToolTipPrefix);

        text = stripplotWidget->getPVS();
        reaffectText(map, &text, w1);
        stripplotWidget->setPVS(text);
        QStringList vars = text.split(";", SKIP_EMPTY_PARTS);

        int NumberOfCurves = min(vars.count(), caStripPlot::MAXCURVES);

// look for a description replacement for the Legend of stripplot

        QVariant legenddata = w1->property("Legend");
        if (legenddata.isValid()) {
            if (legenddata.type()==QVariant::StringList)
                extra_legend = legenddata.value<QStringList>();

            if (legenddata.type()==QVariant::String)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                extra_legend = legenddata.value<QString>().split(";",QString::KeepEmptyParts);
#else
                extra_legend = legenddata.value<QString>().split(";",Qt::KeepEmptyParts);
#endif

        }
        qCDebug(caStripPlotLog) << "extra_legend" << extra_legend << extra_legend.count();
        if (extra_legend.count()>0){
            for(int i=0; i< NumberOfCurves; i++) {
                qCDebug(caStripPlotLog) << "legend for" << legend;
                if (i<extra_legend.count()){
                    QString element=extra_legend.at(i);
                    if (!element.isNull() && !element.isEmpty()){
                        legend.append(element);
                    } else legend.append(vars.at(i));
                }else legend.append(vars.at(i));

            }
        }else legend=vars;

// go through the defined curves and add monitor

        qCDebug(caStripPlotLog) << "legend" << legend;
        if(NumberOfCurves > 0) stripplotWidget->defineCurves(legend, stripplotWidget->getUnits(), stripplotWidget->getPeriod(),  stripplotWidget->width(),  NumberOfCurves);
        for(int i=0; i< NumberOfCurves; i++) {
            pv = vars.at(i).trimmed();
            if(pv.size() > 0) {
                if(i==0) {  // user defaults, will be redefined when limits from channel
                    stripplotWidget->setYscale(stripplotWidget->getYaxisLimitsMin(i), stripplotWidget->getYaxisLimitsMax(i));
                }
                specData[1] = i;            // curve number
                specData[0] = vars.count(); // number of curves
                int num = addMonitor(myWidget, &kData, pv, w1, specData, map, &pv);
                integerList.append(num);
                nbMonitors++;
                stripplotWidget->showCurve(i, true);

                tooltip.append(pv);
                tooltip.append("<br>");
            }
        }

        // finish tooltip
        tooltip.append(ToolTipPostfix);
        if (!vncServer)
            stripplotWidget->setToolTip(tooltip);

        title = stripplotWidget->getTitlePlot();
        if(reaffectText(map, &title, w1)) stripplotWidget->setTitlePlot(title);
        title = stripplotWidget->getTitleX();
        if(reaffectText(map, &title, w1)) stripplotWidget->setTitleX(title);
        title = stripplotWidget->getTitleY();
        if(reaffectText(map, &title, w1)) stripplotWidget->setTitleY(title);

        integerList.insert(0, nbMonitors); /* set property into stripplotWidget */
        stripplotWidget->setProperty("MonitorList", integerList);

        stripplotWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caTable* tableWidget = qobject_cast<caTable *>(w1)) {

        qCDebug(caTableLog) << "create caTable" << tableWidget->getPVS();
        w1->setProperty("ObjectType", caTable_Widget);

        QStringList vars = tableWidget->getPVS().split(";", SKIP_EMPTY_PARTS);
        tableWidget->setColumnCount(3);
        tableWidget->setRowCount(vars.count());

        // go through the pvs and add monitor
        for(int i=0; i< vars.count(); i++) {
            pv = vars.at(i);
            if(pv.size() > 0) {
                QTableWidgetItem *item;
                specData[0] = i;            // table row
                int num = addMonitor(myWidget, &kData, pv, w1, specData, map, &pv);
                integerList.append(num);
                nbMonitors++;
                item = new QTableWidgetItem(pv);
                item->setTextAlignment(Qt::AlignAbsolute | Qt:: AlignLeft);
                tableWidget->setItem(i,0, item);
            }
        }
        tableWidget->setColumnSizes(tableWidget->getColumnSizes());

        integerList.insert(0, nbMonitors); /* set property into stripplotWidget */
        tableWidget->setProperty("MonitorList", integerList);

        tableWidget->setProperty("Taken", true);
        tableWidget->setToolTip("select row or columns, then with Ctrl+C you can copy to the clipboard\ninside X11 you can then do shft+ins\nwhen doubleclicking on a value, you may execute a shell script for that device");

        connect(tableWidget, SIGNAL(TableDoubleClickedSignal(QString)), this, SLOT(Callback_TableDoubleClicked(QString)));

        //==================================================================================================================
    } else if(caWaveTable* wavetableWidget = qobject_cast<caWaveTable *>(w1)) {

        qCDebug(caWaveTableLog) << "create caWaveTable";
        w1->setProperty("ObjectType", caWaveTable_Widget);

        if(wavetableWidget->getPV().size() > 0) {

            // add also the FTVL field in order to know if we have signed or unsigned data
            specData[0] = 1;
            int num = addMonitor(myWidget, &kData, wavetableWidget->getPV().trimmed() + ".FTVL", w1, specData, map, &pv);
            integerList.append(num);
            nbMonitors++;

            specData[0] = 0;
            num = addMonitor(myWidget, &kData, wavetableWidget->getPV().trimmed(), w1, specData, map, &pv);
            integerList.append(num);
            nbMonitors++;
            wavetableWidget->setPV(pv);

            connect(wavetableWidget, SIGNAL(WaveEntryChanged(QString, int)), this, SLOT(Callback_WaveEntryChanged(QString, int)));
        }

        integerList.insert(0, nbMonitors); /* set property into stripplotWidget */
        wavetableWidget->setProperty("MonitorList", integerList);

        wavetableWidget->setProperty("Taken", true);

        wavetableWidget->setToolTip("select row or columns, then with Ctrl+C you can copy to the clipboard\ninside X11 you can then do shft+ins\nwhen doubleclicking on a value, you can change the value");

        //==================================================================================================================

    } else if(caScan2D* scan2dWidget = qobject_cast<caScan2D *>(w1)) {

        qCDebug(caScan2DLog) << "create caScan2D";

        QString text;

        // if we need to write c    hannels from the camera widget, we do it by timer and slot
        connect(scan2dWidget, SIGNAL(WriteDetectedValuesSignal(QWidget*)), this, SLOT(Callback_WriteDetectedValues(QWidget*)));

        // addmonitor normally will add a tooltip to show the pv; however here we have more than one pv
        QString tooltip;
        QString pvs1= "";
        QString pvs2= "";

        tooltip.append(ToolTipPrefix);

        for(int i=0; i<17; i++) {
            bool alpha = true;
            text = "";

            if(i==0) {text = scan2dWidget->getPV_Data(); if (text.size() > 0) specData[0] = i;}
            if(i==1) {text = scan2dWidget->getPV_Width(); if (text.size() > 0) specData[0] = i;}
            if(i==2) {text = scan2dWidget->getPV_Height(); if (text.size() > 0) specData[0] = i;}
            // for spectrum pseudo levels
            if(i==5) {
                alpha = scan2dWidget->isAlphaMinLevel();
                text = scan2dWidget->getMinLevel();
                if ((text.size() > 0) && alpha) specData[0] = i;
            }
            if(i==6) {
                alpha = scan2dWidget->isAlphaMaxLevel();
                text = scan2dWidget->getMaxLevel();
                if ((text.size() > 0) && alpha) specData[0] = i;
            }
            if (i==8) {text = scan2dWidget->getPV_XCPT(); if (text.size() > 0) specData[0] = 11;}
            if (i==9) {text = scan2dWidget->getPV_YCPT(); if (text.size() > 0) specData[0] = 12;}
            if (i==10) {text = scan2dWidget->getPV_XNEWDATA(); if (text.size() > 0) specData[0] = 13;}
            if (i==11) {text = scan2dWidget->getPV_YNEWDATA(); if (text.size() > 0) specData[0] = 14;}
            if (i==12) {text = scan2dWidget->getPV_SAVEDATA_PATH(); if (text.size() > 0) specData[0] = 15;}
            if (i==13) {text = scan2dWidget->getPV_SAVEDATA_SUBDIR(); if (text.size() > 0) specData[0] = 16;}
            if (i==14) {text = scan2dWidget->getPV_SAVEDATA_FILENAME(); if (text.size() > 0) specData[0] = 17;}

            // for dataprocessing data x,y,w,h ROI read and write
            if(i > 14) {
                QStringList thisString;
                if(i==15) thisString = scan2dWidget->getROIChannelsRead().split(";");
                if(i==16) thisString = scan2dWidget->getROIChannelsWrite().split(";");

                if(thisString.count() == 4 &&
                        thisString.at(0).trimmed().length() > 0 &&
                        thisString.at(1).trimmed().length() > 0  &&
                        thisString.at(2).trimmed().length() > 0 &&
                        thisString.at(3).trimmed().length() > 0) {
                    for(int j=0; j<4; j++) {
                        if(i==15)specData[0] = i+j+3;   // x,y,w,h
                        text = treatMacro(map, thisString.at(j), &doNothing, w1->objectName());
                        if(i==15) {
                            int num = addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
                            integerList.append(num);
                            nbMonitors++;
                            pvs1.append(pv);
                        }
                        if(i==16)pvs2.append(text);
                        if((j<3) && (i==15))pvs1.append(";");
                        if((j<3) && (i==16))pvs2.append(";");
                    }
                }
            }

            if(text.size() > 0 && alpha) {
                text =  treatMacro(map, text, &doNothing, w1->objectName());
                if((i!=15) && (i!=16)) {
                    int num = addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
                    integerList.append(num);
                    nbMonitors++;
                }
                if(i==0) scan2dWidget->setPV_Data(pv);
                if(i==1) scan2dWidget->setPV_Width(pv);
                if(i==2) scan2dWidget->setPV_Height(pv);
                if(i==5) scan2dWidget->setMinLevel(pv);
                if(i==6) scan2dWidget->setMaxLevel(pv);
                if(i==8) scan2dWidget->setPV_XCPT(pv);
                if(i==9) scan2dWidget->setPV_YCPT(pv);
                if(i==10) scan2dWidget->setPV_XNEWDATA(pv);
                if(i==11) scan2dWidget->setPV_YNEWDATA(pv);
                if(i==12) scan2dWidget->setPV_SAVEDATA_PATH(pv);
                if(i==13) scan2dWidget->setPV_SAVEDATA_SUBDIR(pv);
                if(i==14) scan2dWidget->setPV_SAVEDATA_FILENAME(pv);


                if(i>0) tooltip.append("<br>");
                if(i<15) tooltip.append(pv);
                else if(i==15) tooltip.append(pvs1);
                else if(i==16) tooltip.append(pvs2);
            }
        }


        // finish tooltip
        tooltip.append(ToolTipPostfix);
        if (!vncServer)
            scan2dWidget->setToolTip(tooltip);

        integerList.insert(0, nbMonitors); /* set property into stripplotWidget */
        scan2dWidget->setProperty("MonitorList", integerList);

        scan2dWidget->setProperty("Taken", true);
    } else if(caHMIConfig* hmiConfigWidget = qobject_cast<caHMIConfig *>(w1)) {
        qCDebug(caHMILog) << "create caHMIConfig";
        w1->setProperty("ObjectType", caHMIConfig_Widget);

#ifndef MOBILE
        QList<QVariant> indexList;

        QWidget* parentWindow = Q_NULLPTR;
        QWidget* currentWidget = hmiConfigWidget;
        while (currentWidget) {
            if (qobject_cast<QMainWindow*>(currentWidget) || qobject_cast<QDialog*>(currentWidget)){
                parentWindow = currentWidget;
                break;
            }
            currentWidget = currentWidget->parentWidget();
        }

        if (parentWindow != Q_NULLPTR){
            qCDebug(caHMILog) << "Parent window:" << parentWindow->metaObject()->className();
        } else qCWarning(caHMILog) << "Unable to find parent window for caHMIConfig:" << hmiConfigWidget->objectName();

        int index = 0;
        if (hmiConfigWidget->channel().size() > 0) {
            int num = addMonitor(myWidget, &kData, hmiConfigWidget->channel(), w1, specData, map, &pv);
            integerList.append(num);
            indexList.append(index);
            hmiConfigWidget->setChannel(pv);
            nbMonitors++;
        }
        index++;
        if (hmiConfigWidget->channelB().size() > 0) {
            int num = addMonitor(myWidget, &kData, hmiConfigWidget->channelB(), w1, specData, map, &pv);
            integerList.append(num);
            indexList.append(index);
            hmiConfigWidget->setChannelB(pv);
            nbMonitors++;
        }
        index++;
        if (hmiConfigWidget->channelC().size() > 0) {
            int num = addMonitor(myWidget, &kData, hmiConfigWidget->channelC(), w1, specData, map, &pv);
            integerList.append(num);
            indexList.append(index);
            hmiConfigWidget->setChannelC(pv);
            nbMonitors++;
        }
        index++;
        if (hmiConfigWidget->channelD().size() > 0) {
            int num = addMonitor(myWidget, &kData, hmiConfigWidget->channelD(), w1, specData, map, &pv);
            integerList.append(num);
            indexList.append(index);
            hmiConfigWidget->setChannelD(pv);
            nbMonitors++;
        }
        index++;
        if (hmiConfigWidget->outputA().size() > 0) {
            int num = addMonitor(myWidget, &kData, hmiConfigWidget->outputA(), w1, specData, map, &pv);
            integerList.append(num);
            indexList.append(index);
            hmiConfigWidget->setOutputA(pv);
        }
        index++;
        if (hmiConfigWidget->outputB().size() > 0) {
            int num = addMonitor(myWidget, &kData, hmiConfigWidget->outputB(), w1, specData, map, &pv);
            integerList.append(num);
            indexList.append(index);
            hmiConfigWidget->setOutputB(pv);
        }
        index++;

        caHMIConfigTransferItem* transferItem = new caHMIConfigTransferItem(hmiConfigWidget);
        transferItem->setEnabled(true);
        transferItem->setFileName(savedFile[level]);
        transferItem->setObjectName(hmiConfigWidget->objectName());
        transferItem->setOutputA(hmiConfigWidget->outputA());
        transferItem->setOutputB(hmiConfigWidget->outputB());
        transferItem->setChannel(hmiConfigWidget->channel());
        transferItem->setChannelB(hmiConfigWidget->channelB());
        transferItem->setChannelC(hmiConfigWidget->channelC());
        transferItem->setChannelD(hmiConfigWidget->channelD());
        transferItem->setShortcut(hmiConfigWidget->shortcut());
        transferItem->setValue(hmiConfigWidget->value());
        transferItem->setCalculationType(hmiConfigWidget->calculationType());
        transferItem->setCaptureType(hmiConfigWidget->captureType());
        transferItem->setCaptureRange(hmiConfigWidget->captureRange());
        transferItem->setUUID(hmiConfigWidget->uuid());
        transferItem->setPID(QCoreApplication::applicationPid());
        transferItem->setTimestamp(QDateTime::currentMSecsSinceEpoch());
        transferItem->setWidgetCallback(hmiConfigWidget);
        transferItem->setParentWindowCallback(parentWindow);
        QWriteLocker locker(&hmiConfigListLock);
        hmiConfigList.append(transferItem);

        if (HmiSharedEventBus::instance().isInitialized()) {
            QByteArray byteArray;
            QDataStream out(&byteArray, QIODevice::WriteOnly);
            out << *transferItem;
            HmiSharedEventBus::instance().sendEvent(EventTypes::NewCaHMIConfig, byteArray);
        }

        if (HmiSharedConfigListManager::instance().isInitialized()) {
            HmiSharedConfigListManager::instance().updateList([&](QList<QSharedPointer<caHMIConfigTransferItem>> &list) {
                list.append(transferItem->clone());
            });
        }

        connect(hmiConfigWidget, &QObject::destroyed, [transferItem](QObject *sender) {
            Q_UNUSED(sender);
            QString uuid = transferItem->uuid();

            QWriteLocker locker(&hmiConfigListLock);
            auto newEnd = std::remove_if(hmiConfigList.begin(), hmiConfigList.end(),
            [&](caHMIConfigTransferItem *item) {
                if (item->uuid() == uuid) {
                    delete item;
                    return true;
                }
                return false;
            });
            hmiConfigList.erase(newEnd, hmiConfigList.end());

            if (HmiSharedEventBus::instance().isInitialized()) {
                QByteArray byteArray;
                QDataStream out(&byteArray, QIODevice::WriteOnly);
                out << uuid;
                HmiSharedEventBus::instance().sendEvent(EventTypes::CaHMIConfigDeleted, byteArray);
            }

            if (HmiSharedConfigListManager::instance().isInitialized()) {
                HmiSharedConfigListManager::instance().updateList([&](QList<QSharedPointer<caHMIConfigTransferItem>> &list) {
                    auto it = std::remove_if(list.begin(), list.end(),
                                             [&](const QSharedPointer<caHMIConfigTransferItem>& item) {
                                                 return item->uuid() == uuid;
                                             });
                    list.erase(it, list.end());
                });
            }
        });

        // insert dataindex list
        integerList.insert(0, nbMonitors);
        indexList.insert(0, nbMonitors);
        hmiConfigWidget->setProperty("MonitorList", integerList);
        hmiConfigWidget->setProperty("IndexList", indexList);

    #endif

        hmiConfigWidget->setProperty("Taken", true);
    } if(wmSignalRescale* wmSignalRescaleWidget = qobject_cast<wmSignalRescale *>(w1)) {
        qCDebug(wmSignalRescaleLog) << "create wmSignalRescale";
        w1->setProperty("ObjectType", wmSignalRescale_Widget);

        QString outputA = wmHandleSoftChannel(wmSignalRescaleWidget->softChannelA(), map, doNothing, w1->objectName());
        QString outputB = wmHandleSoftChannel(wmSignalRescaleWidget->softChannelB(), map, doNothing, w1->objectName());

        wmSignalRescaleWidget->setSoftChannelA(outputA);
        wmSignalRescaleWidget->setSoftChannelB(outputB);

        if (outputA.length() > 0) {
            int num = addMonitor(myWidget, &kData, outputA, w1, specData, map, &pv);
            integerList.append(num);
            nbMonitors++;
        }

        if (outputB.length() > 0) {
            int num = addMonitor(myWidget, &kData, outputB, w1, specData, map, &pv);
            integerList.append(num);
            nbMonitors++;
        }

        QWidget* container = wmSignalRescaleWidget->parentWidget();

        if (container != Q_NULLPTR) {
            container->installEventFilter(wmSignalRescaleWidget);
        }

        connect(wmSignalRescaleWidget, &wmSignalRescale::internalResizeEvent, this, &CaQtDM_Lib::wmHandleResize);

        integerList.insert(0, nbMonitors);
        wmSignalRescaleWidget->setProperty("MonitorList", integerList);
        wmSignalRescaleWidget->setProperty("Taken", true);
    }

    //==================================================================================================================

    // search for a QTabWidget as nearest parent and set it as property
    if(className.contains("ca")) {
        QWidget *tabWidget = getTabParent(w1);
        w1->setProperty("parentTab",QVariant::fromValue(tabWidget) );
        w1->setAttribute(Qt::WA_Hover, true);
    }

    // make a context menu for object having a monitor
    //if(className.contains("ca") && !className.contains("caRel") && !className.contains("caTable") && !className.contains("caShellCommand") && nbMonitors > 0) {
    if((className.contains("ca") && !className.contains("caTable") && !className.contains("caShellCommand") && nbMonitors > 0) || className.contains("caRel") ||
           className.contains("caInclude") || className.contains("caScript")) {

        w1->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(w1, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenu(const QPoint&)));
        w1->setProperty("Connect", false);
        // in order to get the context on tablets
#ifdef MOBILE
        w1->grabGesture(Qt::TapAndHoldGesture);
        w1->installEventFilter(this);
#else
        if(!thisFileFull.contains(POPUPDEFENITION)) w1->installEventFilter(this);
#endif
    }

    // add our context to AS widgets

    if(className.contains("QEAnalogProgressBar")) {
        w1->setContextMenuPolicy(Qt::CustomContextMenu);
        disconnect(w1, SIGNAL(customContextMenuRequested(const QPoint&)), 0, 0);
        connect(w1, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenu(const QPoint&)));
        w1->setProperty("Connect", false);
    }

}

/**
  * this routine gest fired when a QTextBrowser source file changes
  */
void CaQtDM_Lib::handleFileChanged(const QString &file)
{
    Q_UNUSED(file);
    qCDebug(fileIOLog) << "update" << file;
    updateTextBrowser();
}

/**
  * this routine reloads periodically all QTextBrowsers
  */
void CaQtDM_Lib::updateTextBrowser()
{
    QList<QTextBrowser *> allBrowsers = myWidget->findChildren<QTextBrowser *>();
    foreach(QTextBrowser* widget, allBrowsers) {
        widget->reload();
    }
}

QString CaQtDM_Lib::handle_single_Macro(QString key, QString value, QString Text){
    QString toReplace = "$(" + key+ ")";
    qCDebug(caQtDMLibLog) << "replace in" << Text << toReplace << "with" << value;
    Text.replace(toReplace,value);
    return Text;
}


QString CaQtDM_Lib::handle_Macro_withConst(QString key, QString value, QString Text){
    QStack<int> parenthesesStack;
    QString pattern=key +"=";
    qCDebug(caQtDMLibLog) << "start handle_Macro_withConst2" << key;
    for (int i = 1; i < Text.length(); ++i) {
        if((Text[i-1] == '$')&&(Text[i] == '(')) {
            parenthesesStack.push(i);
        } else if (Text[i] == ')') {
            if (!parenthesesStack.empty()) {
                int start = parenthesesStack.top();
                parenthesesStack.pop();
                QString content = Text.mid(start + 1, i - start - 1);
                qCDebug(caQtDMLibLog) << "Matched text inside parentheses(macro with value):" << content;
                if (content.startsWith(pattern)) {
                    Text.replace("$("+content+")", value);
                    qCDebug(caQtDMLibLog) << " Replaced:" <<Text;
                }
            } else {
                qCDebug(caQtDMLibLog) << "Unmatched closing parenthesis at position" << i;
//                if (!Text.contains("regex\":")){
//                    char asc[MAX_STRING_LENGTH];
//                    QString errorText="Unmatched closing parenthesis: "+Text+"[ POS:+"+QString::number(i)+"]";
//                    qstrncpy(asc,qasc(errorText),MAX_STRING_LENGTH);
//                    postMessage(QtCriticalMsg, asc);
//                }
            }
        }
    }

    while (!parenthesesStack.empty()) {
//        if (!Text.contains("regex\":")){
//            char asc[MAX_STRING_LENGTH];
//            QString errorText="Unmatched opening parenthesis at: "+Text+"[ POS:+"+QString::number(parenthesesStack.top())+"]";
//            qstrncpy(asc,qasc(errorText),MAX_STRING_LENGTH);
//            postMessage(QtCriticalMsg, asc);
//        }
        parenthesesStack.pop();
    }
    return Text;
}

QString CaQtDM_Lib::handle_Macro_Constants(QString Text){
    QStack<int> parenthesesStack;
    for (int i = 1; i < Text.length(); ++i) {
        if((Text[i-1] == '$')&&(Text[i] == '(')) {
            parenthesesStack.push(i);
        } else if (Text[i] == ')') {
            if (!parenthesesStack.empty()) {
                int start = parenthesesStack.top();
                parenthesesStack.pop();
                QString content = Text.mid(start + 1, i - start - 1);
                qCDebug(caQtDMLibLog) << "Matched text inside parentheses(macro with const):" << content;
                QStringList macro_data=content.split("=");
                if (macro_data.count()>1){
                    Text.replace("$("+content+")",macro_data.at(1));
                }

            } else {
                qCDebug(caQtDMLibLog) << "Unmatched closing parenthesis at position" << i;
            }
        }
    }

    while (!parenthesesStack.empty()) {
        qCDebug(caQtDMLibLog) << "Unmatched opening parenthesis at position" << parenthesesStack.top();
        parenthesesStack.pop();
    }
    return Text;
}




QString CaQtDM_Lib::handle_Macro_Scan(QString Text,QMap<QString, QString> map,macro_parser parse){

    QMapIterator<QString, QString> i(map);
    int recursive_counter = 0;
    bool recursive_continue=true;
    while (recursive_continue) {
        QString Text_Backup=Text;
        while (i.hasNext()) {
            i.next();
            switch (parse){
                case parse_simple:{
                    Text=handle_single_Macro(i.key(),i.value(),Text);
                    break;
                }
                case parse_withconst:{
                    Text=handle_Macro_withConst(i.key(),i.value(),Text);
                    break;
                }
            }
        }
        if (Text_Backup.compare(Text)==0){
            qCDebug(caQtDMLibLog) << "finish Loop simple Macro Replace";
            recursive_continue=false;
        }
        if(recursive_counter++ > 10) break;
    }
    return Text;

}

/**
  * this routine uses macro table to replace inside the pv the macro part
  */
QString CaQtDM_Lib::treatMacro(QMap<QString, QString> map, const QString& text, bool *doNothing, QString widgetName)
{
    QString newText = text;
    char asc[MAX_STRING_LENGTH];
    *doNothing = false;
    // a macro exists and when pv contains a right syntax then replace pv
    if(!map.isEmpty()) {
        if(text.contains("$(") && text.contains(")")) {
            // normal macroexchange

            newText=handle_Macro_Scan(newText,map,parse_simple);
            newText=handle_Macro_Scan(newText,map,parse_withconst);

            QMapIterator<QString, QString> i(map);
            i.toFront();
            if(newText.contains("$(")){
                qCDebug(caQtDMLibLog) << "Spezial";
                while (i.hasNext()) {
                    i.next();
                    QString tofind = "$(" + i.key()+"{";
                    int position=newText.indexOf(tofind);
                    while (position!=(-1)){
                        qCDebug(caQtDMLibLog) << "position" <<position;
                        if ((position>=0)&&(position<newText.length())){
                            int json_start = (position-1)+tofind.length();
                            int json_end = newText.indexOf(QString("})"),json_start);

                            qCDebug(caQtDMLibLog) << "newText.mid(): " << newText.mid(json_start,json_end-json_start+1);
                            QString macro_regex = "";
                            QString macro_value = "Parsing Error";
                            bool macro_value_found = false;

                            QJsonParseError parseError;
                            QJsonDocument jsonDocument = QJsonDocument::fromJson(newText.mid(json_start,json_end-json_start+1).toUtf8(), &parseError);
                            if (parseError.error == QJsonParseError::NoError){
                                if(jsonDocument.isObject()) {
                                    QJsonObject jsonobj = jsonDocument.object();
                                    if (jsonobj["regex"].isString()) {
                                        macro_regex = jsonobj["regex"].toString();
                                    }

                                    if (jsonobj["value"].isString()) {
                                        macro_value = jsonobj["value"].toString();
                                        macro_value_found = true;
                                    }
                                }
                            } else {
                                snprintf(asc, MAX_STRING_LENGTH, "JSON Error in (%s): %s", qasc(newText.mid(json_start,json_end-json_start+1)), qasc(parseError.errorString()));
                                postMessage(QtWarningMsg, asc);
                            }

                            QString toReplace = "$(" + i.key() + newText.mid(json_start,json_end-json_start+1) + ")";
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                            QRegExp rx_json;
                            rx_json.setPattern(macro_regex);
#else
                            QRegularExpression rx_json(macro_regex);
#endif
                            if (rx_json.isValid()){
                                QString ReplaceWith=i.value();
                                    if (macro_value_found){
                                    ReplaceWith.replace(rx_json,macro_value);

                                    qCDebug(caQtDMLibLog) << "replace in" << newText << toReplace << "with" << ReplaceWith << "outof " << macro_value;
                                    snprintf(asc, MAX_STRING_LENGTH, "Replace (%s) (%s) with (%s) Regex:(%s)", qasc(i.key()), qasc(i.value()),qasc(macro_value),qasc(macro_regex));
                                    postMessage(QtDebugMsg, asc);
                                    newText.replace(toReplace, ReplaceWith);
                                }else{
                                    snprintf(asc, MAX_STRING_LENGTH, "No Replacement found do simple(%s) (%s) macro resolution", qasc(i.key()), qasc(i.value()));
                                    qCDebug(caQtDMLibLog) << "No Replacement found do simple(%s) (%s) macro resolution" <<  qasc(i.key()) <<  qasc(i.value()) << text;
                                    postMessage(QtWarningMsg, asc);
                                    newText.replace(toReplace, i.value());
                                }
                            }

                        }
                        position=newText.indexOf(tofind,position+1);
                    }
                }

            }
            // unresolved macros with a own constant
            qCDebug(caQtDMLibLog) << "unresolved macros with a own constant";
            if(newText.contains("$(")){

                int recursive_counter=0;
                bool recursive_continue=true;
                while (recursive_continue) {
                    QString newText_Backup=handle_Macro_Constants(newText);
                    qCDebug(caQtDMLibLog) << newText_Backup<< "<-" << newText;
                    if (newText_Backup.compare(newText)==0){
                        qCDebug(caQtDMLibLog) << "finish Loop simple Macro Replace";
                        recursive_continue=false;
                    }
                    newText=newText_Backup;
                    if(recursive_counter++ > 10) break;
                }
            }


            // unresolved macros
            if(newText.contains("$(")){
                QString unresMacro = "";
                QString tofind = "$(";
                int position=newText.indexOf(tofind);
                while (position != (-1)){
                    int wrongmacro_start=(position-1)+tofind.length();
                    int wrongmacro_end  =newText.indexOf(QString(")"), wrongmacro_start);
                    if ((position >= 0)&&(position < newText.length())){
                        unresMacro.append(newText.mid(wrongmacro_start, wrongmacro_end-wrongmacro_start+1));
                    }
                    position=newText.indexOf(tofind,position+1);
                }
                qCDebug(caQtDMLibLog) << unresMacro << "for widget" << widgetName << "in file" << savedFile[level];
                QString key = "%1###%2###%3";
                key = key.arg(unresMacro).arg(widgetName).arg(savedFile[level]);
                unknownMacrosList.insert(key, savedFile[level]);
            }
        }
    } else {
        if(text.contains("$")) *doNothing = true;
    }
    return newText;
}

void CaQtDM_Lib::UndefinedMacrosWindow()
{
    int count=0;
    int thisWidth = 550;
    int thisHeight = 250;
    bool showMax = false;

    macroWindow = new QDialog();
    macroWindow->setWindowTitle(QString::fromUtf8("undefined Macro's"));

#if defined(MOBILE_IOS)
    if(qApp->primaryScreen()->size().height() < 500) {
        thisWidth=430;  // normal for iphone
        thisHeight=200;
    }
    Specials special;
    special.setNewStyleSheet(this, qApp->primaryScreen()->size(), 16, 10);
    QPalette palette;
    palette.setBrush(QPalette::Window, QColor(255,255,224,255));
    macroWindow->setPalette(palette);
    macroWindow->setAutoFillBackground(true);
    macroWindow->setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter, QSize(thisWidth,thisHeight), qApp->primaryScreen()->availableGeometry()));
#elif defined(MOBILE_ANDROID)
    QPalette palette;
    palette.setBrush(QPalette::Window, QColor(255,255,224,255));
    setPalette(palette);
    macroWindow->setAutoFillBackground(true);
    showMax = true;
#else
    macroWindow->move(this->x() + this->width() / 2 - thisWidth / 2 , this->y() + this->height() /2 -thisHeight/2);
#endif

    QVBoxLayout *l = new QVBoxLayout();
    macroTable = new QTableWidget();
    QPushButton *pushbutton = new QPushButton("close");
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(Callback_UndefinedMacrowindowExit()));

    l->addWidget(macroTable);
    l->addWidget(pushbutton);

    macroWindow->setLayout(l);

    macroTable->clear();
    macroTable->setColumnCount(3);
    macroTable->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    macroTable->setHorizontalHeaderLabels(QString("unresolved macro;widget;filename").split(";"));
    macroTable->setAlternatingRowColors(true);
    macroTable->horizontalHeader()->setStretchLastSection(true);

    QMap<QString, QString>::const_iterator i = unknownMacrosList.constBegin();
    if(unknownMacrosList.count() > 0) macroTable->setRowCount(unknownMacrosList.count());
    else macroTable->setRowCount(1);
    while (i != unknownMacrosList.constEnd()) {
        QStringList list = i.key().split("###");
        qCDebug(caQtDMLibLog) << i.key() << "macro variable" << list.at(0) << "in widget" << list.at(1) << "in file" << list.at(2) << "is undefined";
        if (list.length() < 3) continue;

        QTableWidgetItem *macroItem = new QTableWidgetItem(list.at(0));
        macroItem->setFlags(macroItem->flags() & ~Qt::ItemIsEditable);
        macroTable->setItem(count, 0, macroItem);

        QTableWidgetItem *widgetItem = new QTableWidgetItem(list.at(1));
        widgetItem->setFlags(widgetItem->flags() & ~Qt::ItemIsEditable);
        macroTable->setItem(count, 1, widgetItem);

        QTableWidgetItem *fileNameItem = new QTableWidgetItem(list.at(2));
        fileNameItem->setFlags(fileNameItem->flags() & ~Qt::ItemIsEditable);
        macroTable->setItem(count++, 2, fileNameItem);

        ++i;
    }
    macroTable->resizeColumnsToContents();
    if(macroTable->columnWidth(0) > 400) macroTable->setColumnWidth(0,400);

    // set width of window
    int w = 0;
    count = macroTable->columnCount();
    for (int i = 0; i < count; i++) w += macroTable->columnWidth(i);
    int maxW = (w + count + macroTable->verticalHeader()->width() + macroTable->verticalScrollBar()->width());
    macroWindow->setMinimumWidth(maxW+25);

    connect(this, &CaQtDM_Lib::Signal_Closing, macroWindow, &QDialog::close);
    if(!showMax) showNormal();
    else showMaximized();
    macroWindow->exec();
    macroWindow->close();
    macroWindow->deleteLater();
}

void CaQtDM_Lib::GlobalShortcutWindow() {
    int width = 550;
    int height = 250;

    shortcutWindow = new QDialog();
    shortcutWindow->setWindowTitle("Global Shortcuts");
    QVBoxLayout *layout = new QVBoxLayout();
    QTableWidget *table = new QTableWidget();
    QPushButton *closeButton = new QPushButton("Close");
    connect(closeButton, SIGNAL(clicked()), this, SLOT(Callback_GlobalShortcutWindowExit()));

    shortcutWindow->move(this->x() + this->width() / 2 - width / 2, this->y() + this->height() / 2 - height / 2);

    layout->addWidget(table);
    layout->addWidget(closeButton);

    shortcutWindow->setLayout(layout);

    table->clear();
    table->setColumnCount(5);
    table->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    table->setHorizontalHeaderLabels(QString("caHMIConfig;Trigger;Filename;Process;Enabled").split(";"));
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setStretchLastSection(true);

    QList<QSharedPointer<caHMIConfigTransferItem>> externalItemSharedPointersHolders;

    QMap<caHMIConfigTransferItem*, bool> globalConfigItems;
    // false = internal item, true = external item

    {
        QReadLocker locker(&hmiConfigListLock);
        foreach (caHMIConfigTransferItem* item, hmiConfigList) {
            if (item && item->captureRange() == caHMIConfig::capRange::Global) {
                globalConfigItems.insert(item, false);
            }
        }
    }

    QSet<QString> uuidsToRemove;
    qint64 threeSecondsAgo = QDateTime::currentDateTime().addSecs(-3).toMSecsSinceEpoch();
#ifndef MOBILE
    if (HmiSharedConfigListManager::instance().isInitialized()) {
        HmiSharedConfigListManager::instance().updateList([&](QList<QSharedPointer<caHMIConfigTransferItem>> &list) {
            QMutableListIterator<QSharedPointer<caHMIConfigTransferItem>> iterator(list);
            while (iterator.hasNext()) {
                auto item = iterator.next();
                if (item->timestamp() < threeSecondsAgo) {
                    uuidsToRemove.insert(item->uuid());
                    iterator.remove();
                }
            }
        });
    }

    {
        QWriteLocker locker(&externalHmiConfigListLock);
        QMutableListIterator<QSharedPointer<caHMIConfigTransferItem>> iterator(externalHmiConfigList);
        while (iterator.hasNext()) {
            auto item = iterator.next();
            if (item.data() == Q_NULLPTR || uuidsToRemove.contains(item->uuid())) {
                iterator.remove();
            } else if (item->captureRange() == caHMIConfig::capRange::Global) {
                globalConfigItems.insert(item.data(), true);
            }
        }
    }

    if (uuidsToRemove.count() > 0 && HmiSharedEventBus::instance().isInitialized()) {
        foreach (QString uuid, uuidsToRemove) {
            QByteArray byteArray;
            QDataStream out(&byteArray, QIODevice::WriteOnly);
            out << uuid;
            HmiSharedEventBus::instance().sendEvent(EventTypes::CaHMIConfigDeleted, byteArray);
        }
    }

    if (globalConfigItems.count() > 0) table->setRowCount(globalConfigItems.count());
    else table->setRowCount(1);

    int count = 0;
    for (auto it = globalConfigItems.constBegin(); it != globalConfigItems.constEnd(); ++it) {
        caHMIConfigTransferItem *item = it.key(); // The raw pointer to the item
        bool isExternal = it.value();

        QTableWidgetItem *name = new QTableWidgetItem(item->objectName());
        name->setFlags(name->flags() & ~Qt::ItemIsEditable);
        table->setItem(count, 0, name);

        QTableWidgetItem *trigger = Q_NULLPTR;
        if (item->captureType() == caHMIConfig::capType::KeyboardSet) {
            trigger = new QTableWidgetItem(item->shortcut().toString(QKeySequence::SequenceFormat::NativeText));
        } else if (item->captureType() == caHMIConfig::capType::KeyboardValue) {
            trigger = new QTableWidgetItem("ALL KEYS");
        } else if (item->captureType() == caHMIConfig::MousePress) {
            trigger = new QTableWidgetItem("Mouse Press");
        } else if (item->captureType() == caHMIConfig::capType::MouseMove) {
            trigger = new QTableWidgetItem("Mouse Move");
        }
        if (trigger != Q_NULLPTR) {
            trigger->setFlags(trigger->flags() & ~Qt::ItemIsEditable);
            table->setItem(count, 1, trigger);
        }

        QTableWidgetItem *fileName = new QTableWidgetItem(item->fileName());
        fileName->setFlags(fileName->flags() & ~Qt::ItemIsEditable);
        table->setItem(count, 2, fileName);

        QTableWidgetItem *process = new QTableWidgetItem(item->pid() == QApplication::applicationPid() ? "This (" + QString::number(item->pid()) + ")" : QString::number(item->pid()));
        process->setFlags(process->flags() & ~Qt::ItemIsEditable);
        table->setItem(count, 3, process);

        QWidget *wrapper = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(wrapper);
        QCheckBox *enabled = new QCheckBox(table);
        enabled->setChecked(item->enabled());

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
        connect(enabled, &QCheckBox::checkStateChanged, [item, isExternal, externalItemSharedPointersHolders](int state){
#else
        connect(enabled, &QCheckBox::stateChanged, [item, isExternal, externalItemSharedPointersHolders](int state){
#endif
            if (item == Q_NULLPTR) return;
            QString uuid;
            bool enabled = state == Qt::Checked;
            if (isExternal) {
                QWriteLocker locker(&externalHmiConfigListLock);
                uuid = item->uuid();
                item->setEnabled(enabled);
            } else {
                QWriteLocker locker(&hmiConfigListLock);
                uuid = item->uuid();
                item->setEnabled(enabled);
            }

            if (HmiSharedEventBus::instance().isInitialized()) {
                QByteArray data;
                QDataStream out(&data, QIODevice::WriteOnly);
                out << uuid << enabled;
                HmiSharedEventBus::instance().sendEvent(EventTypes::CaHMIConfigEnabledChanged, data);
            }
        });

        layout->addWidget(enabled, 0, Qt::AlignCenter);
        layout->setContentsMargins(0, 0, 0, 0);
        wrapper->setLayout(layout);

        table->setCellWidget(count, 4, wrapper);

        count++;
    }

    if (globalConfigItems.count() > 0) table->setRowCount(count);

    table->resizeColumnsToContents();

    int w = 0;
    count = table->columnCount();
    for (int i = 0; i < count; i++) w += table->columnWidth(i);
    int maxW = (w + count + table->verticalHeader()->width() + table->verticalScrollBar()->width());
    table->setMinimumWidth(maxW + 25);

    connect(this, &CaQtDM_Lib::Signal_Closing, shortcutWindow, &QDialog::close);

    shortcutWindow->showNormal();
    shortcutWindow->exec();
    if (shortcutWindow == Q_NULLPTR) return;
    shortcutWindow->close();
    shortcutWindow->deleteLater();
#endif
}

void CaQtDM_Lib::Callback_UndefinedMacrowindowExit(){
    macroWindow->close();
}

void CaQtDM_Lib::Callback_GlobalShortcutWindowExit(){
    shortcutWindow->close();
}

ControlsInterface * CaQtDM_Lib::getControlInterface(QString plugininterface)
{
    // find the plugin we are going to use
    if(!controlsInterfaces.isEmpty()) {
        QMapIterator<QString, ControlsInterface *> i(controlsInterfaces);
        while (i.hasNext()) {
            i.next();
            if(i.key() == plugininterface) {
                qCDebug(caQtDMLibLog) << "interface returned for requested" << plugininterface ;
                return  i.value();
            }
        }
    }
    return  (ControlsInterface *) Q_NULLPTR;
}

void CaQtDM_Lib::FlushAllInterfaces()
{
    // flush all plugins
    if(!controlsInterfaces.isEmpty()) {
        QMapIterator<QString, ControlsInterface *> i(controlsInterfaces);
        while (i.hasNext()) {
            i.next();
            ControlsInterface *plugininterface = i.value();
            if(plugininterface != (ControlsInterface *) Q_NULLPTR) plugininterface->FlushIO();
        }
    }
}

/**
 * routine to create an epics monitor
 */
int CaQtDM_Lib::addMonitor(QWidget *thisW, knobData *kData, QString pv, QWidget *w, int *specData, QMap<QString, QString> map, QString *pvRep)
{
    QMutex *mutex= Q_NULLPTR;
    bool doNothing = false;

    int indx;
    QString pluginName="";
    QString pluginFlavor="";
    ControlsInterface *plugininterface = (ControlsInterface *) 0;

    w->setProperty("Connect", false);
    int rate = DEFAULTRATE;  // default will be 5Hz

    if(pv.size() == 0) return -1;

    qCDebug(caQtDMLibLog) << "add monitor for " << pv << "soft=" << kData->soft;

    // replace macros
    QString trimmedPV = treatMacro(map, pv.trimmed(), &doNothing, w->objectName());

    // is there a json string ?
    int pos = trimmedPV.indexOf(".{");
    if((pos != -1) && trimmedPV.contains("caqtdm_monitor")) {
        int status;
        char asc[MAX_STRING_LENGTH];
        QString JSONString = trimmedPV.mid(pos+1);
        trimmedPV = trimmedPV.mid(0, pos);
        status = parseForDisplayRate(JSONString, rate);
        trimmedPV = trimmedPV + "." + JSONString;
        if(!status) {
            snprintf(asc, MAX_STRING_LENGTH, "JSON parsing error on %s ,should be like {\"caqtdm_monitor\":{\"maxdisplayrate\":10}}", (char*) qasc(pv.trimmed()));
        } else {
            snprintf(asc, MAX_STRING_LENGTH, "pv %s display rate set to maximum %dHz", qasc(trimmedPV), rate);
        }
        postMessage(QtDebugMsg, asc);
    }

    if (trimmedPV.contains(".{}")){
       trimmedPV.truncate(trimmedPV.indexOf(".{}"));
    } else if (trimmedPV.contains(".{")) {
        bool status;
        char asc[MAX_STRING_LENGTH];
        int pos = trimmedPV.indexOf(".{");
        QString JSONString = trimmedPV.mid(pos+1);
        trimmedPV = trimmedPV.mid(0, pos);
        status = checkJsonString(JSONString);
        if(!status) {
            snprintf(asc, MAX_STRING_LENGTH, "JSON parsing error on %s ,should be a better jsong string", (char*) qasc(pv.trimmed()));
        }
        trimmedPV = trimmedPV + "." + JSONString;
        if (trimmedPV.contains(".{}")) trimmedPV.truncate(trimmedPV.indexOf(".{}"));
    }

#ifndef VERSION_INT
#  define VERSION_INT(V,R,M,P) ( ((V)<<24) | ((R)<<16) | ((M)<<8) | (P))
#  define EPICS_VERSION_INT  VERSION_INT(EPICS_VERSION, EPICS_REVISION, EPICS_MODIFICATION, EPICS_PATCH_LEVEL)
#endif

#if defined(EPICS_VERSION_INT) && (EPICS_VERSION_INT >= VERSION_INT(3,15,0,0) || EPICS_VERSION_INT >= VERSION_INT(7,0,0,0))
        // do nothing
        qCDebug(caQtDMLibLog) << "for new epics use" << trimmedPV;
#else
        qCDebug(caQtDMLibLog) << "for old epics truncate" << trimmedPV;
        if (trimmedPV.contains(".{")) trimmedPV.truncate(trimmedPV.indexOf(".{"));
#endif

    *pvRep = trimmedPV;

    // find out what kind of interface has to be used for this pv, default is epics3 or whatever is specified on the command line with -cs

    // specified with the channel
    pos = trimmedPV.indexOf("://");
    if(pos != -1) {
        pluginName = trimmedPV.mid(0, pos);
        trimmedPV = trimmedPV.mid(pos+3);

        // take care of some specialties epics4 (one can specify epics4://, pva:// or ca://
        if(pluginName.contains("epics4")) pluginFlavor = "pva";  // default when epics4 is specified
        else if(pluginName.contains("ca")) {
            pluginName = "epics4";
            pluginFlavor = "ca";
        } else if(pluginName.contains("pva")) {
            pluginName = "epics4";
            pluginFlavor = "pva";
        }

    // not specified with the channel
    } else {

        // no default plugin specified on command line
        if(defaultPlugin.isEmpty()) {
#ifdef PVAISDEFAULTPROVIDER
           pluginName = "epics4";
           pluginFlavor = "ca";
#else
           pluginName = "epics3";
#endif
        // a default plugin is specied on the command line
        } else {
           pluginName = defaultPlugin;
           if(pluginName.contains("epics4")) pluginFlavor = "pva";  // default when epics4 is specified
        }
        if(kData->soft) pluginName = "intern";
        if(mutexKnobDataP->getSoftPV(trimmedPV, &indx, thisW)) pluginName = "intern";
    }

    *pvRep = trimmedPV;
    qstrncpy(kData->pluginName, (char*) qasc(pluginName),caqtdm_string_t_length);
    qstrncpy(kData->pluginFlavor, (char*) qasc(pluginFlavor),caqtdm_string_t_length);

    memset(&kData->pv,0,MAXPVLEN);
    qstrncpy(kData->pv, qasc(trimmedPV), MAXPVLEN-1);

    // find the plugin we are going to use ("intern" is the softPV pseudo plugin,
    // exact match so that the "internal" plugin is not swallowed here)
    if(pluginName != QLatin1String("intern")) {
        plugininterface = getControlInterface(pluginName);
        // and set it to the widget and the pointer to the data
        kData->pluginInterface = (void *) plugininterface;
        if(kData->pluginInterface == (void *) Q_NULLPTR) {
            char asc[MAX_STRING_LENGTH];
            snprintf(asc, MAX_STRING_LENGTH, "could not find a control plugin for %s with name %s\n", (char*) qasc(trimmedPV), (char*) qasc(pluginName.trimmed()));
            postMessage(QtCriticalMsg, asc);
        } else {
            qCDebug(caQtDMLibLog) << "control interface for" << trimmedPV << "plugin:" << pluginName;
        }
    }

    // for soft channels the rate has perhaps already been defined before
    QVariant oldRate = w->property("RATE");
    if (oldRate.isValid()) {
        rate = w->property("RATE").value<int>();
    }

    if(doNothing) {
        char asc[MAX_STRING_LENGTH];
        snprintf(asc, MAX_STRING_LENGTH, "Info: malformed pv '%s' (due to macro?)", (char*) qasc(trimmedPV));
        postMessage(QtWarningMsg, asc);
    }

    // when we defined already the same cacalc software channel, then do nothing
    if(mutexKnobDataP->getSoftPV(kData->pv, &indx, thisW)) {
        qCDebug(caQtDMLibLog) << "software channel already defined" << w;
        knobData *kPtr = mutexKnobDataP->GetMutexKnobDataPtr(indx);  // use pointer
        if(kPtr != (knobData*) Q_NULLPTR) {
            if(caCalc *calc = qobject_cast<caCalc *>(w)) {
                Q_UNUSED(calc);
                return kPtr->index;
            }
        }
    }

    // set pvname as tooltip
    QString tooltip;
    tooltip.append(ToolTipPrefix);
    tooltip.append(trimmedPV);

    if(!w->toolTip().isEmpty()) {
        tooltip.append(": ");
        tooltip.append(w->toolTip());
    }

    tooltip.append(ToolTipPostfix);
    if (!vncServer)
        w->setToolTip(tooltip);

    memcpy(kData->specData, specData, sizeof(int) * NBSPECS);
    kData->thisW = (void*) thisW;
    kData->dispW = (void*) w;

    mutex = new QMutex;
    kData->mutex = (void*)  mutex;

    // keep actual object name
    memset(&kData->dispName,0,MAXDISPLEN);
    qstrncpy(kData->dispName, qasc(w->objectName().toLower()), (size_t)  MAXDISPLEN-1);


    QString classname = w->metaObject()->className();
    memset(&kData->clasName,0,MAXDISPLEN);
    qstrncpy(kData->clasName, qasc(classname.toLower()), (size_t) MAXDISPLEN-1);


    // keep actual filename
    memset(&kData->fileName,0,MAXFILELEN);
    qstrncpy(kData->fileName, (char*) qasc(savedFile[level]), (size_t) MAXFILELEN-1);

    if (QLineEdit *lineedit = qobject_cast<QLineEdit *>(w)) {
        lineedit->setText("");
    }

    // get an index in the data list
    int num = mutexKnobDataP->GetMutexKnobDataIndex();
    if(num == -1) {
        qCCritical(caQtDMLibLog) << "this should never happen";
        return num;
    }

    // insert into the softpv list when we create a soft channel
    if(kData->soft) {
        qstrncpy(kData->pluginName, "intern",caqtdm_string_t_length);
        mutexKnobDataP->InsertSoftPV(kData->pv, num, thisW);
    }

    // did we use a new soft channel here, then set it
    if(mutexKnobDataP->getSoftPV(kData->pv, &indx, thisW)) kData->soft= true;

    // initialize channels
    qCDebug(caQtDMLibLog) << "pv" << kData->pv << "create channel index=" << num << indx << "soft=" << kData->soft << "plugin=" << kData->pluginName;

    kData->index = num;
    kData->edata.monitorCount = 0;
    kData->edata.displayCount = 0;
    kData->edata.precision = 0; //default
    kData->edata.units[0] = '\0';
    kData->edata.dataB =(void*) Q_NULLPTR;
    kData->edata.dataSize = 0;
    kData->edata.initialize = true;
    kData->edata.lastTimeMs = QDateTime::currentMSecsSinceEpoch();
    kData->edata.repRate = rate;   // default 5 Hz

    // update data structure
    mutexKnobDataP->SetMutexKnobData(num, *kData);

    // in case of a soft channel there is nothing to do
    if(kData->soft) {
        memset(kData, 0, sizeof (knobData));
        return num;
    }

    // define data acquisition
    if(plugininterface != (ControlsInterface *) Q_NULLPTR) plugininterface->pvAddMonitor(num, kData, rate, false);

    // add for this widget the io info
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QVariant v = qVariantFromValue(kData->edata.info);
#else
    QVariant v = QVariant::fromValue(kData->edata.info);
#endif
    QVariant var1=w->property("InfoList");
    QVariantList infoList1 = var1.toList();
    infoList1.append(v);
    w->setProperty("InfoList", infoList1);

 #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QVariant plugin = qVariantFromValue(kData->pluginInterface);
#else
    QVariant plugin = QVariant::fromValue(kData->pluginInterface);
#endif
    QVariant var2=w->property("Interface");
    QVariantList infoList2 = var2.toList();
    infoList2.append(plugin);
    w->setProperty("Interface", infoList2);

    // clear data
    memset(kData, 0, sizeof (knobData));

    return num;
}

/**
  * treat visibility of our objects
  */
int CaQtDM_Lib::setObjectVisibility(QWidget *w, double value)
{
    bool visible = true;
    bool valid = false;
    double result;

    if(caFrame *frameWidget = qobject_cast<caFrame *>(w)) {
        // treat visibility if defined
        ComputeVisibility(caFrame, frameWidget)
        if(frameWidget->getVisibilityMode() == caFrame::Background) {
            if(visible) frameWidget->setAutoFillBackground(true);
            else frameWidget->setAutoFillBackground(false);
            return visible;
        }
    } else if (caInclude *includeWidget = qobject_cast<caInclude *>(w)) {
        // treat visibility if defined
        ComputeVisibility(caInclude, includeWidget)
    } else if (caGraphics *graphicsWidget = qobject_cast<caGraphics *>(w)) {
        // treat visibility if defined
        ComputeVisibility(caGraphics, graphicsWidget)
    } else if(caImage *imageWidget = qobject_cast<caImage *>(w)) {
        // treat visibility if defined
        ComputeVisibility(caImage, imageWidget)
    } else if(caPolyLine *polyLineWidget = qobject_cast<caPolyLine *>(w)) {
        // treat visibility if defined
        ComputeVisibility(caPolyLine, polyLineWidget)
    } else if(caLabel *labelWidget = qobject_cast<caLabel *>(w)) {
        // treat visibility if defined
        ComputeVisibility(caLabel,labelWidget)
    } else if(caLabelVertical *labelverticalWidget = qobject_cast<caLabelVertical *>(w)) {
        // treat visibility if defined
        ComputeVisibility(caLabelVertical,labelverticalWidget)
    }

    if(!visible) {
        if(caPolyLine *polylineWidget = qobject_cast<caPolyLine *>(w)) polylineWidget->setHide(true);
        else if(caGraphics *graphicsWidget = qobject_cast<caGraphics *>(w)) graphicsWidget->setHide(true);
        else w->hide();
    } else {
        if(caPolyLine *polylineWidget = qobject_cast<caPolyLine *>(w)) polylineWidget->setHide(false);
        else if(caGraphics *graphicsWidget = qobject_cast<caGraphics *>(w)) graphicsWidget->setHide(false);
        else w->show();
    }
    return visible;
}

void CaQtDM_Lib::setCalcToNothing(QWidget* w) {
    if(caFrame *frame = qobject_cast<caFrame *>(w)) {
        frame->setVisibilityCalc("");
    }
    else if(caInclude *include = qobject_cast<caInclude *>(w)) {
        include->setVisibilityCalc("");
    }
    else if(caImage *image = qobject_cast<caImage *>(w)) {
        image->setVisibilityCalc("");
    }
    else if(caGraphics *graphic = qobject_cast<caGraphics *>(w)) {
        graphic->setVisibilityCalc("");
    }
    else if(caPolyLine *line = qobject_cast<caPolyLine *>(w)) {
        line->setVisibilityCalc("");
    }
    else if(caLabel *label = qobject_cast<caLabel *>(w)) {
        label->setVisibilityCalc("");
    }
    else if(caLabelVertical *labelvertical = qobject_cast<caLabelVertical *>(w)) {
        labelvertical->setVisibilityCalc("");
    }
    else if(caCalc *calc = qobject_cast<caCalc *>(w)) {
        calc->setCalc("");
    }
}

bool CaQtDM_Lib::Python_Error(QWidget *w, QString message)
{
#ifdef PYTHON

    PyObject *errObj = NULL, *errData = NULL, *errTraceback = NULL, *pystring = NULL;
    char errorType[1024], errorInfo[1024], asc[MAX_STRING_LENGTH];
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
    const char *errorStr = 0;
#else
    char *errorStr = 0;
#endif
    // get latest python exception info
    PyErr_Fetch(&errObj, &errData, &errTraceback);

    pystring = PyObject_Str(errObj);
#if PY_MAJOR_VERSION >= 3
    bool pystringcheck = PyUnicode_Check(pystring);
    errorStr = PyUnicode_AsUTF8 (pystring);
#else
    bool pystringcheck = PyString_Check(pystring);
    errorStr = PyString_AsString(pystring);
#endif
    if ((errObj != NULL) && (pystring != NULL) && (pystringcheck)) {
       strcpy(errorType, errorStr);
    } else {
       strcpy(errorType, "<unknown exception type>");
    }
    Py_XDECREF(pystring);

    pystring = PyObject_Str(errData);
#if PY_MAJOR_VERSION >= 3
    pystringcheck = PyUnicode_Check(pystring);
    errorStr = PyUnicode_AsUTF8 (pystring);
#else
    pystringcheck = PyString_Check(pystring);
    errorStr = PyString_AsString(pystring);
#endif
    if ((errData != NULL) && (pystring != NULL) && (pystringcheck)) {
       strcpy(errorInfo, errorStr);
    } else {
       strcpy(errorInfo, "<unknown exception data>");
    }
    Py_XDECREF(pystring);

    snprintf(asc, MAX_STRING_LENGTH, "%s %s : %s %s", qPrintable(message), qasc(w->objectName()), errorType, errorInfo);

    Py_XDECREF(errObj);
    Py_XDECREF(errData);
    Py_XDECREF(errTraceback);

    postMessage(QtWarningMsg, asc);
    setCalcToNothing(w);
    Py_Finalize();
#else
    Q_UNUSED(w);
    Q_UNUSED(message);
#endif
    return true;
}

/**
  * routine used by the above routine for calculating the visibilty of our objects
  * and generates animation parameter
  */
bool CaQtDM_Lib::CalcVisibility(QWidget *w, double &result, bool &valid)
{

    double valueArray[MAX_CALC_INPUTS];
    char post[calcstring_length];
    char calcString[calcstring_length];
    long status;
    short errnum;
    bool visible = true;
    QString calcQString = "";

    if(caFrame *frame = qobject_cast<caFrame *>(w)) {
        calcQString = frame->getVisibilityCalc();
    }
    else if(caInclude *include = qobject_cast<caInclude *>(w)) {
        calcQString = include->getVisibilityCalc();
    }
    else if(caImage *image = qobject_cast<caImage *>(w)) {
        calcQString = image->getVisibilityCalc();
    }
    else if(caGraphics *graphic = qobject_cast<caGraphics *>(w)) {
        calcQString = graphic->getVisibilityCalc();
    }
    else if(caPolyLine *line = qobject_cast<caPolyLine *>(w)) {
        calcQString = line->getVisibilityCalc();
    }
    else if(caLabel *label = qobject_cast<caLabel *>(w)) {
        calcQString = label->getVisibilityCalc();
    }
    else if(caLabelVertical *labelvertical = qobject_cast<caLabelVertical *>(w)) {
        calcQString = labelvertical->getVisibilityCalc();
    }
    else if(caCalc *calc = qobject_cast<caCalc *>(w)) {
        calcQString = calc->getCalc();
    }else if(caHMIConfig *cahmiconfig = qobject_cast<caHMIConfig *>(w)) {
        calcQString = cahmiconfig->value();
    }

    // no calc
    if(calcQString.length() < 1) {
        valid = true;
        return true;
    }

    calcQString=calcQString.trimmed();
    qstrncpy(calcString, qasc(calcQString),calcstring_length);

    // any monitors ?
    QVariant monitorList=w->property("MonitorList");
    QVariantList MonitorList = monitorList.toList();
    QVariant indexList=w->property("IndexList");
    QVariantList IndexList = indexList.toList();

    if(MonitorList.size() == 0) return true;

    int nbMonitors = MonitorList.at(0).toInt();
    qCDebug(caQtDMLibLog) << "number of monitors" << nbMonitors << "calc=" << calcString;
    if(nbMonitors > 0)  {

        setlocale(LC_NUMERIC, "C");

        // Regexp will used when is marked with %/regexp/
     QString pattern="%\\/(\\S+)\\/";
     QString captured_Calc="";
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QRegExp checkregexp(pattern);
        checkregexp.setMinimal(true);
        int pos=checkregexp.indexIn(calcString);
        if (pos != -1){
            captured_Calc = checkregexp.cap(1);
        }
#else
        // Achtung setminimal der alten Implementierung ist noch nicht getested!!!!
        // erster Test ok aber noch das eine oder andere Fragezeichen!
        QRegularExpression checkregexp(pattern);
        QRegularExpressionMatch match = checkregexp.match(calcString);
        qsizetype pos=match.capturedStart();
        qCDebug(caQtDMLibLog) << "Regex calcString:" << calcString;
        if (match.hasMatch()){
            captured_Calc = match.captured(1);
            qCDebug(caQtDMLibLog) << "Regex captured_Calc:" << captured_Calc;
        }
#endif

        if (pos != -1){
            knobData *ptr = mutexKnobDataP->GetMutexKnobDataPtr(MonitorList.at(1).toInt());
            if(ptr != (knobData *) Q_NULLPTR) {
                char dataString[STRING_EXCHANGE_SIZE];
                int caFieldType= ptr->edata.fieldtype;

                if((caFieldType == caSTRING || caFieldType == caENUM || caFieldType == caCHAR) && ptr->edata.dataB != (void*) Q_NULLPTR) {
                    if(ptr->edata.dataSize < STRING_EXCHANGE_SIZE) {
                        memcpy(dataString, (char*) ptr->edata.dataB, (size_t) ptr->edata.dataSize);
                        dataString[ptr->edata.dataSize] = '\0';

                        // in case of enum we have to get the right string from the value
                        if(caFieldType == caENUM) {
                            QString String(dataString);
                            QStringList list;
                            //list = String.split(";");
                            list = String.split((QChar)27);
                            if((ptr->edata.fieldtype == caENUM)  && ((int) ptr->edata.ivalue < list.count() ) && (list.count() > 0)) {
                                if(list.at((int) ptr->edata.ivalue).trimmed().size() != 0)  {  // string seems to empty, give value
                                    QString strng = list.at((int) ptr->edata.ivalue);
                                    QByteArray ba = strng.toLatin1();
                                    qstrncpy(dataString, ba.data(),STRING_EXCHANGE_SIZE);
                                }
                            }
                        }

                    } else  {
                        char asc[MAX_STRING_LENGTH];
                        snprintf(asc, MAX_STRING_LENGTH, "Invalid channel data type %s", qasc(w->objectName()));
                        postMessage(QtDebugMsg, asc);
                        valid = false;
                        return true;
                    }
                }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                QRegExp check_A(captured_Calc);
                check_A.setMinimal(false);
                //check_A.indexIn(dataString);
                qCDebug(caQtDMLibLog) << "captured_Calc" <<captured_Calc << dataString << check_A.capturedTexts() << check_A.isValid() << check_A.exactMatch(dataString);
                if (check_A.exactMatch(dataString)){
#else
                QRegularExpression check_A(captured_Calc);
                // Ignore warning "Don't create temporary qRegularExpression objects...", when using a static expression the code doesn't work.
                if (check_A.match(dataString).hasMatch()){
#endif
                    result=1;
                    valid = true;
                    return true;
                } else{
                    result=0;
                    valid = true;
                    return false;
                }
            }

            // special function used for animation purposes through cacalc
        } else if(calcQString.startsWith("%QRect")) {
            if(caCalc *calc = qobject_cast<caCalc *>(w)) {
                qCDebug(caCalcLog) << "qrect for cacalc detected";
                for(int i=0; i<4; i++) valueArray[i] = -1;  //say default value will not do anything
                for(int i=0; i<nbMonitors;i++) {
                    knobData *ptr = mutexKnobDataP->GetMutexKnobDataPtr(MonitorList.at(i+1).toInt());
                    if(ptr != (knobData*) Q_NULLPTR) {
                        qCDebug(caCalcLog) << "calculate from index" << i << ptr->index << ptr->pv << ptr->edata.connected << ptr->edata.rvalue << IndexList.at(i+1).toInt();
                        // when connected
                        int j = IndexList.at(i+1).toInt(); // input a,b,c,d
                        if(ptr->edata.connected) {
                            switch (ptr->edata.fieldtype){
                            case caINT:
                            case caLONG:{
                                valueArray[j] = ptr->edata.ivalue;
                                break;
                            }
                            default:{
                                valueArray[j] = ptr->edata.rvalue;
                            }
                            }
                        } else {
                            valueArray[j] = -1;
                        }
                    }
                }

                bool somethingToSend = false;
                for(int i=0; i<4; i++) {
                    if(valueArray[i] != -1) {
                        somethingToSend = true;
                        break;
                    }
                }
                QString qrectscan=calc->getCalc();
                qrectscan=qrectscan.right(qrectscan.length()-6);
                if (!qrectscan.isEmpty()){
                    parseForQRectConst(qrectscan,valueArray);
                }

                if(somethingToSend) {
                    if (calc->getTextLine()!="%QRect"){
                      calc->setTextLine("%QRect");
                    }
                    valid = false;
                    QRect rect(valueArray[0], valueArray[1], valueArray[2], valueArray[3]);
                    calc->setValue(rect);
                    result = 1;
                    return true;
                }
            }
            valid = false;
            result = 0;
            return true;

#ifdef PYTHON
            // python function
        } else if(calcQString.startsWith("%P/")) {

            Py_Initialize();
#define MAXMONITORS 4
            PyObject *pArgs, *pValue, *pValueA[MAX_CALC_INPUTS], *pFunc;
            PyObject *pGlobal = PyDict_New();
            PyObject *pLocal;

            for(int i=0; i < MAXMONITORS; i++) valueArray[i] = 0.0;
            for(int i=0; i< nbMonitors;i++) {
                knobData *ptr = mutexKnobDataP->GetMutexKnobDataPtr(MonitorList.at(i+1).toInt());
                if(ptr == (knobData*) Q_NULLPTR) {
                    valid = false;
                    return false;
                }
            }

            PyDict_SetItemString( pGlobal, "__builtins__", PyEval_GetBuiltins() );

            // get rid of %P/ and last / on new line
            calcQString = calcQString.mid(3, calcQString.length()-4);

            char newCalc[2048];
            strcpy(newCalc, qasc(calcQString));

            //Create a new module object
            QString myModule("myModule"+w->objectName());
            PyObject *pNewMod = PyModule_New((char*) qasc(myModule));

            PyModule_AddStringConstant(pNewMod, "__file__", "");

            //Get the dictionary object from my module
            pLocal = PyModule_GetDict(pNewMod);

            //Define my function in the newly created module, when error then we get a null pointer back
            pValue = PyRun_String(qasc(calcQString), Py_file_input, pGlobal, pLocal);
            if(pValue == (PyObject *) Q_NULLPTR) {
                valid = false;
                Py_DECREF(pNewMod);
                return Python_Error(w, "probably a syntax error on the python function (calc will be disabled)");
            }
            Py_DECREF(pValue);

            //Get a pointer to the function I just defined
            pFunc = PyObject_GetAttrString(pNewMod, "PythonCalc");
            if((pFunc == (PyObject *) Q_NULLPTR) || (!PyCallable_Check(pFunc))) {
                valid = false;
                Py_DECREF(pNewMod);
                return Python_Error(w, "python function not found, must be called PythonCalc (calc will be disabled)");
            }

            //Build a tuple to hold my arguments (just the number 4 in this case)
            pArgs = PyTuple_New(MAXMONITORS);
            for(int i=0; i< MAXMONITORS; i++) pValueA[i] = PyFloat_FromDouble(0.0);
            for(int i=0; i< nbMonitors; i++) {
                knobData *ptr = mutexKnobDataP->GetMutexKnobDataPtr(MonitorList.at(i+1).toInt());
                if(ptr != (knobData*) Q_NULLPTR) {
                    // when connected
                    int j = IndexList.at(i+1).toInt(); // input a,b,c,d
                    if(ptr->edata.connected) {
                        valueArray[j] = ptr->edata.rvalue;
                    } else {
                        valueArray[j] = 0.0;
                    }
                    pValueA[i] = PyFloat_FromDouble(valueArray[i]);
                }
            }
            for(int i=0; i < MAXMONITORS; i++)  PyTuple_SetItem(pArgs, i, pValueA[i]);

            //Call my function, passing it the number four
            pValue = PyObject_CallObject(pFunc, pArgs);
            if (pValue != (PyObject *) Q_NULLPTR) {
                result = PyFloat_AsDouble(pValue);
                Py_DECREF(pValue);
                Py_DECREF(pArgs);
                Py_XDECREF(pFunc);
                Py_DECREF(pNewMod);
                valid = true;
            } else {
                result = 0.0;
                valid = false;
                Py_DECREF(pArgs);
                Py_XDECREF(pFunc);
                Py_DECREF(pNewMod);
                return Python_Error(w, "some error in the python function (calc will be disabled)");
            }

            return visible;
#else
        } else if(calcQString.startsWith("%P/")) {
            char asc[MAX_STRING_LENGTH];
            snprintf(asc, MAX_STRING_LENGTH, "python is not enabled in this caqtdm version(calc will be disabled) %s", qasc(w->objectName()));
            postMessage(QtWarningMsg, asc);
            setCalcToNothing(w);
            valid = false;
            return visible;
#endif
            //normal EPICS Calculation
        } else {

            // scan and get the channels
            for(int i=0; i < MAX_CALC_INPUTS; i++) valueArray[i] = 0.0;
            for(int i=0; i< nbMonitors;i++) {
                knobData *ptr = mutexKnobDataP->GetMutexKnobDataPtr(MonitorList.at(i+1).toInt());
                if(ptr != (knobData*) Q_NULLPTR) {
                    qCDebug(caQtDMLibLog) << "calculate from index" << i << ptr->index << ptr->pv << ptr->edata.connected << ptr->edata.rvalue << ptr->edata.ivalue << IndexList.at(i+1).toInt();
                    // when connected
                    int j = IndexList.at(i+1).toInt(); // input a,b,c,d
                    if(ptr->edata.connected) {
                        switch (ptr->edata.fieldtype){
                            case caINT:
                            case caLONG:
                            case caENUM: {
                                valueArray[j] = ptr->edata.ivalue;
                                break;
                            }
                            default:{
                                valueArray[j] = ptr->edata.rvalue;
                            }
                        }
                    } else {
                        valueArray[j] = 0.0;
                    }
                    // for first record
                    if(i==0 && ptr->edata.connected) {
                        valueArray[4] = 0.0;                                 /* E: Reserved */
                        valueArray[5] = 0.0;                                 /* F: Reserved */
                        valueArray[6] = ptr->edata.valueCount;               /* G: count */
                        valueArray[7] = ptr->edata.upper_disp_limit;         /* H: hopr */
                        valueArray[8] = ptr->edata.status;                   /* I: status */
                        valueArray[9] = ptr->edata.severity;                 /* J: severity */
                        valueArray[10] = ptr->edata.precision;               /* K: precision */
                        valueArray[11] = ptr->edata.lower_disp_limit;        /* L: lopr */
                    }
                }
            }
            // calculate
            status = postfix(calcString, post, &errnum);
            if(status) {
                char asc[MAX_STRING_LENGTH];
                snprintf(asc, MAX_STRING_LENGTH, "Invalid Calc %s for %s (calc will be disabled)", calcString, qasc(w->objectName()));
                setCalcToNothing(w);
                postMessage(QtDebugMsg, asc);
                valid = false;
                return true;
            }
            // Perform the calculation
            status = calcPerform(valueArray, &result, post);
            if(!status) {
                visible = (result?true:false);
                qCDebug(caQtDMLibLog) << "valid result" << result << visible;
                valid = true;
                return visible;
            } else {
                char asc[MAX_STRING_LENGTH];
                snprintf(asc, MAX_STRING_LENGTH, "invalid calc %s for %s (calc will be disabled)", calcString, qasc(w->objectName()));
                setCalcToNothing(w);
                postMessage(QtDebugMsg, asc);
                valid = false;
                return true;
            }
        }
    }
    valid = false;
    return true;
}

/**
  * computes alarm state from the channels
  */
short CaQtDM_Lib::ComputeAlarm(QWidget *w)
{
    short status;
    // any monitors ?
    QVariant var=w->property("MonitorList");
    QVariantList list = var.toList();
    int nbMonitors = list.at(0).toInt();
    qCDebug(caQtDMLibLog) << "number of monitors" << nbMonitors;
    status = NO_ALARM;
    if(nbMonitors > 0)  {

        // medm uses however only first channel
        knobData *ptr = mutexKnobDataP->GetMutexKnobDataPtr(list.at(1).toInt());
        if(ptr != (knobData *) Q_NULLPTR) {
            // when connected
            if(ptr->edata.connected) {
                status = status | ptr->edata.severity;
            } else {
                return NOTCONNECTED;
            }
        }
    }
    return status;
}

/**
  * update of linear and circular widget (they have common properties)
  */

void CaQtDM_Lib::UpdateGauge(EAbstractGauge *widget, const knobData &data)
{
    if(data.edata.connected) {
        double maxval = 1000.0;
        double minval = 0.0;

        if((widget->getDisplayLimits() == EAbstractGauge::Channel_Limits) && (data.edata.initialize)) {
            // HOPR and LOPR
            if(data.edata.upper_disp_limit == data.edata.lower_disp_limit) {
                // set some default
                widget->setMaxValue(1000.0);
                widget->setMinValue(0.0);
                maxval = 1000.0;
                minval = 0.0;
            } else {
                if(!qIsNaN(data.edata.lower_disp_limit)) {
                    widget->setMinValue(data.edata.lower_disp_limit);
                    minval = data.edata.lower_disp_limit;
                } else {
                    widget->setMinValue(0.0);
                    minval = 0.0;
                }
                if(!qIsNaN(data.edata.upper_disp_limit)) {
                    widget->setMaxValue(data.edata.upper_disp_limit);
                    maxval = data.edata.upper_disp_limit;
                } else {
                    widget->setMaxValue(1000.0);
                    maxval = 1000;
                }
            }

            widget->update();
        } else {
            maxval = widget->maxValue();
            minval = widget->minValue();
        }

        if((widget->getAlarmLimits() == EAbstractGauge::Channel_Alarms) && (data.edata.initialize)) {
            // when no alarms defined
            if(data.edata.lower_alarm_limit == data.edata.upper_alarm_limit) {
                // set limits to extreme values
                widget->setLowError(minval);
                widget->setHighError(maxval);
            } else {
                if(!qIsNaN(data.edata.lower_alarm_limit)) widget->setLowError(data.edata.lower_alarm_limit); else  widget->setLowError(minval);
                if(!qIsNaN(data.edata.upper_alarm_limit)) widget->setHighError(data.edata.upper_alarm_limit); else widget->setHighError(maxval);
            }

            if(data.edata.lower_warning_limit == data.edata.upper_warning_limit) {
                // set limits to extreme values
                widget->setLowWarning(minval);
                widget->setHighWarning(maxval);
            } else {
                if(!qIsNaN(data.edata.lower_warning_limit)) widget->setLowWarning(data.edata.lower_warning_limit); else  widget->setLowWarning(minval);
                if(!qIsNaN(data.edata.upper_warning_limit)) widget->setHighWarning(data.edata.upper_warning_limit); else  widget->setHighWarning(maxval);
            }
        } else if((widget->getAlarmLimits() == EAbstractGauge::None) && (data.edata.initialize)) {

            maxval = widget->maxValue();
            minval = widget->minValue();

            widget->setLowError(minval);
            widget->setHighError(maxval);
            widget->setLowWarning(minval);
            widget->setHighWarning(maxval);
        }

        widget->setValue(data.edata.rvalue);
        widget->update();

    }
}

void CaQtDM_Lib::UpdateMeter(caMeter *widget, const knobData &data)
{
    if(data.edata.connected) {

        if((widget->getLimitsMode() == caMeter::Channel) && (data.edata.initialize)) {
            // HOPR and LOPR
            if(data.edata.upper_disp_limit == data.edata.lower_disp_limit) {
                // set some default
                widget->setMaxValue(1000.0);
                widget->setMinValue(0.0);
            } else {
                if(!qIsNaN(data.edata.lower_disp_limit)) {
                    widget->setMinValue(data.edata.lower_disp_limit);
                } else {
                    widget->setMinValue(0.0);
                }
                if(!qIsNaN(data.edata.upper_disp_limit)) {
                    widget->setMaxValue(data.edata.upper_disp_limit);
                } else {
                    widget->setMaxValue(1000.0);
                }
            }

            widget->update();
        }

        int precMode = widget->getPrecisionMode();
        if((precMode != caMeter::User) && (data.edata.initialize)) {
            widget->setPrecision(data.edata.precision);
            widget->updateMeter();
        }
        widget->setValueUnits(data.edata.rvalue, data.edata.units);
    }

}

/**
 * updates my widgets through monitor and emit signal
 */
void CaQtDM_Lib::Callback_UpdateWidget(int indx, QWidget *w,
                                       const QString& units,
                                       const QString& fec,
                                       const QString& String,
                                       const knobData& data)
{
    Q_UNUSED(indx);
    Q_UNUSED(fec);

    if(!AllowsUpdate) return;

    // thread mutexknobdata emits to all instances of this class, later we will have to filter on the emit side to enhance performance
    bool thisInstance = false;
    QWidget *widget = w;
    if(w == (QWidget*) Q_NULLPTR) return;
    while (widget->parentWidget()) {
        widget = widget->parentWidget() ;
        if(widget == myWidget) {
            thisInstance = true;
            break;
        }
    }
    if(!thisInstance) {
        return;
    }

    // any caWidget with caWidgetInterface
    if (caWidgetInterface* wif = dynamic_cast<caWidgetInterface *>(w)) {
        wif->caDataUpdate(units, String, data);
    }

    // calc ==================================================================================================================
    else if(caCalc *calcWidget = qobject_cast<caCalc *>(w)) {
        bool valid;
        double result;
        switch (data.edata.fieldtype){
            case caINT:
            case caLONG:
            case caENUM: {
                result = data.edata.ivalue;
                break;
            }
            default:{
                result = data.edata.rvalue;
            }
        }
        qCDebug(caCalcLog) << "we have a caCalc" << calcWidget->getVariable() << "  " <<  data.pv << "fieldtype" << data.edata.fieldtype;

        if(calcWidget->getDataCount() > 0) {
            if(!data.edata.connected) result = qQNaN();
            mutexKnobDataP->UpdateSoftPV(calcWidget->getVariable(), result, myWidget, data.specData[0], calcWidget->getDataCount());
        } else {
            CalcVisibility(w, result, valid);  // visibility not used, but calculation yes
            if(valid) {
                if (!QString::compare(calcWidget->getVariable(), data.pv, Qt::CaseSensitive)){
                    calcWidget->setValue(result);
                    mutexKnobDataP->UpdateSoftPV(data.pv, result, myWidget, 0, 1);
                    qCDebug(caCalcLog) << "we have a caCalc" << calcWidget->getVariable() << "  " <<  data.pv;

                    // be sure to update softchannels in the soft waves

                    //QList<caCalc *> all = myWidget->findChildren<caCalc *>();
                    if (allCalcs_Vectors.count()>0){
                        foreach(caCalc* w, allCalcs_Vectors) {
                            // when cacalc is a waveform composed from individual channels
                            // get items of this waveform
                            QList<QString> pvList = w->getPVList();
                            for (int j = 0; j < pvList.size(); j++) {
                                if (!QString::compare(calcWidget->getVariable(), pvList[j], Qt::CaseSensitive)) {
                                    mutexKnobDataP->UpdateSoftPV(w->getVariable(), result, myWidget, j, pvList.count());
                                }
                            }
                        }
                    }
                }
            }
        }

        // caLabel ==================================================================================================================
    } else if(caLabel *labelWidget = qobject_cast<caLabel *>(w)) {
        qCDebug(caLabelLog) << "we have a label";

        if(data.edata.connected) {
            int colorMode = labelWidget->getColorMode();
            if(colorMode == caLabel::Static) {
                // done at initialisation, we have to set it back after no connect
                if(!labelWidget->property("Connect").value<bool>()) {
                    QColor fg = labelWidget->property("FColor").value<QColor>();
                    QColor bg = labelWidget->property("BColor").value<QColor>();
                    labelWidget->setForeground(fg);
                    labelWidget->setBackground(bg);
                    labelWidget->setProperty("Connect", true);
                }
            } else if(colorMode == caLabel::Alarm) {
                short status = ComputeAlarm(w);
                labelWidget->setAlarmColors(status);
            }

            setObjectVisibility(labelWidget, data.edata.rvalue);

        } else {
            SetColorsNotConnected(labelWidget);
        }

        // caLabelVertical ==================================================================================================================
    } else if(caLabelVertical *labelverticalWidget = qobject_cast<caLabelVertical *>(w)) {
        qCDebug(caLabelVerticalLog) << "we have a label";

        if(data.edata.connected) {
            int colorMode = labelverticalWidget->getColorMode();
            if(colorMode == caLabelVertical::Static) {
                // done at initialisation, we have to set it back after no connect
                if(!labelverticalWidget->property("Connect").value<bool>()) {
                    QColor fg = labelverticalWidget->property("FColor").value<QColor>();
                    QColor bg = labelverticalWidget->property("BColor").value<QColor>();
                    labelverticalWidget->setForeground(fg);
                    labelverticalWidget->setBackground(bg);
                    labelverticalWidget->setProperty("Connect", true);
                }
            } else if(colorMode == caLabelVertical::Alarm) {
                short status = ComputeAlarm(w);
                labelverticalWidget->setAlarmColors(status);
            }

            setObjectVisibility(labelverticalWidget, data.edata.rvalue);

        } else {
            SetColorsNotConnected(labelverticalWidget);
        }

        // caInclude ==================================================================================================================
    } else if(caInclude *includeWidget = qobject_cast<caInclude *>(w)) {
        qCDebug(caIncludeLog) << "we have an include";

        // visibility
        if(data.specData[0] == 0) {
            setObjectVisibility(includeWidget, data.edata.rvalue);

        // absolute positioning
        } else {
            QWidget *dispW;
            int posx, posy;
            int posX=0;
            int posY=0;
            double factX = 1.0;
            double factY = 1.0;
            QString pos;

            QVariant resizeX = this->property("RESIZEX");
            QVariant resizeY = this->property("RESIZEY");
            if (resizeX.isValid()) factX = this->property("RESIZEX").value<double>();
            if (resizeY.isValid()) factY = this->property("RESIZEY").value<double>();

            qCDebug(caIncludeLog) << "we got a position at " << data.specData[1] << data.edata.rvalue << data.pv;
            int adjustMargin = data.specData[2];
            if(data.specData[0] == 1) {
                includeWidget->updateXpositionsList(data.specData[1], (int) data.edata.rvalue);
            } else {
                includeWidget->updateYpositionsList(data.specData[1], (int) data.edata.rvalue);
            }
            if(includeWidget->getXposition(data.specData[1], posx, 0, pos)) {posX = posx;}
            if(includeWidget->getYposition(data.specData[1], posy, 0, pos)) {posY = posy;}
            memcpy(&dispW, &data.specData[3], sizeof(QWidget*));

            // move to correct position
            int xpos = qRound((double) posX * includeWidget->getXcorrection() * factX);
            int ypos = qRound((double) posY * includeWidget->getYcorrection() * factY);

            dispW->move(xpos + adjustMargin/2*factX, ypos + adjustMargin/2*factY);
            if (includeWidget->getStacking() == caInclude::Positions){
               includeWidget->update_position(dispW,xpos + adjustMargin/2,ypos + adjustMargin/2);
            }
            // recalculate eventually the size
            if(includeWidget->getAdjustSize()) {
                int maximumX = includeWidget->getXmaximum() + dispW->width();
                int maximumY = includeWidget->getYmaximum() + dispW->height();

                //includeWidget->resize((maximumX + adjustMargin) * factX, (maximumY + adjustMargin) * factY);
                foreach(QWidget* l ,includeWidget->findChildren<QWidget *>()){
                   QRect resizedata=l->childrenRect();
                   if ((resizedata.width()>0) && (resizedata.height()>0)){
                       QRect sizedata=l->rect();
                       if ((sizedata.height()*sizedata.width())<(resizedata.height()*resizedata.width())) {
                             l->resize(resizedata.width(),resizedata.height());
                       }

                   }
                }

                //QRect resizedata=includeWidget->childrenRect();
                //includeWidget->resize(resizedata.width(),resizedata.height());
                //includeWidget->getIncludeFrame()->resize(resizedata.width(),resizedata.height());
                //qDebug()<< "includeWidget->resize(1)"<<maximumX<<adjustMargin<<factX<<resizedata.width()<<resizedata.height();

                if (includeWidget->getStacking() == caInclude::Positions){
//                    QList<QVariant> integerList;
//                    integerList.insert(0, includeWidget->geometry().x());
//                    integerList.insert(1, includeWidget->geometry().y());
//                    integerList.insert(2, maximumX);
//                    integerList.insert(3, maximumY);
                    //includeWidget->setProperty("GeometryList", integerList);
                    //qDebug()<< "includeWidget->resize(2)"<<maximumX <<maximumY;
                    QRect resizedata=includeWidget->childrenRect();
                    QRect sizedata=includeWidget->rect();
                    if ((sizedata.height()*sizedata.width())<(resizedata.height()*resizedata.width())) {
                        qCDebug(caIncludeLog) << "resizedata to:" << resizedata;
                        includeWidget->resize(resizedata.width(),resizedata.height());
                        includeWidget->update_geometrysave();
                    }
                    qCDebug(caIncludeLog) << "sizedata" << sizedata << "resizedata" << resizedata << includeWidget;



                    //includeWidget->update_geometrysave();
                }
                includeWidget->updateGeometry();
                // when the include is packed into a scroll area, set the scrollbars too
                ResizeScrollBars(includeWidget, factX * (maximumX + adjustMargin), factY * (maximumY + adjustMargin));
            }
        }

        // caFrame ==================================================================================================================
    } else if(caFrame *frameWidget = qobject_cast<caFrame *>(w)) {
        qCDebug(caFrameLog) << "we have a frame";

        setObjectVisibility(frameWidget, data.edata.rvalue);

        // caMenu ==================================================================================================================
    } else if (caMenu *menuWidget = qobject_cast<caMenu *>(w)) {
        qCDebug(caMenuLog) << "we have a menu" << data.pv << data.edata.connected << data.specData[0];

        if(data.edata.connected) {
            // set enum strings
            if((data.edata.fieldtype == caENUM) && (data.specData[0] == 0)) {
                QStringList stringlist = String.split((QChar)27);
                menuWidget->populateCells(stringlist);
                if(menuWidget->getLabelDisplay()) menuWidget->setCurrentIndex(0);
                else {
                    menuWidget->setIndex((int) data.edata.ivalue);
                }

            // when mask specified, use it
            } else if(data.specData[0] == 1) {
                switch (data.edata.fieldtype){
                    case caINT:
                    case caLONG:{
                        menuWidget->setMaskValue(data.edata.ivalue);
                        break;
                    }
                    default:{
                        menuWidget->setMaskValue(data.edata.rvalue);
                    }
                }
            }
            if (menuWidget->getColorMode() == caMenu::Alarm) {
                menuWidget->setAlarmColors(data.edata.severity);
                qCDebug(caMenuLog) << "caMenu severity" << data.edata.severity;
            } else {
                SetColorsBack(menuWidget);
            }

        } else {
            SetColorsNotConnected(menuWidget);
        }
        menuWidget->setAccessW(data.edata.accessW);
        updateAccessCursor(menuWidget);

        // caChoice ==================================================================================================================
    } else if (caChoice *choiceWidget = qobject_cast<caChoice *>(w)) {
        qCDebug(caChoiceLog) << "we have a choiceButton" << String << (int) data.edata.ivalue << choiceWidget;

        if(data.edata.connected) {
            QStringList stringlist = String.split((QChar)27);
            // set enum strings
            if(data.edata.fieldtype == caENUM) {
                // at initialisatioon or when list changes
                if((data.edata.initialize) || (stringlist != choiceWidget->getList())) {
                    choiceWidget->populateCells(stringlist, (int) data.edata.ivalue);
                    // otherwise just set value
                } else {
                    choiceWidget->setValue((int) data.edata.ivalue);
                }
                if (choiceWidget->getColorMode() == caChoice::Alarm) {
                    choiceWidget->setAlarmColors(data.edata.severity);
                    // case of static mode
                } else {
                    SetColorsBack(choiceWidget);
                }
            }
        } else {
            SetColorsNotConnected(choiceWidget);
        }
        choiceWidget->setAccessW(data.edata.accessW);
        updateAccessCursor(choiceWidget);

        // caThermo ==================================================================================================================
    } else if (caThermo *thermoWidget = qobject_cast<caThermo *>(w)) {
        qCDebug(caThermoLog) << "we have a thermometer";

        if(data.edata.connected) {
            bool channelLimitsEnabled = false;
            if(thermoWidget->getLimitsMode() == caThermo::Channel) channelLimitsEnabled= true;
            // take limits from channel, in case of user limits these should already been set
            if((channelLimitsEnabled) && (data.edata.initialize) ) {
                // when limits are the same, do nothing
                if(data.edata.upper_disp_limit != data.edata.lower_disp_limit) {
                    if(thermoWidget->getDirection() == caThermo::Down  || thermoWidget->getDirection() == caThermo::Left) {
                        thermoWidget->setMinValue(data.edata.upper_disp_limit);
                        thermoWidget->setMaxValue(data.edata.lower_disp_limit);
                    } else {
                        thermoWidget->setMaxValue(data.edata.upper_disp_limit);
                        thermoWidget->setMinValue(data.edata.lower_disp_limit);
                    }
                }
            }
            thermoWidget->setValue(data.edata.rvalue);

            // set colors when connected
            // case of alarm mode
            if ((thermoWidget->getColorMode() == caThermo::Alarm_Default) || (thermoWidget->getColorMode() == caThermo::Alarm_Static)) {
                if(channelLimitsEnabled) {
                    thermoWidget->setAlarmColors(data.edata.severity);
                } else {
                    thermoWidget->setUserAlarmColors(data.edata.rvalue);
                }

                // case of static mode
            } else {
                SetColorsBack(thermoWidget);
            }
            int precMode = thermoWidget->getPrecisionMode();
            if((precMode != caThermo::User) && (data.edata.initialize)) thermoWidget->setPrecision(data.edata.precision);

            // set no connection color
        } else {
            SetColorsNotConnected(thermoWidget);
        }

        // caSlider ==================================================================================================================
    } else if (caSlider *sliderWidget = qobject_cast<caSlider *>(w)) {

        if(data.edata.connected) {
            bool highChannelLimitEnabled = false;
            bool lowChannelLimitEnabled = false;
            bool channelLimitsEnabled = false;
            if(sliderWidget->getHighLimitMode() == caSlider::Channel) highChannelLimitEnabled= true;
            if(sliderWidget->getLowLimitMode()  == caSlider::Channel) lowChannelLimitEnabled= true;
            if(highChannelLimitEnabled && lowChannelLimitEnabled) channelLimitsEnabled = true;

            // take limits from channel, in case of user limits these should already been set
            if((highChannelLimitEnabled || lowChannelLimitEnabled) && data.edata.initialize ) {
                // when limits are the same, do nothing
                if(data.edata.upper_disp_limit != data.edata.lower_disp_limit) {
                    disconnect(w, SIGNAL(valueChanged (double)), 0, 0);
                    if((highChannelLimitEnabled)) sliderWidget->setMaxValue(data.edata.upper_disp_limit);
                    if((lowChannelLimitEnabled))  sliderWidget->setMinValue(data.edata.lower_disp_limit);
                    connect(w, SIGNAL(valueChanged(double)), this, SLOT(Callback_SliderValueChanged(double)));
                }
            }

            sliderWidget->setCtrlMaximum(data.edata.upper_ctrl_limit);
            sliderWidget->setCtrlMinimum(data.edata.lower_ctrl_limit);

            // disconnect signal to prevent from firing again
            disconnect(w, SIGNAL(valueChanged (double)), 0, 0);
            sliderWidget->setSliderValue(data.edata.rvalue);
            // reconnect signal
            connect(w, SIGNAL(valueChanged(double)), this, SLOT(Callback_SliderValueChanged(double)));

            sliderWidget->setAccessW((bool) data.edata.accessW);
            updateAccessCursor(sliderWidget);

            // set colors when connected
            // case of alarm mode
            if ((sliderWidget->getColorMode() == caSlider::Alarm_Default) || (sliderWidget->getColorMode() == caSlider::Alarm_Static)) {
                if(channelLimitsEnabled) {
                    sliderWidget->setAlarmColors(data.edata.severity);
                } else {
                    sliderWidget->setUserAlarmColors(data.edata.rvalue);
                }

                // case of static, default mode
            } else {
                SetColorsBack(sliderWidget);
            }

            int precMode = sliderWidget->getPrecisionMode();
            if((precMode != caSlider::User) && (data.edata.initialize)) sliderWidget->setPrecision(data.edata.precision);

            // set no connection color
        } else {
            SetColorsNotConnected(sliderWidget);
        }

        // caClock ==================================================================================================================
    } else if (caClock *clockWidget = qobject_cast<caClock *>(w)) {
        if(data.edata.connected) {
            if(clockWidget->getTimeType() == caClock::ReceiveTime) {
                clockWidget->setAlarmColors(data.edata.severity);
                QDateTime dattim = QDateTime::fromMSecsSinceEpoch(data.edata.actTimeMs);
                clockWidget->updateClock(dattim.time());
            }
            // set no connection color
        } else {
            SetColorsNotConnected(clockWidget);
        }

        // linear gauge (like thermometer) ==================================================================================================================
    } else if (caLinearGauge *lineargaugeWidget = qobject_cast<caLinearGauge *>(w)) {
        qCDebug(caLinearGaugeLog) << "we have a linear gauge";
        Q_UNUSED(lineargaugeWidget);
        EAbstractGauge *gauge =  qobject_cast<EAbstractGauge *>(w);
        if(data.edata.connected) {
            if(!gauge->isConnected()) gauge->setConnected(true);
            UpdateGauge(gauge, data);
        } else {
            if(gauge->isConnected()) gauge->setConnected(false);
        }

        // circular gauge  ==================================================================================================================
    } else if (caCircularGauge *circulargaugeWidget = qobject_cast<caCircularGauge *>(w)) {
        qCDebug(caCircularGaugeLog) << "we have a linear gauge";
        Q_UNUSED(circulargaugeWidget);
        EAbstractGauge *gauge =  qobject_cast<EAbstractGauge *>(w);
        if(data.edata.connected) {
            if(!gauge->isConnected()) gauge->setConnected(true);
            UpdateGauge(gauge, data);
        } else {
            if(gauge->isConnected()) gauge->setConnected(false);
        }

        // simple meter ==================================================================================================================
    } else if (caMeter *meterWidget = qobject_cast<caMeter *>(w)) {
        qCDebug(caMeterLog) << "we have a simple meter";

        if(data.edata.connected) {
            UpdateMeter(meterWidget, data);
            // set colors when connected
            // case of alarm mode
            if (meterWidget->getColorMode() == caMeter::Alarm) {
                meterWidget->setAlarmColors(data.edata.severity);
                // case of static mode
            } else {
                SetColorsBack(meterWidget);
            }
        } else {
            SetColorsNotConnected(meterWidget);
        }

        // byte ==================================================================================================================
    } else if (caByte *byteWidget = qobject_cast<caByte *>(w)) {

        if(data.edata.connected) {
            int colorMode = byteWidget->getColorMode();
            if(colorMode == caByte::Static) {
                if(!byteWidget->property("Connect").value<bool>()) {
                    byteWidget->setProperty("Connect", true);
                }
                byteWidget->setValue(data.edata.ivalue);
            } else if(colorMode == caByte::Alarm) {
                byteWidget->setValue(data.edata.ivalue);
                byteWidget->setAlarmColors(data.edata.severity);
            }
        } else {
            SetColorsNotConnected(byteWidget);
        }

        // byte ==================================================================================================================
    } else if (caByteController *bytecontrollerWidget = qobject_cast<caByteController *>(w)) {

        if(data.edata.connected) {
            int colorMode = bytecontrollerWidget->getColorMode();
            if(colorMode == caByteController::Static) {
                if(!bytecontrollerWidget->property("Connect").value<bool>()) {
                    bytecontrollerWidget->setProperty("Connect", true);
                }
                bytecontrollerWidget->setValue(data.edata.ivalue);
            } else if(colorMode == caByteController::Alarm) {
                bytecontrollerWidget->setValue(data.edata.ivalue);
                bytecontrollerWidget->setAlarmColors(data.edata.severity);
            }
        } else {
            SetColorsNotConnected(bytecontrollerWidget);
        }

       // replacemacro ==================================================================================================================
     } else if(replaceMacro* replaceMacroWidget = qobject_cast<replaceMacro *>(w)) {

        if(data.edata.connected) {
            QStringList stringlist = String.split((QChar)27);
            // set enum strings
            if(data.edata.fieldtype == caENUM) {
                // at initialisation or when list changes
                if((data.edata.initialize) || (stringlist != replaceMacroWidget->getValueList())) {
                    replaceMacroWidget->updateValueList(stringlist);
                }
                SetColorsBack(replaceMacroWidget);
            }
        } else {
            SetColorsNotConnected(replaceMacroWidget);
        }

        // lineEdit and textEntry ====================================================================================================
    } else if (caLineEdit *lineeditWidget = qobject_cast<caLineEdit *>(w)) {

        qCDebug(caLineEditLog) << "we have a linedit or textentry" << lineeditWidget << data.edata.rvalue <<  data.edata.ivalue;

        QColor bg = lineeditWidget->property("BColor").value<QColor>();
        QColor fg = lineeditWidget->property("FColor").value<QColor>();

        // units crap
        if(data.edata.connected && (data.specData[0] == 1)) {
            lineeditWidget->appendUnits(String);
        }

        if(data.edata.connected && (data.specData[0] == 0)) {
            lineeditWidget->setDatatype(data.edata.fieldtype);
            // enum string
            if(data.edata.fieldtype == caENUM || data.edata.fieldtype == caSTRING || data.edata.fieldtype == caCHAR) {
                QStringList list;
                lineeditWidget->setValueType(false);
                int colorMode = lineeditWidget->getColorMode();
                if(colorMode == caLineEdit::Static || colorMode == caLineEdit::Default) { // done at initialisation
                    if(!lineeditWidget->property("Connect").value<bool>()) {                    // but was disconnected before
                        lineeditWidget->setAlarmColors(data.edata.severity, (double) data.edata.ivalue, bg, fg);
                        lineeditWidget->setProperty("Connect", true);
                    }
                } else {
                    lineeditWidget->setAlarmColors(data.edata.severity, (double) data.edata.ivalue, bg, fg);
                }
                list = String.split((QChar)27);

                qCDebug(caLineEditLog) << lineeditWidget << String << list << data.pv << (int) data.edata.ivalue << data.edata.valueCount;

                if((data.edata.fieldtype == caENUM)  && (list.count() == 0)) {
                    QString str= QString::number((int) data.edata.ivalue);
                    lineeditWidget->setTextLine(str);                              // no string, but number
                } else if((data.edata.fieldtype == caENUM)  && ((int) data.edata.ivalue < list.count() ) && (list.count() > 0)) {
                    if(list.at((int) data.edata.ivalue).trimmed().size() == 0)  {  // string seems to empty, give value
                        QString str= QString::number((int) data.edata.ivalue);
                        lineeditWidget->setTextLine(str);
                    } else {                                                       // we have a string, we want as string
                        if(lineeditWidget->getFormatType() != caLineEdit::enumeric) {
                           lineeditWidget->setTextLine(list.at((int) data.edata.ivalue));
                        } else {
                            QString str= QString::number((int) data.edata.ivalue); // we have  astring, we want as number
                            lineeditWidget->setTextLine(str);
                        }
                    }
                } else if((data.edata.fieldtype == caENUM)  && ((int) data.edata.ivalue >= list.count()) && (list.count() > 0)) {
                    QString str= QString::number((int) data.edata.ivalue);
                    lineeditWidget->setTextLine(str);
                } else if (data.edata.fieldtype == caENUM) {
                    lineeditWidget->setTextLine("???");

                // just one char (display as character when string format is specified, otherwise as number in specified format
                } else if((data.edata.fieldtype == caCHAR) && (data.edata.nelm == 1)) {
                    if(lineeditWidget->getFormatType() == caLineEdit::string) {
                       QString str = QString(QChar((int) data.edata.ivalue));
                       lineeditWidget->setTextLine(str);
                    } else {
                        lineeditWidget->setValue(data.edata.ivalue, "");
                    }

                // one or more strings, or a char array
                } else {
                    if(data.edata.nelm == 1) {
                        lineeditWidget->setTextLine(String);
                    } else if(list.count() > 0) {
                        lineeditWidget->setTextLine(list.at(0));
                    }
                }

                // access control for textentry
                if (caTextEntry *textentryWidget = qobject_cast<caTextEntry *>(w)) {
                    textentryWidget->setAccessW((bool) data.edata.accessW);
                    updateAccessCursor(textentryWidget);
                }

                // double
            } else {
                int colorMode = lineeditWidget->getColorMode();
                int precMode = lineeditWidget->getPrecisionMode();
                int limitsMode = lineeditWidget->getLimitsMode();
                lineeditWidget->setValueType(true);

                if(colorMode == caLineEdit::Static || colorMode == caLineEdit::Default) { // done at initialisation
                    if(!lineeditWidget->property("Connect").value<bool>()) {                      // but was disconnected before
                        switch (data.edata.fieldtype){
                            case caINT:
                            case caLONG:{
                                lineeditWidget->setAlarmColors(data.edata.severity, data.edata.ivalue, bg, fg);
                                break;
                            }
                            default:{
                                lineeditWidget->setAlarmColors(data.edata.severity, data.edata.rvalue, bg, fg);
                            }
                        }
                        lineeditWidget->setProperty("Connect", true);
                    }
                } else {
                    switch (data.edata.fieldtype){
                        case caINT:
                        case caLONG:{
                            lineeditWidget->setAlarmColors(data.edata.severity, data.edata.ivalue, bg, fg);
                            break;
                        }
                        default:{
                            lineeditWidget->setAlarmColors(data.edata.severity, data.edata.rvalue, bg, fg);
                        }
                    }
                }

                if(limitsMode == caLineEdit::Channel) {
                    //set the limits when they are equal the correction will disable them
                        lineeditWidget->setMaxValue(data.edata.upper_disp_limit);
                        lineeditWidget->setMinValue(data.edata.lower_disp_limit);
                }

                if((precMode != caLineEdit::User) && (data.edata.initialize)) {
                    lineeditWidget->setFormat(data.edata.precision);
                }
                switch (data.edata.fieldtype){
                    case caINT:
                    case caLONG:{
                        lineeditWidget->setValue(data.edata.ivalue, units);
                        break;
                    }
                    default:{
                        lineeditWidget->setValue(data.edata.rvalue, units);
                    }
                }

                // access control for textentry
                if (caTextEntry *textentryWidget = qobject_cast<caTextEntry *>(w)) {
                    textentryWidget->setAccessW((bool)data.edata.accessW);
                    updateAccessCursor(textentryWidget);
                }
            }

        } else if(data.specData[0] == 0) {
            lineeditWidget->setValueType(false);
            lineeditWidget->setTextLine("");
            lineeditWidget->setAlarmColors(NOTCONNECTED, 0.0, bg, fg);        \
            lineeditWidget->setProperty("Connect", false);
        }

        if(data.edata.connected) {
            if (caTextEntry *textentryWidget = qobject_cast<caTextEntry *>(w)) {
                textentryWidget->updateText(lineeditWidget->text());
            }
        }

        // multilinestring ====================================================================================================
    } else if (caMultiLineString *multilinestringWidget = qobject_cast<caMultiLineString *>(w)) {

        qCDebug(caMultiLineStringLog) << "we have a multilinedit" << multilinestringWidget << data.edata.rvalue <<  data.edata.ivalue;

        QColor bg = multilinestringWidget->property("BColor").value<QColor>();
        QColor fg = multilinestringWidget->property("FColor").value<QColor>();

        if(data.edata.connected) {
            QString separator((QChar)27);
            QString CR((QChar)13);

            // enum string
            if(data.edata.fieldtype == caENUM || data.edata.fieldtype == caSTRING || data.edata.fieldtype == caCHAR) {

                int colorMode = multilinestringWidget->getColorMode();
                if(colorMode == caMultiLineString::Static || colorMode == caMultiLineString::Default) { // done at initialisation
                    if(!multilinestringWidget->property("Connect").value<bool>()) {                    // but was disconnected before
                        multilinestringWidget->setAlarmColors(data.edata.severity, (double) data.edata.ivalue, bg, fg);
                        multilinestringWidget->setProperty("Connect", true);
                    }
                } else {
                    multilinestringWidget->setAlarmColors(data.edata.severity, (double) data.edata.ivalue, bg, fg);
                }

                qCDebug(caMultiLineStringLog) << multilinestringWidget << String << data.pv << (int) data.edata.ivalue << data.edata.valueCount;

                // an enum
                if(data.edata.fieldtype == caENUM) {
                    QString enumList = String;
                    enumList.replace(separator, CR);
                    multilinestringWidget->setTextLine(enumList);

                // just one char
                } else if((data.edata.fieldtype == caCHAR) && (data.edata.nelm == 1)) {
                    QString str = QString(QChar((int) data.edata.ivalue));
                    multilinestringWidget->setTextLine(str);

                // one or more strings, or a char array
                } else if(data.edata.fieldtype == caCHAR){
                    multilinestringWidget->setTextLine(String);
                } else if(data.edata.fieldtype == caSTRING) {
                    QString List = String;
                    List.replace(separator, CR);
                    multilinestringWidget->setTextLine(List);
                }
            } else {
                multilinestringWidget->setTextLine("not supported");
            }

        } else {
            multilinestringWidget->setTextLine("");
            multilinestringWidget->setAlarmColors(NOTCONNECTED, 0.0, bg, fg);        \
            multilinestringWidget->setProperty("Connect", false);
        }

        // Graphics ==================================================================================================================
    } else if (caGraphics *graphicsWidget = qobject_cast<caGraphics *>(w)) {
        qCDebug(caGraphicsLog) << "caGraphics" << graphicsWidget->objectName() << graphicsWidget->getColorMode() << data.pv;

        if(data.edata.connected) {
            int colorMode = graphicsWidget->getColorMode();
            if(colorMode == caGraphics::Static) {
                // done at initialisation, we have to set it back after no connect
                if(!graphicsWidget->property("Connect").value<bool>()) {
                    QColor fg = graphicsWidget->property("FColor").value<QColor>();
                    QColor lg = graphicsWidget->property("LColor").value<QColor>();
                    graphicsWidget->setForeground(fg);
                    graphicsWidget->setLineColor(lg);
                    graphicsWidget->setProperty("Connect", true);
                }
            } else if(colorMode == caGraphics::Alarm) {
                short status = ComputeAlarm(w);
                graphicsWidget->setAlarmColors(status);
            }

            setObjectVisibility(graphicsWidget, data.edata.rvalue);

        } else {
            SetColorsNotConnected(graphicsWidget);
        }

        // Polyline ==================================================================================================================
    } else if (caPolyLine *polylineWidget = qobject_cast<caPolyLine *>(w)) {

        if(data.edata.connected) {
            int colorMode = polylineWidget->getColorMode();
            if(colorMode == caPolyLine::Static) {
                // done at initialisation, we have to set it back after no connect
                if(!polylineWidget->property("Connect").value<bool>()) {
                    QColor fg = polylineWidget->property("FColor").value<QColor>();
                    QColor lg = polylineWidget->property("LColor").value<QColor>();
                    polylineWidget->setForeground(fg);
                    polylineWidget->setLineColor(lg);
                    polylineWidget->setProperty("Connect", true);
                }
            } else if(colorMode == caPolyLine::Alarm) {
                short status = ComputeAlarm(w);
                polylineWidget->setAlarmColors(status);
            }

            setObjectVisibility(polylineWidget, data.edata.rvalue);

        } else {
            SetColorsNotConnected(polylineWidget);
        }

        // Led ==================================================================================================================
    } else if (caLed *ledWidget = qobject_cast<caLed *>(w)) {
        qCDebug(caLedLog) << "led" << ledWidget->objectName();
        Qt::CheckState state = Qt::Unchecked;

        if(data.edata.connected) {
            int colorMode = ledWidget->getColorMode();
            if(colorMode == caLed::Static) {
                getStatesToggleAndLed(ledWidget, data, String, state);
                ledWidget->setState(state);
            } else {
                ledWidget->setAlarmColors(data.edata.severity);
            }
        } else {
            ledWidget->setAlarmColors(NOTCONNECTED);
        }

        // ApplyNumeric and Numeric =====================================================================================================
    } else if (caApplyNumeric *applynumericWidget = qobject_cast<caApplyNumeric *>(w)) {
        qCDebug(caApplyNumericLog) << "caApplyNumeric" << applynumericWidget->objectName() << data.pv << data.edata.monitorCount;

        if(data.edata.connected) {
            ComputeNumericMaxMinPrec(applynumericWidget, data);

            int colorMode = applynumericWidget->getColorMode();
            if(colorMode == caApplyNumeric::Static || colorMode == caApplyNumeric::Default) {
                applynumericWidget->setConnectedColors(true);
            } else {
                applynumericWidget->setAlarmColors(data.edata.severity);
            }

            if(data.edata.fieldtype == caSTRING ||  data.edata.fieldtype ==  caCHAR) {
                char asc[MAX_STRING_LENGTH];
                snprintf(asc, MAX_STRING_LENGTH, "caApplyNumeric %s does not treat other then numerical values", qasc(w->objectName()));
                postMessage(QtCriticalMsg, asc);
                applynumericWidget->setEnabled(false);
            } else {
                applynumericWidget->silentSetValue(data.edata.rvalue);
                applynumericWidget->setAccessW((bool) data.edata.accessW);
                updateAccessCursor(applynumericWidget);
            }

        } else {
            applynumericWidget->setConnectedColors(false);
        }

        // Numeric =====================================================================================================
    } else if (caNumeric *numericWidget = qobject_cast<caNumeric *>(w)) {
        qCDebug(caNumericLog) << "caNumeric" << numericWidget->objectName() << data.pv;

        if(data.edata.connected) {
            ComputeNumericMaxMinPrec(numericWidget, data);
            numericWidget->setConnectedColors(true);

            int colorMode = numericWidget->getColorMode();
            if(colorMode == caNumeric::Static || colorMode == caNumeric::Default) {
                numericWidget->setConnectedColors(true);
            } else {
                numericWidget->setAlarmColors(data.edata.severity);
            }

            if(data.edata.fieldtype == caSTRING ||  data.edata.fieldtype ==  caCHAR) {
                char asc[MAX_STRING_LENGTH];
                snprintf(asc, MAX_STRING_LENGTH, "caNumeric %s does not treat other then numerical values", qasc(w->objectName()));
                postMessageAndLog(QtCriticalMsg, asc, caQtDMLibLog);
                numericWidget->setEnabled(false);
            } else {
                numericWidget->silentSetValue(data.edata.rvalue);
                numericWidget->setAccessW((bool) data.edata.accessW);
                updateAccessCursor(numericWidget);
            }

        } else {
            numericWidget->setConnectedColors(false);
        }

        // Numeric =====================================================================================================
    } else if (caSpinbox *spinboxWidget = qobject_cast<caSpinbox *>(w)) {
        qCDebug(caSpinboxLog) << "caSpinbox" << spinboxWidget->objectName() << data.pv;

        if(data.edata.connected) {
            ComputeNumericMaxMinPrec(spinboxWidget, data);
            int colorMode = spinboxWidget->getColorMode();
            if(colorMode == caSpinbox::Static || colorMode == caSpinbox::Default) {
                spinboxWidget->setConnectedColors(true);
            } else {
                spinboxWidget->setAlarmColors(data.edata.severity);
            }

            if(data.edata.fieldtype == caSTRING || data.edata.fieldtype ==  caCHAR) {
                char asc[MAX_STRING_LENGTH];
                snprintf(asc, MAX_STRING_LENGTH, "caSpinbox %s does not treat other then numerical values", qasc(w->objectName()));
                postMessageAndLog(QtCriticalMsg, asc, caQtDMLibLog);
                spinboxWidget->setEnabled(false);
            } else {
                spinboxWidget->silentSetValue(data.edata.rvalue);
                spinboxWidget->setAccessW((bool) data.edata.accessW);
                updateAccessCursor(spinboxWidget);
            }

        } else {
            spinboxWidget->setConnectedColors(false);
        }

        // Toggle =====================================================================================================
    } else if (caToggleButton *togglebuttonWidget = qobject_cast<caToggleButton *>(w)) {
        qCDebug(caToggleButtonLog) << "caToggleButton" << togglebuttonWidget->objectName() << data.pv;
        Qt::CheckState state = Qt::Unchecked;

        if(data.edata.connected) {
            int colorMode = togglebuttonWidget->getColorMode();
            if(colorMode == caToggleButton::Alarm) {
                togglebuttonWidget->setAlarmColors(data.edata.severity);
            } else {
                SetColorsBack(togglebuttonWidget);
            }

            getStatesToggleAndLed(togglebuttonWidget, data, String, state);
            togglebuttonWidget->setState(state);
            togglebuttonWidget->setAccessW((bool) data.edata.accessW);
            updateAccessCursor(togglebuttonWidget);
        } else {
            SetColorsNotConnected(togglebuttonWidget);
        }

        // cartesian plot ==================================================================================================================
    } else if (caCartesianPlot *cartesianplotWidget = qobject_cast<caCartesianPlot *>(w)) {
        qCDebug(caCartesianPlotLog) << "caCartesianPlot" << cartesianplotWidget->objectName() << data.pv << data.specData[0] << data.specData[1]  << data.specData[2];

        int curvNB = data.specData[0];    // curve or scale number
        int curvType = data.specData[1];  // Xonly, Yonly, XY_both;
        int XorY = data.specData[2];      // X=0; Y=1, Trigger=2, Count=3, Erase=4, Scale=5/6;

        // when scaling is by channel, limits have to be different, otherwise auto will be set
        // when scaling is user, then limits were already set and will not change
        // when scaling is auto, then nothing will be done
        // when scaling is given by pv's, the limits will be given by these

        if(data.edata.connected) {
            // scale first time on first curve
            if(data.edata.initialize && curvNB == 0 && XorY <= caCartesianPlot::CH_Y) {
                if(XorY == caCartesianPlot::CH_X && cartesianplotWidget->getXscaling() == caCartesianPlot::Channel) {
                    if(data.edata.lower_disp_limit != data.edata.upper_disp_limit) {
                        cartesianplotWidget->setScaleX(data.edata.lower_disp_limit, data.edata.upper_disp_limit);
                    } else {
                        char asc[MAX_STRING_LENGTH];
                        snprintf(asc, MAX_STRING_LENGTH, "PV <%s> (x axis) in widget <%s> is set to channel scaling, but the channel limits are invalid. Therefore, the x axis scaling for the widget is reset to auto.", data.pv, qasc(w->objectName()));
                        postMessageAndLog(QtWarningMsg, asc, caCartesianPlotLog);
                        cartesianplotWidget->setXscaling(caCartesianPlot::Auto);
                    }
                } else if(XorY == caCartesianPlot::CH_Y && cartesianplotWidget->getYscaling() == caCartesianPlot::Channel) {
                    if(data.edata.lower_disp_limit != data.edata.upper_disp_limit) {
                        cartesianplotWidget->setScaleY(data.edata.lower_disp_limit, data.edata.upper_disp_limit);
                    } else {
                        char asc[MAX_STRING_LENGTH];
                        snprintf(asc, MAX_STRING_LENGTH, "PV <%s> (y axis) in widget <%s> is set to channel scaling, but the channel limits are invalid. Therefore, the y axis scaling for the widget is reset to auto.", data.pv, qasc(w->objectName()));
                        postMessageAndLog(QtWarningMsg, asc, caCartesianPlotLog);
                        cartesianplotWidget->setYscaling(caCartesianPlot::Auto);
                    }
                }
            }

            // monitor for scale will change the above set limits
            if((XorY == caCartesianPlot::CH_Xscale) || (XorY == caCartesianPlot::CH_Yscale)) {
                if(XorY == caCartesianPlot::CH_Xscale) {
                    cartesianplotWidget->setScaleXlimits(data.edata.rvalue, curvNB);
                } else {
                    cartesianplotWidget->setScaleYlimits(data.edata.rvalue, curvNB);
                }
                qCDebug(caCartesianPlotLog) << "scale monitor from" << data.pv << data.edata.rvalue << "min/max" << data.specData[0] << "scale" << XorY;
            }

            if(!cartesianplotWidget->property("Connect").value<bool>()) {
                cartesianplotWidget->setProperty("Connect", true);
                cartesianplotWidget->setAllProperties();
            }

            // value(s)
            if(XorY == caCartesianPlot::CH_X || XorY == caCartesianPlot::CH_Y) {
                // data from vector
                if(data.edata.valueCount > 0 && data.edata.dataB != (void*) Q_NULLPTR) {
                    Cartesian(cartesianplotWidget, curvNB, curvType, XorY, data);
                    // data from value
                } else {
                    double p = data.edata.rvalue;
                    cartesianplotWidget->setData(&p, 1, curvNB, curvType, XorY);
                    cartesianplotWidget->displayData(curvNB, curvType);
                }

                // trigger channel
            } else if(XorY == caCartesianPlot::CH_Trigger) {
                for(int i=0; i < caCartesianPlot::curveCount; i++) {
                    double p;
                    cartesianplotWidget->setData(&p, 1, i, curvType, XorY);
                    cartesianplotWidget->displayData(i, curvType);
                }

                // count channel
            } else if(XorY == caCartesianPlot::CH_Count) {
                qCDebug(caCartesianPlotLog) << "count channel" << data.edata.rvalue << qRound(data.edata.rvalue);
                if(data.edata.rvalue >= 0.5) cartesianplotWidget->setCountNumber(qRound(data.edata.rvalue));

                // erase channel
            } else if(XorY == caCartesianPlot::CH_Erase) {
                if(cartesianplotWidget->getEraseMode() == caCartesianPlot::ifnotzero) {
                    if((int) data.edata.rvalue != 0) cartesianplotWidget->erasePlots();
                } else if(cartesianplotWidget->getEraseMode() == caCartesianPlot::ifzero){
                    if((int) data.edata.rvalue == 0) cartesianplotWidget->erasePlots();
                }
            }

            // not connected
        } else if(strstr(data.pluginName, "archive") == (char*) Q_NULLPTR) {
            cartesianplotWidget->setCountNumber(0);
            cartesianplotWidget->setWhiteColors();
            cartesianplotWidget->setProperty("Connect", false);
        }

        // waterfall plot ==================================================================================================================
    } else if (caWaterfallPlot *waterfallplotWidget = qobject_cast<caWaterfallPlot *>(w)) {
        qCDebug(caWaterfallPlotLog) << "caWaterfallPlot" << waterfallplotWidget->objectName() << data.pv;

        int pvType = data.specData[0];      // waveform=0; Count=1
        int countRequested = data.specData[1];

        if(data.edata.connected) {

            if(pvType == 1) {
                qCDebug(caWaterfallPlotLog) << "count channel" << data.edata.rvalue << qRound(data.edata.rvalue);
                if(data.edata.rvalue >= 0.5) waterfallplotWidget->setCountNumber(qRound(data.edata.rvalue));
                waterfallplotWidget->setCountReceived(true);
            } else if(waterfallplotWidget->getCountReceived() || !countRequested) {

                // scale first time on first curve
                if(data.edata.initialize) {

                    double min = waterfallplotWidget->getIntensityMin();
                    double max = waterfallplotWidget->getIntensityMax();

                    if((waterfallplotWidget->getIntensityScalingMin() == caWaterfallPlot::Channel) && (waterfallplotWidget->getIntensityScalingMax() == caWaterfallPlot::Channel)) {
                        qCDebug(caWaterfallPlotLog) << "channels" <<  data.edata.lower_disp_limit << data.edata.upper_disp_limit;
                        if(data.edata.lower_disp_limit < data.edata.upper_disp_limit) {
                            waterfallplotWidget->setIntensityMin(data.edata.lower_disp_limit);
                            waterfallplotWidget->setIntensityMax(data.edata.upper_disp_limit);
                        } else {
                            waterfallplotWidget->setIntensityMin(0.0);
                            waterfallplotWidget->setIntensityMax(10.0);
                        }
                    } else if(waterfallplotWidget->getIntensityScalingMin() == caWaterfallPlot::Channel) {
                        qCDebug(caWaterfallPlotLog) << "channel1" ;
                        if(data.edata.lower_disp_limit < max) {
                            waterfallplotWidget->setIntensityMin(data.edata.lower_disp_limit);
                        }
                    }  else if(waterfallplotWidget->getIntensityScalingMax() == caWaterfallPlot::Channel) {
                        qCDebug(caWaterfallPlotLog) << "channel2" ;
                        if(data.edata.upper_disp_limit > min) {
                            waterfallplotWidget->setIntensityMax(data.edata.upper_disp_limit);
                        }
                    }
                    waterfallplotWidget->InitData(data.edata.valueCount);
                }

                // value(s)
                if((data.edata.valueCount > 0) && (data.edata.dataB != (void*) Q_NULLPTR)) {
                    WaterFall(waterfallplotWidget, data);
                } else {
                    double p = data.edata.rvalue;
                    waterfallplotWidget->setData(&p, 1);
                    waterfallplotWidget->displayData();
                }
            }

            // not connected
        } else {

        }

        // stripchart ==================================================================================================================
    } else if(caStripPlot *stripplotWidget = qobject_cast<caStripPlot *>(w)) {

        int actPlot= data.specData[1];
        if(data.edata.connected) {

            // scaling
            if(data.edata.initialize) {
                if(stripplotWidget->getYscalingMin(actPlot) == caStripPlot::Channel) {
                    stripplotWidget->setYaxisLimitsMin(actPlot, data.edata.lower_disp_limit);
                }
                if(stripplotWidget->getYscalingMax(actPlot) == caStripPlot::Channel) {
                    stripplotWidget->setYaxisLimitsMax(actPlot, data.edata.upper_disp_limit);
                }
                if(stripplotWidget->getYaxisLimitsMin(actPlot) == stripplotWidget->getYaxisLimitsMax(actPlot)) {
                    stripplotWidget->setYaxisLimitsMin(actPlot, 0.0);
                    stripplotWidget->setYaxisLimitsMax(actPlot, 10.0);
                }
                // do this for redisplaying legend with correct limits
                stripplotWidget->resize(stripplotWidget->geometry().width()+1, stripplotWidget->geometry().height());
                stripplotWidget->resize(stripplotWidget->geometry().width()-1, stripplotWidget->geometry().height());
            }


            switch (data.edata.fieldtype){
                case caINT:
                case caLONG:{
                    stripplotWidget->setData(data.edata.actTimeMs, data.edata.ivalue, actPlot);
                    break;
                }
                default:{
                    stripplotWidget->setData(data.edata.actTimeMs, data.edata.rvalue, actPlot);
                }
            }

            if(data.edata.initialize) {
                stripplotWidget->startPlot();
            }

        }

        // animated gif ==================================================================================================================
    } else if(caImage *imageWidget = qobject_cast<caImage *>(w)) {

        double valueArray[MAX_CALC_INPUTS];
        char post[calcstring_length];
        char calcString[calcstring_length];
        long status;
        short errnum;
        double result;

        qCDebug(caImageLog) << "we have a image" << imageWidget << data.edata.fieldtype << data.edata.dataSize << String;

        if(!data.edata.connected) {
            imageWidget->setInvalid(Qt::white);
            return;
        } else {
            imageWidget->setValid();
        }

        // treat visibility if defined, do nothing when not visible
        if(!setObjectVisibility(imageWidget, data.edata.rvalue)) return; // do nothing when not visible

        // treat image frame number, if non dynamic image, do nothing
        if(imageWidget->getFrameCount() <= 1) {
            return;
            // empty calc string, animation was already set
        } else if(imageWidget->getImageCalc().size() == 0) {
            return;
        } else {
            // any monitors ?
            QVariant var=imageWidget->property("MonitorList");
            QVariantList list = var.toList();
            int nbMonitors = list.at(0).toInt();
            qCDebug(caImageLog) << imageWidget << "number of monitors" << nbMonitors;
            if(nbMonitors > 0)  {

                // get calc string
                qCDebug(caImageLog) << "get calc string:" << imageWidget->getImageCalc();
                qstrncpy(calcString, (char*) qasc(imageWidget->getImageCalc()),calcstring_length);

                // scan and get the channels
                for(int i=0; i < 4; i++) valueArray[i] = 0.0;
                for(int i=0; i< nbMonitors;i++) {
                    knobData *ptr = mutexKnobDataP->GetMutexKnobDataPtr(list.at(i+1).toInt());
                    if(ptr != (knobData *) Q_NULLPTR) {
                        // when connected
                        if(ptr->edata.connected) {
                            if (ptr->edata.fieldtype == caINT) {
                                valueArray[i] = ptr->edata.ivalue;
                            } else {
                                valueArray[i] = ptr->edata.rvalue;
                            }
                        } else {
                            valueArray[i] = 0.0;
                        }
                    }
                }

                status = postfix(calcString, post, &errnum);
                if(status) {
                    imageWidget->setInvalid(Qt::black);
                    qCDebug(caImageLog) << "invalid calc 1" << calcString;
                }

                // Perform the calculation
                status = calcPerform(valueArray, &result, post);
                if(!status) {
                    // Result is valid, convert to frame number
                    if(result < 0.0) {
                        imageWidget->setInvalid(Qt::black);
                        qCDebug(caImageLog) << "no valid frame";
                    } else {
                        qCDebug(caImageLog) << "frame ok=" << (int)(result +.5);
                        imageWidget->setFrame((int)(result +.5));
                    }
                } else {
                    qCDebug(caImageLog) << "invalid calc 2";
                    imageWidget->setInvalid(Qt::black);
                }

                // no monitors
            } else {

            }
        }

        // table with pv name, value and unit==========================================================================
    } else if(caTable *tableWidget = qobject_cast<caTable *>(w)) {

        int row= data.specData[0];

        if(data.edata.connected) {
            // enum string
            if(data.edata.fieldtype == caENUM || data.edata.fieldtype == caSTRING) {
                int colorMode = tableWidget->getColorMode();
                if(colorMode == caTable::Alarm) {
                    tableWidget->displayText(row, 1, data.edata.severity, String);
                } else {
                    tableWidget->displayText(row, 1, 0, String);
                }
                // double
            } else {
                if(data.edata.initialize) {
                    tableWidget->setFormat(row, data.edata.precision);
                }
                tableWidget->setValue(row, 1, data.edata.severity, data.edata.rvalue, data.edata.units);
            }

        } else {
            tableWidget->displayText(row, 1, NOTCONNECTED, "NC");
            tableWidget->displayText(row, 2, NOTCONNECTED, "NC");
        }

        // table for waveform values==========================================================================
    } else if(caWaveTable *wavetableWidget = qobject_cast<caWaveTable *>(w)) {

        if(data.edata.connected) {
            // data from vector
            if(data.specData[0] == 0) {
                if(data.edata.valueCount > 0 && data.edata.dataB != (void*) Q_NULLPTR) {
                    if((wavetableWidget->getPrecisionMode() != caWaveTable::User) && (data.edata.initialize)) {
                        wavetableWidget->setActualPrecision(data.edata.precision);
                    }
                    if(data.edata.fieldtype == caSTRING) {
                        QStringList list;
                        list = String.split((QChar)27);
                        wavetableWidget->setStringList(list, data.edata.status, list.size());
                    } else {
                        WaveTable(wavetableWidget, data);
                    }
                } else {
                    QStringList list;
                    for(int i=0; i<qMax(1,wavetableWidget->getNumberOfRows()); i++) {
                        for(int j=0; j<wavetableWidget->getNumberOfColumns(); j++) list.append("????");
                    }
                    wavetableWidget->setStringList(list, NOTCONNECTED, list.size());
                }
            } else if(data.specData[0] == 1) {
                QStringList list;
                list = String.split((QChar)27);
                // here we have to be carefull, while a waveform will give you an index to
                // a list ("STRING", "CHAR", "UCHAR", "SHORT", "USHORT", "LONG", "ULONG", "FLOAT", "DOUBLE", "ENUM")
                // however it could be something else
                if (data.edata.connected && (data.edata.ivalue>=0)){
                 if(data.edata.ivalue < list.count()) wavetableWidget->setDataType(list.at(data.edata.ivalue));
                }
            }

            wavetableWidget->setAccessW(data.edata.accessW);

        } else if(data.specData[0] == 0){
            QStringList list;
            for(int i=0; i<qMax(1,wavetableWidget->getNumberOfRows()); i++) {
                for(int j=0; j<wavetableWidget->getNumberOfColumns(); j++) list.append("NC");
            }
            wavetableWidget->setStringList(list, NOTCONNECTED, list.size());
        }

        // bitnames table with text and coloring according the value=========================================================
    } else if (caBitnames *bitnamesWidget = qobject_cast<caBitnames *>(w)) {
        if(data.edata.connected) {
            // set enum strings
            if(data.edata.fieldtype == caENUM) {
                bitnamesWidget->setEnumStrings(String);
            } else if(data.edata.fieldtype == caINT || data.edata.fieldtype == caLONG) {
                bitnamesWidget->setValue((int) data.edata.ivalue);
            } else if(data.edata.fieldtype == caFLOAT || data.edata.fieldtype == caDOUBLE ) {
                bitnamesWidget->setValue(qRound(data.edata.rvalue));
            }
        } else {
            // todo
        }

        // camera =========================================================
    } else if (caCamera *cameraWidget = qobject_cast<caCamera *>(w)) {

        qCDebug(caCameraLog) << data.pv << data.edata.connected << data.specData[0];
        if(data.edata.connected) {
            if(data.specData[0] == 1) {        // width channel
                cameraWidget->setWidth((int) data.edata.rvalue);
            } else if(data.specData[0] == 2) { // height channel
                cameraWidget->setHeight((int) data.edata.rvalue);
            } else if(data.specData[0] == 3) { // mode overwrite channel if present
                if(cameraWidget->testDecodemodeStr(String)) cameraWidget->setDecodemodeStr(String);
                else  {
                    char asc[MAX_STRING_LENGTH];
                    snprintf(asc, MAX_STRING_LENGTH, "camera mode %s from pv %s not recognized", qasc(String), qasc(cameraWidget->getPV_ColormodeChannel()));
                    postMessageAndLog(QtDebugMsg, asc, caQtDMLibLog);
                }
            } else if(data.specData[0] == 4) { // packing mode overwrite channel if present
                if(cameraWidget->testPackingmodeStr(String)) cameraWidget->setPackingmodeStr(String);
                else  {
                    char asc[MAX_STRING_LENGTH];
                    snprintf(asc,MAX_STRING_LENGTH, "camera packing mode %s from pv %s not recognized", qasc(String), qasc(cameraWidget->getPV_PackingmodeChannel()));
                    postMessageAndLog(QtDebugMsg, asc, caQtDMLibLog);
                }
            } else if(data.specData[0] == 5) { // minimum level channel if present
                cameraWidget->updateMin((int) data.edata.rvalue);
            } else if(data.specData[0] == 6) { // maximum level channel if present
                cameraWidget->updateMax((int) data.edata.rvalue);
            } else if(data.specData[0] == 7) { // value1 if present
                cameraWidget->dataProcessing(data.edata.rvalue, 0);
            } else if(data.specData[0] == 8) { // value2 if present
                cameraWidget->dataProcessing(data.edata.rvalue, 1);
            } else if(data.specData[0] == 9) { // value3 if present
                cameraWidget->dataProcessing(data.edata.rvalue, 2);
            } else if(data.specData[0] == 10) { // value4 if present
                cameraWidget->dataProcessing(data.edata.rvalue, 3);
            } else if(data.specData[0] == 0) { // data channel
                QMutex *datamutex;
                datamutex = (QMutex*) data.mutex;
                datamutex->lock();
                cameraWidget->showImage(data.edata.dataSize, (char*) data.edata.dataB, data.edata.fieldtype);
                datamutex->unlock();
            } else if(data.specData[0] == 15) {
                if(data.edata.valueCount > 0 && data.edata.dataB != (void*) Q_NULLPTR) {
                    CameraWaveform(cameraWidget, 0, 0, 1, data);
                }
            } else if(data.specData[0] == 16) {
                if(data.edata.valueCount > 0 && data.edata.dataB != (void*) Q_NULLPTR) {
                    CameraWaveform(cameraWidget, 0, 0, 0, data);
                }
            }
        } else if(data.specData[0] < 15){
            cameraWidget->showDisconnected();
            // todo
        }

        // scan2d =========================================================
    } else if (caScan2D *scan2dWidget = qobject_cast<caScan2D *>(w)) {

        qCDebug(caScan2DLog) << "Callback_UpdateWidget: caScan2D" << data.pv << data.edata.connected << data.specData[0];
        if (data.edata.connected) {
            switch (data.specData[0]) {
            case 1:        // width channel
                scan2dWidget->setWidth((int) data.edata.rvalue); break;
            case 2: // height channel
                scan2dWidget->setHeight((int) data.edata.rvalue); break;
            case 5: // minimum level channel if present
                scan2dWidget->updateMin((int) data.edata.rvalue); break;
            case 6: // maximum level channel if present
                scan2dWidget->updateMax((int) data.edata.rvalue); break;
            case 11: // XCPT
                scan2dWidget->setXCPT((int) data.edata.rvalue); break;
            case 12: // YCPT
                scan2dWidget->setYCPT((int) data.edata.rvalue); break;
            case 13: // XNEWDATA
                scan2dWidget->setXNEWDATA((int) data.edata.rvalue); break;
            case 14: // YNEWDATA
                scan2dWidget->setYNEWDATA((int) data.edata.rvalue); break;
            case 15: // SAVEDATA_PATH
                scan2dWidget->setSAVEDATA_PATH(String); break;
            case 16: // SAVEDATA_SUBDIR
                scan2dWidget->setSAVEDATA_SUBDIR(String); break;
            case 17: // SAVEDATA_FILENAME
                scan2dWidget->setSAVEDATA_FILENAME(String); break;
            case 18:  // value1 if present
                scan2dWidget->dataProcessing(data.edata.rvalue, 0); break;
            case 19: // value 2 if present
                scan2dWidget->dataProcessing(data.edata.rvalue, 1); break;
            case 20: // value3 if present
                scan2dWidget->dataProcessing(data.edata.rvalue, 2); break;
            case 21: // value4 if present
                scan2dWidget->dataProcessing(data.edata.rvalue, 3); break;
            case 0: // data channel
                scan2dWidget->newArray(data.edata.dataSize, (float*) data.edata.dataB); break;
            default: // ?
                break;
            }
        } else {
            //scan2dWidget->showDisconnected();
        }

        // messagebutton, yust treat access ==========================================================================
    } else if (caMessageButton *messagebuttonWidget = qobject_cast<caMessageButton *>(w)) {

        if(data.edata.connected) {

            if (messagebuttonWidget->getColorMode() == caMessageButton::Alarm) {
                messagebuttonWidget->setAlarmColors(data.edata.severity);
                // case of static mode
            } else {
                SetColorsBack(messagebuttonWidget);
            }
            // enable or disable this button
            if(data.specData[0] == 1) {
                if((data.edata.rvalue != 0) || (data.edata.ivalue != 0)) {
                    messagebuttonWidget->setEnabled(false);
                } else {
                    messagebuttonWidget->setEnabled(true);
                }
            }

        } else {
            SetColorsNotConnected(messagebuttonWidget);
        }
        messagebuttonWidget->setAccessW((bool) data.edata.accessW);
        updateAccessCursor(messagebuttonWidget);

    } else if (caHMIConfig *cahmiConfigWidget = qobject_cast<caHMIConfig*>(w)) {
        Q_UNUSED(cahmiConfigWidget)
    } else if (wmSignalRescale *wmSignalRescaleWidget = qobject_cast<wmSignalRescale*>(w)) {
        Q_UNUSED(wmSignalRescaleWidget)
    } else if (genSoftPV *genSoftPVWidget = qobject_cast<genSoftPV*>(w)) {
        Q_UNUSED(genSoftPVWidget)
    } else {
        // something else (user defined monitors with non ca imageWidgets ?) ==============================================
        qCWarning(caQtDMLibLog) << "unrecognized widget" << w->metaObject()->className();
    }
}

void CaQtDM_Lib::Cartesian(caCartesianPlot *widget, int curvNB, int curvType, int XorY, const knobData &data)
{
    QMutex *datamutex;
    datamutex = (QMutex*) data.mutex;
    datamutex->lock();
    switch(data.edata.fieldtype) {
    case caFLOAT: {
        float* P = (float*) data.edata.dataB;
        widget->setData(P, data.edata.valueCount, curvNB, curvType, XorY);
        datamutex->unlock();
        widget->displayData(curvNB, curvType);
    }
        break;
    case caDOUBLE: {
        double* P = (double*) data.edata.dataB;
        widget->setData(P, data.edata.valueCount, curvNB, curvType, XorY);
        datamutex->unlock();
        widget->displayData(curvNB, curvType);
    }
        break;
    case caLONG: {
        int32_t* P = (int32_t*) data.edata.dataB;
        widget->setData(P, data.edata.valueCount,curvNB, curvType, XorY);
        datamutex->unlock();
        widget->displayData(curvNB, curvType);
    }
        break;
    case caINT: {
        int16_t* P = (int16_t*) data.edata.dataB;
        widget->setData(P, data.edata.valueCount, curvNB, curvType, XorY);
        datamutex->unlock();
        widget->displayData(curvNB, curvType);
    }
        break;
    case caCHAR: {
        int8_t* P = (int8_t*) data.edata.dataB;
        widget->setData(P, data.edata.valueCount, curvNB, curvType, XorY);
        datamutex->unlock();
        widget->displayData(curvNB, curvType);
    }
        break;
    case caENUM: {
        int16_t* P = ( int16_t*) data.edata.dataB;
        widget->setData(P ,data.edata.valueCount, curvNB, curvType, XorY);
        datamutex->unlock();
        widget->displayData(curvNB, curvType);
    }
        break;
    default:
        datamutex->unlock();
        break;
    }
}

void CaQtDM_Lib::CameraWaveform(caCamera *widget, int curvNB, int curvType, int XorY, const knobData &data)
{
    QMutex *datamutex;
    datamutex = (QMutex*) data.mutex;
    datamutex->lock();
    switch(data.edata.fieldtype) {
    case caFLOAT: {
        float* P = (float*) data.edata.dataB;
        widget->setData(P, data.edata.valueCount, curvNB, curvType, XorY);
        datamutex->unlock();
    }
        break;
    case caDOUBLE: {
        double* P = (double*) data.edata.dataB;
        widget->setData(P, data.edata.valueCount, curvNB, curvType, XorY);
        datamutex->unlock();
    }
        break;
    case caLONG: {
        int32_t* P = (int32_t*) data.edata.dataB;
        widget->setData(P, data.edata.valueCount,curvNB, curvType, XorY);
        datamutex->unlock();
    }
        break;
    case caINT: {
        int16_t* P = (int16_t*) data.edata.dataB;
        widget->setData(P, data.edata.valueCount, curvNB, curvType, XorY);
        datamutex->unlock();
    }
        break;
    case caCHAR: {
        int8_t* P = (int8_t*) data.edata.dataB;
        widget->setData(P, data.edata.valueCount, curvNB, curvType, XorY);
        datamutex->unlock();
    }
        break;
    case caENUM: {
        int16_t* P = ( int16_t*) data.edata.dataB;
        widget->setData(P, data.edata.valueCount, curvNB, curvType, XorY);
        datamutex->unlock();
    }
        break;
    default:
        datamutex->unlock();
        break;
    }
}

void CaQtDM_Lib::WaveTable(caWaveTable *widget, const knobData &data)
{
    QMutex *datamutex;
    datamutex = (QMutex*) data.mutex;
    datamutex->lock();

    switch(data.edata.fieldtype) {
    case caFLOAT: {
        float* P = (float*) data.edata.dataB;
        widget->setData(P, data.edata.severity, data.edata.valueCount);
        datamutex->unlock();
    }
        break;
    case caDOUBLE: {
        double* P = (double*) data.edata.dataB;
        widget->setData(P, data.edata.severity, data.edata.valueCount);
        datamutex->unlock();
    }
        break;
    case caLONG: {
        int32_t* P = (int32_t*) data.edata.dataB;
        widget->setData(P, data.edata.severity, data.edata.valueCount);
        datamutex->unlock();
    }
        break;
    case caINT: {
        int16_t* P = (int16_t*) data.edata.dataB;
        widget->setData(P, data.edata.severity, data.edata.valueCount);
        datamutex->unlock();
    }
        break;
    case caCHAR: {
        char * P = (char*) data.edata.dataB;
        widget->setData(P, data.edata.severity, data.edata.valueCount);
        datamutex->unlock();
    }
        break;
    default:
        datamutex->unlock();
        break;
    }
}


void CaQtDM_Lib::WaterFall(caWaterfallPlot *widget, const knobData &data)
{
    QMutex *datamutex;
    datamutex = (QMutex*) data.mutex;
    datamutex->lock();
    switch(data.edata.fieldtype) {
    case caFLOAT: {
        float* P = (float*) data.edata.dataB;
        widget->setData(P, data.edata.valueCount);
        datamutex->unlock();
        widget->displayData();
    }
        break;
    case caDOUBLE: {
        double* P = (double*) data.edata.dataB;
        widget->setData(P, data.edata.valueCount);
        datamutex->unlock();
        widget->displayData();
    }
        break;
    case caLONG: {
        int32_t* P = (int32_t*) data.edata.dataB;
        widget->setData(P, data.edata.valueCount);
        datamutex->unlock();
        widget->displayData();
    }
        break;
    case caINT: {
        int16_t* P = (int16_t*) data.edata.dataB;
        widget->setData(P, data.edata.valueCount);
        datamutex->unlock();
        widget->displayData();
    }
        break;
    case caENUM: {
        int16_t* P = ( int16_t*) data.edata.dataB;
        widget->setData(P, data.edata.valueCount);
        datamutex->unlock();
        widget->displayData();
    }
        break;
    default:
        datamutex->unlock();
        break;
    }
}

void CaQtDM_Lib::getStatesToggleAndLed(QWidget *widget, const knobData &data, const QString &String, Qt::CheckState &state)
{
    QString trueString, falseString;
    bool ok1, ok2;
    if (caLed *ledWidget = qobject_cast<caLed *>(widget)) {
        trueString = ledWidget->getTrueValue().trimmed();
        falseString = ledWidget->getFalseValue().trimmed();
    } else if(caToggleButton *togglebuttonWidget = qobject_cast<caToggleButton *>(widget)) {
        trueString = togglebuttonWidget->getTrueValue().trimmed();
        falseString = togglebuttonWidget->getFalseValue().trimmed();
    }

    if(data.edata.fieldtype == caINT || data.edata.fieldtype == caLONG) {
        int trueValue = trueString.toInt(&ok1);
        int falseValue = falseString.toInt(&ok2);
        if(ok1 && trueValue == data.edata.ivalue) {
            state = Qt::Checked;
        } else if(ok2 && falseValue == data.edata.ivalue) {
            state = Qt::Unchecked;
        } else {
            state = Qt::PartiallyChecked;
        }
    } else if(data.edata.fieldtype == caFLOAT || data.edata.fieldtype == caDOUBLE ) {
        double trueValue = trueString.toDouble(&ok1);
        double falseValue = falseString.toDouble(&ok2);
        if(ok1 && trueValue == data.edata.rvalue) {
            state = Qt::Checked;
        } else if(ok2 && falseValue == data.edata.rvalue) {
            state = Qt::Unchecked;
        } else {
            state = Qt::PartiallyChecked;
        }
    } else if(data.edata.fieldtype == caENUM || data.edata.fieldtype == caSTRING|| data.edata.fieldtype == caCHAR) {
        int trueValue = trueString.toInt(&ok1);
        int falseValue = falseString.toInt(&ok2);
        state = Qt::PartiallyChecked;

        QString str = "";
        QStringList list;
        list = String.split((QChar)27);

        if((int) data.edata.ivalue < list.count()  && (list.count() > 0))  str = list.at((int) data.edata.ivalue);

        // integer value given
        if(ok1) {
            if(trueValue == data.edata.ivalue) {
                state = Qt::Checked;
            }
            // string value
        } else {
            if(trueString.compare(str) == 0) state = Qt::Checked;
        }

        // integer value given
        if(ok2) {
            if(falseValue == data.edata.ivalue) {
                state = Qt::Unchecked;
            }
            // string value
        } else {
            if(falseString.compare(str) == 0) state = Qt::Unchecked;
        }
    }
}

void CaQtDM_Lib::messageWindowOutput(const QtMsgType type, const QString &message) {
    postMessage(type, message.toLatin1().data());
}

void CaQtDM_Lib::Callback_CaCalc(double value)
{
    int indx;
    caCalc *caCalcWidget = qobject_cast<caCalc *>(sender());
    qCDebug(caCalcLog) << "Callback_CaCalc from sender" << value << caCalcWidget << caCalcWidget->getVariable();

    knobData *kPtr = mutexKnobDataP->getMutexKnobDataPV(caCalcWidget, caCalcWidget->getVariable());
    if(kPtr != (knobData *) Q_NULLPTR) {
        // when softpv treat it and get out
        if(mutexKnobDataP->getSoftPV(caCalcWidget->getVariable(), &indx, (QWidget*) kPtr->thisW)) {
            if(kPtr->soft) {
                qCDebug(caCalcLog) << "write softpv at" << kPtr->index << kPtr->pv << "with value" << value;
                kPtr = mutexKnobDataP->GetMutexKnobDataPtr(indx);  // use pointer
                kPtr->edata.rvalue = value;
                kPtr->edata.ivalue = (int) value;
                kPtr->edata.monitorCount++;
            }
        }
    }
}

/**
 * callback will write value to device
 */
void CaQtDM_Lib::Callback_EApplyNumeric(double value)
{
    int32_t idata = (int32_t) llround(value);
    double rdata = value;

    caApplyNumeric *numeric = qobject_cast<caApplyNumeric *>(sender());
    if(!numeric) return;
    if(!numeric->getAccessW()) return;
    if(numeric->getPV().length() > 0) {
        TreatOrdinaryValue(numeric->getPV(), rdata,  idata, "", (QWidget*) numeric);
    }
}
/**
 * callback will write value to device
 */
void CaQtDM_Lib::Callback_ENumeric(double value)
{
    int32_t idata = (int32_t) llround(value);
    double rdata = value;

    caNumeric *numeric = qobject_cast<caNumeric *>(sender());
    if(!numeric) return;
    if(!numeric->getAccessW()) return;
    if(numeric->getPV().length() > 0) {
        TreatOrdinaryValue(numeric->getPV(), rdata,  idata, "", (QWidget*) numeric);
    }
}

/**
 * callback will write value to device
 */
void CaQtDM_Lib::Callback_Spinbox(double value)
{
    int32_t idata = (int32_t) llround(value);
    double rdata = value;

    caSpinbox *numeric = qobject_cast<caSpinbox *>(sender());
    if(!numeric) return;
    if(!numeric->getAccessW()) return;
    if(numeric->getPV().length() > 0) {
        TreatOrdinaryValue(numeric->getPV(), rdata,  idata, "", (QWidget*) numeric);
    }
}

void CaQtDM_Lib::Callback_SliderValueChanged(double value)
{
    int32_t idata = (int32_t) value;
    double rdata = value;

    caSlider *numeric = qobject_cast<caSlider *>(sender());
    if(!numeric->getAccessW()) return;
    if(numeric->getPV().length() > 0) {
        TreatOrdinaryValue(numeric->getPV(), rdata, idata,  "", (QWidget*) numeric);
    }
}

/**
 * callback will write value to device
 */
void CaQtDM_Lib::Callback_ToggleButton(bool type)
{
    bool ok1, ok2;
    double rvalue = 0.0;
    int32_t ivalue = 0;
    QString svalue ="";
    caToggleButton *w = qobject_cast<caToggleButton *>(sender());
    if(!w->getAccessW()) return;

    QString trueString = w->getTrueValue().trimmed();
    QString falseString = w->getFalseValue().trimmed();
    if(type) svalue = trueString; else svalue = falseString;

    int trueValueI = trueString.toInt(&ok1);
    int falseValueI = falseString.toInt(&ok2);
    if(ok1 && type) {
        ivalue = trueValueI;
    } else if(ok2  && !type) {
        ivalue = falseValueI;
    }

    double trueValueD = trueString.toDouble(&ok1);
    double falseValueD = falseString.toDouble(&ok2);
    if(ok1 && type) {
        rvalue = trueValueD;
    } else if(ok2 && !type) {
        rvalue = falseValueD;
    }

    if(w->getPV().length() > 0) {
        TreatOrdinaryValue(w->getPV(), rvalue, ivalue, svalue, (QWidget*) w);
    }
}

/**
 * updates the text of a QLineEdit widget given by its name
 */
void CaQtDM_Lib::Callback_UpdateLine(const QString& text, const QString& name)
{
    qCDebug(caQtDMLibLog) << "Callback_UpdateLine" << text << name;
    QLineEdit *lineedit = this->findChild<QLineEdit*>(name);
    if (lineedit != NULL) lineedit->setText(text);
}

/**
 * callback will write value to device
 */
void CaQtDM_Lib::Callback_ChoiceClicked(const QString& text)
{
    char errmess[SMALL_STRING_LENGTH];

    caChoice *choice = qobject_cast<caChoice *>(sender());

    choice->updateChoice();

    if(!choice->getAccessW()) return;

    if(choice->getPV().length() > 0) {
        qCDebug(caChoiceLog) << "choice_clicked" << text << choice->getPV();
        QStringsToChars(choice->getPV().trimmed(), text,  choice->objectName().toLower());
        //ControlsInterface * plugininterface = (ControlsInterface *) choice->property("Interface").value<void *>();
        ControlsInterface *plugininterface = getPluginInterface((QWidget*) choice);
        if(plugininterface != (ControlsInterface *) Q_NULLPTR) {
            knobData *kPtr;
            if((kPtr = GetMutexKnobDataPV((QWidget*) choice, param1)) != (knobData *) Q_NULLPTR) {
                if(!plugininterface->pvSetValue(kPtr, 0.0, 0, param2, param3, errmess, 0)) {
                    plugininterface->pvSetValue(param1, 0.0, 0, param2, param3, errmess, 0);
                }
            }
        }
    }
}

/**
 * callback will write value to device
 */
void CaQtDM_Lib::Callback_MenuClicked(const QString& text)
{
    char errmess[SMALL_STRING_LENGTH];
    caMenu *menu = qobject_cast<caMenu *>(sender());

    if(!menu->getAccessW()) return;

    if(menu->getPV().length() > 0) {
        qCDebug(caMenuLog) << "menu_clicked" << text << menu->getPV();
        QStringsToChars(menu->getPV().trimmed(), text,  menu->objectName().toLower());
        ControlsInterface *plugininterface = getPluginInterface((QWidget*) menu);
        if(plugininterface != (ControlsInterface *) Q_NULLPTR) {
            knobData *kPtr;
            if((kPtr = GetMutexKnobDataPV((QWidget*) menu, param1)) != (knobData *) Q_NULLPTR) {
                if(!plugininterface->pvSetValue(kPtr, 0.0, 0, param2, param3, errmess, 0)) {
                    plugininterface->pvSetValue(param1, 0.0, 0, param2, param3, errmess, 0);
                }
            }
        }
    }
    // display label again when configured with it
    if(menu->getLabelDisplay()) {
        menu->setCurrentIndex(0);
    }
}

/**
 * callback will write value to device
 */
void CaQtDM_Lib::Callback_TextEntryChanged(const QString& text)
{
    FormatType fType;
    QWidget *w1 = qobject_cast<QWidget *>(sender());
    caTextEntry *w = qobject_cast<caTextEntry *>(sender());

    if(!w->getAccessW()) return;

    fType = (FormatType) w->getFormatType();
    TreatRequestedValue(w->getPV(), text, fType, w1);
}

void CaQtDM_Lib::Callback_WaveEntryChanged(const QString& text, int index)
{
    caWaveTable::FormatType fType;
    QWidget *w1 = qobject_cast<QWidget *>(sender());
    caWaveTable *w = qobject_cast<caWaveTable *>(sender());

    if(!w->getAccessW()) return;

    qCDebug(caWaveTableLog) << "should write" << text << "at index" << index;
    fType = w->getFormatType();
    TreatRequestedWave(w->getPV(), text, fType, index, w1);
}

void CaQtDM_Lib::Callback_ExternalHmiEventReceived(int eventType, int senderPid, qint64 timestamp, const QByteArray& payload)
{
#ifndef MOBILE
    Q_UNUSED(timestamp)
    if (senderPid == QApplication::applicationPid()) return; // ignore own events
    if (eventType == EventTypes::KeyPress) {
        QDataStream in(payload);

        int key;
        int modifiers;

        in >> key >> modifiers;
        if (in.status() != QDataStream::Ok) return;
        Qt::KeyboardModifiers qtModifiers = static_cast<Qt::KeyboardModifiers>(modifiers);
        QKeySequence sequence = QKeySequence(key | modifiers);
        qCDebug(caQtDMLibLog) << "received keys" << sequence.toString() << "from" << senderPid;
        QKeyEvent constructed(QEvent::KeyPress, key, qtModifiers, sequence.toString());
        this->hmiHandleIncomingEvent(Q_NULLPTR, &constructed, &constructed, true);
    } else if (eventType == EventTypes::MouseMove || eventType == EventTypes::MousePress) {
        QDataStream in(payload);

        int x, y;
        in >> x >> y;
        if (in.status() != QDataStream::Ok || qAbs(x) > 100000 || qAbs(y) > 100000) return;
        qCDebug(caQtDMLibLog) << "received mouse " << x << y << "from" << senderPid;
        QEvent::Type type = QEvent::None;
        if (eventType == EventTypes::MouseMove) type = QEvent::MouseMove;
        else if (eventType == EventTypes::MousePress) type = QEvent::MouseButtonPress;
        if (type == QEvent::None) return;
        QMouseEvent constructed(type, QPointF(x, y), QPointF(x, y), Qt::MouseButton::NoButton, Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
        this->hmiHandleIncomingEvent(Q_NULLPTR, &constructed, &constructed, true);
    }
#endif
}

void CaQtDM_Lib::hmiHandleKeyPressed(QObject *target, QKeyEvent *event)
{
#ifndef MOBILE
    HmiSharedEventBus& eventBus = HmiSharedEventBus::instance();
    if (eventBus.isInitialized()) {
        QByteArray byteArray;
        QDataStream out(&byteArray, QIODevice::WriteOnly);
        out << event->key() << event->modifiers();

        eventBus.sendEvent(EventTypes::KeyPress, byteArray);
    }
#endif
    this->hmiHandleIncomingEvent(target, event, event, false);
}

void CaQtDM_Lib::hmiHandleMouse(QObject *target, QMouseEvent *event)
{
    QWidget* window = Q_NULLPTR;
    QWidget* target_widget = Q_NULLPTR;
    target_widget = qobject_cast<QWidget*>(target);
    window = target_widget;
    while (window != Q_NULLPTR) {
        if (window == this) {
            break;
        }
        window = window->parentWidget();
    }
    if (window == Q_NULLPTR) return;

    QPoint point = event->pos();
    QPoint corrected = point;
    if (target != window && target_widget != Q_NULLPTR){
        corrected = target_widget->mapTo(window, point);
    }
    QMouseEvent correctedEvent = QMouseEvent(event->type(), corrected, corrected, event->button(), event->buttons(), event->modifiers());
#ifndef MOBILE
    HmiSharedEventBus& eventBus = HmiSharedEventBus::instance();
    if (eventBus.isInitialized()) {
        int type = EventTypes::Invalid;

        if (event->type() == QEvent::MouseMove) {
            type = EventTypes::MouseMove;
        } else if (event->type() == QEvent::MouseButtonPress) {
            type = EventTypes::MousePress;
        }

        // Throttle only the shared-bus publication of mouse moves
        bool throttled = (type == EventTypes::MouseMove
                          && this_hmiMouseMoveSendTimer.isValid()
                          && this_hmiMouseMoveSendTimer.elapsed() < MOUSE_THROTTLE_INTERVAL_MS);
        if (type != EventTypes::Invalid && !throttled) {
            if (type == EventTypes::MouseMove) this_hmiMouseMoveSendTimer.restart();
            QByteArray byteArray;
            QDataStream out(&byteArray, QIODevice::WriteOnly);
            out << correctedEvent.pos().x() << correctedEvent.pos().y();
            eventBus.sendEvent(type, byteArray);
        }
    }
#endif
    this->hmiHandleIncomingEvent(target, &correctedEvent, event, false);
}

bool CaQtDM_Lib::containsShortcut(const QKeySequence& sequence, Qt::Key qtKey, Qt::KeyboardModifiers qtModifiers) {
    for (int i = 0; i < sequence.count(); ++i) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QKeyCombination combination = sequence[i];
        if (combination.key() == qtKey && combination.keyboardModifiers() == qtModifiers) {
            return true;
        }
#else
        int keyVal = sequence[i];
        Qt::KeyboardModifiers currentModifiers = static_cast<Qt::KeyboardModifiers>(keyVal & Qt::KeyboardModifierMask);
        Qt::Key currentKey = static_cast<Qt::Key>(keyVal & ~Qt::KeyboardModifierMask);

        if (currentKey == qtKey && currentModifiers == qtModifiers) {
            return true;
        }
#endif
    }
    return false;
}

bool CaQtDM_Lib::hasCommonParent(QObject *origin, QObject *target) {
    if (origin == Q_NULLPTR || target == Q_NULLPTR) return false;
    QObject* parent = origin->parent();

    QObject* next = target;
    while (next != Q_NULLPTR) {
        if (next == parent) return true;
        next = next->parent();
    }
    return false;
}

void CaQtDM_Lib::hmiHandleIncomingEvent(QObject *target, QEvent *event, QEvent *originalEvent, bool isSourceExternal)
{
    Q_UNUSED(target);

    QWidget* window = Q_NULLPTR;
    QWidget* target_widget = Q_NULLPTR;
    if (!isSourceExternal) {
        target_widget = qobject_cast<QWidget*>(target);
        window = target_widget;
        while (window != Q_NULLPTR) {
            if (window == this) {
                break;
            }
            window = window->parentWidget();
        }
        if (window == Q_NULLPTR) return;
    }

    QReadLocker locker(&hmiConfigListLock);
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (keyEvent == Q_NULLPTR) return;
        foreach (caHMIConfigTransferItem *item, hmiConfigList) {
            if (item->enabled() == false) continue;
            if ((isSourceExternal || (window != item->parentWindowCallback() && window != this)) && item->captureRange() == caHMIConfig::capRange::Local) {
                continue;
            }
            if (item->captureType() != caHMIConfig::capType::MouseMove && item->captureType() != caHMIConfig::capType::MousePress) {
                int key = keyEvent->key();
                Qt::Key qtKey = static_cast<Qt::Key>(key);
                int modifiers = keyEvent->modifiers();
                Qt::KeyboardModifiers qtModifiers = static_cast<Qt::KeyboardModifiers>(modifiers);
                QKeySequence shortcut = item->shortcut();
                caHMIConfig *widget = item->widgetCallback();
                if (widget == Q_NULLPTR) continue;
                if (item->captureType() == caHMIConfig::capType::KeyboardSet) {
                    if (containsShortcut(shortcut, qtKey, qtModifiers)) {
                        qCDebug(caQtDMLibLog) << "Correct key pressed" << widget->objectName() << qtKey;
                        emit widget->caHMIConfigKeyPressReceived(shortcut);

                        if (item->calculationType() == caHMIConfig::calcType::SetValue) {
                            FormatType fType;
                            if (item->value().canConvert<double>()){
                                fType = FormatType::decimal;
                                QVariant value = QVariant(item->value().toDouble());
                                emit widget->caHMIConfigValueSet(value);
                            } else {
                                fType = FormatType::string;
                                QVariant value = item->value();
                                emit widget->caHMIConfigValueSet(value);
                            }

                            QString text = item->value().toString();
                            QWidget *w1 = qobject_cast<QWidget *>(widget);

                            if (item->outputA().length() == 0) return;
                            TreatRequestedValue(item->outputA(), text, fType, w1);
                        } else if (item->calculationType() == caHMIConfig::calcType::Calc) {
                            QString calc = item->value().toString();
                            double result;
                            bool valid;
                            CalcVisibility(widget, result, valid);
                            if (valid){
                                FormatType fType = FormatType::decimal;
                                QWidget *w1 = qobject_cast<QWidget *>(widget);
                                QString text = QString::number(result);

                                if (item->outputA().length() == 0) return;
                                TreatRequestedValue(item->outputA(), text, fType, w1);
                            }
                        }
                    }
                } else if (item->captureType() == caHMIConfig::capType::KeyboardValue) {
                    FormatType fType = FormatType::decimal;
                    QWidget *w1 = qobject_cast<QWidget *>(widget);
                    QString keyText = QString::number(key);
                    QString keyModifiersText = QString::number(qtModifiers);

                    emit widget->caHMIConfigKeyPressReceived(shortcut);

                    if (item->outputA().length() != 0 ) {
                        TreatRequestedValue(item->outputA(), keyText, fType, w1);
                    }

                    if (item->outputB().length() != 0) {
                        TreatRequestedValue(item->outputB(), keyModifiersText, fType, w1);
                    }
                }
            }
        }
    } else if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
        QPoint point = mouseEvent->pos();

        foreach (caHMIConfigTransferItem *item, hmiConfigList) {
            if (item->enabled() == false) continue;
            if ((isSourceExternal || !hasCommonParent(item->widgetCallback(), target)) && item->captureRange() == caHMIConfig::capRange::Local) {
                continue;
            }
            if ((item->captureType() == caHMIConfig::capType::MouseMove && event->type() == QEvent::MouseMove) || (item->captureType() == caHMIConfig::capType::MousePress && event->type() == QEvent::MouseButtonPress )) {
                caHMIConfig *widget = item->widgetCallback();
                if (widget == Q_NULLPTR) continue;

                if (item->captureRange() == caHMIConfig::capRange::Local) {
                    mouseEvent = dynamic_cast<QMouseEvent*>(originalEvent);
                    QWidget *parent = widget->parentWidget();
                    if (parent != Q_NULLPTR && target_widget != Q_NULLPTR) {
                        point = target_widget->mapTo(parent, mouseEvent->pos());
                    } else point = mouseEvent->pos();
                }

                QRect rect = QRect(point, widget->mouseRectSize());
                emit widget->caHMIConfigMouse(point);
                emit widget->caHMIConfigMouse(rect);
                emit widget->caHMIConfigMouseX(point.x());
                emit widget->caHMIConfigMouseY(point.y());

                FormatType fType = FormatType::decimal;
                QWidget *w1 = qobject_cast<QWidget *>(widget);
                QString xText = QString::number(point.x());
                QString yText = QString::number(point.y());

                if (item->outputA().length() != 0 ) {
                    TreatRequestedValue(item->outputA(), xText, fType, w1);
                }

                if (item->outputB().length() != 0) {
                    TreatRequestedValue(item->outputB(), yText, fType, w1);
                }

            }
        }
    }

}

void CaQtDM_Lib::wmHandleResize(QObject* target, QWidget* wmSignalRescaleWidget, QResizeEvent *event, const QString &channelA, const QString &channelB)
{
    Q_UNUSED(target)
    QSize size = event->size();
    if (channelA.length() > 0) {
        TreatRequestedValue(channelA, QString::number(size.width()), FormatType::decimal, wmSignalRescaleWidget);
    }
    if (channelB.length() > 0) {
        TreatRequestedValue(channelB, QString::number(size.height()), FormatType::decimal, wmSignalRescaleWidget);
    }
}

QString CaQtDM_Lib::wmHandleSoftChannel(QString channel, QMap<QString, QString> map, bool doNothing, QString objectName)
{
    channel = channel.trimmed();
    if (channel.isEmpty()) return channel;
    channel = treatMacro(map, channel, &doNothing, objectName);

    auto foundIt = softvars.find(channel);
    if (foundIt != softvars.end()) {
        return foundIt.key();
    } else {
        qCWarning(caQtDMLibLog) << "wmSignalRescale output channel" << channel << "in file" << savedFile[level] << "is not a softPV, output to soft channel is now disabled";
        return QString();
    }
}

/**
 * callback will call the specified ui file
 */
void CaQtDM_Lib::Callback_RelatedDisplayClicked(int indx)
{
    caRelatedDisplay *w = qobject_cast<caRelatedDisplay *>(sender());
    QStringList files = w->getFiles().split(";");
    QStringList args = w->getArgs().split(";");
    QStringList removeParents = w->getReplaceModes().split(";");

    qCDebug(caRelatedDisplayLog) << "Callback_RelatedDisplayClicked: indx=" << indx
                                 << "rawFiles=" << w->getFiles() << "fileCount=" << files.count();


    // special case where macros are coming from a macro definition file
    // when specified with %(read filename) in the argument list
    args=treat_read_MacroCommand(args);

    // get global macro, replace specified keys and build the macro string of caRelatedDisplay, but
    // only when some replacement is requested; otherwise we may get a clash when a macrokey is used with other value
    QList<replaceMacro *> all = myWidget->findChildren<replaceMacro *>();
    if(all.count() > 0) {
        QVariant macroString = this->property("macroString");
        if(!macroString.isNull()) {
            QMap<QString, QString> mapActualized = actualizeMacroMap();
            qCDebug(caRelatedDisplayLog) << "actualized macro map" << mapActualized;
            if(!mapActualized.isEmpty()) {
                // go now through our arguments and replace the value of the specified macro name
                for(int j=0; j< args.count(); j++) {
                    args[j] = actualizeMacroString(mapActualized, args[j]);
                }
            }
        }
    }

    // find position of this window
    int xpos = this->pos().x();
    int ypos = this->pos().y();
    QString geometry = "+%1+%2";
    geometry = geometry.arg(xpos).arg(ypos);



    if (!vncServer) {

        // do we have to remove this window while removeparent was specified
        if(indx < removeParents.count()) {
            QString removeParent = removeParents.at(indx);
            removeParent = removeParent.toLower();
            if(removeParent.contains("true")) {
                this->close();
            } else {
                // in case we do not remove the parent let the window manager position the new window
                geometry = "";
                // however it is possible that the user wanted a fixed position
                if((w->getPosition().x() != -1) || w->getPosition().y() != -1) {
                    if(w->getPosition().x() < 0) xpos = 0; else xpos= w->getPosition().x();
                    if(w->getPosition().y() < 0) ypos = 0; else ypos = w->getPosition().y();
                    geometry = "+%1+%2";
                    geometry = geometry.arg(xpos).arg(ypos);
                }
            }
        }

        // open new file and
        if(indx < files.count() && indx < args.count()) {
            emit Signal_OpenNewWFile(files[indx].trimmed(), args[indx].trimmed(), geometry, "true");
        } else if(indx < files.count()) {
            emit Signal_OpenNewWFile(files[indx].trimmed(), "", geometry, "true");
        }

    }
#ifdef WEB
    else if (!slaveServer) {
        // start new process and vnc or novnc

        if (indx >= files.count()) {
            qCWarning(webRelatedDisplay) << "Tried to open unavailable file through caRelatedDisplay (index was larger or equal than/to file count)";
            QMessageBox::critical(this, "Error", "Tried to open unavailable file through caRelatedDisplay (index was larger or equal than/to file count)");
            return;
        }

        QString file = files[indx].trimmed();
        qCDebug(webRelatedDisplay) << "RelatedDisplay(master): rawEntry=" << files[indx] << "len=" << files[indx].size()
                                   << "trimmed=" << file << "len=" << file.size();
        QString absolutePath = webResolveDisplayPath(file);
        qCDebug(webRelatedDisplay) << "RelatedDisplay(master): resolved=" << QDir::toNativeSeparators(absolutePath);

        if (absolutePath.isNull()) {
            qCWarning(webRelatedDisplay) << "caRelatedDisplay ui file not found";
            QMessageBox::critical(this, "Error - File not found", "The specified path is either invalid or CAQTDM_DISPLAY_PATH hasn't been correctly set");
            return;
        }

        quint16 new_vnc_port;
        quint16 new_web_port;

        QString macros;
        if (indx < args.count()) {
            macros = args[indx].trimmed();
        }

        VncWebChildProcess* existing = getWebChildProcess(absolutePath, macros);

        QString webOpenPath = webOpenPathFromDisplayPath(absolutePath);

        if (existing != nullptr) {
            if (existing->process() == nullptr || existing->process()->state() == QProcess::ProcessState::NotRunning) {
                existing->deleteLater();
            } else if (WebSocketServer::instance().isInitialized()) {
                WebSocketServer::instance().sendOpenFileRequest(webOpenPath, macros);
                return;
            }
        }

        int processCount;
        {
            QReadLocker locker(&CaQtDM_Lib::webChildProcessesLock);
            processCount = CaQtDM_Lib::webChildProcesses.count();
        }

        if (processCount >= CaQtDM_Lib::webInstanceLimit - 1) {
            qCInfo(webRelatedDisplay) << "Couldn't start child process, instance limit reached";
            QMessageBox::critical(this, "Error - Couldn't start instance", "Maximum instance limit reached");
            return;
        }

        if (!WebPortPool::instance()->allocate(new_vnc_port, new_web_port)) {
            qCWarning(webRelatedDisplay) << "Failed to allocate ports for new instance" << file << "- pool exhausted ("
                       << WebPortPool::instance()->freeCount() << "free)";
            QMessageBox::critical(this, "Error - Couldn't start instance", "Maximum instance limit reached");
            return;
        }

        VncWebChildProcess *item = startVncChildProcess(new_vnc_port, new_web_port, absolutePath, macros, this);

        if (WebSocketServer::instance().isInitialized()) {
            WebSocketServer::instance().sendOpenFileRequest(webOpenPath, macros);
        }

        addWebChildProcess(absolutePath, macros, item);
    } else {
        QString file = files[indx].trimmed();
        qCDebug(webRelatedDisplay) << "RelatedDisplay(slave): rawEntry=" << files[indx] << "len=" << files[indx].size()
                                   << "trimmed=" << file << "len=" << file.size();
        QString absolutePath = webResolveDisplayPath(file);
        qCDebug(webRelatedDisplay) << "RelatedDisplay(slave): resolved=" << QDir::toNativeSeparators(absolutePath);

        if (absolutePath.isNull()) {
            qCInfo(webRelatedDisplay) << "caRelatedDisplay ui " << file <<" file not found";
            QMessageBox::critical(this, "Error - File not found", "The specified path is either invalid or CAQTDM_DISPLAY_PATH hasn't been correctly set");
            return;
        }

        QString webOpenPath = webOpenPathFromDisplayPath(absolutePath);

        QString macros;
        if (indx < args.count()) {
            macros = args[indx].trimmed();
        }

        if (WebSocketServer::instance().isInitialized()) {
            WebSocketServer::instance().sendOpenFileRequest(webOpenPath, macros);
        }
    }
#endif
}


#ifdef WEB
QString CaQtDM_Lib::getChildProcessKey(QString absoluteFilePath, QString macros) {
    QString key = absoluteFilePath;
    if (macros.length() > 0) {
        key += '\0';
        key += macros;
    }
    return key;
}

void CaQtDM_Lib::addWebChildProcess(QString absoluteFilePath, QString macros, VncWebChildProcess* childProcess) {

    QString key = getChildProcessKey(absoluteFilePath, macros);

    QWriteLocker webChildProcessesLocker(&webChildProcessesLock);
    webChildProcesses.insert(key, childProcess);
}

/**
 * @brief This function queries vncWebChildProcesses for an item,
 * if the result is nullptr or the process is no longer running,
 * then the item is also removed from the list.
 * @param absoluteFilePath The absolute file path to the ui file that the child process has loaded
 * @param macros The corresponding macros of the child process
 * @return The resulting item or nullptr if nothing was found or the item itself is a nullptr
 */
VncWebChildProcess* CaQtDM_Lib::getWebChildProcess(QString absoluteFilePath, QString macros) {

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    const auto skip_empty_parts = Qt::SkipEmptyParts;
#else
    const auto skip_empty_parts = QString::SkipEmptyParts;
#endif

    QString key;
    QStringList incomingMacros = macros.split(QChar(';'), (Qt::SplitBehavior)skip_empty_parts);
    incomingMacros.sort();

    foreach (QString item, webChildProcesses.keys()) {
        QStringList itemParts = item.split(QChar('\0'));

        if (itemParts.isEmpty() || itemParts[0] != absoluteFilePath) {
            continue;
        }

        QStringList itemMacros;
        if (itemParts.length() > 1) {
            itemMacros = itemParts[1].split(';', skip_empty_parts);
            itemMacros.sort();
        }

        if (incomingMacros == itemMacros) {
            key = item;
            break;
        }
    }

    if (key.isEmpty()) {
        return nullptr;
    }

    auto result = webChildProcesses.find(key);
    if (result != webChildProcesses.end()) {
        if (result.value() == nullptr) {
            webChildProcesses.remove(key);
            return nullptr;
        }

        if (result.value()->process() == nullptr || result.value()->process()->state() == QProcess::ProcessState::NotRunning) {
            webChildProcesses.remove(key);
            return nullptr;
        }

        return result.value();
    }
    return nullptr;
}

VncWebChildProcess* CaQtDM_Lib::startVncChildProcess(quint16 vncPort, quint16 webPort, QString file, QString macros, QWidget *parent) {
    QString caQtDM_executable = QApplication::applicationFilePath();
    QString executable;

    if (!noVncPlugin) {
        QString web_path = qgetenv("CAQTDM_WEB_PATH");
        if (web_path.length() <= 0) {
            web_path = QDir(QApplication::applicationDirPath() + "/../caQtDM_Web").absolutePath();
        }

        QString websockify_path = QDir(web_path + "/websockify").absolutePath();
        QFileInfo websockify_info(websockify_path);
        if (!websockify_info.exists()) {
            qCCritical(webChildProcess) << "websockify does not exist inside caQtDM_Web folder, unable to start another instance for caRelatedDisplay, please ensure you specified the right CAQTDM_WEB_PATH and the websockify folder exists within it.";
            if (parent) QMessageBox::critical(parent, "Error", WEB_CARELATED_DISPLAY_ERROR_MSG);
            return nullptr;
        }
        if (websockify_info.isFile()) {
            qCCritical(webChildProcess) << "websockify is a file and not a folder, please ensure that you initialized the websockify git submodule.";
            if (parent) QMessageBox::critical(parent, "Error", WEB_CARELATED_DISPLAY_ERROR_MSG);
            return nullptr;
        }
        if (!websockify_info.isDir()) {
            qCCritical(webChildProcess) << "websockify is neither a folder nor a file, please check if you provided the right CAQTDM_WEB_PATH";
            if (parent) QMessageBox::critical(parent, "Error", WEB_CARELATED_DISPLAY_ERROR_MSG);
            return nullptr;
        }
        QFile runScript(websockify_path + "/run");

        if (!runScript.exists()) {
            qCCritical(webChildProcess) << "websockify run script, please check if you provided the right CAQTDM_WEB_PATH and if websockify was correctly cloned inside it";
            if (parent) QMessageBox::critical(parent, "Error", WEB_CARELATED_DISPLAY_ERROR_MSG);
            return nullptr;
        }

        executable = runScript.fileName();
    } else executable = caQtDM_executable;

    VncWebChildProcess *item = new VncWebChildProcess(vncPort, webPort);
    QProcess *child = new QProcess(item);
    child->setProgram(executable);

    QList<QString> process_args;

    if (!noVncPlugin) {
        process_args.append(QString::number(vncPort));
        process_args.append("--");
        process_args.append(caQtDM_executable);
    }
    process_args.append("-server");
    if (noVncPlugin) {
        process_args.append("-novnc");
        if (noVncReadonly)
            process_args.append("-novnc_readonly");
    }
    process_args.append("-slave_server");
    process_args.append("-server_port");
    process_args.append(QString::number(vncPort));
    process_args.append("-web_server_port");
    process_args.append(QString::number(webPort));

    if(macros.trimmed().length() > 0) {
        process_args.append("-macro");
        process_args.append(macros.trimmed());
    }

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();

    if (!noVncPlugin) {
        environment.remove("LD_PRELOAD");
    }

    if (webTimeout != 0 && interactionBasedTimeout) {
        environment.insert("CAQTDM_TIMEOUT_HOURS", QString::number((double)webTimeout / 60 / 60));
        process_args.append("-web_interaction_timeout");
    } else if (webTimeout != 0) {
        process_args.append("-web_timeout");
        process_args.append(QString::number(webTimeout));
    }

    process_args.append("-host");
    process_args.append(webHost);

    if (WebLauncherManager::instance().isInitialized()) {
        process_args.append("-web_launcher_root_file");
        process_args.append(WebLauncherManager::instance().getRootFile());
    }

    process_args.append(file);

    child->setProcessEnvironment(environment);
    child->setArguments(process_args);

    child->start();

    item->setProcess(child);
    return item;
}
#endif

/**
 * callback will write value to device
 */
void CaQtDM_Lib::Callback_MessageButton(int type)
{
    QWidget *w1 = qobject_cast<QWidget *>(sender());
    caMessageButton *w = qobject_cast<caMessageButton *>(sender());

    if(!w->getAccessW()) return;
    if(!w->isEnabled()) return;

    if(type == 0) {         // pressed
        if(w->getPressMessage().size() > 0){
            bool isdouble = false;
            bool isint = false;
            bool ishex = false;
            w->getPressMessage().toDouble(&isdouble);
            w->getPressMessage().toLong(&isint);
            w->getPressMessage().toLong(&ishex,16);
            if (isdouble || isint ||ishex)
                TreatRequestedValue(w->getPV(), w->getPressMessage(), decimal, w1);
            else
                TreatRequestedValue(w->getPV(), w->getPressMessage(), string, w1);
        }
    } else if(type == 1) {  // released
        if(w->getReleaseMessage().size() > 0){
            bool isdouble = false;
            bool isint = false;
            bool ishex = false;
            w->getReleaseMessage().toDouble(&isdouble);
            w->getReleaseMessage().toLong(&isint);
            w->getReleaseMessage().toLong(&ishex,16);
            if (isdouble || isint ||ishex)
                TreatRequestedValue(w->getPV(), w->getReleaseMessage(), decimal, w1);
            else
                TreatRequestedValue(w->getPV(), w->getReleaseMessage(), string, w1);
        }
    }
}

/**
 * callback will write value to device
 */
void CaQtDM_Lib::Callback_ByteControllerClicked(int bit)
{
    QWidget *w1 = qobject_cast<QWidget *>(sender());
    caByteController *w = qobject_cast<caByteController *>(sender());

    if(!w->getAccessW()) return;

    long number = w->getValue();

    // bit set
    if(w->bitState(w->getValue(), bit)) {
        number &= ~(1 << bit);
        TreatOrdinaryValue(w->getPV(), (double) number, (int32_t) number, "",  w1);
        // bit not set
    } else {
        number |= 1 << bit;
        TreatOrdinaryValue(w->getPV(), (double) number, (int32_t) number, "",  w1);
    }
}

/**
 * callback will execute script
 */
void CaQtDM_Lib::Callback_ScriptButton()
{
#ifndef MOBILE
#ifdef WEB
    if (vncServer && !webAllowInsecureCaShellCommands) {
        QMessageBox::critical(this, "Error", "caScriptButton script execution disabled due to security reasons");
        return;
    }
#endif
    QString command = "";
    bool displayWindow,CloseOnExit0;
    caScriptButton *w = qobject_cast<caScriptButton *>(sender());
    command.append(w->getScriptCommand());

    // this here is needed because of unknown character transformations on Windows
    // Linux needed to be checked if this is better too. Reason for this implementation
    // are changes in QProcess qt4 -> qt5
#ifndef _MSC_VER
    if(w->getScriptParam().size() > 0) {
        command.append("\n\"");
        command.append( w->getScriptParam());
        command.append(" \"");
    }
#endif
    displayWindow = w->getDisplayShowExecution();
    CloseOnExit0 = w->getCloseExit0();

    if(w->getAccessW()) {
        processWindow *t = new processWindow(this, displayWindow,CloseOnExit0, w);
        connect(t, SIGNAL(processClose()), this, SLOT(processTerminated()));
#ifdef _MSC_VER
        t->setArguments(w->getScriptParam());
#endif
        t->start(command);
        w->setToolTip("process running, to kill use right mouse button");
        w->setAccessW(false);
        updateAccessCursor(w);
        w->setProcess(t);
    }
#endif
}

void CaQtDM_Lib::clearSelection(){

    QList<caLineDraw *> drawChild = parent()->findChildren<caLineDraw *>();
    foreach(caLineDraw *ld, drawChild){
        ld->clearSelection();
    }

    QList<caLineEdit *> editChild = parent()->findChildren<caLineEdit *>();
    foreach(caLineEdit *le, editChild){
        le->setSelection(0,0);
    }

    QList<caMultiLineString *> multiChild = parent()->findChildren<caMultiLineString *>();
    foreach(caMultiLineString *mls, multiChild){
        mls->clearSelection();
    }

    QList<caWaveTable *> waveChild = parent()->findChildren<caWaveTable *>();
    foreach(caWaveTable *wt, waveChild){
        wt->clearSelection();
    }

    QList<caTable *> tableChild = parent()->findChildren<caTable *>();

    foreach(caTable *tt, tableChild){
        tt->clearSelection();
    }
}

void CaQtDM_Lib::Callback_CopyMarked(){
    QWidget *widg = QApplication::focusWidget();

    caLineDraw *draw = qobject_cast<caLineDraw *>(widg);
    if(draw){ draw->copy(); }

    caWaveTable *wavetable = qobject_cast<caWaveTable *>(widg);
    if(wavetable){ wavetable->copy(); }

    caTable *table = qobject_cast<caTable *>(widg);
    if(table){ table->copy(); }

    caMultiLineString *multiline = qobject_cast<caMultiLineString *>(widg);
    if(multiline){ multiline->copy(); }

    caLineEdit *lineedit = qobject_cast<caLineEdit *>(widg);
    if(lineedit){ lineedit->copy(); }
}

void CaQtDM_Lib::processTerminated()
{
#ifndef MOBILE
    qCDebug(caQtDMLibLog) << "caQtDM -- process terminated callback";
    processWindow *t = qobject_cast<processWindow *>(sender());
    QWidget *w = t->getProcessCaller();
    caScriptButton *w1 = qobject_cast<caScriptButton *>(w);
    if(w1 != (QWidget*) Q_NULLPTR) {
        w1->setToolTip("process terminated !");
        w1->setAccessW(true);
        updateAccessCursor(w);
        w->setEnabled(true);
    }

    if(t != (processWindow *) Q_NULLPTR) {
        w1->setProcess((void*) Q_NULLPTR);
        t->deleteLater();
    }
#endif
}

/**
 * callback will execute a shell command
 */
void CaQtDM_Lib::Callback_TableDoubleClicked(const QString& pv)
{
    QString command = "";
    caTable *w = qobject_cast<caTable *>(sender());
    if(w->getScriptCommand().trimmed().size() == 0) return;
    command.append(w->getScriptCommand().trimmed());
    command.append(" ");
    command.append(pv);

    if(w->getScriptParam().size() > 0) {
        command.append( w->getScriptParam().trimmed());
    }
    command.append("&");
    shellCommand(command);
}


/**
 * callback will execute a shell command
 */

void CaQtDM_Lib::Callback_ShellCommandClicked(int indx)
{
#ifdef WEB
    if (vncServer && !webAllowInsecureCaShellCommands) {
        QMessageBox::critical(this, "Error", "caShellCommand execution disabled due to security reasons");
        return;
    }
#endif
    QString separator((QChar)27);

    caShellCommand *choice = qobject_cast<caShellCommand *>(sender());

    QStringList commands = choice->getFiles().split(";");
    QString argslist = choice->getArgs();

    // we can have a semicolum (;) between quotes that should not be treated as separator
    // this character is normally used for shell script, therefore instead of a real
    // parsing, replace the ; between quotes by an unused character \e
    bool inside = false;
    for (int i = 0; i < argslist.size(); i++) {
        if((!inside) && ((argslist.at(i) == QChar('\'')) || (argslist.at(i) == QChar('\"')))) {
            inside = true;
        } else if((inside) && ((argslist.at(i) == QChar('\'')) || (argslist.at(i) == QChar('\"')))) {
            inside = false;
        }
        if (inside) {
            if(argslist.at(i) == QChar(';')) {
                argslist.replace(i, 1, separator);
            }
        }
    }

    // split now the string with the ; separator
    QStringList args = argslist.split(";");

    if(indx < commands.count() && indx < args.count()) {
        QString command;
        command.append(commands[indx].trimmed());
        command.append(" ");
        // replace the special character back
        args[indx].replace(separator, ";");
        command.append(args[indx].trimmed());
        // replace medm by caQtDM
        command.replace("camedm ", "caQtDM ");
        command.replace("piomedm ", "caQtDM ");
        if(!command.contains("G_CS_medm")) command.replace("medm ", "caQtDM ");
        shellCommand(command);
    } else if(indx < commands.count()) {
        QString command;
        command.append(commands[indx].trimmed());
        shellCommand(command);
    }
}

void CaQtDM_Lib::shellCommand(QString command) {
#ifndef MOBILE
    QVariant macroString = this->property("macroString");
    command.replace("&T", thisFileShort);
    command.replace("&A", thisFileFull);
    command.replace("&S", macroString.toString());

    // Code to return the list of all the process variable given with &D

    if(command.contains("&D")) {
        QStringList pv_list;
        for (int i=0; i < mutexKnobDataP->GetMutexKnobDataSize(); i++) {
            knobData kData = mutexKnobDataP->GetMutexKnobData(i);
            if((kData.index != -1) && (myWidget == (QWidget*) kData.thisW)) {
                QString pv = kData.pv;
                pv_list.append(pv);
            }
        }
        pv_list.removeDuplicates();
        pv_list.sort();
        command.replace("&D", pv_list.join(" "));
    }
#if defined(linux) || defined(__FreeBSD__)
    int windid = this->winId();
    command.replace("&X", QString::number(windid));
#endif
    command = command.trimmed();
    postMessage(QtDebugMsg, (char*) qasc(command.trimmed()));
#if !defined(linux) && !defined(__APPLE__)
    if(command.endsWith("&")) command.remove(command.size()-1, 1);
    qCDebug(caQtDMLibLog) << "execute:" << command;
    proc = new myQProcess( this);
    proc->start(command.trimmed(), QIODevice::ReadWrite);
#else
    qCDebug(caQtDMLibLog) << "shellcommand" << command;
    // I had too many problems with QProcess start, use standard execl
    if(!command.endsWith("&")) command.append("&");
    int status = Execute((char*) qasc(command));
    if(status != 0) {
        QMessageBox::information(0,"FailedToStart or Error", command);
    }
#endif
#else
    Q_UNUSED(command);
#endif
}


void CaQtDM_Lib::closeWindow()
{
#ifdef WEB
    if (vncServer && !slaveServer && this->property("open_as_popupwindow").isNull()) {
        QMessageBox::critical(this, "Error", "Closing of main process prohibited in web mode");
        return;
    }
#endif
    this->close();
}

void CaQtDM_Lib::showNormalWindow()
{
    this->showNormal();
}

void CaQtDM_Lib::showMaxWindow()
{
    this->showMaximized();
}

void CaQtDM_Lib::showMinWindow()
{
    this->showMinimized();
}

void CaQtDM_Lib::showFullWindow()
{
    this->showFullScreen();
}
void CaQtDM_Lib::resizeFullWindow(QRect& q)
{
    QMainWindow *main = this->findChild<QMainWindow *>();
    main->centralWidget()->setProperty("mainlayoutPresent",true);
    main->centralWidget()->setProperty("allowResize",false);
    if ((q.width()>0)&&(q.height()>0)){
        this->resize(q.width(),q.height());
        this->setFixedSize(q.width(),q.height());
    }
}
/**
  * when closing the window, we will clear all associated monitors and free data
  */
void CaQtDM_Lib::closeEvent(QCloseEvent* ce)
{
    Q_UNUSED(ce);
    emit Signal_Closing();

    QString thisFileName =  property("fileString").toString().section('/',-1);
    QString launchFile = (QString) qgetenv("CAQTDM_LAUNCHFILE");
    bool doIosExit = false;
    if(thisFileName.contains(launchFile)) {
        doIosExit = true;
    }

    killTimer(loopTimerID);

    AllowsUpdate = false;

    for(int i=0; i < mutexKnobDataP->GetMutexKnobDataSize(); i++) {

        knobData kData =  mutexKnobDataP->GetMutexKnobData(i);

        if((kData.index != -1) && (myWidget == (QWidget*) kData.thisW)) {
            QString pv = kData.pv;
            QWidget* w = (QWidget*) kData.thisW;
            short soft = kData.soft;
            qCDebug(caQtDMLibLog) << pv << "clear monitor at" << i << "index="  << kData.index << "plugin" << kData.pluginName;
            if(soft) {
                mutexKnobDataP->RemoveSoftPV(pv, w, kData.index);
            } else {
               ControlsInterface * plugininterface = getControlInterface(kData.pluginName);
               if(plugininterface != (ControlsInterface *) 0) plugininterface->pvClearMonitor(&kData);
            }
            kData.index = -1;
            //kData.pv[0] = '\0';
            mutexKnobDataP->SetMutexKnobData(i, kData);
        }
    }

    Sleep::msleep(200);

    // get rid of memory that was allocated before for this window.
    // it has not been done previously, while otherwise in the datacallback
    // you can run into trouble
    for(int i=0; i < mutexKnobDataP->GetMutexKnobDataSize(); i++) {
        knobData *kPtr = mutexKnobDataP->GetMutexKnobDataPtr(i);
        if(kPtr != (knobData *) Q_NULLPTR) {
            if(myWidget == (QWidget*) kPtr->thisW) {
                ControlsInterface * plugininterface = getControlInterface(kPtr->pluginName);
                if(plugininterface != (ControlsInterface *) 0) plugininterface->pvFreeAllocatedData(kPtr);
                kPtr->thisW = (void*) Q_NULLPTR;
                if(kPtr->mutex != (QMutex *) Q_NULLPTR) {
                    QMutex *mutex = (QMutex *) kPtr->mutex;
                    delete mutex;
                    kPtr->mutex = (QMutex *) Q_NULLPTR;
                }
            }
        }
    }
    mutexKnobDataP->initHighestCountPV();

    // in case of network launcher, close the application when launcher window is closed
    if(doIosExit) {
        emit Signal_IosExit();
    }
}

/**
  * will display a context menu, composed of the channels associated to the object
  */
void CaQtDM_Lib::DisplayContextMenu(QWidget* w)
{
#ifdef WEB
    // prevent opening of popup windows when main window is to small to properly display them
    if (vncServer && (this->width() < 380 || this->height() < 560)) {
        if (WebSocketServer::instance().isInitialized())
            WebSocketServer::instance().sendError("Widget is to small to be able to display a context menu or popup window");
        return;
    }
#endif

    QMenu myMenu;
    QPoint cursorPos =QCursor::pos() ;
    QString ClassName;
    QString ObjectName;
    bool onMain = false;
    QString separator((QChar)27);
    int limitsDefault = false;
    int precMode = false;
    int limitsMode = false;
    int highLimit = false;
    int lowLimit = false;
    int Precision = 0;
    const char *caTypeStr[] = {"DBF_STRING", "DBF_INT", "DBF_FLOAT", "DBF_ENUM", "DBF_CHAR", "DBF_LONG", "DBF_DOUBLE"};
    char colMode[20] = {""};
    QString calcString = "";
    QString imageString = "";
    QStringList urlStrings;
    double limitsMax=0.0, limitsMin=0.0;
    bool validExecListItems = false;
    QStringList execListItems;
    int dataIndex = -1;

    urlStrings.clear();

    if(w != (QWidget*) Q_NULLPTR) {
        ClassName = w->metaObject()->className();
        ObjectName = w->objectName();
    } else {
        ClassName = "?";
        ObjectName = "?";
    }

    // execution list for context menu defined ?
    QString execList = (QString)  qgetenv("CAQTDM_EXEC_LIST");
    execList = execList.replace("://", separator+"//");

    if(!execList.isNull() && execList.size() > 0) {
#ifdef _MSC_VER
        execListItems= execList.split(";", SKIP_EMPTY_PARTS);
#else
        execListItems= execList.split(":", SKIP_EMPTY_PARTS);
#endif
        for(int i=0; i<execListItems.count(); i++) {
            validExecListItems = true;
        }
    }

    // perhaps still the older MEDM list is defined
    if(!validExecListItems) {
        execList = (QString)  qgetenv("MEDM_EXEC_LIST");

        if(!execList.isNull() && execList.size() > 0) {
#ifdef _MSC_VER
            execListItems= execList.split(";", SKIP_EMPTY_PARTS);
#else
            execListItems= execList.split(":", SKIP_EMPTY_PARTS);
#endif
            for(int i=0; i<execListItems.count(); i++) {
                validExecListItems = true;
            }
        }
    }

    if (w == nullptr) return;
    // get the monitor list back for this widget
    QVariant monitorList=w->property("MonitorList");
    QVariantList MonitorList = monitorList.toList();

    int nbMonitors = 0;
    if(MonitorList.size() > 0) nbMonitors = MonitorList.at(0).toInt();

    if(caWidgetInterface* wif = dynamic_cast<caWidgetInterface *>(w)) {   // any caWidget with caWidgetInterface
        QString pv[20];
        wif->getWidgetInfo(pv, nbMonitors, limitsDefault, precMode, limitsMode, Precision, colMode, limitsMax, limitsMin);
        // problem here not yet solved, while some major changes would be needed
        // the widget itsself only knows its pv's, however when same pv's come from different plugins, some confusion will occur
        for(int i=0; i<nbMonitors; i++) {
            knobData *kPtr =  mutexKnobDataP->getMutexKnobDataPV(w, pv[i]);
            if(kPtr != (knobData*) Q_NULLPTR) {
            MonitorList.append(kPtr->index);
        }
        }
        MonitorList.insert(0, nbMonitors);

    } else if(caImage* imageWidget = qobject_cast<caImage *>(w)) {
        GetDefinedCalcString(caImage, imageWidget, calcString);
        imageString =imageWidget->getImageCalc();

    } else if(caFrame* frameWidget = qobject_cast<caFrame *>(w)) {
        GetDefinedCalcString(caFrame, frameWidget, calcString);

    } else if(caInclude* includeWidget = qobject_cast<caInclude *>(w)) {
        GetDefinedCalcString(caInclude, includeWidget, calcString);

    } else if(caLabel* labelWidget = qobject_cast<caLabel *>(w)) {
        GetDefinedCalcString(caLabel, labelWidget, calcString);

    } else if(caLabelVertical* labelverticalWidget = qobject_cast<caLabelVertical *>(w)) {
        GetDefinedCalcString(caLabelVertical, labelverticalWidget, calcString);

    } else if(caGraphics* graphicsWidget = qobject_cast<caGraphics *>(w)) {
        GetDefinedCalcString(caGraphics, graphicsWidget, calcString);
        if(graphicsWidget->getColorMode() == caGraphics::Alarm) qstrncpy(colMode, "Alarm", sizeof(colMode));
        else qstrncpy(colMode, "Static", sizeof(colMode));

    } else if(caPolyLine* polylineWidget = qobject_cast<caPolyLine *>(w)) {
        GetDefinedCalcString(caPolyLine, polylineWidget, calcString);
        if(polylineWidget->getColorMode() == caPolyLine::Alarm) qstrncpy(colMode, "Alarm", sizeof(colMode));
        else qstrncpy(colMode, "Static", sizeof(colMode));

    } else if(caCalc* calcWidget = qobject_cast<caCalc *>(w)) {
        calcString = calcWidget->getCalc();

    } else if(caChoice* choiceWidget = qobject_cast<caChoice *>(w)) {
        if(choiceWidget->getColorMode() == caChoice::Alarm) qstrncpy(colMode, "Alarm", sizeof(colMode));
        else qstrncpy(colMode, "Static", sizeof(colMode));

    } else if(caLineEdit* lineeditWidget = qobject_cast<caLineEdit *>(w)) {
        if(lineeditWidget->getPrecisionMode() == caLineEdit::User) {
            precMode = true;
            Precision = lineeditWidget->getPrecision();
        }
        if(lineeditWidget->getLimitsMode() == caLineEdit::User) {
            limitsMode = true;
            limitsMax = lineeditWidget->getMaxValue();
            limitsMin = lineeditWidget->getMinValue();
        }
        if(lineeditWidget->getColorMode() == caLineEdit::Alarm_Default) qstrncpy(colMode, "Alarm", sizeof(colMode));
        else if(lineeditWidget->getColorMode() == caLineEdit::Alarm_Static) qstrncpy(colMode, "Alarm", sizeof(colMode));
        else qstrncpy(colMode, "Static", sizeof(colMode));

    } else if(caMultiLineString* multilinestringWidget = qobject_cast<caMultiLineString *>(w)) {
        if(multilinestringWidget->getColorMode() == caMultiLineString::Alarm_Default) qstrncpy(colMode, "Alarm", sizeof(colMode));
        else if(multilinestringWidget->getColorMode() == caMultiLineString::Alarm_Static) qstrncpy(colMode, "Alarm", sizeof(colMode));
        else qstrncpy(colMode, "Static", sizeof(colMode));

    } else if (caApplyNumeric* applynumericWidget = qobject_cast<caApplyNumeric *>(w)) {
        if(applynumericWidget->getPrecisionMode() == caApplyNumeric::User) {
            precMode = true;
            Precision = applynumericWidget->decDigits();
        } else if(nbMonitors > 0) {
            dataIndex = MonitorList.at(1).toInt();
            knobData *kPtr = mutexKnobDataP->GetMutexKnobDataPtr(dataIndex);
            if(kPtr != (knobData *) Q_NULLPTR) Precision =  kPtr->edata.precision;
        }

    } else if (caNumeric* numericWidget = qobject_cast<caNumeric *>(w)) {
        if(numericWidget->getPrecisionMode() == caNumeric::User) {
            precMode = true;
            Precision = numericWidget->decDigits();
        } else if(nbMonitors > 0) {
            dataIndex = MonitorList.at(1).toInt();
            knobData *kPtr = mutexKnobDataP->GetMutexKnobDataPtr(dataIndex);
            if(kPtr != (knobData *) Q_NULLPTR) Precision =  kPtr->edata.precision;
        }

    } else if (caSpinbox* spinboxWidget = qobject_cast<caSpinbox *>(w)) {
        if(spinboxWidget->getPrecisionMode() == caSpinbox::User) {
            precMode = true;
            Precision = spinboxWidget->decDigits();
        } else if(nbMonitors > 0) {
            dataIndex = MonitorList.at(1).toInt();
            knobData *kPtr = mutexKnobDataP->GetMutexKnobDataPtr(dataIndex);
            if(kPtr != (knobData *) Q_NULLPTR) Precision =  kPtr->edata.precision;
        }

    } else if(caSlider* sliderWidget = qobject_cast<caSlider *>(w)) {
        if(sliderWidget->getHighLimitMode() == caSlider::User) {
            highLimit = true;
            limitsMax = sliderWidget->getMaxValue();
        }
        if(sliderWidget->getLowLimitMode() == caSlider::User) {
            lowLimit = true;
            limitsMin = sliderWidget->getMinValue();
        }
        if(sliderWidget->getPrecisionMode() == caSlider::User) {
            precMode = true;
            Precision = sliderWidget->getPrecision();
        } else if(nbMonitors > 0) {
            dataIndex = MonitorList.at(1).toInt();
            knobData *kPtr = mutexKnobDataP->GetMutexKnobDataPtr(dataIndex);
            if(kPtr != (knobData *) Q_NULLPTR) Precision =  kPtr->edata.precision;
        }
        if((sliderWidget->getColorMode() == caSlider::Alarm_Default) || (sliderWidget->getColorMode() == caSlider::Alarm_Static)) qstrncpy(colMode, "Alarm", sizeof(colMode));
        else qstrncpy(colMode, "Static", sizeof(colMode));

    } else if(caThermo* thermoWidget = qobject_cast<caThermo *>(w)) {
        if(thermoWidget->getLimitsMode() == caThermo::User) {
            limitsMode = true;
            limitsMax = thermoWidget->maxValue();
            limitsMin = thermoWidget->minValue();
        }
        if(nbMonitors > 0) {
            dataIndex = MonitorList.at(1).toInt();
            knobData *kPtr = mutexKnobDataP->GetMutexKnobDataPtr(dataIndex);
            if(kPtr != (knobData *) Q_NULLPTR) {
               if(kPtr->edata.lower_disp_limit == kPtr->edata.upper_disp_limit) {
                limitsDefault = true;
                limitsMax = thermoWidget->maxValue();
                limitsMin = thermoWidget->minValue();
               }
            }
        }
        if((thermoWidget->getColorMode() == caThermo::Alarm_Default) || (thermoWidget->getColorMode() == caThermo::Alarm_Static)) qstrncpy(colMode, "Alarm", sizeof(colMode));
        else qstrncpy(colMode, "Static", sizeof(colMode));

    } else if(caByte* byteWidget = qobject_cast<caByte *>(w)) {
        if(byteWidget->getColorMode() == caByte::Alarm) qstrncpy(colMode, "Alarm", sizeof(colMode));
        else qstrncpy(colMode, "Static", sizeof(colMode));

    } else if(caByteController* bytecontrollerWidget = qobject_cast<caByteController *>(w)) {
        if(bytecontrollerWidget->getColorMode() == caByteController::Alarm) qstrncpy(colMode, "Alarm", sizeof(colMode));
        else qstrncpy(colMode, "Static", sizeof(colMode));

    } else if(caScriptButton* scriptbuttonWidget =  qobject_cast< caScriptButton *>(w)) {
        Q_UNUSED(scriptbuttonWidget);
        // add action : kill associated process if running
        myMenu.addAction(KILLPROCESS);

    } else if(caMimeDisplay* mimeWidget = qobject_cast<caMimeDisplay *>(w)) {
        urlStrings = mimeWidget->getFilesList();

    } else if(ClassName.contains("QE")) {
        qCDebug(caQtDMLibLog) << "treat" << w;

        // must be mainwindow
    } else if((w==myWidget->parent()->parent()) && (nbMonitors == 0)) {
        qCDebug(caQtDMLibLog) << "must be mainwindow?" << w << myWidget->parent()->parent();
        onMain = true;
        myMenu.addAction(UNDEFINEDMACROS);
        myMenu.addAction(GLOBALSHORTCUTS);
        if (!vncServer) {
            myMenu.addAction(PRINTWINDOW);
            myMenu.addAction(RAISEWINDOW);
        }
        myMenu.addAction(RELOADWINDOW);
        myMenu.addAction(INCLUDES);
    }

    // add some more actions
    if(caWidgetInterface* wif = dynamic_cast<caWidgetInterface *>(w)) { // any caWidget with caWidgetInterface
        wif->createContextMenu(myMenu);

        // for the camera cameraWidget
    } else if(caCamera * cameraWidget = qobject_cast< caCamera *>(w)) {
        QAction *menuAction;
        menuAction = myMenu.addAction(TOGGLESIZE);
        menuAction->setCheckable(true);
        if(cameraWidget->getFitToSize() == caCamera::Yes) menuAction->setChecked(true);
        else  menuAction->setChecked(false);
        addColorTableActions;

        // for the scan2d scan2dWidget
    } else if(caScan2D * scan2dWidget = qobject_cast< caScan2D *>(w)) {
        QAction *menuAction;
        menuAction = myMenu.addAction(TOGGLESIZE);
        menuAction->setCheckable(true);
        if(scan2dWidget->getFitToSize() == caScan2D::Yes) menuAction->setChecked(true);
        else  menuAction->setChecked(false);
        addColorTableActions;

        // for the waterfall waterfallplotWidget
    } else if(caWaterfallPlot * waterfallplotWidget = qobject_cast< caWaterfallPlot *>(w)) {
        Q_UNUSED(waterfallplotWidget);
        addColorTableActions;

        // for the slider
    } else if(caSlider * sliderWidget = qobject_cast< caSlider *>(w)) {
        Q_UNUSED(sliderWidget);
        myMenu.addAction(GETINFO);
        myMenu.addAction(CHANGEVALUE);

    } else if(caWaveTable* wavetableWidget = qobject_cast<caWaveTable *>(w)) {
        wavetableWidget->clearSelection();
        myMenu.addAction(GETINFO);
        myMenu.addAction(COPYDATACSV);
        if (wavetableWidget->getAccessW()) {
            myMenu.addAction(PASTEDATACSV);
        }

        // all other widgets
    } else if(!onMain){
        // construct info for the pv we are pointing at
        myMenu.addAction(GETINFO);
    }

    // for stripplot add one more action
    if(caStripPlot* stripplotWidget = qobject_cast<caStripPlot *>(w)) {
        Q_UNUSED(stripplotWidget);
        myMenu.addAction(CHANGEAXIS);
    }

    // for cartesian plot add more actions
    if(caCartesianPlot* cartesianplotWidget = qobject_cast<caCartesianPlot *>(w)) {
        Q_UNUSED(cartesianplotWidget);
        myMenu.addAction(CHANGEAXIS);
        myMenu.addAction(QWhatsThis::createAction());
        myMenu.addAction(RESETZOOM);
        myMenu.addAction(COPYIMAGE);
    }

    // for catextentry add filedialog
    if(caTextEntry* catextentryWidget = qobject_cast<caTextEntry *>(w)) {
        if(catextentryWidget->getAccessW()) {
            if(nbMonitors > 0) {
                dataIndex = MonitorList.at(1).toInt();
                knobData *kPtr = mutexKnobDataP->GetMutexKnobDataPtr(dataIndex);
                if((kPtr != (knobData *) Q_NULLPTR) && (strlen(kPtr->pv) > 0)) {
                    myMenu.addAction(INPUTDIALOG);
                    if(((kPtr->edata.fieldtype == caSTRING) || (kPtr->edata.fieldtype == caCHAR)) && !vncServer) {
                        myMenu.addAction(FILEDIALOG);
                    }
                }
            }
        }
    }

    // for some widgets one more action
    if(caSlider * widget = qobject_cast< caSlider *>(w)) {Q_UNUSED(widget); myMenu.addAction(CHANGELIMITS);}
    if(caLineEdit* widget = qobject_cast<caLineEdit *>(w)) {Q_UNUSED(widget); myMenu.addAction(CHANGELIMITS);}
    if(caThermo* widget = qobject_cast<caThermo *>(w)){Q_UNUSED(widget);  myMenu.addAction(CHANGELIMITS);}
    if(caNumeric* widget = qobject_cast<caNumeric *>(w)) {Q_UNUSED(widget); myMenu.addAction(CHANGELIMITS);}
    if(caApplyNumeric* widget = qobject_cast<caApplyNumeric *>(w)) {Q_UNUSED(widget); myMenu.addAction(CHANGELIMITS);}
    if(caSpinbox* widget = qobject_cast<caSpinbox *>(w)) {Q_UNUSED(widget); myMenu.addAction(CHANGELIMITS);}
    if(caLinearGauge* widget = qobject_cast<caLinearGauge *>(w)) {Q_UNUSED(widget); myMenu.addAction(CHANGELIMITS);}
    if(caCircularGauge* widget = qobject_cast<caCircularGauge *>(w)) {Q_UNUSED(widget); myMenu.addAction(CHANGELIMITS);}
    if(caMeter* widget = qobject_cast<caMeter *>(w)) {Q_UNUSED(widget); myMenu.addAction(CHANGELIMITS);}

    // add to context menu, the actions requested by the environment variable caQtDM_EXEC_LIST
    if(validExecListItems) {
        for(int i=0; i<execListItems.count(); i++) {
            execListItems[i] = execListItems[i].replace(separator+"//", "://");
            QStringList item = execListItems[i].split(";");
            if(item.count() > 1) {
                if(!item[1].contains("&P") && onMain) myMenu.addAction(item[0]);
                else if(item[1].contains("&P") && !onMain && nbMonitors > 0) myMenu.addAction(item[0]);
            }
        }
    }

    connect(this, &CaQtDM_Lib::Signal_Closing, &myMenu, &QWidget::close);
    QAction* selectedItem = myMenu.exec(cursorPos);

    if (selectedItem) {
        if(selectedItem->text().contains(KILLPROCESS)) {
            if(caScriptButton* scriptbuttonWidget =  qobject_cast< caScriptButton *>(w)) {
#ifndef MOBILE
                processWindow *t= (processWindow *) scriptbuttonWidget->getProcess();
                if(t != (processWindow *) Q_NULLPTR) t->tryTerminate();
#else
                Q_UNUSED(scriptbuttonWidget);
#endif
            }

        } else  if(selectedItem->text().contains(RAISEWINDOW)) {
            QMainWindow *mainWindow = (QMainWindow *) this->parentWidget();
            mainWindow->showNormal();

            if(messageWindowP != (MessageWindow *) Q_NULLPTR) {
                messageWindowP->setWindowState( (windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
                messageWindowP->raise();
                messageWindowP->activateWindow();
            }

        } else  if(selectedItem->text().contains(TOGGLESIZE)) {
            if(caCamera * cameraWidget = qobject_cast< caCamera *>(w)) {
                if(cameraWidget->getFitToSize() == caCamera::Yes) cameraWidget->setFitToSize(caCamera::No);
                else cameraWidget->setFitToSize(caCamera::Yes);
            }
            else if(caScan2D * scan2dWidget = qobject_cast< caScan2D *>(w)) {
                if(scan2dWidget->getFitToSize() == caScan2D::Yes) scan2dWidget->setFitToSize(caScan2D::No);
                else scan2dWidget->setFitToSize(caScan2D::Yes);
            }

        } else  if(selectedItem->text().contains(SETWAVELEN)) {
            if(caCamera * cameraWidget = qobject_cast< caCamera *>(w)) cameraWidget->setColormap(caCamera::mono_to_wavelength);
            else if(caScan2D * scan2dWidget = qobject_cast< caScan2D *>(w)) scan2dWidget->setColormap(caScan2D::spectrum_wavelength);
            else if(caWaterfallPlot * waterfallplotWidget = qobject_cast< caWaterfallPlot *>(w)) waterfallplotWidget->setColormap(caWaterfallPlot::spectrum_wavelength);

        } else  if(selectedItem->text().contains(SETHOT)) {
            if(caCamera * cameraWidget = qobject_cast< caCamera *>(w)) cameraWidget->setColormap(caCamera::mono_to_hot);
            else if(caScan2D * scan2dWidget = qobject_cast< caScan2D *>(w)) scan2dWidget->setColormap(caScan2D::spectrum_hot);
            else if(caWaterfallPlot * waterfallplotWidget = qobject_cast< caWaterfallPlot *>(w)) waterfallplotWidget->setColormap(caWaterfallPlot::spectrum_hot);

        } else  if(selectedItem->text().contains(SETHEAT)) {
            if(caCamera * cameraWidget = qobject_cast< caCamera *>(w)) cameraWidget->setColormap(caCamera::mono_to_heat);
            else if(caScan2D * scan2dWidget = qobject_cast< caScan2D *>(w)) scan2dWidget->setColormap(caScan2D::spectrum_heat);
            else if(caWaterfallPlot * waterfallplotWidget = qobject_cast< caWaterfallPlot *>(w)) waterfallplotWidget->setColormap(caWaterfallPlot::spectrum_heat);

        } else  if(selectedItem->text().contains(SETJET)) {
            if(caCamera * cameraWidget = qobject_cast< caCamera *>(w)) cameraWidget->setColormap(caCamera::mono_to_jet);
            else if(caScan2D * scan2dWidget = qobject_cast< caScan2D *>(w)) scan2dWidget->setColormap(caScan2D::spectrum_jet);
            else if(caWaterfallPlot * waterfallplotWidget = qobject_cast< caWaterfallPlot *>(w)) waterfallplotWidget->setColormap(caWaterfallPlot::spectrum_jet);

        } else  if(selectedItem->text().contains(SETCUSTOM)) {
            if(caCamera * cameraWidget = qobject_cast< caCamera *>(w)) cameraWidget->setColormap(caCamera::mono_to_custom);
            else if(caScan2D * scan2dWidget = qobject_cast< caScan2D *>(w)) scan2dWidget->setColormap(caScan2D::spectrum_custom);
            else if(caWaterfallPlot * waterfallplotWidget = qobject_cast< caWaterfallPlot *>(w)) waterfallplotWidget->setColormap(caWaterfallPlot::spectrum_custom);

        } else  if(selectedItem->text().contains(SETGREY)) {
            if(caCamera * cameraWidget = qobject_cast< caCamera *>(w)) cameraWidget->setColormap(caCamera::color_to_mono);
            else if(caScan2D * scan2dWidget = qobject_cast< caScan2D *>(w)) scan2dWidget->setColormap(caScan2D::grey);
            else if(caWaterfallPlot * waterfallplotWidget = qobject_cast< caWaterfallPlot *>(w)) waterfallplotWidget->setColormap(caWaterfallPlot::grey);

        } else  if(selectedItem->text().contains(SETASIS)) {
            if(caCamera * cameraWidget = qobject_cast< caCamera *>(w)) cameraWidget->setColormap(caCamera::as_is);
            else if(caScan2D * scan2dWidget = qobject_cast< caScan2D *>(w)) scan2dWidget->setColormap(caScan2D::grey);
            else if(caWaterfallPlot * waterfallplotWidget = qobject_cast< caWaterfallPlot *>(w)) waterfallplotWidget->setColormap(caWaterfallPlot::grey);

        } else  if(selectedItem->text().contains(INCLUDES)) {
            QString info;
            info.append(InfoPrefix);
            int totalTime=0;
            info.append("<strong>the indicated times are approxative</strong><br><br>");
            QMap<QString, includeData>::const_iterator data = includeFilesList.constBegin();
            while (data != includeFilesList.constEnd()) {
                includeData value = data.value();
                info.append(tr("%1 %2 <strong>%3</strong> times, average load time=<strong>%4ms</strong> total load time=<strong>%5ms</strong><br>").
                            arg(data.key()).arg(value.text).arg(value.count).arg(value.ms).arg(value.ms * value.count));
                totalTime = totalTime + value.count * value.ms;
                ++data;
            }
            qCDebug(caQtDMLibLog) << totalTime;

            info.append(InfoPostfix);
            myMessageBox box(this);
            box.setText("<html>" + info + "</html>");
            connect(this, &CaQtDM_Lib::Signal_Closing, &box, &QWidget::close);
            box.exec();

        } else  if(selectedItem->text().contains(GETINFO)) {
            QString info;
            info.append(InfoPrefix);
            info.append("---------------------------------------------------------------------------------------<br>");

            info.append("Object: ");
            info.append(ObjectName);
            if(caCalc* widget = qobject_cast<caCalc *>(w)) {info.append(", Variable: " + widget->getVariable());}
            info.append("<br>");

            if(!urlStrings.isEmpty()) {
                info.append("<br>Commands:<br>");
                for(int i=0; i<urlStrings.count(); i++) {
                info.append(QString::number(i+1));
                info.append(": ");
#if QT_VERSION > 0x050000
                info.append(urlStrings.at(i).toHtmlEscaped());
#else
                info.append(Qt::escape(urlStrings.at(i)));
#endif
                info.append("<br>");
                }
            }

            if(!calcString.isEmpty()) {
                info.append("<br>");
                if(caCalc* widget = qobject_cast<caCalc *>(w)) {Q_UNUSED(widget); info.append("Calc: "); } else info.append("VisibilityCalc: ");
#if QT_VERSION > 0x050000
                info.append(QString(calcString).toHtmlEscaped());
#else
                info.append(Qt::escape(calcString));
#endif
                info.append("<br>");
            }
            if(!imageString.isEmpty()) {
                info.append("ImageCalc: ");
#if QT_VERSION > 0x050000
                info.append(QString(imageString).toHtmlEscaped());
#else
                info.append(Qt::escape(imageString));
#endif
                info.append("<br>");
            }

            info.append("<br>! configuration values are only fetched at panel start<br>");

            for(int i=0; i< nbMonitors; i++) {

                dataIndex = MonitorList.at(i+1).toInt();
                knobData *kPtr =  mutexKnobDataP->GetMutexKnobDataPtr(dataIndex);

                if((kPtr != (knobData *) Q_NULLPTR)) {
                    char asc[MAX_STRING_LENGTH] = {'\0'};
                    char timestamp[TIMESTAMP_STRING_LENGTH] = {'\0'};
                    char description[MAX_STRING_LENGTH] = {'\0'};
                    info.append("<br>");
                    info.append(kPtr->pv);

                    info.append("<br>");
                    info.append("Plugin: ");
                    info.append(kPtr->pluginName);
                    if(strlen(kPtr->pluginFlavor) > 0) {
                        info.append(" Flavor: ");
                        info.append(kPtr->pluginFlavor);
                    }
                    ControlsInterface * plugininterface = getControlInterface(kPtr->pluginName);
                    if(plugininterface == (ControlsInterface *) Q_NULLPTR) {
                         if(!kPtr->soft)info.append(" : not loaded");
                    } else {
                         info.append(" : loaded");
                    }
                    if((plugininterface != (ControlsInterface *) Q_NULLPTR) || (kPtr->soft)) {
                        if(kPtr->soft) {
                            info.append(" : soft channel from caCalc");
                        } else if(kPtr->edata.connected) {
                            info.append(" & connected");
                        } else {
                            info.append(" but not connected");
                        }

                        if((kPtr->soft) || (kPtr->edata.connected)) {

                        info.append("<br>=====================================");
                        if(!kPtr->soft) {
                            info.append("<br>Description: ");
                            if(plugininterface != (ControlsInterface *) Q_NULLPTR) {
                                if (qstrcmp(kPtr->pluginName, "archiveHTTP") == 0) {
                                        char specificKey[MAX_STRING_LENGTH];
                                        // Use the key created by archiverCommon to distinguish data for the same pv but different widgets and curves.
                                        qstrncpy(specificKey, reinterpret_cast<char*>(kPtr->edata.info), sizeof(specificKey));
                                        plugininterface->pvGetDescription(specificKey, description);
                                } else {
                                    plugininterface->pvGetDescription(kPtr->pv, description);
                                }
                            }
                            info.append(description);
                            info.append("<br>"); // for Timestamp
                            if(plugininterface != (ControlsInterface *) Q_NULLPTR) plugininterface->pvGetTimeStamp(kPtr->pv, timestamp);
                            info.append(timestamp);
                        }
                        info.append("<br>Type: ");
                        info.append(caTypeStr[kPtr->edata.fieldtype]);

                        snprintf(asc, MAX_STRING_LENGTH, "<br>Count: %d", kPtr->edata.valueCount);
                        info.append(asc);

                        info.append("<br>Value: ");
                        const std::string edataUnits = QString::fromLatin1((const char*)&kPtr->edata.units,strlen(kPtr->edata.units)).toStdString();
                        switch (kPtr->edata.fieldtype) {
                        case caCHAR:
                            snprintf(asc, MAX_STRING_LENGTH, "%ld (0x%lx)", kPtr->edata.ivalue, kPtr->edata.ivalue);
                            info.append(asc);
                            break;
                        case caSTRING:
                            if(kPtr->edata.valueCount <= 1) {
                                info.append((char*)kPtr->edata.dataB);
                            } else {
                                QString States((char*) kPtr->edata.dataB);
                                QStringList list = States.split((QChar)27);
                                for(int j=0; j<list.count(); j++) {
                                    snprintf(asc, MAX_STRING_LENGTH, "<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;%d %s", j, qasc(list.at(j)));
                                    info.append(asc);
                                }
                            }
                            break;

                        case caENUM:{
                            snprintf(asc, MAX_STRING_LENGTH, "%ld %s", kPtr->edata.ivalue, edataUnits.c_str());
                            info.append(asc);
                            snprintf(asc, MAX_STRING_LENGTH, "<br>nbStates: %d", kPtr->edata.enumCount);
                            info.append(asc);
                            info.append("<br>States: ");
                            if(kPtr->edata.enumCount > 0) {
                                QString States((char*) kPtr->edata.dataB);
                                //QStringList list = States.split(";");
                                QStringList list = States.split((QChar)27);
                                for(int j=0; j<list.count(); j++) {
                                    snprintf(asc, MAX_STRING_LENGTH, "<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;%d %s", j, qasc(list.at(j)));
                                    info.append(asc);
                                }
                            }
                            break;
                        }
                        case caINT:
                        case caLONG:
                            snprintf(asc, MAX_STRING_LENGTH, "%ld (0x%lx) %s", kPtr->edata.ivalue, kPtr->edata.ivalue, edataUnits.c_str());
                            info.append(asc);
                            break;
                        case caFLOAT:
                        case caDOUBLE:
                            snprintf(asc, MAX_STRING_LENGTH, "%lf %s", kPtr->edata.rvalue, edataUnits.c_str());
                            info.append(asc);
                            break;

                        default:
                            snprintf(asc, MAX_STRING_LENGTH, "unhandled epics type");
                            info.append(asc);
                        }

                        // severity
                        switch(kPtr->edata.severity) {
                        case NO_ALARM:
                            info.append("<br>Severity: NO_ALARM");
                            break;
                        case MINOR_ALARM:
                            info.append("<br>Severity: MINOR_ALARM");
                            break;
                        case MAJOR_ALARM:
                            info.append("<br>Severity: MAJOR_ALARM");
                            break;
                        case INVALID_ALARM:
                            info.append("<br>Severity: INVALID_ALARM");
                            break;
                        case NOTCONNECTED:
                            info.append("<br>Severity: NOT_CONNECTED");
                            break;
                        default:
                            info.append("<br>Severity: UNKNOWN");
                        }

                        // status
                        info.append("<br>Alarm status: ");
                        if(kPtr->edata.status <= 22) info.append(AlarmStatusStrings[kPtr->edata.status]);
                        else info.append("???");

                        // ioc
                        info.append("<br>IOC: ");
                        info.append(kPtr->edata.fec);

                        // precision
                        if(!precMode) {
                            snprintf(asc, MAX_STRING_LENGTH, "<br>Precision (channel) :%d ", kPtr->edata.precision);
                            Precision = kPtr->edata.precision;
                        } else {
                            snprintf(asc, MAX_STRING_LENGTH, "<br>Precision (user) :%d ", Precision);
                        }

                        info.append(asc);
                        if(ClassName.contains("Gauge")) {
                            snprintf(asc, MAX_STRING_LENGTH, "not used for scale") ;
                            info.append(asc);
                        }

                        // limits
                        if(limitsMode) {
                            snprintf(asc, MAX_STRING_LENGTH, "<br>User alarm: MIN:%g  MAX:%g ", limitsMin, limitsMax);
                            info.append(asc);
                        }
                        if(highLimit){
                            snprintf(asc, MAX_STRING_LENGTH, "<br>User alarm MAX:%g ", limitsMax);
                            info.append(asc);
                        }
                        if(lowLimit) {
                            snprintf(asc, MAX_STRING_LENGTH, "<br>User alarm MIN:%g ", limitsMin);
                            info.append(asc);
                        }
                        if(limitsDefault) {
                            snprintf(asc, MAX_STRING_LENGTH, "<br>Default limits: MIN:%g  MAX:%g ",limitsMin, limitsMax);
                            info.append(asc);
                        }
                        snprintf(asc, MAX_STRING_LENGTH, "<br>LOPR:%g  HOPR:%g ", kPtr->edata.lower_disp_limit, kPtr->edata.upper_disp_limit);
                        info.append(asc);
                        snprintf(asc, MAX_STRING_LENGTH, "<br>LOLO:%g  HIHI:%g ", kPtr->edata.lower_alarm_limit, kPtr->edata.upper_alarm_limit);
                        info.append(asc);
                        snprintf(asc, MAX_STRING_LENGTH, "<br>LOW :%g  HIGH:%g ", kPtr->edata.lower_warning_limit, kPtr->edata.upper_warning_limit);
                        info.append(asc);
                        snprintf(asc, MAX_STRING_LENGTH, "<br>DRVL:%g  DRVH:%g ",kPtr->edata.lower_ctrl_limit, kPtr->edata.upper_ctrl_limit);
                        info.append(asc);
                        info.append("<br>");

                        //colormode
                        if(strlen(colMode) > 0) {
                            snprintf(asc,  MAX_STRING_LENGTH, "Colormode: %s", colMode);
                            info.append(asc);
                            info.append("<br>");
                        }

                        // access
                        if(kPtr->edata.accessR==1 && kPtr->edata.accessW==1) snprintf(asc, MAX_STRING_LENGTH, "Access: ReadWrite");
                        else if(kPtr->edata.accessR==1 && kPtr->edata.accessW==0) snprintf(asc, MAX_STRING_LENGTH, "Access: ReadOnly");
                        else if(kPtr->edata.accessR==0 && kPtr->edata.accessW==1) snprintf(asc, MAX_STRING_LENGTH, "Access: WriteOnly"); // not possible
                        else snprintf(asc, MAX_STRING_LENGTH, "Access: NoAccess");
                        info.append(asc);
                        info.append("<br>");
                    }
                    }
                }
            }
            info.append(InfoPostfix);

            myMessageBox box(this);
            box.setText("<!DOCTYPE html><html>" + info + "</html>");
            connect(this, &CaQtDM_Lib::Signal_Closing, &box, &QWidget::close);
            box.exec();

        // add a file dialog to simplify user path+file input
        } else if(selectedItem->text().contains(FILEDIALOG)) {
            QFileDialog dialog(this);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            dialog.setFileMode(QFileDialog::DirectoryOnly);
#else
            dialog.setOption(QFileDialog::ShowDirsOnly, true);
#endif
            if (dialog.exec()) {
                QStringList fileNames = dialog.selectedFiles();
                if(!fileNames[0].isEmpty()) {
                    if(caTextEntry* textentryWidget = qobject_cast<caTextEntry *>(w)) {
                        FormatType fType = (FormatType) textentryWidget->getFormatType();
                        TreatRequestedValue(textentryWidget->getPV(), fileNames[0], fType, w);
                    }
                }
            }

        } else if(selectedItem->text().contains(INPUTDIALOG)) {
            bool ok;
            QString text = QInputDialog::getText(this, tr("Input data"), tr("Input:"), QLineEdit::Normal,"", &ok);
            if (ok && !text.isEmpty()) {
                if(caTextEntry* textentryWidget = qobject_cast<caTextEntry *>(w)) {
                   FormatType fType = (FormatType) textentryWidget->getFormatType();
                   TreatRequestedValue(textentryWidget->getPV(), text, fType, w);
                }
            }

        } else if(selectedItem->text().contains(PRINTWINDOW)) {
            print();

        } else if(selectedItem->text().contains(GLOBALSHORTCUTS)) {
            GlobalShortcutWindow();

        } else if(selectedItem->text().contains(UNDEFINEDMACROS)) {
            UndefinedMacrosWindow();

        } else if(selectedItem->text().contains(RELOADWINDOW)) {
            emit Signal_ReloadWindowL();

        } else if(selectedItem->text().contains(CHANGEAXIS)) {
            if(caStripPlot* stripplotWidget = qobject_cast<caStripPlot *>(w)) {
                limitsStripplotDialog dialog(stripplotWidget, mutexKnobDataP, "stripplot modifications", this);
                connect(this, &CaQtDM_Lib::Signal_Closing, &dialog, &QWidget::close);
                dialog.exec();
            } else if(caCartesianPlot* cartesianplotWidget = qobject_cast<caCartesianPlot *>(w)) {
                limitsCartesianplotDialog dialog(cartesianplotWidget, mutexKnobDataP, "cartesianplot modifications", this);
                connect(this, &CaQtDM_Lib::Signal_Closing, &dialog, &QWidget::close);
                dialog.exec();
                if (dialog.getChannelScalingWasReset()) {
                   char asc[MAX_STRING_LENGTH];
                   snprintf(asc, MAX_STRING_LENGTH, "Selected scaling \"channel\" has been reset to \"auto\" in cartesian plot: \"%s\" because the limits provided by the PV are invalid.", qasc(cartesianplotWidget->objectName()));
                   postMessage(QtWarningMsg, asc);
                }
            }
        } else if(selectedItem->text().contains(RESETZOOM)) {
            if(caCartesianPlot* cartesianplotWidget = qobject_cast<caCartesianPlot *>(w)) {
                cartesianplotWidget->resetZoom();

                /*************************************************/
                // oops, case of axis defines by channel forgotten
                if(cartesianplotWidget->getXscaling() == caCartesianPlot::Channel) {

                    QString pvs =cartesianplotWidget->getPV(0);
                    QStringList vars = pvs.split(";");
                    if((vars.size()== 2) || (vars.at(1).trimmed().length() > 0)) {
                        knobData *kPtr =  mutexKnobDataP->getMutexKnobDataPV(w, vars.at(0).trimmed());
                        if(kPtr != (knobData *) Q_NULLPTR) {
                            if(kPtr->edata.lower_disp_limit != kPtr->edata.upper_disp_limit) {
                                cartesianplotWidget->setScaleX(kPtr->edata.lower_disp_limit, kPtr->edata.upper_disp_limit);
                            } else {
                                cartesianplotWidget->setXscaling(caCartesianPlot::Auto);
                            }
                        }
                    }
                }
                if(cartesianplotWidget->getYscaling() == caCartesianPlot::Channel) {
                    /*************************************************/
                    // oops, case of axis defines by channel forgotten
                    QString pvs =cartesianplotWidget->getPV(0);
                    QStringList vars = pvs.split(";");
                    if((vars.size()== 2) || (vars.at(1).trimmed().length() > 0)) {
                        knobData *kPtr =  mutexKnobDataP->getMutexKnobDataPV(w, vars.at(1).trimmed());
                        if(kPtr != (knobData *) 0) {
                            if(kPtr->edata.lower_disp_limit != kPtr->edata.upper_disp_limit) {
                                cartesianplotWidget->setScaleY(kPtr->edata.lower_disp_limit, kPtr->edata.upper_disp_limit);
                            } else {
                                cartesianplotWidget->setYscaling(caCartesianPlot::Auto);
                            }
                        }
                    }

                }
                /*************************************************/
            }

        } else if(selectedItem->text().contains(CHANGEVALUE)) {
            if(caSlider* sliderWidget = qobject_cast<caSlider *>(w)) {
                sliderDialog dialog(sliderWidget, mutexKnobDataP, "slider Increment/Value change", this);
                connect(this, &CaQtDM_Lib::Signal_Closing, &dialog, &QWidget::close);
                dialog.exec();
            }

        } else if(selectedItem->text().contains(CHANGELIMITS)) {
            limitsDialog dialog(w, mutexKnobDataP, "Limits/Precision change", this);
            connect(this, &CaQtDM_Lib::Signal_Closing, &dialog, &QWidget::close);
            dialog.exec();
        } else if(selectedItem->text().contains(COPYIMAGE)) {
            if(caCartesianPlot* cartesianplotWidget = qobject_cast<caCartesianPlot *>(w)) {
                cartesianplotWidget->copyImage();
            }
        } else if(selectedItem->text().contains(COPYDATACSV)) {
            if(caWaveTable* wavetableWidget = qobject_cast<caWaveTable *>(w)) {
                wavetableWidget->copyDataCSV();
            }
        } else if(selectedItem->text().contains(PASTEDATACSV)) {
            if(caWaveTable* wavetableWidget = qobject_cast<caWaveTable *>(w)) {
                wavetableWidget->pasteDataCSV();
            }
        } else {
            // any action from environment ?
            if(validExecListItems) {
                for(int i=0; i<execListItems.count(); i++) {
                    QStringList item = execListItems[i].split(";");
                    if(item.count() > 1) {
                        if(selectedItem->text().contains(item[0])) {
                            QString command = item[1];
                            if(command.contains("&P") && nbMonitors > 0) {
                                dataIndex = MonitorList.at(1).toInt();
                                knobData *kPtr =  mutexKnobDataP->GetMutexKnobDataPtr(dataIndex);
                                if(kPtr != (knobData *) 0) command.replace("&P", kPtr->pv);
                                // Remove internal pseudo-extensions used by caCartesianPlots with archived channels
                                command.replace(".X", "");
                                command.replace(".Y", "");
                                command.replace(".minX", "");
                                command.replace(".maxX", "");
                                command.replace(".minY", "");
                                command.replace(".maxY", "");
                            }
                            shellCommand(command);
                        }
                    }
                }
            }
        }

    } else {
        // nothing was choosen
    }
}

/**
  * callback will call the above routine
  */
void CaQtDM_Lib::ShowContextMenu(const QPoint& position) // this is a slot
{
    Q_UNUSED(position);
    QWidget *w = qobject_cast<QWidget *>(sender());
    qCDebug(caQtDMLibLog) << "showcontextmenu" << qobject_cast<QWidget *>(sender());

    // very special case, where caRelatedDisplay is on top and recovers other ca objects, so that context has to be found
    if (caRelatedDisplay *w1 = qobject_cast<caRelatedDisplay *>(w)) {
        QList<QWidget*> wList;
        QPoint globalPos = qobject_cast< QWidget* >( sender() )->mapToGlobal( position );
        QWidget *widgetAt = qApp->widgetAt(globalPos);
        while (widgetAt != (QWidget *) 0) {
            QString className = widgetAt->metaObject()->className();
            if((className.contains("ca")) && (widgetAt != w1) && (!className.contains("caInclude")) && (!className.contains("caRel"))) {
                DisplayContextMenu(widgetAt);
                break;
            }
            wList.append(widgetAt);
            widgetAt->setAttribute(Qt::WA_TransparentForMouseEvents);
            widgetAt = qApp->widgetAt(globalPos);
        }
        foreach(QWidget* widget, wList) {
            widget->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        }
        return;
    }

    // when this widget has no monitors, it could be that an eventual underlying widget has monitors
    // get the monitor list back for this widget
    QVariant monitorList = w->property("MonitorList");
    QVariantList MonitorList = monitorList.toList();
    int nbMonitors = 0;
    bool found = false;

    if(MonitorList.size() > 0) nbMonitors = MonitorList.at(0).toInt();

    // no monitors, then find a first underlying ca widget with monitors
    if(nbMonitors == 0) {
        QList<QWidget*> wList;
        QPoint globalPos = qobject_cast< QWidget* >( sender() )->mapToGlobal( position );
        QWidget *widgetAt = qApp->widgetAt(globalPos);
        while (widgetAt != (QWidget *) Q_NULLPTR) {
            monitorList=widgetAt->property("MonitorList");
            MonitorList = monitorList.toList();
            if(MonitorList.size() > 0) nbMonitors = MonitorList.at(0).toInt();
            QString className = widgetAt->metaObject()->className();
            if((nbMonitors > 0) && (className.contains("ca")) && (widgetAt != w) && (!className.contains("caInclude")) && (!className.contains("caRel"))) {
                DisplayContextMenu(widgetAt);
                found = true;
                break;
            }
            wList.append(widgetAt);
            widgetAt->setAttribute(Qt::WA_TransparentForMouseEvents);
            widgetAt = qApp->widgetAt(globalPos);
        }
        foreach(QWidget* widget, wList) {
            widget->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        }
    }
       // normal case
    if(!found) DisplayContextMenu(qobject_cast<QWidget *>(sender()));

}

/**
  * this function will return true when a pv is doing something on its own (Ex: incrementing)
  */
bool CaQtDM_Lib::SoftPVusesItsself(QWidget* widget, QMap<QString, QString> map)
{
    if (caCalc *calcWidget = qobject_cast<caCalc *>(widget)) {
        QString strng[5];
        QString JSONString = "";
        bool doNothing = false;
        int rate = 5;

        strng[0] = calcWidget->getChannelA();
        strng[1] = calcWidget->getChannelB();
        strng[2] = calcWidget->getChannelC();
        strng[3] = calcWidget->getChannelD();
        strng[4] = calcWidget->getVariable();
        for(int i=0; i<5; i++) {
            QString trimmedPV = strng[i].trimmed();
            int pos = trimmedPV.indexOf(".{");  // jason string
            if(pos != -1) JSONString = trimmedPV.mid(pos+1);
            if(pos != -1) trimmedPV = trimmedPV.mid(0, pos);
            strng[i] = treatMacro(map, trimmedPV, &doNothing, widget->objectName());
            if(i==4) {
                char asc[ MAX_STRING_LENGTH];
                QString pv = calcWidget->getVariable();
                calcWidget->setVariable(strng[i]);  // update variable name when macro used
                if(pos != -1) {
                    int status = parseForDisplayRate(JSONString, rate);
                    if(!status) {
                        snprintf(asc, MAX_STRING_LENGTH, "JSON parsing error on %s ,should be like {\"caqtdm_monitor\":{\"maxdisplayrate\":10}}", (char*) qasc(pv.trimmed()));
                    } else {
                        snprintf(asc, MAX_STRING_LENGTH, "pv %s display rate set to maximum %dHz", qasc(trimmedPV), rate);
                    }
                    postMessage(QtDebugMsg, asc);
                }
            }
        }

        // keep this rate for further use
        QVariant oldRate = widget->property("RATE");
        if (!oldRate.isValid()) {
            calcWidget->setProperty("RATE", rate);
        }

        for(int i=0; i<4; i++) {
            if(strng[4] == strng[i]) return true;
        }
        return false;
    } else {
        return false;
    }
}

/**
  * get channels and create the monitors for calculating the visibility of the objects
  */
int CaQtDM_Lib::InitVisibility(QWidget* widget, knobData* kData, QMap<QString, QString> map, int *specData, QString info)
{
    QString tooltip;
    QString pv;
    bool doNothing;

    tooltip.append(ToolTipPrefix);
    if(info.size() > 0) {
        tooltip.append(info);
        tooltip.append("<br>");
    }

    QList<QVariant> monitorList;
    QList<QVariant> indexList;
    int num, nbMon = 0;
    QString strng[4];
    QString visibilityCalc;
    QString text;

    // caCalc has no visibility, it is a calc entity
    if (caCalc *calcWidget = qobject_cast<caCalc *>(widget)) {
        getVisibilityChannels(calcWidget);
        visibilityCalc = calcWidget->getCalc();
    } else if (caImage *imageWidget = qobject_cast<caImage *>(widget)) {
        // while caImage uses also ImageCalc, we do not skip this, even when visibility = StaticV
        getVisibilityChannels(imageWidget);
        visibilityCalc = imageWidget->getVisibilityCalc();
    } else if (caGraphics *graphicsWidget = qobject_cast<caGraphics *>(widget)) {
        if(graphicsWidget->getVisibility() == caGraphics::StaticV && graphicsWidget->getColorMode() == caGraphics::Static) return 0;
        getVisibilityChannels(graphicsWidget);
        visibilityCalc = graphicsWidget->getVisibilityCalc();
    } else if (caPolyLine *polylineWidget = qobject_cast<caPolyLine *>(widget)) {
        if(polylineWidget->getVisibility() == caPolyLine::StaticV && polylineWidget->getColorMode() == caPolyLine::Static) return 0;
        getVisibilityChannels(polylineWidget);
        visibilityCalc = polylineWidget->getVisibilityCalc();
    } else if (caInclude *includeWidget = qobject_cast<caInclude *>(widget)) {
        if(includeWidget->getVisibility() == caInclude::StaticV) return 0;
        getVisibilityChannels(includeWidget);
        visibilityCalc = includeWidget->getVisibilityCalc();
    } else if (caFrame *frameWidget = qobject_cast<caFrame *>(widget)) {
        if(frameWidget->getVisibility() == caFrame::StaticV) return 0;
        getVisibilityChannels(frameWidget);
        visibilityCalc = frameWidget->getVisibilityCalc();
    } else if (caLabel *labelWidget = qobject_cast<caLabel *>(widget)) {
        if(labelWidget->getVisibility() == caLabel::StaticV && labelWidget->getColorMode() == caLabel::Static) return 0;
        getVisibilityChannels(labelWidget);
        visibilityCalc = labelWidget->getVisibilityCalc();
    } else if (caLabelVertical *labelverticalWidget = qobject_cast<caLabelVertical *>(widget)) {
        if(labelverticalWidget->getVisibility() == caLabelVertical::StaticV && labelverticalWidget->getColorMode() == caLabelVertical::Static) return 0;
        getVisibilityChannels(labelverticalWidget);
        visibilityCalc = labelverticalWidget->getVisibilityCalc();
    } else {
        qCWarning(caQtDMLibLog) << "widget has not been defined for visibility";
        return 0;
    }

    /* add monitors for this if any */
    for(int i=0; i<4; i++) {
        if((num = addMonitor(myWidget, kData, strng[i], widget, specData, map, &pv)) >= 0) {
            if (caCalc *calcWidget = qobject_cast<caCalc *>(widget)) {
               replaceVisibilityChannels(calcWidget)
            } else if (caImage *imageWidget = qobject_cast<caImage *>(widget)) {
                replaceVisibilityChannels(imageWidget)
            } else if (caGraphics *graphicsWidget = qobject_cast<caGraphics *>(widget)) {
                replaceVisibilityChannels(graphicsWidget)
            } else if (caPolyLine *polylineWidget = qobject_cast<caPolyLine *>(widget)) {
                replaceVisibilityChannels(polylineWidget)
            } else if (caInclude *includeWidget = qobject_cast<caInclude *>(widget)) {
                replaceVisibilityChannels(includeWidget)
            } else if (caFrame *frameWidget = qobject_cast<caFrame *>(widget)) {
                replaceVisibilityChannels(frameWidget)
            } else if (caLabel *labelWidget = qobject_cast<caLabel *>(widget)) {
                replaceVisibilityChannels(labelWidget)
            } else if (caLabelVertical *labelverticalWidget = qobject_cast<caLabelVertical *>(widget)) {
                replaceVisibilityChannels(labelverticalWidget)
            }

            tooltip.append(pv);
            tooltip.append("<br>");
            monitorList.append(num);
            indexList.append(i);
            nbMon++;
        }
    }
    tooltip.append(ToolTipPostfix);
    if (!vncServer)
        if(nbMon> 0) widget->setToolTip(tooltip);

    // replace macros for imagecalc
    if (caImage *imageWidget = qobject_cast<caImage *>(widget)) {
        text =  treatMacro(map, imageWidget->getImageCalc(), &doNothing, widget->objectName());
        imageWidget->setImageCalc(text);
    }

    /* replace also some macro values in the visibility calc string */
    text =  treatMacro(map, visibilityCalc, &doNothing, widget->objectName());

    monitorList.insert(0, nbMon);
    indexList.insert(0, nbMon);

    /* set property into widget */
    if (caCalc *calcWidget = qobject_cast<caCalc *>(widget)) {
        calcWidget->setCalc(text);
        calcWidget->setProperty("MonitorList", monitorList);
        calcWidget->setProperty("IndexList", indexList);
    } else if (caImage *imageWidget = qobject_cast<caImage *>(widget)) {
        imageWidget->setVisibilityCalc(text);
        imageWidget->setProperty("MonitorList", monitorList);
        imageWidget->setProperty("IndexList", indexList);
    } else if (caGraphics *graphicsWidget = qobject_cast<caGraphics *>(widget)) {
        graphicsWidget->setVisibilityCalc(text);
        graphicsWidget->setProperty("MonitorList", monitorList);
        graphicsWidget->setProperty("IndexList", indexList);
    } else if (caPolyLine *polylineWidget = qobject_cast<caPolyLine *>(widget)) {
        polylineWidget->setVisibilityCalc(text);
        polylineWidget->setProperty("MonitorList", monitorList);
        polylineWidget->setProperty("IndexList", indexList);
    } else if (caInclude *includeWidget = qobject_cast<caInclude *>(widget)) {
        includeWidget->setVisibilityCalc(text);
        includeWidget->setProperty("MonitorList", monitorList);
        includeWidget->setProperty("IndexList", indexList);
    } else if (caFrame *frameWidget = qobject_cast<caFrame *>(widget)) {
        frameWidget->setVisibilityCalc(text);
        frameWidget->setProperty("MonitorList", monitorList);
        frameWidget->setProperty("IndexList", indexList);
    } else if (caLabel *labelWidget = qobject_cast<caLabel *>(widget)) {
        labelWidget->setVisibilityCalc(text);
        labelWidget->setProperty("MonitorList", monitorList);
        labelWidget->setProperty("IndexList", indexList);
    } else if (caLabelVertical *labelverticalWidget = qobject_cast<caLabelVertical *>(widget)) {
        labelverticalWidget->setVisibilityCalc(text);
        labelverticalWidget->setProperty("MonitorList", monitorList);
        labelverticalWidget->setProperty("IndexList", indexList);
    }

    // when no monitors we have a static visibility calculation, except for cacalc
    QString className = widget->metaObject()->className();
    if((nbMon == 0) && !className.contains("caCalc") && text.length() > 0) {
        double valueArray[MAX_CALC_INPUTS];
        char post[calcstring_length], calcString[calcstring_length], asc[MAX_STRING_LENGTH];
        short errnum;

        qstrncpy(calcString, qasc(text),calcstring_length);

        for(int i=0; i < MAX_CALC_INPUTS; i++) valueArray[i] = 0.0;
        long status = postfix(calcString, post, &errnum);
        if(status) {
            snprintf(asc, MAX_STRING_LENGTH, "Invalid Calc %s for %s (calc not performed)", calcString, qasc(widget->objectName()));
            setCalcToNothing(widget);
            postMessage(QtDebugMsg, asc);
        } else {
            // Perform the calculation
            double result;
            long status = calcPerform(valueArray, &result, post);
            if(!status) {
                bool visible = (result?true:false);
                //set visibility
                if(!visible) {
                    if(caPolyLine *polylineWidget = qobject_cast<caPolyLine *>(widget)) polylineWidget->setHide(true);
                    else if(caGraphics *graphicsWidget = qobject_cast<caGraphics *>(widget)) graphicsWidget->setHide(true);
                    else widget->hide();
                } else {
                    if(caPolyLine *polylineWidget = qobject_cast<caPolyLine *>(widget)) polylineWidget->setHide(false);
                    else if(caGraphics *graphicsWidget = qobject_cast<caGraphics *>(widget)) graphicsWidget->setHide(false);
                    else widget->show();
                }
            } else {
                snprintf(asc, MAX_STRING_LENGTH, "invalid calc %s for %s (calc not performed)", calcString, qasc(widget->objectName()));
                setCalcToNothing(widget);
                postMessage(QtDebugMsg, asc);
            }
        }
    }

    return nbMon;
}

/**
  * computes max, min and precision for our wheelswitches
  */
void CaQtDM_Lib::ComputeNumericMaxMinPrec(QWidget* widget, const knobData& data)
{
    double maxValue = 1.0, minValue = 0.0;
    int precMode=0, limitsMode=0;
    int width, prec, caMode = 0;
    double maxAbsHoprLopr = 0.0;
    bool fixedFormat = false;

    if(data.edata.initialize) {

        if (caApplyNumeric *applynumericWidget = qobject_cast<caApplyNumeric *>(widget)) {
            precMode = applynumericWidget->getPrecisionMode();
            limitsMode = applynumericWidget->getLimitsMode();
            fixedFormat = applynumericWidget->getFixedFormat();
            caMode = caApplyNumeric::Channel;
        } else if (caNumeric *numericWidget = qobject_cast<caNumeric *>(widget)) {
            precMode = numericWidget->getPrecisionMode();
            limitsMode = numericWidget->getLimitsMode();
            fixedFormat = numericWidget->getFixedFormat();
            caMode = caNumeric::Channel;
        } else if (caSpinbox *spinboxWidget = qobject_cast<caSpinbox *>(widget)) {
            precMode = spinboxWidget->getPrecisionMode();
            limitsMode = spinboxWidget->getLimitsMode();
            fixedFormat = spinboxWidget->getFixedFormat();
            caMode = caSpinbox::Channel;
        }

        if(limitsMode == caMode) {
            if((data.edata.upper_disp_limit == data.edata.lower_disp_limit) ||
                    (fabs(data.edata.upper_disp_limit - data.edata.lower_disp_limit) <= 0.001)) {
                maxValue = 100000.0;
                minValue = -100000.0;
            } else {
                maxValue = data.edata.upper_disp_limit;
                minValue = data.edata.lower_disp_limit;
            }
        } else {

            if (caApplyNumeric *applynumericWidget = qobject_cast<caApplyNumeric *>(widget)) {
                maxValue = applynumericWidget->getMaxValue();
                minValue = applynumericWidget->getMinValue();
            } else if (caNumeric *numericWidget = qobject_cast<caNumeric *>(widget)) {
                maxValue = numericWidget->getMaxValue();
                minValue = numericWidget->getMinValue();
            } else if (caSpinbox *spinboxWidget = qobject_cast<caSpinbox *>(widget)) {
                maxValue = spinboxWidget->getMaxValue();
                minValue = spinboxWidget->getMinValue();
            }

        }

        if (caApplyNumeric *applynumericWidget = qobject_cast<caApplyNumeric *>(widget)) {
            applynumericWidget->setMaxValue(maxValue);
            applynumericWidget->setMinValue(minValue);
        } else if (caNumeric *numericWidget = qobject_cast<caNumeric *>(widget)) {
            numericWidget->setMaxValue(maxValue);
            numericWidget->setMinValue(minValue);
        } else if (caSpinbox *spinboxWidget = qobject_cast<caSpinbox *>(widget)) {
            spinboxWidget->setMaxValue(maxValue);
            spinboxWidget->setMinValue(minValue);
        }

        if(!fixedFormat) {
            if(precMode == caMode) {
                // the widget decides how many decimals it accepts from the
                // channel; default 4 keeps existing panels as they were
                int maxChannelPrec = 4;
                if (caApplyNumeric *w = qobject_cast<caApplyNumeric *>(widget))    maxChannelPrec = w->getMaxChannelPrecision();
                else if (caNumeric *w = qobject_cast<caNumeric *>(widget))         maxChannelPrec = w->getMaxChannelPrecision();
                else if (caSpinbox *w = qobject_cast<caSpinbox *>(widget))         maxChannelPrec = w->getMaxChannelPrecision();

                prec = data.edata.precision;
                if(prec < 0) prec = 0;
                if(prec > maxChannelPrec) prec = maxChannelPrec;
                maxAbsHoprLopr= qMax(fabs(maxValue), fabs(minValue));
                if(qIsFinite(data.edata.rvalue)) maxAbsHoprLopr = qMax(maxAbsHoprLopr, fabs(data.edata.rvalue));
                if(maxAbsHoprLopr > 1.0) {
                    width = (int)log10(maxAbsHoprLopr) + 2 + prec;
                } else {
                    width = 2 + prec;
                }

                if (caApplyNumeric *applynumericWidget = qobject_cast<caApplyNumeric *>(widget)) {
                    qCDebug(caApplyNumericLog) << width-prec-1 << prec;
                    applynumericWidget->setDigits(width-prec-1, prec);
                } else if (caNumeric *numericWidget = qobject_cast<caNumeric *>(widget)) {
                    qCDebug(caNumericLog) << width-prec-1 << prec;
                    numericWidget->setDigits(width-prec-1, prec);
                } else if (caSpinbox *spinboxWidget = qobject_cast<caSpinbox *>(widget)) {
                    qCDebug(caSpinboxLog) << width-prec-1 << prec;
                    spinboxWidget->setDigits(width-prec-1, prec);
                }

            } else {
                maxAbsHoprLopr= qMax(fabs(maxValue), fabs(minValue));
                // see above: the value may reach beyond the limits
                if(qIsFinite(data.edata.rvalue)) maxAbsHoprLopr = qMax(maxAbsHoprLopr, fabs(data.edata.rvalue));

                if(maxAbsHoprLopr > 1.0) {
                    width = (int)log10(maxAbsHoprLopr) + 2 ;
                } else {
                    width = 2;
                }

                if (caApplyNumeric *applynumericWidget = qobject_cast<caApplyNumeric *>(widget)) {
                    applynumericWidget->setIntDigits(width-1);
                } else if (caNumeric *numericWidget = qobject_cast<caNumeric *>(widget)) {
                    numericWidget->setIntDigits(width-1);
                } else if (caSpinbox *spinboxWidget = qobject_cast<caSpinbox *>(widget)) {
                    spinboxWidget->setIntDigits(width-1);
                }

            }
        }
    }
}

/**
  * posting a message, will display on the message window
  */
void CaQtDM_Lib::postMessage(QtMsgType type, char *msg)
{
    if(messageWindowP == (MessageWindow *) Q_NULLPTR) return;
    messageWindowP->postMsgEvent(type, msg);
}

void CaQtDM_Lib::postMessageAndLog(QtMsgType type, char *msg, QMessageLogger::CategoryFunction category) {
    switch (type) {
    case QtDebugMsg:
        qCDebug(category) << msg;
        break;
    case QtInfoMsg:
        qCInfo(category) << msg;
        break;
    case QtWarningMsg:
        qCWarning(category) << msg;
        break;
    case QtCriticalMsg:
        qCCritical(category) << msg;
        break;
    case QtFatalMsg:
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        qCFatal(category) << msg;
#else
        qFatal("%s", msg);
#endif
	break;
    default:
        qCWarning(category) << msg;
        break;
    }

    postMessage(type, msg);
}

/**
  * execute an application on linux
  */
#ifndef MOBILE
#if defined(linux) || defined(__APPLE__)
int CaQtDM_Lib::Execute(char *command)
{
    int status;
    pid_t pid;
    /*
    QTextStream out(stdout);
    out << command << endl << endl;
    out.flush();
*/
    pid = fork ();
    if (pid == 0) {
        execl ("/bin/sh", "/bin/sh", "-c", command, NULL);
    } else if (pid < 0) {
        status = -1;
    } else if (waitpid (pid, &status, 0) != pid) {
        status = -1;
    }

    return status;
}
#endif
#endif

void CaQtDM_Lib::TreatOrdinaryValue(QString pvo, double value, int32_t idata,  QString svalue, QWidget *w)
{
    char errmess[SMALL_STRING_LENGTH];
    int indx;

    QString pv = pvo.trimmed();

    qCDebug(caQtDMLibLog) << "treatordinary value " << pv << w;
    knobData *kPtr = mutexKnobDataP->getMutexKnobDataPV(w, pv);
    if(kPtr != (knobData *) Q_NULLPTR) {
        qCDebug(caQtDMLibLog) << "try to find out if this is a soft pv" << pv;
        // when softpv treat it and get out
        if(mutexKnobDataP->getSoftPV(pv, &indx, (QWidget*) kPtr->thisW)) {
            if(kPtr->soft) {
                qCDebug(caQtDMLibLog) << "write softpv";
                kPtr = mutexKnobDataP->GetMutexKnobDataPtr(indx);  // use pointer
                kPtr->edata.rvalue = value;
                // set value also into widget, will be overwritten when driven from other channels
                caCalc * ww = (caCalc*) kPtr->dispW;
                ww->setValue(value);
                return;
            };
        }
    } else {
        qCCritical(caQtDMLibLog) << "internal error; return while pv <" << pv << "> not found";
        return;
    }

    QStringsToChars(pv, svalue, w->objectName().toLower());
    ControlsInterface * plugininterface = getControlInterface(kPtr->pluginName);
    if(plugininterface != (ControlsInterface *) Q_NULLPTR) {
        knobData *kPtr;
        if((kPtr = GetMutexKnobDataPV(w, param1)) != (knobData *) Q_NULLPTR) {
            if(!plugininterface->pvSetValue(kPtr, value, idata, param2, param3, errmess, 0)) {
               plugininterface->pvSetValue(param1, value, idata, param2, param3, errmess, 0);
            }
        }
    }
}

/**
  * these routines will get a value from a string with hex and octal representation
  */
long CaQtDM_Lib::getLongValueFromString(char *textValue, FormatType fType, char **end)
{
    if(fType == octal) {
        // Internally we represent octal values with 'O' prefix, but C-Standard is '0'
        if(strlen(textValue) > (size_t) 1 && textValue[0] == 'O') {
            textValue[0] = '0';
        }
        return strtoul(textValue, end, 8);
    } else if(fType == hexadecimal) {
        return strtoul(textValue, end, 16);
    } else {
        if((strlen(textValue) > (size_t) 2) && (textValue[0] == '0') && (textValue[1] == 'x' || textValue[1] == 'X')) {
            return strtoul(textValue, end, 16);
        } else if(strlen(textValue) > (size_t) 1 && textValue[0] == 'O') {
            // Same as for regular octal conversion
            textValue[0] = '0';
            return strtoul(textValue, end, 8);
        } else {
            return strtol(textValue, end, 10);
        }
    }
}

double CaQtDM_Lib::getDoubleValueFromString(char *textValue, FormatType fType, char **end)
{
    if(fType == octal) {
        return (double) strtoul(textValue, end, 8);
    } else if(fType == hexadecimal) {
        return (double) strtoul(textValue, end, 16);
    } else {
        if((strlen(textValue) > (size_t) 2) && (textValue[0] == '0') && (textValue[1] == 'x' || textValue[1] == 'X')) {
            return (double) strtoul(textValue, end, 16);
        } else {
            return strtod(textValue, end);
        }
    }
}

/**
  * this routine will treat the string, command, value to write to the pv
  */
void CaQtDM_Lib::TreatRequestedValue(QString pvo, QString text, FormatType fType, QWidget *w)
{
    char errmess[SMALL_STRING_LENGTH];
    double value;
    long longValue;
    char *end = Q_NULLPTR, textValue[SMALL_STRING_LENGTH];
    bool match;
    int indx;
    ControlsInterface * plugininterface = (ControlsInterface *) Q_NULLPTR;

    QString pv = pvo.trimmed();

    FormatType fTypeNew;

    if(fType == octal) fTypeNew = octal;
    else if(fType == hexadecimal) fTypeNew = hexadecimal;
    else if(fType == string) fTypeNew = string;
    else fTypeNew = decimal;

    knobData *kPtr = mutexKnobDataP->getMutexKnobDataPV(w, pv);
    if(kPtr == (knobData *) Q_NULLPTR) return;
    knobData *auxPtr = kPtr;

    // when softpv get index to where it is defined
    if(mutexKnobDataP->getSoftPV(kPtr->pv, &indx, (QWidget*) kPtr->thisW)) {
        kPtr = mutexKnobDataP->GetMutexKnobDataPtr(indx);  // use pointer
        if(kPtr == (knobData *) Q_NULLPTR) return;
    } else {
        //plugininterface = (ControlsInterface *) w->property("Interface").value<void *>();
        plugininterface = getPluginInterface((QWidget*) w);
        if(plugininterface == (ControlsInterface *) Q_NULLPTR) return;
    }

    if(!kPtr->soft) {
        if(plugininterface == (ControlsInterface *) Q_NULLPTR) return;
    }

    qCDebug(caQtDMLibLog) << "fieldtype:" << kPtr->edata.fieldtype;
    switch (kPtr->edata.fieldtype) {
    case caSTRING:
        qCDebug(caQtDMLibLog) << "set string" << text << plugininterface->pluginName();
        if(plugininterface != (ControlsInterface *) Q_NULLPTR) {
           if(!plugininterface->pvSetValue(kPtr,  0.0, 0, (char*) qasc(text), (char*) qasc(w->objectName()), errmess, 0)) {
               QByteArray data;
               for (int i = 0; i < text.length(); ++i) {
                   QChar ch = text[i];
                   ushort unicode = ch.unicode();
                   if (unicode >= 128 && unicode <= 255) {
                       QByteArray utf8Bytes = QString(ch).toUtf8();
                       for (int j = 0; j < utf8Bytes.length(); ++j) {
                           data.append(static_cast<unsigned char>(utf8Bytes[j]));
                       }
                   } else {
                       data.append(ch.toLatin1());
                   }
               }
              plugininterface->pvSetValue(kPtr->pv, 0.0, 0, (char*) data.constData() , (char*) qasc(w->objectName()), errmess, 0);
           }
        }
        break;

    case caENUM:
    case caINT:
    case caLONG:
        qstrncpy(textValue, qasc(text), sizeof(textValue));
        // Check for an enum text
        match = false;
        if(kPtr->edata.dataB != (void*)0 && kPtr->edata.enumCount > 0) {
            QString strng((char*) kPtr->edata.dataB);
            //QStringList list = strng.split(";", QString::SkipEmptyParts);
            QStringList list = strng.split((QChar)27, SKIP_EMPTY_PARTS);
            for (int i=0; i<list.size(); i++) {
                if(!text.compare(list.at(i).trimmed())) {
                    qCDebug(caQtDMLibLog) << "set enum text" << textValue;
                    if(plugininterface != (ControlsInterface *) Q_NULLPTR) {
                        if(!plugininterface->pvSetValue(kPtr, 0.0, 0, textValue, (char*) qasc(w->objectName().toLower()), errmess, 0)) {
                            plugininterface->pvSetValue((char*) kPtr->pv, 0.0, 0, textValue, (char*) qasc(w->objectName().toLower()), errmess, 0);
                        }
                    }
                    match = true;
                    break;
                }
            }
        }

        if(!match) {
            qCDebug(caQtDMLibLog) << "assume it is a number";
            // Assume it is a number
            longValue = getLongValueFromString(textValue, fTypeNew, &end);

            // number must be between the enum possibilities
            if(kPtr->edata.fieldtype == caENUM) {
                if(*end == 0 && end != textValue && longValue >= 0 && longValue <= kPtr->edata.enumCount) {
                    qCDebug(caQtDMLibLog) << "decode value *end=0, set a longvalue to enum" << longValue;
                    if(plugininterface != (ControlsInterface *) Q_NULLPTR) {
                        if(!plugininterface->pvSetValue(kPtr,  0.0, (int32_t) longValue, textValue, (char*) qasc(w->objectName().toLower()), errmess, 0)) {
                           plugininterface->pvSetValue((char*) kPtr->pv, 0.0, (int32_t) longValue, textValue, (char*) qasc(w->objectName().toLower()), errmess, 0);
                        }
                    }
                } else {
                    char asc[MAX_STRING_LENGTH];
                    snprintf(asc, MAX_STRING_LENGTH, "Invalid value: pv=%s value= \"%s\"\n", kPtr->pv, textValue);
                    postMessage(QtDebugMsg, asc);
                    if(caTextEntry* widget = qobject_cast<caTextEntry *>((QWidget*) auxPtr->dispW)) {
                        Q_UNUSED(widget);
                    }
                }
                // normal int or long
            } else
                qCDebug(caQtDMLibLog) << "set normal longvalue" << longValue;
                if(plugininterface != (ControlsInterface *) Q_NULLPTR) {
                    if(!plugininterface->pvSetValue(kPtr, 0.0, (int32_t) longValue, textValue, (char*) qasc(w->objectName().toLower()), errmess, 0)) {
                       plugininterface->pvSetValue((char*) kPtr->pv, 0.0, (int32_t) longValue, textValue, (char*) qasc(w->objectName().toLower()), errmess, 0);
                    }
                }
        }

        break;

    case caCHAR:
        if(fType == string) {
            if(kPtr->edata.nelm > 1) {
               qCDebug(caQtDMLibLog) << "set string" << text;
               if(plugininterface != (ControlsInterface *) 0) {
                   if(!plugininterface->pvSetValue(kPtr,  0.0, 0, (char*) qasc(text), (char*) qasc(w->objectName().toLower()), errmess, 0)) {
                       QByteArray data;
                       for (int i = 0; i < text.length(); ++i) {
                           QChar ch = text[i];
                           ushort unicode = ch.unicode();
                           if (unicode >= 128 && unicode <= 255) {
                               QByteArray utf8Bytes = QString(ch).toUtf8();
                               for (int j = 0; j < utf8Bytes.length(); ++j) {
                                   data.append(static_cast<unsigned char>(utf8Bytes[j]));
                               }
                           } else {
                               data.append(ch.toLatin1());
                           }
                       }
                      plugininterface->pvSetValue((char*) kPtr->pv, 0.0, 0, (char*) data.constData(), (char*) qasc(w->objectName().toLower()), errmess, 0);
                   }
               }
            } else {  // single char written through its ascii code while character entered
               text = text.trimmed();
               if(text.size()> 0) {
                 QChar c = text.at(0);
                 if(plugininterface != (ControlsInterface *) Q_NULLPTR) {
                     if(!plugininterface->pvSetValue(kPtr,  0.0, (int)c.toLatin1(), (char*)  "", (char*) qasc(w->objectName().toLower()), errmess, 2)) {
                         plugininterface->pvSetValue((char*) kPtr->pv, 0.0, (int)c.toLatin1(), (char*)  "", (char*) qasc(w->objectName().toLower()), errmess, 2);
                     }
                 }
               }
            }
            break;
        }
        qCDebug(caQtDMLibLog) << "fall through default case";

    default:
        match = false;
        qCDebug(caQtDMLibLog) << "assume it is a double";
        qstrncpy(textValue, qasc(text),SMALL_STRING_LENGTH);
        value = getDoubleValueFromString(textValue, fTypeNew, &end);
        if(*end == '\0' && end != textValue) {        // decoded
            match = true;
        }
        if(match) {
            qCDebug(caQtDMLibLog) << "decoded as double, and set as double" << value;
            if(kPtr->soft) {
                kPtr->edata.rvalue = value;
                // set value also into widget, will be overwritten when driven from other channels
                caCalc * ww = (caCalc*) kPtr->dispW;
                ww->setValue(value);
            } else {
                if(plugininterface != (ControlsInterface *) Q_NULLPTR) {

                    // test for caTextEntry if outside bounds
                    if(caTextEntry* textentryWidget = qobject_cast<caTextEntry *>(w)) {
                      if (textentryWidget->getMinValue()!=textentryWidget->getMaxValue()){
                        if(value > textentryWidget->getMaxValue()) return;
                        if(value < textentryWidget->getMinValue()) return;
                      }

                    }

                    if(!plugininterface->pvSetValue(kPtr,  value, 0, textValue, (char*) qasc(w->objectName().toLower()), errmess, 1)) {
                       plugininterface->pvSetValue((char*) kPtr->pv, value, 0, textValue, (char*) qasc(w->objectName().toLower()), errmess, 1);
                    }
                 }
            }

        } else {
            char asc[MAX_STRING_LENGTH];
            snprintf(asc, MAX_STRING_LENGTH, "Invalid value: pv=%s value= \"%s\"\n", kPtr->pv, textValue);
            postMessage(QtDebugMsg, asc);
            if(caTextEntry* widget = qobject_cast<caTextEntry *>((QWidget*) auxPtr->dispW)) {
                Q_UNUSED(widget);
            }
        }
        break;
    }
}

/**
  * this routine will treat the values to write to the pv
  */
void CaQtDM_Lib::TreatRequestedWave(QString pvo, QString text, caWaveTable::FormatType fType, int index, QWidget *w)
{
    char    errmess[SMALL_STRING_LENGTH], sdata[40], asc[MAX_STRING_LENGTH];
    int32_t data32[1];
    int16_t data16[1];
    float   fdata[1];
    double  value, ddata[1];
    long    longValue;
    char    *end = Q_NULLPTR, textValue[SMALL_STRING_LENGTH];
    bool    match;

    QString pv = pvo.trimmed();

    //ControlsInterface * plugininterface = (ControlsInterface *) w->property("Interface").value<void *>();
    ControlsInterface *plugininterface = getPluginInterface((QWidget*) w);
    if(plugininterface == (ControlsInterface *) Q_NULLPTR) return;

    FormatType fTypeNew;

    if(fType == caWaveTable::octal) fTypeNew = octal;
    else if(fType == caWaveTable::hexadecimal) fTypeNew = hexadecimal;
    else if(fType == caWaveTable::string) fTypeNew = string;
    else fTypeNew = decimal;

    qCDebug(caQtDMLibLog) << "treat requested wave";
    knobData *kPtr = mutexKnobDataP->getMutexKnobDataPV(w, pv);
    if(kPtr == (knobData *) Q_NULLPTR)return;

    if(index >= kPtr->edata.valueCount) return;

    QMutex *datamutex;
    datamutex = (QMutex*) kPtr->mutex;
    datamutex->lock();

    qCDebug(caQtDMLibLog) << "fieldtype:" << kPtr->edata.fieldtype;
    switch (kPtr->edata.fieldtype) {

    case caINT:
    case caLONG:
    case caCHAR:
        qstrncpy(textValue, qasc(text),SMALL_STRING_LENGTH);
        longValue = getLongValueFromString(textValue, fTypeNew, &end);

        if(kPtr->edata.fieldtype == caLONG) {
            int32_t* P = (int32_t*) kPtr->edata.dataB;
            P[index] = (int32_t) longValue;
            if(!plugininterface->pvSetWave(kPtr,  fdata, ddata, data16, P, sdata, kPtr->edata.valueCount,
                                  (char*) qasc(w->objectName().toLower()), errmess))
            {
                plugininterface->pvSetWave((char*) kPtr->pv,  fdata, ddata, data16, P, sdata, kPtr->edata.valueCount,
                                  (char*) qasc(w->objectName().toLower()), errmess);
            }
        } else if(kPtr->edata.fieldtype == caINT) {
            int16_t* P = (int16_t*) kPtr->edata.dataB;
            P[index] = (int16_t) longValue;
            if(!plugininterface->pvSetWave(kPtr, fdata, ddata, P, data32, sdata, kPtr->edata.valueCount,
                                  (char*) qasc(w->objectName().toLower()), errmess))
            {
                plugininterface->pvSetWave((char*) kPtr->pv, fdata, ddata, P, data32, sdata, kPtr->edata.valueCount,
                                  (char*) qasc(w->objectName().toLower()), errmess);
            }
        } else {
            if(fTypeNew == string) {
                char* P = (char*) kPtr->edata.dataB;
                P[index] = textValue[0];
                if(!plugininterface->pvSetWave(kPtr,  fdata, ddata, data16, data32, P, kPtr->edata.valueCount,
                                     (char*) qasc(w->objectName().toLower()), errmess))
                {
                    plugininterface->pvSetWave((char*) kPtr->pv,  fdata, ddata, data16, data32, P, kPtr->edata.valueCount,
                                     (char*) qasc(w->objectName().toLower()), errmess);
                }
            } else {
                char* P = (char*) kPtr->edata.dataB;
                P[index] = (char) ((int) longValue);
                if(!plugininterface->pvSetWave(kPtr, fdata, ddata, data16, data32, P, kPtr->edata.valueCount,
                                     (char*) qasc(w->objectName().toLower()), errmess))
                {
                    plugininterface->pvSetWave((char*) kPtr->pv, fdata, ddata, data16, data32, P, kPtr->edata.valueCount,
                                     (char*) qasc(w->objectName().toLower()), errmess);
                }
            }
        }


        break;

    case caSTRING:
        snprintf(asc, MAX_STRING_LENGTH, "writing of strings for treatRequestedWave is not yet supported\n");
        postMessage(QtDebugMsg, asc);
        break;

    case caFLOAT:
    case caDOUBLE:
        match = false;
        // Treat as a double
        qstrncpy(textValue, qasc(text),SMALL_STRING_LENGTH);
        value = getDoubleValueFromString(textValue, fTypeNew, &end);
        if(*end == '\0' && end != textValue) {        // decoded
            match = true;
        }
        if(match) {
            if(kPtr->edata.fieldtype == caFLOAT) {
                float* P = (float*) kPtr->edata.dataB;
                P[index] = (float) value;
                if(!plugininterface->pvSetWave(kPtr,  P, ddata, data16, data32, sdata, kPtr->edata.valueCount,
                                     (char*) qasc(w->objectName().toLower()), errmess))
                {
                    plugininterface->pvSetWave((char*) kPtr->pv,  P, ddata, data16, data32, sdata, kPtr->edata.valueCount,
                                     (char*) qasc(w->objectName().toLower()), errmess);
                }
            } else  {
                double* P = (double*) kPtr->edata.dataB;
                P[index] = value;
                if(!plugininterface->pvSetWave(kPtr, fdata, P, data16, data32, sdata, kPtr->edata.valueCount,
                                     (char*) qasc(w->objectName().toLower()), errmess))
                {
                    plugininterface->pvSetWave((char*) kPtr->pv, fdata, P, data16, data32, sdata, kPtr->edata.valueCount,
                                     (char*) qasc(w->objectName().toLower()), errmess);
                }
            }
        } else {
            char asc[MAX_STRING_LENGTH];
            snprintf(asc, MAX_STRING_LENGTH, "Invalid value: pv=%s value= \"%s\"\n", kPtr->pv, textValue);
            postMessage(QtDebugMsg, asc);
        }
        break;

    default:
        snprintf(asc, MAX_STRING_LENGTH, "unhandled epics type (%d) in treatRequestedWave\n", kPtr->edata.fieldtype);
        postMessage(QtDebugMsg, asc);
    }

    datamutex->unlock();
}


bool CaQtDM_Lib::parseForQRectConst(QString &inputc, double *valueArray)
{
    // Parse data
    bool success = false;

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(inputc.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qCWarning(caQtDMLibLog) << "failed to parse:" << inputc << "with error:" << parseError.errorString();
    } else {
        // Retrieve the main object
        if (document.isObject()) {
            QJsonObject root = document.object();
            if (root["valueconst"].isArray()) {
                QJsonArray jsonarr = root["valueconst"].toArray();
                for (unsigned int j = 0; j < jsonarr.size(); j++){
                    if (jsonarr[j].isDouble()) {
                       valueArray[j] = static_cast<int>(jsonarr[j].toDouble());
                    }
                }
                success = true;
            }
        }
    }

    return success;
}


int CaQtDM_Lib::parseForDisplayRate(QString &inputc, int &rate)
{
    // Parse data
    bool success = false;

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(inputc.toUtf8(), &parseError);
    // Did it go wrong?
    if (parseError.error != QJsonParseError::NoError) {
        qCCritical(caQtDMLibLog) << "failed to parse:" << inputc << "with error:" << parseError.errorString();
        inputc = "{}";
        return success;
    } else {
        // Retrieve the main object
        if(!document.isObject()) {
            qCDebug(caQtDMLibLog) << "The root element is not an object";
        } else {
            QJsonObject root = document.object();
            // check for monitor
            if (root["caqtdm_monitor"].isObject()) {
                qCDebug(caQtDMLibLog) << "monitor detected";
                // Retrieve nested object
                QJsonObject obj = root["caqtdm_monitor"].toObject();
                if (obj["maxdisplayrate"].isDouble()) {
                    qCDebug(caQtDMLibLog) << "maxdisplayrate detected";
                    rate = obj["maxdisplayrate"].toDouble();
                    qCDebug(caQtDMLibLog) << "decode value =" << rate;
                    success = true;
                }
                root.remove("caqtdm_monitor");
                document = QJsonDocument(root);
                inputc = QString::fromUtf8(document.toJson(QJsonDocument::Compact));
            }
        }
    }

    return success;
}

bool CaQtDM_Lib::checkJsonString(QString &inputc)
{
    // test if we have a valid json string

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(inputc.toUtf8(), &parseError);;

    qCDebug(caQtDMLibLog) << "checking" << inputc;

    // Did it go wrong?, when yes then get rid of it
    if (parseError.error != QJsonParseError::NoError) {
        inputc = "{}";
        qCDebug(caQtDMLibLog) << "checkJsonString -- failed to parse:" << inputc << "with error:" << parseError.errorString();
        return false;
    }

    return true;
}

void CaQtDM_Lib::allowResizing(bool allowresize)
{
    allowResize = allowresize;
    QTimer::singleShot(50, this, SLOT(updateResize()));
}

// android does not resize correctly (it does not report its size), doing it a little bit later works
void CaQtDM_Lib::updateResize()
{
#ifdef MOBILE_ANDROID
    QApplication::processEvents();
    QResizeEvent *re = new QResizeEvent(size(), size());
    resizeEvent(re);
    delete re;
#endif
}

// this will probably not work for all kind of mobiles
qreal CaQtDM_Lib::fontResize(double factX, double factY, QVariantList list, int usedIndex)
{
    qreal fontSize;
    if(list.at(usedIndex).toInt() < 0) {
        fontSize = qMin(factX, factY) * (float) list.at(5).toInt();
    } else {
        fontSize = qMin(factX, factY) * (float) list.at(usedIndex).toInt();
    }
    qCDebug(caQtDMLibLog) << "font sizes" << list.at(4).toInt() << list.at(5).toInt() << "dpi" << qApp->primaryScreen()->physicalDotsPerInch();
    qCDebug(caQtDMLibLog) << "caTable fontsize=" << qMin(factX, factY) << qMin(factX, factY) * (double) list.at(4).toInt();
    qCDebug(caQtDMLibLog) << "logical dots per inch" << qApp->primaryScreen()->logicalDotsPerInch()  << "pixel ratio" << qApp->primaryScreen()->devicePixelRatio();
#ifdef MOBILE_ANDROID
    fontSize = fontSize * (float) qApp->primaryScreen()->logicalDotsPerInch() / (float) qApp->primaryScreen()->physicalDotsPerInch();
#endif
    if(fontSize < MIN_FONT_SIZE) fontSize = MIN_FONT_SIZE;
    return fontSize;
}

QRect CaQtDM_Lib::widgetResize(QWidget* w,double factX, double factY){
    QVariant var=w->property("GeometryList");
    if (var.isValid()){
        QVariantList list = var.toList();
        double x = (double) list.at(0).toInt() * factX;
        double y = (double) list.at(1).toInt() * factY;
        double width = (double) list.at(2).toInt() *factX;
        double height = (double) list.at(3).toInt() *factY;
        if(width < 1.0) width=1.0;
        if(height < 1.0) height = 1.0;
        return QRect(qRound(x), qRound(y), qRound(width), qRound(height));
    }else{
      return w->rect();
    }
}


void CaQtDM_Lib::resizeSpecials(QString className, QWidget *widget, QVariantList list, double factX, double factY)
{
    // for horizontal or vertical line we still have to set the linewidth and for a frame the border framewidth
    if(!className.compare("caFrame") || !className.compare("QFrame")) {
        double linewidth;
        QFrame * line = (QFrame *) widget;
        if(line->frameShape() == QFrame::HLine || line->frameShape() == QFrame::VLine) {
            if(line->frameShape() != QFrame::HLine) {
                qCDebug(caQtDMLibLog) << "resize vertical line" << widget << (double) list.at(4).toInt() * factX;
                linewidth = (double) list.at(4).toInt() * factX;
            } else {
                qCDebug(caQtDMLibLog) << "resize horizontal line" << widget << (double) list.at(4).toInt() * factY;
                linewidth = (double) list.at(4).toInt() * factY;
            }
            line->setLineWidth(qRound(linewidth));
        } else {
            qCDebug(caQtDMLibLog) << "resize frame" << widget << (double) list.at(5).toInt() * qMin(factX, factY);
            if ((list.count()>=5) && list.at(4).isValid()){
                linewidth = (double) list.at(4).toInt() * qMin(factX, factY);
                line->setLineWidth(qRound(linewidth));
            }
        }
    }

    else if(!className.compare("caTable")) {
        caTable *table = (caTable *) widget;

        QFont f = table->font();
        qreal fontSize = fontResize(factX, factY, list, 4);
        f.setPointSize(qRound(fontSize));

        table->setUpdatesEnabled(false);
        for(int i = 0; i < table->rowCount(); ++i) {
            for(int j = 0; j < table->columnCount(); ++j) {
                QTableWidgetItem* selectedItem = table->item(i, j);
                if(selectedItem != (QTableWidgetItem*) Q_NULLPTR) selectedItem->setFont(f);
            }
        }
        table->setValueFont(f);
        table->verticalHeader()->setDefaultSectionSize((int) (qMin(factX, factY)*20));

        QString styleSheet("QHeaderView { font-size: %1pt; }");
        styleSheet = styleSheet.arg( qRound(fontSize));
        table->horizontalHeader()->setStyleSheet(styleSheet);
        table->verticalHeader()->setStyleSheet(styleSheet);
        table->setUpdatesEnabled(true);
    }

    else if(!className.compare("caWaveTable")) {
        caWaveTable *table = (caWaveTable *) widget;

        QFont f = table->font();
        qreal fontSize = fontResize(factX, factY, list, 4);
        f.setPointSize(qRound(fontSize));

        table->setUpdatesEnabled(false);
        for(int i = 0; i < table->rowCount(); ++i) {
            for(int j = 0; j < table->columnCount(); ++j) {
                QTableWidgetItem* selectedItem = table->item(i, j);
                if(selectedItem != (QTableWidgetItem*) Q_NULLPTR) selectedItem->setFont(f);
            }
        }
        table->setValueFont(f);
        table->verticalHeader()->setDefaultSectionSize((int) (qMin(factX, factY)*20));

        QString styleSheet("QHeaderView { font-size: %1pt; }");
        styleSheet = styleSheet.arg( qRound(fontSize));
        table->horizontalHeader()->setStyleSheet(styleSheet);
        table->verticalHeader()->setStyleSheet(styleSheet);

        table->setUpdatesEnabled(true);
    }

    else if(!className.compare("QLabel")) {
        QLabel *label = (QLabel *) widget;
        className = label->parent()->metaObject()->className();
        if(!className.contains("Numeric") && !className.contains("caSpinbox")) {  // would otherwise interfere with our wheelswitch or spinbox
            QFont f = label->font();
            qreal fontSize = fontResize(factX, factY, list, 4);
            f.setPointSize(qRound(fontSize));

            label->setFont(f);
        }
    }

    else if((!className.compare("caMenu")) ||
            (!className.compare("QPlainTextEdit")) ||
            (!className.compare("QTextEdit")) ||
            (!className.compare("QLineEdit")) ||
            (!className.compare("QRadioButton")) ||
            (!className.compare("QComboBox")) ||
            (!className.compare("QCheckBox"))
            ) {
        QFont f = widget->font();
        qreal fontSize = fontResize(factX, factY, list, 4);
        f.setPointSize(qRound(fontSize));
        //in case of a textedit widget, one has to selct the text to change the font (if somebody mixes fonts, he has bad luck)
        if(!className.compare("QTextEdit")) {
            QTextEdit *textEdit = (QTextEdit *) widget;
            QTextCursor cursor = textEdit->textCursor();
            textEdit->selectAll();
            textEdit->setFontPointSize(fontSize);
            textEdit->setTextCursor( cursor );
        }
        widget->setFont(f);
    }

    else if(!className.compare("caStripPlot") || !className.compare("caCartesianPlot")) {
        QwtPlot *plot = (QwtPlot *) widget;
        qreal fontSize = fontResize(factX, factY, list, 4);

        // change font of axis ticks
        QFont f = plot->axisFont(QwtPlot::xBottom);
        f.setPointSizeF(fontSize);
        plot->setAxisFont(QwtPlot::xBottom, f);
        plot->setAxisFont(QwtPlot::yLeft, f);

        // change font of labels and title
        f = plot->title().font();
        QwtText title = plot->title().text();
        QwtText titleX = plot->axisTitle(QwtPlot::xBottom).text();
        QwtText titleY = plot->axisTitle(QwtPlot::yLeft).text();
        fontSize = fontResize(factX, factY, list, 6);
        f.setPointSizeF(fontSize);
        title.setFont(f);
        titleX.setFont(f);
        titleY.setFont(f);
        plot->setTitle(title);
        plot->setAxisTitle(QwtPlot::xBottom, titleX);
        plot->setAxisTitle(QwtPlot::yLeft, titleY);

        if(!className.compare("caStripPlot")) {
            caStripPlot * stripplotWidget = (caStripPlot *) widget;
            fontSize = fontResize(factX, factY, list, 7);
            f.setPointSizeF(fontSize);
            if(stripplotWidget->getLegendEnabled()) {
                stripplotWidget->setLegendAttribute(stripplotWidget->getScaleColor(), f, caStripPlot::FONT);
                stripplotWidget->updateLayout();
            }
            stripplotWidget->setTicksResizeFactor(factX, factY);
        } else {
            caCartesianPlot * cartesianplotWidget = (caCartesianPlot *) widget;
            fontSize = fontResize(factX, factY, list, 7);
            f.setPointSizeF(fontSize);
            if(cartesianplotWidget->getLegendEnabled()) {
                cartesianplotWidget->setLegendAttribute(cartesianplotWidget->getScaleColor(), f, caCartesianPlot::FONT);
                cartesianplotWidget->updateLayout();
            }
            plot->axisScaleDraw(QwtPlot::xBottom)->setTickLength(QwtScaleDiv::MajorTick, factY * (double) list.at(8).toInt());
            plot->axisScaleDraw(QwtPlot::xBottom)->setTickLength(QwtScaleDiv::MediumTick, factY * (double) list.at(9).toInt());
            plot->axisScaleDraw(QwtPlot::xBottom)->setTickLength(QwtScaleDiv::MinorTick, factY * (double) list.at(10).toInt());
            plot->axisScaleDraw(QwtPlot::xBottom)->setSpacing(0.0);
            plot->axisScaleDraw(QwtPlot::yLeft)->setTickLength(QwtScaleDiv::MajorTick, factX * (double) list.at(8).toInt());
            plot->axisScaleDraw(QwtPlot::yLeft)->setTickLength(QwtScaleDiv::MediumTick, factX * (double) list.at(9).toInt());
            plot->axisScaleDraw(QwtPlot::yLeft)->setTickLength(QwtScaleDiv::MinorTick, factX * (double) list.at(10).toInt());
            plot->axisScaleDraw(QwtPlot::xBottom)->setSpacing(0.0);
        }
    }

    else if(!className.compare("QGroupBox")) {
        if(list.at(4).toInt() < 0) return; // on android I got -1 for these fonts at initialization, i.e pixelsize
        if(qMin(factX, factY) < 1.0) {
            QGroupBox *box = (QGroupBox *) widget;
            qreal fontSize = qMin(factX, factY) * (double) list.at(4).toInt();
            if(fontSize < MIN_FONT_SIZE) fontSize = MIN_FONT_SIZE;
            QFont f;
            f.setPointSizeF(fontSize);
            box->setFont(f);
        }
    }

    else if(!className.compare("caInclude")) {
        caInclude *includeWidget = (caInclude *) widget;
        int adjustMargin = includeWidget->getMargin();
        int maximumX = 10;
        int maximumY = 10;
        includeWidget->childResizeCall(factX,factY);
        // in case of absolute positioning, reposition the elements
        if(includeWidget->getStacking() == caInclude::Positions) {
/*
            QList<QWidget*> list =  includeWidget->getChildsList();
            for(int j=0; j<list.count(); j++) {
                QString pos;
                QWidget * widget = list[j];
                int posx, posy;
                int posX=0;
                int posY=0;
                if(includeWidget->getXposition(j, posx, 0, pos)) {posX = posx;}
                if(includeWidget->getYposition(j, posy, 0, pos)) {posY = posy;}
                // move to correct position
                int xpos = qRound((double) posX * includeWidget->getXcorrection() * factX);
                int ypos = qRound((double) posY * includeWidget->getYcorrection() * factY);
                widget->move(xpos + adjustMargin/2*factX, ypos + adjustMargin/2*factY);
                maximumX = includeWidget->getXmaximum() + widget->width();
                maximumY = includeWidget->getYmaximum() + widget->height();
             }
*/
            if(includeWidget->getAdjustSize()) {

                //foreach(QWidget* l ,includeWidget->findChildren<QWidget *>()){
                //   QRect resizedata=l->childrenRect();
                //   if ((resizedata.width()>0) && (resizedata.height()>0))
                //   l->resize(resizedata.width(),resizedata.height());
                //}

                QRect resizedata=includeWidget->childrenRect();
                includeWidget->resize(resizedata.width(),resizedata.height());

                //QRect resizedata=includeWidget->scanChildsneededArea();
                qCDebug(caQtDMLibLog) <<"resizedata in resizespezial:"<< resizedata;

                //includeWidget->getIncludeFrame()->resize(resizedata.width(),resizedata.height());
                //includeWidget->resize(resizedata.width(),resizedata.height());
                 // when the include is packed into a scroll area, set the minimumsize too
                maximumX = includeWidget->getXmaximum() + widget->width();
                maximumY = includeWidget->getYmaximum() + widget->height();

                ResizeScrollBars(includeWidget, factX * (maximumX + adjustMargin), factY * (maximumY + adjustMargin));
            }

        } else {
            // when the include is packed into a scroll area, set the minimumsize too
            ResizeScrollBars(includeWidget, factX * (list.at(2).toInt() + adjustMargin), factY * (list.at(3).toInt() + adjustMargin));


        }
    }

    // Tabbar adjustment
    else if(!className.compare("QTabWidget")) {
#ifdef MOBILE
        if(list.at(4).toInt() < 0) return; // on android I got -1 for these fonts at initialization, i.e pixelsize
        qreal fontSize = (qMin(factX, factY) * (double) list.at(4).toInt());
        if(fontSize < MIN_FONT_SIZE) fontSize = MIN_FONT_SIZE;
        if(fontSize > (double) list.at(4).toInt()) fontSize = (double) list.at(4).toInt();
        QString style= "";
        QTabWidget *box = (QTabWidget *) widget;
        qreal height = 1.0;
        QString thisStyle = "QTabBar::tab {font: %1pt;  height:%2em; padding: %3px;}";
        thisStyle = thisStyle.arg(qRound(fontSize)).arg(height).arg(qRound(5.0*qMin(factX, factY)));

        // get eventual stylesheet from property set at start for addition
        QVariant Style=box->property("Stylesheet");
        if(!Style.isNull()) style = Style.toString();
        box->setStyleSheet(thisStyle + style);
#endif
    }
}

#ifndef MOBILE
// a popup can be displayed when hovering over a widget by taking from the dynamic property caqtdmPopupUI a popup ui file and a macro definition.
// this will only work when the widget has at least one monitor and the file contains the character sequence popup
void CaQtDM_Lib::send_delayed_popup_signal(){
    qCDebug(caQtDMLibLog) << "delayed popup timer signal";
    QVariant dynVars;
    bool marker_delay_popup=false;
    QWidget *w= (QWidget*) Q_NULLPTR;

    QList<QWidget *> list=this->findChildren<QWidget *>();
    QMutableListIterator<QWidget*> i(list);
    while (i.hasNext()&&!marker_delay_popup) {
        w=i.next();
        QString className(w->metaObject()->className());
        if (className.contains("ca")){
            dynVars = w->property("delayed_popup_timer");
            if(dynVars.isValid()) {
                if(dynVars.canConvert<bool>()){
                    marker_delay_popup =dynVars.toBool();
                }
            }
        }
    }
    if (marker_delay_popup){
        QString Filename="";
        QString Args="";
        QString geometry="";

        dynVars = w->property("delayed_popup_filename");
        qCDebug(caQtDMLibLog) << dynVars;
        if(dynVars.canConvert<QString>()){
            Filename =dynVars.toString();
        }
        dynVars = w->property("delayed_popup_args");
        if(dynVars.canConvert<QString>()){
            Args =dynVars.toString();
        }
        dynVars = w->property("delayed_popup_geometry");
        if(dynVars.canConvert<QString>()){
            geometry =dynVars.toString();
        }

        qCDebug(fileIOLog) << Filename << Args <<w;
        if(!Filename.isEmpty())  {
            qCDebug(caQtDMLibLog) << "delayed_popup_timer";
            emit Signal_OpenNewWFile(Filename, Args, geometry, "true ToolTip FramelessWindowHint PopUpWindow");
            w->setProperty("delayed_popup_timer",false);
            w->setProperty("delayed_popup_filename","");
            w->setProperty("delayed_popup_args","");
            w->setProperty("delayed_popup_geometry","");

        }
    }
}

bool CaQtDM_Lib::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::HoverEnter) {
        QWidget *w= (QWidget*) obj;
        QVariant dynVars = w->property("caqtdmPopupUI");
        if(!dynVars.isNull()) {
            if(dynVars.canConvert<QString>())  {
                QString Filename="";
                QString Args="";
                QString geometry="";
                QString popupUI = dynVars.toString().trimmed();
                QStringList popupFields = popupUI.split(";", SKIP_EMPTY_PARTS);

                if(popupFields.size() > 0) {
                    if(popupFields[0].contains(POPUPDEFENITION)){
                        Filename=popupFields[0];
                        if(popupFields.size() > 1) {
                            popupFields[1].replace("\"","");// in case of macro surrounded by double quotes
                            Args=popupFields[1];
                        }
                    }
                }
                QPoint pos = QCursor::pos();
                geometry=QString("+%1+%2\0").arg(pos.x()+5).arg(pos.y()+5);
                qCDebug(caQtDMLibLog) << geometry;
                QVariant delayVars = w->property("caqtdmPopupUI_Delay");
                if(!delayVars.isNull()) {
                    if(delayVars.canConvert<int>())  {
                        int timeout = delayVars.toInt();
                        if(popupFields.size() > 0) {
                            w->setProperty("delayed_popup_timer",true);
                            w->setProperty("delayed_popup_filename",Filename);
                            w->setProperty("delayed_popup_args",Args);
                            w->setProperty("delayed_popup_geometry",geometry);
                            qCDebug(fileIOLog) << Filename << Args <<w;
                            QTimer::singleShot(timeout, this, SLOT(send_delayed_popup_signal()));

                        }
                    }
                }else{
                    if (!Filename.isEmpty()) Signal_OpenNewWFile(Filename, Args, geometry, "true ToolTip FramelessWindowHint PopUpWindow");
                }
            }
        }
    } else if(event->type() == QEvent::HoverLeave) {
        QWidget *wo= (QWidget*) obj;
        QVariant dynVars = wo->property("caqtdmPopupUI");
        if(!dynVars.isNull()) {
            dynVars = wo->property("delayed_popup_timer");
            if(dynVars.isValid()) {
                wo->setProperty("delayed_popup_timer",false);
                wo->setProperty("delayed_popup_filename","");
                wo->setProperty("delayed_popup_args","");
            }
        }
        foreach (QWidget *widget, QApplication::topLevelWidgets()) {
            if (CaQtDM_Lib *w = qobject_cast<CaQtDM_Lib *>(widget)) {
                QVariant is_a_popup = w->property("open_as_popupwindow");
                if (!is_a_popup.isNull()){
                    QVariant fileName = w->property("fileString");
                    QString qs = fileName.toString();
                    if(qs.contains(POPUPDEFENITION)) {
                        w->closeWindow();
                    }
                }
            }
        }
    } else if(event->type() == QEvent::HoverMove) {
        //Update for delay visibility Enter and move are a little bit away
        QWidget *wu= (QWidget*) obj;
        QVariant dynVars = wu->property("caqtdmPopupUI");
        if(!dynVars.isNull()) {
            QPoint pos = QCursor::pos();
            QString geometry="";
            geometry=QString("+%1+%2\0").arg(pos.x()+5).arg(pos.y()+5);
            wu->setProperty("delayed_popup_geometry",geometry);
        }

        foreach (QWidget *widget, QApplication::topLevelWidgets()) {
            if (CaQtDM_Lib *w = qobject_cast<CaQtDM_Lib *>(widget)) {
                QVariant is_a_popup = w->property("open_as_popupwindow");
                if (!is_a_popup.isNull()){
                    QVariant fileName = w->property("fileString");
                    QString qs = fileName.toString();
                    if(qs.contains(POPUPDEFENITION)) {
                        Qt::WindowFlags flags = Qt::ToolTip;
                        flags |= Qt::FramelessWindowHint;
                        w->setWindowFlags(flags);
                        QPoint pos = QCursor::pos();
                        pos.setX(pos.x()+5);
                        pos.setY(pos.y()+5);
                        w->move(pos);
                        w->show();
                    }
                }
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

#endif
// treat gesture events (we use tapandhold and fingerswipe, custom gesture)
#ifdef MOBILE
bool CaQtDM_Lib::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Gesture) {
        return gestureEvent(obj, static_cast<QGestureEvent*>(event));
    }
    return QWidget::eventFilter(obj, event);
}

bool CaQtDM_Lib::gestureEvent(QObject *obj, QGestureEvent *event)
{
    if (QGesture *tapAndHold = event->gesture(Qt::TapAndHoldGesture)) {
        //postMessage(QtDebugMsg, (char*) "tapandhold");
        if (caSlider *widget = qobject_cast<caSlider *>(obj)) {
            if(widget->timerActive()) return false;
        }
        tapAndHoldTriggered(obj, static_cast<QTapAndHoldGesture*>(tapAndHold));
    } else if(QGesture *fingerswipe = event->gesture(fingerSwipeGestureType)) {
        //postMessage(QtDebugMsg, (char*) "fingerSwipeGesture");
        fingerswipeTriggered(static_cast<FingerSwipeGesture *>(fingerswipe));
    }
    return true;
}

void CaQtDM_Lib::tapAndHoldTriggered(QObject *obj, QTapAndHoldGesture* tapAndHold)
{
    if (tapAndHold->state() == Qt::GestureFinished) {
        DisplayContextMenu((QWidget*) obj);
    }
}

void CaQtDM_Lib::fingerswipeTriggered(FingerSwipeGesture *swipe) {
    if (swipe->isLeftToRight()) {
        emit Signal_NextWindow();
        //postMessage(QtDebugMsg, (char*) "leftttoright");
    }
    else if (swipe->isRightToLeft()) {
        //postMessage(QtDebugMsg, (char*) "righttoleft");
        emit Signal_NextWindow();
    }
    else if (swipe->isBottomToTop()) {
        //postMessage(QtDebugMsg, (char*) "bottomtotop");
        closeWindow();
    }
    else if (swipe->isTopToBottom()) {
        //postMessage(QtDebugMsg, (char*) "toptobottom");
        closeWindow();
    }
}

// called from parent to define the custom gesture event
void CaQtDM_Lib::grabSwipeGesture(Qt::GestureType fingerSwipeGestureTypeID)
{
    fingerSwipeGestureType = fingerSwipeGestureTypeID;
    grabGesture(fingerSwipeGestureType);
}
#endif

void CaQtDM_Lib::resizeEvent ( QResizeEvent * event )
{
    double factX, factY;

    qCDebug(caQtDMLibLog) << "resize" << event->size();
    QMainWindow *main = this->findChild<QMainWindow *>();
    // it seems that when mainwindow was fixed by user, then the window stays empty ?
    if(main != (QObject*) Q_NULLPTR) {
        main->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


        if (main->centralWidget()){
            QVariant allowResizeVars = main->centralWidget()->property("allowResize");
            if(!allowResizeVars.isNull()) {
                if(allowResizeVars.canConvert<bool>())  {
                    if(!allowResizeVars.toBool()) return;
                }
            }
        }
    }


    qCDebug(caQtDMLibLog) << "allowResize=" << allowResize;
    // when noresizing then fix the size, however for prc files, we will later shrink the display to a minimumsize, so do not fix then

    if(!allowResize) {
        if(!prcFile) main->setFixedSize(myWidget->size());
        return;
    }

    if(myWidget == (QWidget*) 0) return;

    if(firstResize) {
        firstResize = false;
        // keep original width and height (first event on linux/windows was ui window size, on ios however now display size
        // changed to myWidget size
        origWidth = myWidget->width(); //event->size().width();
        origHeight = myWidget->height(); //event->size().height();
        QList<QWidget *> all = myWidget->findChildren<QWidget *>();
        foreach(QWidget* widget, all) {
            QList<QVariant> integerList;
            QString className(widget->metaObject()->className());
            integerList.insert(0, widget->geometry().x());
            integerList.insert(1, widget->geometry().y());
            integerList.insert(2, widget->geometry().width());
            integerList.insert(3, widget->geometry().height());

            // tell polylinewidget about its actual size for resizing its internals
            if (caPolyLine *polylineWidget = qobject_cast<caPolyLine *>(widget))  {
                polylineWidget->setActualSize(QSize(widget->geometry().width(), widget->geometry().height()));
            }

            // for a horizontal or vertical line get the linewidth and for box the framewidth
            if(!className.compare("caFrame") || !className.compare("QFrame") ) {
                QFrame * line = (QFrame *) widget;
                    integerList.insert(4, line->lineWidth());
                    integerList.insert(5, line->frameWidth());
                // for plots get the linewidth
            } else if(!className.compare("caStripPlot") || !className.compare("caCartesianPlot")) {
                QwtPlot * plot = (QwtPlot *) widget;
                integerList.insert(4, plot->axisFont(QwtPlot::xBottom).pointSize());         // label of ticks
                integerList.insert(5, 9);                                                   // empty
                integerList.insert(6, plot->axisTitle(QwtPlot::xBottom).font().pointSize()); // titles have the same font

                if(!className.compare("caStripPlot")) {
                    caStripPlot * stripplotWidget = (caStripPlot *) widget;
                    integerList.insert(7, 9);
                    if(stripplotWidget->getLegendEnabled()) {
                        stripplotWidget->setLegendAttribute(stripplotWidget->getScaleColor(), QFont("arial", 9), caStripPlot::FONT);
                    }
                } else {
                    caCartesianPlot * cartesianplotWidget = (caCartesianPlot *) widget;
                    integerList.insert(7, 7);
                    if( cartesianplotWidget->getLegendEnabled()) {
                         cartesianplotWidget->setLegendAttribute(cartesianplotWidget->getScaleColor(), QFont("arial", 7), caCartesianPlot::FONT);
                    }
                }
                integerList.insert(8, plot->axisScaleDraw(QwtPlot::xBottom)->tickLength(QwtScaleDiv::MajorTick));
                integerList.insert(9, plot->axisScaleDraw(QwtPlot::xBottom)->tickLength(QwtScaleDiv::MediumTick));
                integerList.insert(10, plot->axisScaleDraw(QwtPlot::xBottom)->tickLength(QwtScaleDiv::MinorTick));
                // take care of the led width and height inside its widget
            } else if (caLed *ledWidget = qobject_cast<caLed *>(widget))  {
                integerList.insert(4, widget->font().pointSize());
                integerList.insert(5, ledWidget->ledWidth());
                integerList.insert(6, ledWidget->ledHeight());

            } else if(!className.compare("QTabWidget")) {
                QTabWidget *tabW = (QTabWidget *) widget;
                integerList.insert(4, widget->font().pointSize());
                tabW->setProperty("Stylesheet", tabW->styleSheet());

            } else {
                integerList.insert(4, widget->font().pointSize());
                // on android the above does not work always, the instruction below works, but then the fontscaling
                // may not work at all, due to the pixelratio
                integerList.insert(5, QFontInfo(widget->font()).pointSize());
            }
            widget->setProperty("GeometryList", integerList);
        }
        CartesianPlotsVerticalAlign();
        StripPlotsVerticalAlign();
        return;
    }

    factX = (double) event->size().width() / (double) origWidth;
    factY = (double) event->size().height() / (double) origHeight;

    // for cainclude, we need when updating internal positions to know about the resize factors
    this->setProperty("RESIZEX", factX);
    this->setProperty("RESIZEY", factY);

    QString classNam;

    bool mainlayoutPresent = false;
    if ((main)&&(main->centralWidget())){
        QVariant mainlayoutVars = main->centralWidget()->property("mainlayoutPresent");
        if(!mainlayoutVars.isNull()) {
            if(mainlayoutVars.canConvert<bool>())  {
                mainlayoutPresent = mainlayoutVars.toBool();
            }
        }
    }
    if(main == (QObject*) Q_NULLPTR) {
        QDialog *dialog = this->findChild<QDialog *>();
        if(dialog == (QObject*) Q_NULLPTR) return;  // if not a mainwindow or dialog get out
        if(dialog->layout() != (QObject*) Q_NULLPTR) {
            classNam = dialog->layout()->metaObject()->className();
            mainlayoutPresent = true;
        }
    } else {
        if( (main->centralWidget() != (QObject*) 0) && (main->centralWidget()->layout() != (QObject*) Q_NULLPTR)) {
            classNam = main->centralWidget()->layout()->metaObject()->className();
            mainlayoutPresent = true;
        }
    }

    qCDebug(caQtDMLibLog) << classNam << mainlayoutPresent;

    // if our window is not using a layout, then we we have to do the resizing ourselves
    if(!mainlayoutPresent) {
        qCDebug(caQtDMLibLog) << "no main layout present, we should do the work";

        // if our window is using a layout, then Qt has to do the resizing
    } else {
        qCDebug(caQtDMLibLog) << "main layout present, Qt should do the work" << classNam;

        // centralwidget should manage the layout itsself, we do nothing except changing font for some classes
        if(classNam.contains("Layout")) {
            // resize some minor stuff before leaving this routine
            QList<QWidget *> all = myWidget->findChildren<QWidget *>();
            foreach(QWidget* widget, all) {
                QString className(widget->metaObject()->className());
                QVariant var=widget->property("GeometryList");
                QVariantList list = var.toList();

                if(list.size() >= 4) {
                    resizeSpecials(className, widget, list, factX, factY);
                }
            }
            return;
        }
    }

    // do all resize work ourselves
    QList<QWidget *> all = myWidget->findChildren<QWidget *>();
    foreach(QWidget* widget, all) {
        QString className(widget->metaObject()->className());

        // give wmsignalpropagator the resize factors
        if(!className.compare("wmSignalPropagator")) {
            wmSignalPropagator *signalWidget = (wmSignalPropagator *) widget;
            signalWidget->setResizeFactors(factX, factY);
        }

        if(     !className.contains("QMainWindow")  &&
                !className.contains("QRubberBand")  &&
                !className.contains("Qwt")    &&
                !className.contains("QWidget")    &&

                (className.contains("ca") || className.contains("Q") || className.contains("Line") || !className.compare("wmSignalPropagator") ||
                 className.compare("replacemacro"))
                ) {
            QWidget *w = (QWidget*) widget->parent();
            // if this widget is managed by a layout, do not do anything
            // parent is a layout and must be resized and repositioned

            if((w->layout() != (QObject*) Q_NULLPTR) && (w->objectName().contains("layoutWidget"))) {
                w->setGeometry(widgetResize(w,factX,factY));
                w->updateGeometry();

                // not a layout, widget has to be resized and repositioned
            } else {
                QVariant var=widget->property("GeometryList");
                QVariantList list = var.toList();

                if(list.size() >= 4) {

                    // special case, the position and size of the widget is signal driven, in this case
                    // we need the actual variables.
                    if(widget->property("SIGNALDRIVEN").value<bool>()) {
                        qCDebug(caQtDMLibLog) << widget << "is signaldriven";
                        list[0].setValue(widget->x()/factX);
                        list[1].setValue(widget->y()/factY);
                        list[2].setValue(widget->width()/factX);
                        list[3].setValue(widget->height()/factY);
                    }

                    /* we have to correct first the led width and height before changing the geometry */
                    if (!className.compare("caLed")) {
                        caLed *ledWidget = (caLed *) widget;
                        double width = (double) list.at(5).toInt() * factX;
                        double height = (double) list.at(6).toInt() * factY;
                        if(width < 1.0) width=1.0;
                        if(height < 1.0) height = 1.0;
                        ledWidget->setLedHeight(qRound(height));
                        ledWidget->setLedWidth(qRound(width));
                    }
                    // this is something new (Qt5.13), probably a bug for android and a very dirty fix
#ifdef MOBILE_ANDROID
                    if (!className.compare("caMenu")) {
                        widget->setMinimumWidth(10);
                        double x = (double) list.at(0).toInt() * factX;
                        double y = (double) list.at(1).toInt() * factY;
                        double width = (double) list.at(2).toInt() * factX;
                        double height = (double) list.at(3).toInt() * factY;
                        if(width < 1.0) width=1.0;
                        if(height < 1.0) height = 1.0;
                        QRect rectnew = QRect(qRound(x), qRound(y), qRound(width), qRound(height));
                        rectnew.setWidth(width/2.5);
                    }
#endif

                    widget->setGeometry(widgetResize(widget,factX,factY));
                    resizeSpecials(className, widget, list, factX, factY);
                    widget->updateGeometry();

                }
            }

        }
    }

    CartesianPlotsVerticalAlign();
    StripPlotsVerticalAlign();

}

/**
 *  align cartesian plots vertically when they belong to the same group
 */
void CaQtDM_Lib::CartesianPlotsVerticalAlign()
{
    qCDebug(caCartesianPlotLog) << "cartesian groups nb=" << cartesianGroupList.count();
    if(cartesianGroupList.count() < 1) return;

    // go through our groups
   foreach(int group, cartesianGroupList) {
       int maxExtent = 0;

       // get for this group the maximum extent of scale (probably does not work with autoscale)
       QMap<int,caCartesianPlot* >::iterator i = cartesianList.find(group);
       while (i != cartesianList.end() && i.key() == group) {
           QwtScaleWidget *scaleWidget = i.value()->axisWidget(QwtPlot::yLeft);
           QwtScaleDraw *sd = scaleWidget->scaleDraw();
           sd->setMinimumExtent(0);
           const int extent = qRound(sd->extent(scaleWidget->font()));
           if ( extent > maxExtent ) maxExtent = extent;
           ++i;
       }

       // now set the correct left scale extent to the group
       i = cartesianList.find(group);
       while (i != cartesianList.end() && i.key() == group) {
           QwtScaleWidget *scaleWidget = i.value()->axisWidget(QwtPlot::yLeft);
           qCDebug(caCartesianPlotLog) << i.value() << maxExtent;
           scaleWidget->scaleDraw()->setMinimumExtent(maxExtent);
           ++i;
       }
   }
}

/**
 *  align strip plots vertically when they belong to the same group
 */
void CaQtDM_Lib::StripPlotsVerticalAlign()
{
    qCDebug(caStripPlotLog) << "strip groups nb=" << stripGroupList.count();
    if(stripGroupList.count() < 1) return;

    // go through our groups
   foreach(int group, stripGroupList) {
       int maxExtent = 0;

       // get for this group the maximum extent of scale (probably does not work with autoscale)
       QMap<int,caStripPlot* >::iterator i = stripList.find(group);
       while (i != stripList.end() && i.key() == group) {
           QwtScaleWidget *scaleWidget = i.value()->axisWidget(QwtPlot::yLeft);
           QwtScaleDraw *sd = scaleWidget->scaleDraw();
           sd->setMinimumExtent(0);
           const int extent = qRound(sd->extent(scaleWidget->font()));
           if ( extent > maxExtent ) maxExtent = extent;
           ++i;
       }

       // now set the correct left scale extent to the group
       i = stripList.find(group);
       while (i != stripList.end() && i.key() == group) {
           QwtScaleWidget *scaleWidget = i.value()->axisWidget(QwtPlot::yLeft);
           qCDebug(caStripPlotLog) << i.value() << maxExtent;
           scaleWidget->scaleDraw()->setMinimumExtent(maxExtent);
           ++i;
       }
   }
}

void CaQtDM_Lib::Callback_WriteDetectedValues(QWidget* child)
{
    double x,y,w,h;
    int count=4;
    double values[4] = {0,0,0,0};

    QStringList thisString;
    QWidget *widget = (QWidget *) 0;
    QPointF P1, P2;

    // must fit the definitions in cacamera and cascan2d
    enum ROI_type {none=0, xy_only, xy1_xy2, xyUpleft_xyLowright, xycenter_width_height};
    enum ROI_markertype {box=0, line, arrow};

    ROI_type roiType;

    if (caCamera *cameraWidget = qobject_cast<caCamera *>(child)) {
        roiType = (ROI_type) cameraWidget->getROIwriteType();
        widget = (QWidget*) cameraWidget;
        cameraWidget->getROI(P1, P2);
        thisString = cameraWidget->getROIChannelsWrite().split(";");
    } else if (caScan2D *scan2dWidget = qobject_cast<caScan2D *>(child)) {
        roiType = (ROI_type) scan2dWidget->getROIwriteType();
        widget = (QWidget*) scan2dWidget;
        scan2dWidget->getROI(P1, P2);
        thisString = scan2dWidget->getROIChannelsWrite().split(";");
    } else {
        return;
    }

    double deltax = -(P1.x() - P2.x());
    double deltay = -(P1.y() - P2.y());

    switch (roiType) {
    case none:
        return;
    case xy_only:
        count = 2;
        values[0] = P1.x();
        values[1] = P1.y();
        break;
    case xy1_xy2:
        values[0] = P1.x();
        values[1] = P1.y();
        values[2] = P2.x();
        values[3] = P2.y();
        break;
    case xyUpleft_xyLowright:
        if(deltax < 0 && deltay > 0) {
            values[0] = P2.x();
            values[1] = P1.y();
            values[2] = P1.x();
            values[3] = P2.y();
        } else if(deltax < 0 && deltay < 0) {
            values[0] = P2.x();
            values[1] = P2.y();
            values[2] = P1.x();
            values[3] = P1.y();
        } else if(deltax > 0 && deltay < 0) {
            values[0] = P1.x();
            values[1] = P2.y();
            values[2] = P2.x();
            values[3] = P1.y();
        } else if(deltax > 0 && deltay > 0) {
            values[0] = P1.x();
            values[1] = P1.y();
            values[2] = P2.x();
            values[3] = P2.y();
        }
        break;
    case xycenter_width_height:
    {
        double ROIx = x = P1.x();
        double ROIy = y = P1.y();
        double ROIw = w = P2.x() - P1.x();
        double ROIh = h = P2.y() - P1.y();
        if(ROIw < 0) { x = ROIx + ROIw; w = -ROIw;}
        if(ROIh < 0) { y = ROIy + ROIh; h = -ROIh;}
        values[0] = x+w/2.0;
        values[1] = y+h/2.0;
        values[2]=w;
        values[3]=h;
    }
        break;

    default:
        return;
    }

    if (thisString.count() != count) {
        QString message="not the correct amount of PVs defined for writing ROI values: ";
        message.append(child->objectName());
        messageWindowP->postMsgEvent(QtInfoMsg,  (char*)qasc(message));
        return;
    }

    for(int i=0; i<count; i++) {
        int32_t idata = (int32_t) values[i];
        double rdata = (double) values[i];
        if(thisString.at(i).trimmed().length() > 0) {
            TreatOrdinaryValue(thisString.at(i), rdata,  idata, "", widget);
        }
    }
}

// wrappers for any caWidget with caWidgetInterface (perhaps more needed)
knobData* CaQtDM_Lib::GetMutexKnobDataPtr(int index){
    return mutexKnobDataP->GetMutexKnobDataPtr(index);
}
knobData* CaQtDM_Lib::GetMutexKnobDataPV(QWidget *widget, QString pv){
    return mutexKnobDataP->getMutexKnobDataPV(widget, pv);
}

// initiate drag, one will be able to drop to another Qt-application
void CaQtDM_Lib::mousePressEvent(QMouseEvent *event)
{
    if((event->button() == Qt::LeftButton) ||  (event->button() == Qt::RightButton)) {
        QWidget *w = static_cast<QWidget*>(childAt(event->pos()));
        // this will take the focus away (for caTextEntry mainly, when keepFocus was set)
        // however we have to be carefull with caNumeric keyboard handling
        QLabel *w1 = qobject_cast<QLabel *>(w);
        if(w1 == (QObject*) 0) {
            myWidget->setFocus();
        }
        return;
    }
    QWidget *w = static_cast<QWidget*>(childAt(event->pos()));
    if (!w) return;

    QDrag *drag = new QDrag(this);
    QPoint hotSpot = event->pos() - w->pos();
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-hotspot", QByteArray::number(hotSpot.x()) + " " + QByteArray::number(hotSpot.y()));


    if(caWidgetInterface* wif = dynamic_cast<caWidgetInterface *>(w)) {  // any caWidget with caWidgetInterface
        mimeData->setText(wif->getDragText());
    } else if(caCalc *calcWidget = qobject_cast<caCalc *>(w)) {
        mimeData->setText(calcWidget->getVariable());
    } else if (caMenu *menu1Widget = qobject_cast<caMenu *>(w)) {
        mimeData->setText(menu1Widget->getPV());
    } else if (caMenu *menu2Widget = qobject_cast<caMenu *>(w->parent())) {
        mimeData->setText(menu2Widget->getPV());
    } else if (caChoice *choice1Widget = qobject_cast<caChoice *>(w)) {
        mimeData->setText(choice1Widget->getPV());
    } else if (caChoice *choice2Widget = qobject_cast<caChoice *>(w->parent())) {
        mimeData->setText(choice2Widget->getPV());
    } else if (caThermo *thermoWidget = qobject_cast<caThermo *>(w)) {
        mimeData->setText(thermoWidget->getPV());
    } else if (caSlider *sliderWidget = qobject_cast<caSlider *>(w)) {
        mimeData->setText(sliderWidget->getPV());
    } else if (caClock *clockWidget = qobject_cast<caClock *>(w)) {
        mimeData->setText(clockWidget->getPV());
    } else if (caMeter *meterWidget = qobject_cast<caMeter *>(w)) {
        mimeData->setText(meterWidget->getPV());
    } else if (caLinearGauge *lineargaugeWidget = qobject_cast<caLinearGauge *>(w)) {
        mimeData->setText(lineargaugeWidget->getPV());
    } else if (caCircularGauge *circulargaugeWidget = qobject_cast<caCircularGauge *>(w)) {
        mimeData->setText(circulargaugeWidget->getPV());
    } else if (caByte *byteWidget = qobject_cast<caByte *>(w->parent())) {
        mimeData->setText(byteWidget->getPV());
    } else if (caByteController *bytecontrollerWidget = qobject_cast<caByteController *>(w->parent())) {
        mimeData->setText(bytecontrollerWidget->getPV());
    } else if (caLineEdit *lineeditWidget = qobject_cast<caLineEdit *>(w)) {
        lineeditWidget->setEnabled(true);  // enable after initiating drag for context menu
        mimeData->setText(lineeditWidget->getPV());
    } else if (caMultiLineString *multilinestringWidget = qobject_cast<caMultiLineString *>(w->parent())) {
        multilinestringWidget->setEnabled(true);  // enable after initiating drag for context menu
        mimeData->setText(multilinestringWidget->getPV());
    } else if (caLed *ledWidget = qobject_cast<caLed *>(w)) {
        mimeData->setText(ledWidget->getPV());
    } else if (caApplyNumeric *applynumeric1Widget = qobject_cast<caApplyNumeric *>(w)) {
        mimeData->setText(applynumeric1Widget->getPV());
    } else if (caApplyNumeric *applynumeric2Widget = qobject_cast<caApplyNumeric *>(w->parent())) {
        mimeData->setText(applynumeric2Widget->getPV());
    } else if (caApplyNumeric *applynumeric3Widget = qobject_cast<caApplyNumeric *>(w->parent()->parent())) {
        mimeData->setText(applynumeric3Widget->getPV());
    } else if (caNumeric *numeric1Widget = qobject_cast<caNumeric *>(w)) {
        mimeData->setText(numeric1Widget->getPV());
    } else if (caNumeric *numeric2Widget = qobject_cast<caNumeric *>(w->parent())) {
        mimeData->setText(numeric2Widget->getPV());
    } else if (caSpinbox *spinboxWidget = qobject_cast<caSpinbox *>(w)) {
        mimeData->setText(spinboxWidget->getPV());
    } else if (caToggleButton *togglebuttonWidget = qobject_cast<caToggleButton *>(w)) {
        mimeData->setText(togglebuttonWidget->getPV());
    } else if (caCamera *cameraWidget = qobject_cast<caCamera *>(w->parent()->parent()->parent())) {
        mimeData->setText(cameraWidget->getPV_Data());
    } else if (caMessageButton *messagebuttonWidget = qobject_cast<caMessageButton *>(w)) {
        mimeData->setText(messagebuttonWidget->getPV());
    } else if (caImage *imageWidget = qobject_cast<caImage *>(w->parent())) {
        mimeData->setText(imageWidget->getChannelA());
    } else if (caGraphics *graphicsWidget = qobject_cast<caGraphics *>(w)) {
        mimeData->setText(graphicsWidget->getChannelA());
    } else if (caLabel *labelWidget = qobject_cast<caLabel *>(w)) {
        mimeData->setText(labelWidget->getChannelA());
    } else if (caLabelVertical *labelverticalWidget = qobject_cast<caLabelVertical *>(w)) {
        mimeData->setText(labelverticalWidget->getChannelA());
    } else if (caPolyLine *polylineWidget = qobject_cast<caPolyLine *>(w)) {
        mimeData->setText(polylineWidget->getChannelA());
    } else if (caWaterfallPlot *waterfallplotWidget = qobject_cast<caWaterfallPlot *>(w->parent()->parent())) {
        mimeData->setText(waterfallplotWidget->getPV());
    } else {
        qCDebug(caQtDMLibLog) << "unrecognized widget" << w;
        return;
    }

    // put it also to the clipboard (usefull for microsoft windows)
    const QString text = qasc(mimeData->text());
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text, QClipboard::Clipboard);

    // build a pixmap from pv text
    QFont f = font();
    QFontMetrics metrics(f);
    int width = QMETRIC_QT456_FONT_WIDTH(metrics,mimeData->text())+20;
    //int width = metrics.width(mimeData->text() + 20);
    int height = (int) ((float) metrics.height() * 1.5);

    QPixmap pixmap(width, height);
    pixmap.fill(Qt::black);
    QPainter painter(&pixmap);
    painter.setPen(Qt::red);
    painter.setFont(f);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.drawText(5,height/2+5,mimeData->text() );
    painter.end();

    // set it to the drag structure

    drag->setPixmap(pixmap);
    drag->setMimeData(mimeData);

    QPoint hotSpotNew(-10,-5);

    drag->setHotSpot(hotSpotNew);
    drag->setPixmap(pixmap);

    drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
}

ControlsInterface * CaQtDM_Lib::getPluginInterface(QWidget *w)
{
    QVariant var = w->property("Interface");
    QVariantList list = var.toList();
    if(list.count() > 0) {
        void *ptr = (void*) list.at(0).value<void *>();
        return  (ControlsInterface *) ptr;
    } else {
        return (ControlsInterface *) Q_NULLPTR;
    }
}

QStringList CaQtDM_Lib::treat_read_MacroCommand(QStringList args){

    // special case where macros are coming from a macro definition file
    // when specified with %(read filename) in the argument list

    QString pattern="^\\s*\\%\\s*\\(\\s*read\\s+(.+)\\)$";
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QRegExp re(pattern);
#else
    QRegularExpression re(pattern);
#endif
    for (int j = 0; j < args.count(); ++j) {
        QStringList macro_list = args[j].split(",");
        QStringList macro_list_expanded;
        for (int k = 0; k < macro_list.count(); ++k) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            int match = re.indexIn(macro_list[k]);
            if (match >= 0) {
                QString macroFile = re.cap(1);
#else
            QRegularExpressionMatch match = re.match(macro_list[k]);
            if (match.hasMatch()) {
                QString macroFile = match.captured(1);
#endif
                if(macroFile.length() > 0) {
                    searchFile *s = new searchFile(macroFile);
                    QString fileNameFound = s->findFile();
                    char asc[MAX_STRING_LENGTH];
                    if(fileNameFound.isNull()) {
                        snprintf(asc, MAX_STRING_LENGTH, "macro definition file %s could not be loaded for related display", qasc(macroFile));
                        postMessage(QtCriticalMsg, asc);
                    }
                    else {
                        snprintf(asc, MAX_STRING_LENGTH, "macro definition file %s loaded for related display", qasc(macroFile));
                        postMessage(QtWarningMsg, asc);
                        QString macroString;
                        QFile file(fileNameFound);
                        if (file.open(QFile::ReadOnly)) {
                            macroString = QLatin1String(file.readAll());
                            macroString = macroString.simplified().trimmed();
                            macroString.replace(" ",",");
                            file.close();
                        }
                        QStringList macro_list_from_file = macroString.split(",");
                        macro_list_expanded = macro_list_expanded + macro_list_from_file;
                    }
                }
            }
            else {
                macro_list_expanded.append(macro_list[k]);
            }
        }
        args[j] = macro_list_expanded.join(",");
    }
    //to resolve macros from the files and configured object
    for( int i=0; i<args.count(); ++i )
    {
         bool doNothing;
         QMap<QString, QString> include_map = createMap(args[i]);
         args[i] = treatMacro(include_map, args[i], &doNothing, "");
    }

    return args;
}

void CaQtDM_Lib::themeChanged() {
    QApplication* guiApp = qobject_cast<QApplication*>(qApp);
    QPalette palette = guiApp->palette();
    QString oldColorNormalHex = m_normalTextColorHex;
    QString oldColorDebugHex = m_debugTextColorHex;
    m_normalTextColorHex = palette.color(QPalette::Active, QPalette::Text).name();
    m_debugTextColorHex = palette.color(QPalette::Active, QPalette::Link).name();

 }


#include "loadPlugins.h"
#include "ui_main.h"

extern "C"  {

    QMainWindow *myWidget;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void myMessageOutput(QtMsgType type, const char *msg)
    {
        switch (type) {
        case QtDebugMsg:
            fprintf(stderr, "Debug: %s\n", msg);
            break;
        case QtWarningMsg:
            //fprintf(stderr, "Warning: %s\n", msg);
            break;
        case QtCriticalMsg:
            fprintf(stderr, "Critical: %s\n", msg);
            break;
        case QtFatalMsg:
            fprintf(stderr, "Fatal: %s\n", msg);
            abort();
        }
    }
#else
    void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        QByteArray localMsg = msg.toLocal8Bit();
        const char *file = context.file ? context.file : "";
        const char *function = context.function ? context.function : "";
        switch (type) {
        case QtDebugMsg:
            fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            break;
        case QtInfoMsg:
            fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            break;
        case QtWarningMsg:
            fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            break;
        case QtCriticalMsg:
            fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            break;
        case QtFatalMsg:
            fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            break;
        }
    }
#endif
    int caQtDM_Create (char* filename) {
        int argc = 0;
        char *argv[1];
        Ui::MainWindow ui;

        // set for epics longer waveforms
        QString maxBytes = (QString)  qgetenv("EPICS_CA_MAX_ARRAY_BYTES");
        if(maxBytes.size() == 0) qputenv("EPICS_CA_MAX_ARRAY_BYTES", QByteArray("150000000"));

        QMap<QString, ControlsInterface*> interfaces;
        QMap<QString, QString> OptionList;

        QString macroS;
        QApplication app(argc, argv);
        QApplication::setOrganizationName("Paul Scherrer Institut");
        QApplication::setApplicationName("caQtDM");


        QString FileName(filename);
        searchFile *filecheck = new searchFile(FileName);
        FileName=filecheck->findFile();
        delete filecheck;
        if (FileName.isNull()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
            qCFatal(fileIOLog) << "file" << FileName << "could not be loaded -> exit";
#else
            qFatal("%s", qasc(QString("file " + FileName + " could not be loaded -> exit")));
#endif
        } else {
            qCInfo(fileIOLog) << "file" << FileName << "will be loaded";
        }

        QMainWindow *widget = new QMainWindow;
        ui.setupUi(widget);
        widget->show();

        MessageWindow *messageWindow = new MessageWindow(widget);
        MutexKnobData *mutexKnobData = new MutexKnobData();

        messageWindow->setAllowedAreas(Qt::TopDockWidgetArea);
        QGridLayout *gridLayoutCentral = new QGridLayout(ui.centralwidget);
        QGridLayout *gridLayout = new QGridLayout();
        gridLayoutCentral->addLayout(gridLayout, 0, 0, 1, 1);
        gridLayout->addWidget(messageWindow, 0, 0, 1, 1);
        messageWindow->show();

        // load the control plugins (must be done after setting the environment)
        loadPlugins loadplugins;
        if (!loadplugins.loadAll(interfaces, mutexKnobData, messageWindow, OptionList )) {
            //QMessageBox::critical(this, "Error", "Could not load any plugin");
            QMessageBox(QMessageBox::Information, "Error", "Could not load any plugin", QMessageBox::Yes|QMessageBox::No).exec();
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        qInstallMsgHandler(myMessageOutput);
#else
         qInstallMessageHandler(myMessageOutput);
#endif
        QMainWindow *pWindow =  new CaQtDM_Lib(Q_NULLPTR, FileName, macroS, mutexKnobData, interfaces);
        pWindow->show();

        myWidget = pWindow;        
        return app.exec();
    }

    int getDataValue(char *object, char *pv, int pvMaxLength, double *value)
    {
        bool ok = false;
        QList<QWidget *> all = myWidget->findChildren<QWidget *>();
        foreach(QWidget* widget, all) {
            if(widget->objectName().contains(object)) {
                if(pvMaxLength > 1) qstrncpy(pv, " ",(size_t) pvMaxLength);
                if (caSlider *w = qobject_cast<caSlider *>(widget)) {
                    *value = w->getSliderValue();
                    qstrncpy(pv,  qasc(w->getPV()),(size_t) pvMaxLength);
                    ok = true;
                } else if (caNumeric *w = qobject_cast<caNumeric *>(widget)) {
                    *value = w->value();
                    qstrncpy(pv,  qasc(w->getPV()),(size_t) pvMaxLength);
                    ok = true;
                } else if (caTextEntry *w = qobject_cast<caTextEntry *>(widget)) {
                    *value = w->text().toDouble();
                    qstrncpy(pv,  qasc(w->getPV()),(size_t) pvMaxLength);
                    ok = true;
                } else if (caSpinbox *w = qobject_cast<caSpinbox *>(widget)) {
                    *value = w->value();
                    qstrncpy(pv,  qasc(w->getPV()),(size_t) pvMaxLength);
                    ok = true;
                } else if (caLineEdit *w = qobject_cast<caLineEdit *>(widget)) {
                    *value = w->text().toDouble();
                    qstrncpy(pv,  qasc(w->getPV()),(size_t) pvMaxLength);
                    ok = true;
                } else if (caMeter *w = qobject_cast<caMeter *>(widget)) {
                    *value = w->value();
                    qstrncpy(pv,  qasc(w->getPV()),(size_t) pvMaxLength);
                    ok = true;
                } else if (caThermo *w = qobject_cast<caThermo*>(widget)) {
                    *value = w->value();
                    qstrncpy(pv,  qasc(w->getPV()),(size_t) pvMaxLength);
                    ok = true;
                } else if (caToggleButton *w = qobject_cast<caToggleButton*>(widget)) {
                    *value = w->isChecked();
                    qstrncpy(pv,  qasc(w->getPV()),(size_t) pvMaxLength);
                    ok = true;
                } else if (QCheckBox *w = qobject_cast<QCheckBox*>(widget)) {
                    *value = w->isChecked();
                    ok = true;
                } else if (QPushButton *w = qobject_cast<QPushButton*>(widget)) {
                    *value = w->isChecked();
                    ok = true;
                }
            }
        }
        return ok;
    }

    int getDataString(char *object, char *pv, int pvMaxLength, char *value, int valueMaxLength)
    {
        bool ok = false;
        QList<QWidget *> all = myWidget->findChildren<QWidget*>();
        foreach(QWidget* widget, all) {

            if(widget->objectName().contains(object)) {

                if (caMenu *w = qobject_cast<caMenu*>(widget)) {
                    qstrncpy(pv,  qasc(w->getPV()), (size_t) pvMaxLength);
                    qstrncpy(value,  qasc(w->currentText()),(size_t) valueMaxLength);
                    ok = true;
                }
            }
        }
        return ok;
    }

    int setDataValue(char *object, double value, char *unit)
    {
        bool ok = false;
        QString Unit(unit);
        QList<QWidget *> all = myWidget->findChildren<QWidget *>();
        foreach(QWidget* widget, all) {
            if(widget->objectName().contains(object)) {

                if (caSlider *w = qobject_cast<caSlider *>(widget)) {
                    w->setValue(value);
                    ok = true;
                } else if (caTextEntry *w = qobject_cast<caTextEntry *>(widget)) {
                    w->setValue(value);
                    ok = true;
                } else if (caNumeric *w = qobject_cast<caNumeric *>(widget)) {
                    w->setValue(value);
                    ok = true;
                } else if (caSpinbox *w = qobject_cast<caSpinbox *>(widget)) {
                    w->setValue(value);
                    ok = true;
                } else if (caLineEdit *w = qobject_cast<caLineEdit *>(widget)) {
                    w->setValue(value, Unit);
                    ok = true;
                } else if (caMeter *w = qobject_cast<caMeter *>(widget)) {
                    w->setValueUnits(value, Unit);
                    ok = true;
                } else if (caThermo *w = qobject_cast<caThermo*>(widget)) {
                    w->setValue(value);
                    ok = true;
                } else if (caByte *w = qobject_cast<caByte*>(widget)) {
                    w->setValue((long) value);
                    ok = true;
                } else if (caToggleButton *w = qobject_cast<caToggleButton*>(widget)) {
                    if((bool) value) w->setChecked(true);
                    else w->setChecked(value);
                    ok = true;
                } else if (QPushButton *w = qobject_cast<QPushButton*>(widget)) {
                    if((bool) value) w->setChecked(true);
                    else w->setChecked(value);
                    ok = true;
                } else if (QCheckBox *w = qobject_cast<QCheckBox*>(widget)) {
                    if((bool) value) w->setChecked(true);
                    else w->setChecked(value);
                    ok = true;
                }
            }
        }
        return ok;
    }
}
