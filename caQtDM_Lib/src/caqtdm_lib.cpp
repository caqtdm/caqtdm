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
#include <sys/timeb.h>
#include <QObject>
#include <QToolBar>

// we are using for calculations postfix of epics
// therefore we need this include and also link with the epics libraries
// should probably be changed at some point.
#include <postfix.h>

#ifdef MOBILE_ANDROID
#  include <unistd.h>
#endif

#ifdef linux
#  include <sys/wait.h>
#  include <unistd.h>
#endif

#include "caqtdm_lib.h"
#include "parsepepfile.h"
#include "fileFunctions.h"

#include "myMessageBox.h"
#include "alarmstrings.h"

#ifdef AUSTRALIAN
#include <QEWidget.h>
#endif

#define PRINT(x)
#define min(x,y)   (((x) < (y)) ? (x) : (y))

#define ToolTipPrefix "<p style='background-color:yellow'><font color='#000000'>"
#define ToolTipPostfix "</font></font></p>"

#define InfoPrefix "<p style='background-color:lightyellow'><font color='#000000'>"
#define InfoPostfix "</font></font></p>"

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
    char param1[MAXPVLEN], param2[255], param3[80]; \
    int size1, size2, size3; \
    QByteArray Parameter_1 = x.toLatin1().constData(); \
    QByteArray Parameter_2 = y.toLatin1().constData(); \
    QByteArray Parameter_3 = z.toLatin1().constData(); \
    size1 = qMin(Parameter_1.size(), MAXPVLEN); \
    size2 = qMin(Parameter_2.size(), 255); \
    size3 = qMin(Parameter_3.size(), 80); \
    strncpy(param1, Parameter_1.constData(), size1); \
    strncpy(param2, Parameter_2.constData(), size2); \
    strncpy(param3, Parameter_3.constData(), size3); \
    param1[size1] = '\0'; \
    param2[size2] = '\0'; \
    param3[size3] = '\0';

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

// get visibility channels for the info box
#define getAllPVs(obj) \
    pv[nbPV++] = obj->getChannelA().trimmed(); \
    pv[nbPV++] = obj->getChannelB().trimmed(); \
    pv[nbPV++] = obj->getChannelC().trimmed(); \
    pv[nbPV++] = obj->getChannelD().trimmed(); \

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
    myMenu.addAction("Get Info"); \
    myMenu.addAction("Set Greyscale"); \
    myMenu.addAction("Set Spectrum Wavelength"); \
    myMenu.addAction("Set Spectrum Hot"); \
    myMenu.addAction("Set Spectrum Heat"); \
    myMenu.addAction("Set Spectrum Jet"); \
    myMenu.addAction("Set Spectrum Custom"); \

//===============================================================================================

#define MIN_FONT_SIZE 4

Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QTabWidget*)

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

/**
 * CaQtDM_Lib destructor
 */
CaQtDM_Lib::~CaQtDM_Lib()
{

    disconnect(mutexKnobDataP,
               SIGNAL(Signal_UpdateWidget(int, QWidget*, const QString&, const QString&, const QString&, knobData)), this,
               SLOT(Callback_UpdateWidget(int, QWidget*, const QString&, const QString&, const QString&, knobData)));
    /*
    //qDebug() << "nb elements:" << includeWidgetList.count();
    for (int i = includeWidgetList.count()-1; i >= 0; --i) {
        QWidget *widget;
        widget= includeWidgetList.at(i);
        //qDebug() << "delete" << widget;
        delete widget;
    }
*/
    if(!fromAS) delete myWidget;
    includeWidgetList.clear();
    topIncludesWidgetList.clear();
}

/**
 * CaQtDM_Lib constructor
 */
CaQtDM_Lib::CaQtDM_Lib(QWidget *parent, QString filename, QString macro, MutexKnobData *mKnobData, QMap<QString, ControlsInterface *> interfaces, MessageWindow *msgWindow, bool pepprint, QWidget *parentAS) : QMainWindow(parent)
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

    // file watcher for changes
    watcher = new QFileSystemWatcher(this);
    QObject::connect(watcher, SIGNAL(fileChanged(const QString&)), this, SLOT(handleFileChanged(const QString&)));

    if(parentAS != (QWidget*) 0) {
        fromAS = true;
        myWidget = parentAS;
    }

    //qDebug() << "open file" << filename << "with macro" << macro;
    setAttribute(Qt::WA_DeleteOnClose);

    includeFiles = "";

    // define a layout
    QGridLayout *layout = new QGridLayout;

    // define the file to use
    QFile *file = new QFile;
    file->setFileName(filename);

    // treat ui file */
    QFileInfo fi(filename);

    includeFiles.append(fi.absoluteFilePath());
    includeFiles.append("<br>");

    if(!fromAS) {
        if(filename.lastIndexOf(".ui") != -1) {

            file->open(QFile::ReadOnly);

            myWidget = loader.load(file, this);

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

            QString uiString = QString(uiIntern);
            uiString= uiString.arg(filename);
            QByteArray *array= new QByteArray();
            array->append(uiString);

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

        } else {
            qDebug() << "caQtDM -- internal error with fileName= " << filename;
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
            reaffectText(map, &Title);
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

    // connect close launchfile action to parent
    connect(this, SIGNAL(Signal_IosExit()), parent, SLOT(Callback_IosExit()));

    qRegisterMetaType<knobData>("knobData");

    // connect signals to slots for exchanging data
    connect(mutexKnobDataP, SIGNAL(Signal_QLineEdit(const QString&, const QString&)), this,
            SLOT(Callback_UpdateLine(const QString&, const QString&)));

    connect(mutexKnobDataP,
            SIGNAL(Signal_UpdateWidget(int, QWidget*, const QString&, const QString&, const QString&, knobData)), this,
            SLOT(Callback_UpdateWidget(int, QWidget*, const QString&, const QString&, const QString&, knobData)));

    if(!fromAS) {
        connect(this, SIGNAL(Signal_OpenNewWFile(const QString&, const QString&, const QString&)), parent,
                SLOT(Callback_OpenNewFile(const QString&, const QString&, const QString&)));
    }

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenu(const QPoint&)));

    level=0;

    // say for all widgets that they have to be treated, will be set to true when treated to avoid multiple use
    // by findChildren, and get the list of all the includes at this level

    includeWidgetList.clear();
    topIncludesWidgetList.clear();

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

    if(nbIncludes > 0) {
        splash = new SplashScreen(parent);
        splash->setMaximum(nbIncludes);
        splash->show();
        splash->setProgress(0);
    }

    savedFile[0] = fi.baseName();
    savedMacro[0] = macro;

    scanWidgets(myWidget->findChildren<QWidget *>(), macro);

    // build a list for getting all soft pv
    mutexKnobDataP->BuildSoftPVList(myWidget);

    // setup changeevent for QTabWidgets
    QList<QTabWidget *> allTabs = myWidget->findChildren<QTabWidget *>();
    foreach(QTabWidget* widget, allTabs) {
        connect(widget, SIGNAL(currentChanged(int)), this, SLOT(Callback_TabChanged(int)));
    }

    // start a timer
    loopTimerID = startTimer(1000);

    // all interfaces flush io
    FlushAllInterfaces();

    // due to crash in connection with the splash screen, changed
    // these instructions to the botton of this class
    if(nbIncludes > 0) {
        Sleep::msleep(200);
        // this seems to causes the crash and is not really needed here?
        //splash->finish(this);

        splash->deleteLater();
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
}

/**
 * find for a widget if it is contained in a parent QTabWidget
 */
QTabWidget* CaQtDM_Lib::getTabParent(QWidget *w1)
{
    QObject *Parent = w1->parent();
    QTabWidget *tabWidget = (QTabWidget*) 0;
    //qDebug() << w1->objectName() << i;

    while(Parent != (QObject*) 0) {
        //qDebug() << "parent=" << Parent;
        Parent = Parent->parent();
        if(Parent != (QObject*) 0) {
            if(QTabWidget* widget = qobject_cast<QTabWidget *>(Parent)) {
                tabWidget = widget;
                return tabWidget;
            }
        } else {
            return (QTabWidget *) 0;
        }
    }
    return (QTabWidget *) 0;
}

/**
 * this routine will go through all are ca objects, find their nearest QTabWidget if any and decides
 * to enable or disable their monitor
 */
void CaQtDM_Lib::EnableDisableIO()
{
#ifdef IO_OPTIMIZED_FOR_TABWIDGETS

    void *ptr;

    // any tabwidgets in this window ? when not do nothing
    //qDebug() << "================================";
    QList<QTabWidget *> all = myWidget->findChildren<QTabWidget *>();
    if(all.count() == 0) return;

    // go through are QTabWidgets
    foreach(QTabWidget* widget, all) {

        // go through their pages
        for(int i=0; i<widget->count(); i++) {

            //qDebug() << widget << widget->currentIndex() << widget->tabText(i);
            QList<QWidget*> children = widget->widget(i)->findChildren<QWidget *>();

            // go through our ca objects on this page (except for caStripplot and cawaterfallplot, needing history data)
            foreach(QWidget* w1, children) {
                QString className = w1->metaObject()->className();
                if(className.contains("ca") &&
                        !className.contains("caStripPlot") &&
                        !className.contains("caWaterfallPlot")) {

                    // nearest parent tab
                    QTabWidget* tabWidget = w1->property("parentTab").value<QTabWidget*>();

                    // no tab
                    if(tabWidget != (QTabWidget*)0) {
                        // the widget to be considered
                        if(tabWidget == widget) {
                            bool hidden = false;
                            //qDebug() << w1->objectName() << "sitting in " << tabWidget << "at position" << i << "actual position is" << tabWidget->currentIndex();

                            if(!tabWidget->isVisible()) {
                                hidden = true;
                            } else if(i == tabWidget->currentIndex()) {
                                //qDebug() << "thus on visible tab";
                                hidden = false;
                            } else {
                                //qDebug() << "thus on hidden tab";
                                hidden = true;
                            }

                            // get the associated monitor pointers and add or remove the event
                            QVariant var=w1->property("InfoList");
                            QVariantList infoList = var.toList();
                            for(int j=0; j<infoList.count(); j++) {
                                ptr = (void*) infoList.at(j).value<void *>();
                                if(ptr != (void*) 0) {
                                    ControlsInterface * plugininterface = (ControlsInterface *) w1->property("Interface").value<void *>();
                                    if(plugininterface != (ControlsInterface *) 0) {
                                        if(!hidden) {
                                            plugininterface->pvAddEvent(ptr);
                                        } else {
                                            plugininterface->pvClearEvent(ptr);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    FlushAllInterfaces();
#endif
}


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

}

/**
 * this routine reaffects a text when macro is used
 */
bool CaQtDM_Lib::reaffectText(QMap<QString, QString> map, QString *text) {
    bool doNothing;
    if(text->size() > 0) {
        if(text->contains("$(") && text->contains(")")) {
            *text =  treatMacro(map, *text, &doNothing);
            return true;
        }
    }
    return false;
}

/**
 * this routine creates a QMap from a macro string
 */
QMap<QString, QString> CaQtDM_Lib::createMap(const QString& macro)
{
    //qDebug() << "treat macro" << macro;
    QMap<QString, QString> map;
    // macro of type A=MMAC3,B=STR,C=RMJ:POSA:2 to be used for replacements of pv in child widgets
    if(macro != NULL) {
        QStringList vars = macro.split(",", QString::SkipEmptyParts);
        for(int i=0; i< vars.count(); i++) {
            /*          this would be ok if medm did not allow also an equal sign after the equal sign
            QStringList keyvalue = vars.at(i).split("=", QString::SkipEmptyParts);
            if(keyvalue.count() == 2) {
                map.insert(keyvalue.at(0).trimmed(), keyvalue.at(1));
            }
*/
            int pos = vars.at(i).indexOf("=");
            if(pos != -1) {
                QString key = vars.at(i).mid(0, pos);
                QString value = vars.at(i).mid(pos+1);
                map.insert(key.trimmed(), value);
            } else {
                qDebug() <<"macro" <<  macro << "could not be parsed";
            }
        }
    }
    //qDebug() << "create map from macro:" << macro;
    //qDebug() << "resulting map=" << map;
    return map;
}

void CaQtDM_Lib::scanWidgets(QList<QWidget*> list, QString macro)
{
    // get first all primary softs (inorder that pv working on their own value will always be treated first
    //qDebug() << " ------------ first pass treat softs being involved in itsself (incrementing)";
    foreach(QWidget *w1, list) {
        HandleWidget(w1, macro, true, true);
    }
    //qDebug() << " ------------ first pass other softs";
    // other softpvs
    foreach(QWidget *w1, list) {
        HandleWidget(w1, macro, true, false);
    }
    // other pvs
    //qDebug() << " ------------ no first pass other stuff";
    foreach(QWidget *w1, list) {
        HandleWidget(w1, macro, false, false);
    }
}

/**
 * this routine handles the initialization of all widgets
 */
void CaQtDM_Lib::HandleWidget(QWidget *w1, QString macro, bool firstPass, bool treatPrimary)
{
    QMap<QString, QString> map;
    knobData kData;
    int specData[5];
    memset(&kData, 0, sizeof (knobData));
    bool doNothing;
    QString pv;
    // an object has normally at least one pv, in case of visibility monitors
    // the exact number of monitors is computed, when zero monitors
    // then no info will be shown
    int nbMonitors = 1;

    QVariant test=w1->property("Taken");
    if(!test.isNull()) {
        if(test.toBool()) return;
    }

    //qDebug() << w1->metaObject()->className() << w1->objectName();
    QString className(w1->metaObject()->className());

    if(className.contains("ca") || className.contains("QTextBrowser")) {
        PRINT(printf("\n%*c %s macro=<%s>", 15 * level, '+', qPrintable(w1->objectName()), qPrintable(macro)));
        map = createMap(macro);
    }

    QColor bg = w1->property("background").value<QColor>();
    QColor fg = w1->property("foreground").value<QColor>();
    QColor lg = w1->property("lineColor").value<QColor>();

    // keep original colors
    w1->setProperty("BColor", bg);
    w1->setProperty("FColor", fg);
    w1->setProperty("LColor", lg);

    // when first pass specified, treat only caCalc
    //==================================================================================================================
    if(firstPass) {
        if(caCalc* calcWidget = qobject_cast<caCalc *>(w1)) {

            bool doit;
            w1->setProperty("ObjectType", caCalc_Widget);

            kData.soft = true;
            addMonitor(myWidget, &kData, calcWidget->getVariable().toLatin1().constData(), w1, specData, map, &pv);

            //qDebug() <<  "firstpass" << firstPass <<  "treatPrimary:" << treatPrimary << calcWidget->getVariable() << calcWidget << SoftPVusesItsself(calcWidget, map);

            // softchannels calculating with themselves are done first
            if(SoftPVusesItsself(calcWidget, map) && treatPrimary) {
                doit=true;
                //qDebug() << "softchannels calculating with themselves have to be done first: doit";

            // softchannels not using themselves are done second
            } else if(!SoftPVusesItsself(calcWidget, map) && !treatPrimary) {
                doit=true;
                //qDebug() << "softchannels not using themselves are done second: doit";

            // softchannels not using themselves, but that just define themselves
            } else {
                //qDebug() << "softchannels that just define themselves: dont";
                return;
            }

            // other channels if any
            kData.soft = false;
            nbMonitors = InitVisibility(w1, &kData, map, specData, calcWidget->getVariable().toLatin1().constData());

            // when no monitors then inititalize value
            if(nbMonitors == 0) {
                //qDebug() << "update " << calcWidget->getVariable().toLatin1().constData()<< "initial value" << calcWidget->getInitialValue();
                calcWidget->setValue(calcWidget->getInitialValue());
                mutexKnobDataP->UpdateSoftPV(calcWidget->getVariable().toLatin1().constData(), calcWidget->getInitialValue(), myWidget);
            }

            w1->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(w1, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenu(const QPoint&)));
            w1->setProperty("Connect", false);

            calcWidget->setProperty("Taken", true);
        }

        return;
    }

    // not a ca widget, but offer the possibility to load files into the text browser by using macros
    //==================================================================================================================
    if(QTextBrowser* browserWidget = qobject_cast<QTextBrowser *>(w1)) {

        //qDebug() << "create QTextBrowser";

        QString source = browserWidget->source().toString();
        if(reaffectText(map, &source))  browserWidget->setSource(source);
        QString fileName = browserWidget->source().path();

        if(!fileName.isEmpty()) {
            qDebug() << "watch file" << source;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
            bool success = watcher->addPath(fileName);
            if(!success) qDebug() << fileName << "can not be watched for changes";
            else qDebug() << fileName << "is watched for changes";
#else
            watcher->addPath(fileName);
#endif
        }
        browserWidget->setProperty("Taken", true);

        // the different widgets to be handled
        //==================================================================================================================
    } else if(caImage* imageWidget = qobject_cast<caImage *>(w1)) {

        //qDebug() << "create caImage";
        w1->setProperty("ObjectType", caImage_Widget);

        // any error messages for this object?
        while(imageWidget->anyMessages()) {
            //qDebug() << imageWidget->getMessages();
            QString message = imageWidget->getMessages();
            if(message.contains("Info:")) postMessage(QtWarningMsg, (char*)message.toLatin1().constData());
            else postMessage(QtCriticalMsg, (char*) message.toLatin1().constData());
        }

        nbMonitors = InitVisibility(w1, &kData, map, specData, "");

        // empty calc string, set animation
        if(imageWidget->getImageCalc().size() == 0) {
            //qDebug() <<  "no calc for image";
            imageWidget->setFrame(0);
            imageWidget->startMovie();
        }

        imageWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caRelatedDisplay* relatedWidget = qobject_cast<caRelatedDisplay *>(w1)) {

        //qDebug() << "create caRelatedDisplay" << relatedWidget << relatedWidget->getLabels() << relatedWidget->getArgs() <<  relatedWidget->getFiles();
        w1->setProperty("ObjectType", caRelatedDisplay_Widget);

        QString text;

        text = relatedWidget->getLabels();
        if(reaffectText(map, &text))  relatedWidget->setLabels(text);

        text = relatedWidget->getArgs();
        if(reaffectText(map, &text))  relatedWidget->setArgs(text);

        text = relatedWidget->getFiles();
        if(reaffectText(map, &text))  relatedWidget->setFiles(text);

        text = relatedWidget->getLabel();
        if(reaffectText(map, &text))  relatedWidget->setLabel(text);

        connect(relatedWidget, SIGNAL(clicked(int)), this, SLOT(Callback_RelatedDisplayClicked(int)));
        connect(relatedWidget, SIGNAL(triggered(int)), this, SLOT(Callback_RelatedDisplayClicked(int)));

        if(relatedWidget->isElevated()) relatedWidget->raise();

        relatedWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caShellCommand* shellWidget = qobject_cast<caShellCommand *>(w1)) {

        //qDebug() << "create caShellCommand";
        w1->setProperty("ObjectType", caShellCommand_Widget);

        QString text;
        text= shellWidget->getLabels();
        if(reaffectText(map, &text))  shellWidget->setLabels(text);

        text = shellWidget->getArgs();
        if(reaffectText(map, &text))  shellWidget->setArgs(text);

        text = shellWidget->getFiles();
        if(reaffectText(map, &text)) shellWidget->setFiles(text);

        text = shellWidget->getLabel();
        if(reaffectText(map, &text))  shellWidget->setLabel(text);

        connect(shellWidget, SIGNAL(clicked(int)), this, SLOT(Callback_ShellCommandClicked(int)));
        connect(shellWidget, SIGNAL(triggered(int)), this, SLOT(Callback_ShellCommandClicked(int)));

        if(shellWidget->isElevated()) shellWidget->raise();

        shellWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caMenu* menuWidget = qobject_cast<caMenu *>(w1)) {

        //qDebug() << "create caMenu";
        w1->setProperty("ObjectType", caMenu_Widget);

        QString text = menuWidget->getPV();
        if(text.size() > 0) {
            text =  treatMacro(map, text, &doNothing);
            addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
            connect(menuWidget, SIGNAL(activated(QString)), this, SLOT(Callback_MenuClicked(QString)));
            menuWidget->setPV(text);
        }

        if(menuWidget->isElevated()) menuWidget->raise();

        menuWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caCamera* cameraWidget = qobject_cast<caCamera *>(w1)) {

        //qDebug() << "create caCamera";
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
            if(i==3) text = cameraWidget->getPV_Code();
            if(i==4) text = cameraWidget->getPV_BPP();
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

                if(thisString.count() == 4 &&
                        thisString.at(0).trimmed().length() > 0 &&
                        thisString.at(1).trimmed().length() > 0  &&
                        thisString.at(2).trimmed().length() > 0 &&
                        thisString.at(3).trimmed().length() > 0) {
                    for(int j=0; j<4; j++) {
                        if(i==7)specData[0] = i+j;   // x,y,w,h
                        text = treatMacro(map, thisString.at(j), &doNothing);
                        if(i==7)addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
                        if(i==7)pvs1.append(pv);
                        if(i==8)pvs2.append(text);
                        if((j<3) && (i==7))pvs1.append(";");
                        if((j<3) && (i==8))pvs2.append(";");
                    }
                }
            }

            if(text.size() > 0 && alpha) {
                specData[0] = i;   // pv type
                text =  treatMacro(map, text, &doNothing);
                if((i!=7) && (i!=8)) addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
                if(i==0) cameraWidget->setPV_Data(pv);
                if(i==1) cameraWidget->setPV_Width(pv);
                if(i==2) cameraWidget->setPV_Height(pv);
                if(i==3) cameraWidget->setPV_Code(pv);
                if(i==4) cameraWidget->setPV_BPP(pv);
                if(i==5) cameraWidget->setMinLevel(pv);
                if(i==6) cameraWidget->setMaxLevel(pv);
                if(i==7) cameraWidget->setROIChannelsRead(pvs1);
                if(i==8) cameraWidget->setROIChannelsWrite(pvs2);
                if(i>0) tooltip.append("<br>");
                if(i<=6) tooltip.append(pv);
                else if(i==7) tooltip.append(pvs1);
                else if(i==8) tooltip.append(pvs2);
            } else if (i==3) {  // code missing (assume 1 for Helge)
                cameraWidget->setCode(1);
            } else if (i==4) {  // bpp missing (assume 3 for Helge)
                cameraWidget->setBPP(3);
            }
        }
        // finish tooltip
        tooltip.append(ToolTipPostfix);
        cameraWidget->setToolTip(tooltip);

        cameraWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caChoice* choiceWidget = qobject_cast<caChoice *>(w1)) {

        //qDebug() << "create caChoice";
        w1->setProperty("ObjectType", caChoice_Widget);

        QString text = choiceWidget->getPV();
        if(text.size() > 0) {
            text =  treatMacro(map, text, &doNothing);
            addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
            connect(choiceWidget, SIGNAL(clicked(QString)), this, SLOT(Callback_ChoiceClicked(QString)));
            choiceWidget->setPV(text);
        }

        if(choiceWidget->isElevated()) choiceWidget->raise();

        choiceWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caLabel* labelWidget = qobject_cast<caLabel *>(w1)) {

        //qDebug() << "create caLabel";
        w1->setProperty("ObjectType", caLabel_Widget);

        nbMonitors = InitVisibility(w1, &kData, map, specData, "");

        QString text =  treatMacro(map, labelWidget->text(), &doNothing);
        text.replace(QString::fromWCharArray(L"\u00A6"), " ");    // replace Â¦ with a blanc (was used in macros for creating blancs)
        labelWidget->setText(text);

        labelWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caLabelVertical* labelverticalWidget = qobject_cast<caLabelVertical *>(w1)) {

        //qDebug() << "create caLabelVertical";
        w1->setProperty("ObjectType", caLabelVertical_Widget);

        nbMonitors = InitVisibility(w1, &kData, map, specData, "");

        QString text =  treatMacro(map, labelverticalWidget->text(), &doNothing);
        text.replace(QString::fromWCharArray(L"\u00A6"), " ");    // replace Â¦ with a blanc (was used in macros for creating blancs)
        labelverticalWidget->setText(text);

        labelverticalWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caTextEntry* textentryWidget = qobject_cast<caTextEntry *>(w1)) {

        //qDebug() << "create caTextEntry";
        w1->setProperty("ObjectType", caTextEntry_Widget);

        if(textentryWidget->getPV().size() > 0) {
            textentryWidget->setEnabled(true);
            addMonitor(myWidget, &kData, textentryWidget->getPV(), w1, specData, map, &pv);
            textentryWidget->setPV(pv);
            connect(textentryWidget, SIGNAL(TextEntryChanged(const QString&)), this,
                    SLOT(Callback_TextEntryChanged(const QString&)));
        }
        // default format, format from ui file will be used normally except for channel precision
        textentryWidget->setFormat(1);
        textentryWidget->clearFocus();

        if(textentryWidget->isElevated()) textentryWidget->raise();

        textentryWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caLineEdit* lineeditWidget = qobject_cast<caLineEdit *>(w1)) {

        //qDebug() << "create caLineEdit";
        w1->setProperty("ObjectType", caLineEdit_Widget);

        if(lineeditWidget->getPV().size() > 0) {
            lineeditWidget->setCursor(QCursor());
            lineeditWidget->setReadOnly(true);

            lineeditWidget->setAlignment(lineeditWidget->alignment());
            addMonitor(myWidget, &kData, lineeditWidget->getPV(), w1, specData, map, &pv);
            lineeditWidget->setPV(pv);
        }

        // default format, format from ui file will be used normally except for channel precision
        lineeditWidget->setFormat(1);

        lineeditWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caGraphics* graphicsWidget = qobject_cast<caGraphics *>(w1)) {

        //qDebug() << "create caGraphics";
        w1->setProperty("ObjectType", caGraphics_Widget);

        nbMonitors = InitVisibility(w1, &kData, map, specData, "");
        graphicsWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caPolyLine* polylineWidget = qobject_cast<caPolyLine *>(w1)) {

        //qDebug() << "create caPolyLine";
        w1->setProperty("ObjectType", caPolyLine_Widget);

        nbMonitors = InitVisibility(w1, &kData, map, specData, "");
        polylineWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if (caApplyNumeric* applynumericWidget = qobject_cast<caApplyNumeric *>(w1)){

        //qDebug() << "create caAppyNumeric";
        w1->setProperty("ObjectType", caApplyNumeric_Widget);

        if(applynumericWidget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, applynumericWidget->getPV(), w1, specData, map, &pv);
            applynumericWidget->setPV(pv);
            connect(applynumericWidget, SIGNAL(clicked(double)), this, SLOT(Callback_EApplyNumeric(double)));
        }
        if(applynumericWidget->isElevated()) applynumericWidget->raise();

        applynumericWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if (caNumeric* numericWidget = qobject_cast<caNumeric *>(w1)){

        //qDebug() << "create caNumeric";
        w1->setProperty("ObjectType", caNumeric_Widget);

        if(numericWidget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, numericWidget->getPV(), w1, specData, map, &pv);
            numericWidget->setPV(pv);
            connect(numericWidget, SIGNAL(valueChanged(double)), this, SLOT(Callback_ENumeric(double)));
        }
        if(numericWidget->isElevated()) numericWidget->raise();

        numericWidget->setProperty("Taken", true);


        //==================================================================================================================
    } else if (caSpinbox* spinboxWidget = qobject_cast<caSpinbox *>(w1)){

        //qDebug() << "create caSpinbox";
        w1->setProperty("ObjectType", caSpinbox_Widget);

        if(spinboxWidget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, spinboxWidget->getPV(), w1, specData, map, &pv);
            spinboxWidget->setPV(pv);
            connect(spinboxWidget, SIGNAL(valueChanged(double)), this, SLOT(Callback_Spinbox(double)));
        }
        if(spinboxWidget->isElevated()) spinboxWidget->raise();

        spinboxWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if (caMessageButton* messagebuttonWidget = qobject_cast<caMessageButton *>(w1)) {

        //qDebug() << "create caMessageButton" <<  messagebuttonWidget->getPV();
        w1->setProperty("ObjectType", caMessageButton_Widget);

        QString text;

        if(messagebuttonWidget->getPV().size() > 0) {
            specData[0] = 0;
            addMonitor(myWidget, &kData, messagebuttonWidget->getPV(), w1, specData, map, &pv);
            messagebuttonWidget->setPV(pv);
        }

        if(messagebuttonWidget->getDisablePV().size() > 0) {
            specData[0] = 1;
            addMonitor(myWidget, &kData, messagebuttonWidget->getDisablePV(), w1, specData, map, &pv);
            messagebuttonWidget->setDisablePV(pv);
        }

        connect(messagebuttonWidget, SIGNAL(messageButtonSignal(int)), this, SLOT(Callback_MessageButton(int)));

        text = messagebuttonWidget->getLabel();
        if(reaffectText(map, &text))  messagebuttonWidget->setLabel(text);

        if(messagebuttonWidget->isElevated()) messagebuttonWidget->raise();

        messagebuttonWidget->setProperty("Taken", true);


        //==================================================================================================================
    } else if(caToggleButton* togglebuttonWidget = qobject_cast<caToggleButton *>(w1)) {

        //qDebug() << "create caToggleButton";
        w1->setProperty("ObjectType", caToggleButton_Widget);

        connect(togglebuttonWidget, SIGNAL(toggleButtonSignal(bool)), this, SLOT(Callback_ToggleButton(bool)));

        addMonitor(myWidget, &kData, togglebuttonWidget->getPV(), w1, specData, map, &pv);
        togglebuttonWidget->setPV(pv);

        QString text =  treatMacro(map, togglebuttonWidget->text(), &doNothing);
        text.replace(QString::fromWCharArray(L"\u00A6"), " ");    // replace Â¦ with a blanc (was used in macros for creating blancs)
        togglebuttonWidget->setText(text);

        if(togglebuttonWidget->isElevated()) togglebuttonWidget->raise();

        togglebuttonWidget->setProperty("Taken", true);


        //==================================================================================================================
    } else if(caScriptButton* scriptbuttonWidget = qobject_cast<caScriptButton *>(w1)) {

        QString text;
        //qDebug() << "create caScriptButton";
        w1->setProperty("ObjectType", caScriptButton_Widget);

        connect(scriptbuttonWidget, SIGNAL(scriptButtonSignal()), this, SLOT(Callback_ScriptButton()));

        text= scriptbuttonWidget->getScriptParam();
        if(reaffectText(map, &text))  scriptbuttonWidget->setScriptParam(text);
        scriptbuttonWidget->setToolTip("process never started !");

        if(scriptbuttonWidget->isElevated()) scriptbuttonWidget->raise();

        scriptbuttonWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caLed* ledWidget = qobject_cast<caLed *>(w1)) {

        //qDebug() << "create caLed";
        w1->setProperty("ObjectType", caLed_Widget);

        if(ledWidget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, ledWidget->getPV(), w1, specData, map, &pv);
            ledWidget->setPV(pv);
        }

        ledWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caBitnames* bitnamesWidget = qobject_cast<caBitnames *>(w1)) {

        //qDebug() << "create caBitnames";
        w1->setProperty("ObjectType", caBitnames_Widget);

        if(bitnamesWidget->getEnumPV().size() > 0 && bitnamesWidget->getValuePV().size() > 0) {
            addMonitor(myWidget, &kData, bitnamesWidget->getEnumPV(), w1, specData, map, &pv);
            bitnamesWidget->setEnumPV(pv);
            addMonitor(myWidget, &kData, bitnamesWidget->getValuePV(), w1, specData, map, &pv);
            bitnamesWidget->setValuePV(pv);
        }

        bitnamesWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caSlider* sliderWidget = qobject_cast<caSlider *>(w1)) {

        //qDebug() << "create caSlider";
        w1->setProperty("ObjectType", caSlider_Widget);

        if(sliderWidget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, sliderWidget->getPV(), w1, specData, map, &pv);
            sliderWidget->setPV(pv);
            connect(sliderWidget, SIGNAL(valueChanged(double)), this, SLOT(Callback_SliderValueChanged(double)));
        }

        if(sliderWidget->isElevated())sliderWidget->raise();

        sliderWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caClock* clockWidget = qobject_cast<caClock *>(w1)) {

        //qDebug() << "create caClock";
        w1->setProperty("ObjectType", caClock_Widget);

        if(clockWidget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, clockWidget->getPV(), w1, specData, map, &pv);
            clockWidget->setPV(pv);
        }

        clockWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caThermo* thermoWidget = qobject_cast<caThermo *>(w1)) {

        //qDebug() << "create caThermo";
        w1->setProperty("ObjectType", caThermo_Widget);

        if(thermoWidget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, thermoWidget->getPV(), w1, specData, map, &pv);
            thermoWidget->setPV(pv);
        }
        // for an opposite direction, invert maximum and minimum

        if(thermoWidget->getDirection() == caThermo::Down || thermoWidget->getDirection() == caThermo::Left) {
            double max = thermoWidget->maxValue();
            double min = thermoWidget->minValue();
            thermoWidget->setMinValue(max);
            thermoWidget->setMaxValue(min);
        }

        thermoWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caLinearGauge* lineargaugeWidget = qobject_cast<caLinearGauge *>(w1)) {

        //qDebug() << "create lineargauge for" << lineargaugeWidget->getPV();
        w1->setProperty("ObjectType", caLinearGauge_Widget);

        if(lineargaugeWidget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, lineargaugeWidget->getPV(), w1, specData, map, &pv);
            lineargaugeWidget->setPV(pv);
        }

        lineargaugeWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caCircularGauge* circulargaugeWidget = qobject_cast<caCircularGauge *>(w1)) {

        //qDebug() << "create circulargauge for" << circulargaugeWidget->getPV();
        w1->setProperty("ObjectType", caCircularGauge_Widget);

        if(circulargaugeWidget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, circulargaugeWidget->getPV(), w1, specData, map, &pv);
            circulargaugeWidget->setPV(pv);
        }

        circulargaugeWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caMeter* meterWidget = qobject_cast<caMeter *>(w1)) {

        //qDebug() << "create circulargauge for" << meterWidget->getPV();
        w1->setProperty("ObjectType", caMeter_Widget);

        if(meterWidget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, meterWidget->getPV(), w1, specData, map, &pv);
            meterWidget->setPV(pv);
        }

        meterWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caByte* byteWidget = qobject_cast<caByte *>(w1)) {

        //qDebug() << "create caByte" << w1;
        w1->setProperty("ObjectType", caByte_Widget);

        if(byteWidget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, byteWidget->getPV(), w1, specData, map, &pv);
            byteWidget->setPV(pv);
        }

        byteWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caByteController* bytecontrollerWidget = qobject_cast<caByteController *>(w1)) {

        //qDebug() << "create caByteController" << w1;
        w1->setProperty("ObjectType", caByteController_Widget);

        if(bytecontrollerWidget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, bytecontrollerWidget->getPV(), w1, specData, map, &pv);
            bytecontrollerWidget->setPV(pv);
            connect(bytecontrollerWidget, SIGNAL(clicked(int)), this, SLOT(Callback_ByteControllerClicked(int)));
        }

        bytecontrollerWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caInclude* includeWidget = qobject_cast<caInclude *>(w1)) {

        //qDebug() << "create caInclude" << w1;
        w1->setProperty("ObjectType", caInclude_Widget);

        QWidget *thisW;
        QUiLoader loader;
        bool prcFile = false;

        // define a layout
        QGridLayout *layout = new QGridLayout;
        layout->setContentsMargins(0,0,0,0);

        //qDebug() << "treat caInclude" << w1 << "level=" << level;

        nbMonitors = InitVisibility(w1, &kData, map, specData, "");

        includeWidget->setProperty("Taken", true);

        QString macroS = includeWidget->getMacro();
        if(macroS.size() < 1) {
            if(level>0){
                PRINT(printf("\n    %*c get last macro=%s", 15 * level, ' ', qPrintable(savedMacro[level-1])));
                macroS = savedMacro[level-1];
            } else {
                macroS = savedMacro[level];
            }
        } else {
            //printf("\n    %*c macro=%s", 15 * level, ' ',  qPrintable(macroS));
            savedMacro[level] = macroS;
        }

        macroS = treatMacro(map, macroS, &doNothing);
        savedMacro[level] = treatMacro(map, savedMacro[level], &doNothing);

        // define the file to use
        QString fileName = includeWidget->getFileName().trimmed();
        reaffectText(map, &fileName);

        QString openFile = "";
        int found = fileName.lastIndexOf(".");
        if (found != -1) {
            openFile = fileName.mid(0, found);
        }

        // ui file or prc file ?
        if((openFile.count() > 1) && fileName.contains(".prc")) {
            //qDebug() << "prc file";
            prcFile = true;
        } else {
            //qDebug() << "ui file";
            fileName = openFile.append(".ui");
            prcFile = false;
        }

        // this will check for file existence and when an url is defined, download the file from a http server
        fileFunctions filefunction;
        filefunction.checkFileAndDownload(fileName);
        if(messageWindowP != (MessageWindow *) 0) {
            if(filefunction.lastInfo().length() > 0) messageWindowP->postMsgEvent(QtWarningMsg, (char*) filefunction.lastInfo().toLatin1().constData());
            if(filefunction.lastError().length() > 0)  messageWindowP->postMsgEvent(QtCriticalMsg, (char*)filefunction.lastError().toLatin1().constData());
        }

        searchFile *s = new searchFile(fileName);
        QString fileNameFound = s->findFile();
        if(fileNameFound.isNull()) {
            qDebug() << "file" << fileName << "does not exist";
            if(!includeFiles.contains(fileName)) {
                includeFiles.append(fileName);
                includeFiles.append(" does not exist <br>");
            }
        } else {
            //qDebug() << "load file" << fileName << "with macro" << macroS;
            fileName = fileNameFound;
        }
        delete s;

        // sure file exists ?
        QFileInfo fi(fileName);
        if(fi.exists()) {

            // load prc or ui file
            if(prcFile) {
                // load new file
                ParsePepFile *parsefile = new ParsePepFile(fileName, pepPrint);
                thisW = parsefile->load(this);
                delete parsefile;
            } else {
                QFile *file = new QFile;
                // open and load ui file
                file->setFileName(fileName);
                file->open(QFile::ReadOnly);
                //ftime(&last);
                thisW = loader.load(file, this);
                file->close();
                delete file;
            }

            // some error with loading
            if (!thisW) {
                postMessage(QtDebugMsg, (char*) tr("could not load include file %1").arg(fileName).toLatin1().constData());
                if(!includeFiles.contains(fi.absoluteFilePath())) {
                    includeFiles.append(fi.absoluteFilePath());
                    includeFiles.append(" could not be loaded <br>");
                }
                // seems to be ok
            } else {
                if(!includeFiles.contains(fi.absoluteFilePath())) {
                    includeFiles.append(fi.absoluteFilePath());
                    includeFiles.append(" is loaded <br>");
                }
                includeWidgetList.append(thisW);

                // add includeWidget to the gui
                layout->addWidget(thisW);
                includeWidget->setLayout(layout);
                includeWidget->setLineSize(0);

                // go through its childs
                //QList<QWidget *> childs = thisW->findChildren<QWidget *>();
                level++;

                // keep actual filename
                savedFile[level] = fi.baseName();

                scanWidgets(thisW->findChildren<QWidget *>(), macroS);

                level--;
            }

        } else {
            postMessage(QtDebugMsg, (char*) tr("sorry, could not load include file %1").arg(fileName).toLatin1().constData());
            qDebug() << "sorry, file" << fileName << " does not exist";
        }

        macroS = savedMacro[level];

        // increment splascounter when include is in list
        if(nbIncludes > 0) {
            for (int i = topIncludesWidgetList.count()-1; i >= 0; --i) {
                if(w1 ==  topIncludesWidgetList.at(i)) {
                    splash->setProgress(splashCounter++);
                    break;
                }
            }
        }

        includeWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caFrame* frameWidget = qobject_cast<caFrame *>(w1)) {

        //qDebug() << "treat caFrame" << w1;
        w1->setProperty("ObjectType", caFrame_Widget);

        nbMonitors = InitVisibility(w1, &kData, map, specData, "");

        frameWidget->setProperty("Taken", true);

        QString macroS = frameWidget->getMacro();

        if(macroS.size() < 1) {
            if(level > 0){
                //printf("\n    %*c get macro from previous level=%s w=<%s> ", 15 * level, ' ', qPrintable(savedMacro[level-1]), qPrintable(w1->objectName()));
                macroS = savedMacro[level-1];
                savedMacro[level] = macroS;
            } else {
                macroS = savedMacro[level];
            }
        } else {
            //printf("\n    %*c macro=%s <%s>", 15 * level, ' ', qPrintable(macroS), qPrintable(w1->objectName()));
            savedMacro[level] = macroS;
        }

        macroS = treatMacro(map, macroS, &doNothing);
        savedMacro[level] = treatMacro(map, savedMacro[level], &doNothing);

        //QList<QWidget *> childs = frameWidget->findChildren<QWidget *>();
        level++;

        // get actual filename from previous level
        savedFile[level] = savedFile[level-1];

        scanWidgets(frameWidget->findChildren<QWidget *>(), macroS);

        //foreach(QWidget *child, childs) {
        //    HandleWidget(child, macroS, true, false);
        //    HandleWidget(child, macroS, false, false);
        //}
        level--;

        macroS= savedMacro[level];

        frameWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caCartesianPlot* cartesianplotWidget = qobject_cast<caCartesianPlot *>(w1)) {

        //qDebug() << "treat caCartesianPlot" << w1;
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

        // go through all possible curves and add monitor
        for(int i=0; i< caCartesianPlot::curveCount; i++) {
            QString pvs ="";
            QStringList thisString = cartesianplotWidget->getPV(i).split(";");

            cartesianplotWidget->setColor(cartesianplotWidget->getColor(i), i);
            cartesianplotWidget->setStyle(cartesianplotWidget->getStyle(i), i);
            cartesianplotWidget->setSymbol(cartesianplotWidget->getSymbol(i), i);

            if(thisString.count() == 2 && thisString.at(0).trimmed().length() > 0 && thisString.at(1).trimmed().length() > 0) {
                specData[0] = i; // curve number
                specData[1] = caCartesianPlot::XY_both;
                specData[2] = caCartesianPlot::CH_X; // X
                addMonitor(myWidget, &kData, thisString.at(0), w1, specData, map, &pv);
                tooltip.append(pv);
                pvs = pv;
                specData[2] = caCartesianPlot::CH_Y; // Y
                addMonitor(myWidget, &kData, thisString.at(1), w1, specData, map, &pv);
                tooltip.append(",");
                tooltip.append(pv);
                pvs.append(";");
                pvs.append(pv);
                tooltip.append("<br>");
            } else if(thisString.count() == 2 && thisString.at(0).trimmed().length() > 0 && thisString.at(1).trimmed().length() == 0) {
                specData[0] = i; // curve number
                specData[1] = caCartesianPlot::X_only;
                specData[2] = caCartesianPlot::CH_X; // X
                addMonitor(myWidget, &kData, thisString.at(0), w1, specData, map, &pv);
                pvs.append(pv);
                pvs.append(";");
                tooltip.append(pv);
                tooltip.append("<br>");
            } else if(thisString.count() == 2 && thisString.at(1).trimmed().length() > 0 && thisString.at(0).trimmed().length() == 0) {
                specData[0] = i; // curve number
                specData[1] = caCartesianPlot::Y_only;
                specData[2] = caCartesianPlot::CH_Y; // Y
                addMonitor(myWidget, &kData, thisString.at(1), w1, specData, map, &pv);
                pvs.append(";");
                pvs.append(pv);
                tooltip.append(pv);
                tooltip.append("<br>");
            }
            cartesianplotWidget->setPV(pvs, i);
        }

        // handle trigger channel if any
        triggerChannel = cartesianplotWidget->getTriggerPV();
        if(triggerChannel.trimmed().length() > 0) {
            specData[2] = caCartesianPlot::CH_Trigger; // Trigger
            addMonitor(myWidget, &kData, triggerChannel, w1, specData, map, &pv);
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
                addMonitor(myWidget, &kData, countChannel, w1, specData, map, &pv);
                tooltip.append(pv);
                tooltip.append("<br>");
                cartesianplotWidget->setCountPV(pv);
            }
        } else {
            //qDebug() << "count=" << Number;
        }

        // handle erase channel if any
        eraseChannel = cartesianplotWidget->getErasePV();
        if(eraseChannel.trimmed().length() > 0) {
            specData[2] = caCartesianPlot::CH_Erase; // Count
            addMonitor(myWidget, &kData, eraseChannel, w1, specData, map, &pv);
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
                    addMonitor(myWidget, &kData, thisStrings.at(0), w1, specData, map, &pv);
                    tooltip.append(pv);
                    pvs = pv;
                    specData[0] = 1;
                    specData[2] = caCartesianPlot::CH_Xscale;
                    addMonitor(myWidget, &kData, thisStrings.at(1), w1, specData, map, &pv);
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
                    addMonitor(myWidget, &kData, thisStrings.at(0), w1, specData, map, &pv);
                    tooltip.append(pv);
                    pvs = pv;
                    specData[0] = 1;
                    specData[2] = caCartesianPlot::CH_Yscale;
                    addMonitor(myWidget, &kData, thisStrings.at(1), w1, specData, map, &pv);
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
        cartesianplotWidget->setToolTip(tooltip);

        // reaffect titles
        title = cartesianplotWidget->getTitlePlot();
        if(reaffectText(map, &title)) cartesianplotWidget->setTitlePlot(title);
        title = cartesianplotWidget->getTitleX();
        if(reaffectText(map, &title)) cartesianplotWidget->setTitleX(title);
        title = cartesianplotWidget->getTitleY();
        if(reaffectText(map, &title)) cartesianplotWidget->setTitleY(title);

        cartesianplotWidget->setWhiteColors();

        cartesianplotWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caWaterfallPlot* waterfallplotWidget = qobject_cast<caWaterfallPlot *>(w1)) {

        //qDebug() << "create caWaterfallPlot";
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
                addMonitor(myWidget, &kData, countChannel, w1, specData, map, &pv);
                tooltip.append(pv);
                tooltip.append("<br>");
                waterfallplotWidget->setCountPV(pv);
            }
        } else {
            //qDebug() << "count=" << Number;
        }

        waveChannel = waterfallplotWidget->getPV();
        specData[0] = 0; // waveform
        addMonitor(myWidget, &kData, waveChannel, w1, specData, map, &pv);
        waterfallplotWidget->setPV(pv);
        tooltip.append(pv);
        tooltip.append("<br>");

        // finish tooltip
        tooltip.append(ToolTipPostfix);
        waterfallplotWidget->setToolTip(tooltip);

        waterfallplotWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caStripPlot* stripplotWidget = qobject_cast<caStripPlot *>(w1)) {

        //qDebug() << "create caStripPlot";
        w1->setProperty("ObjectType", caStripPlot_Widget);

        QString text, title;
        QList<QVariant> integerList;
        int nbMon = 0;

        // addmonitor normally will add a tooltip to show the pv; however here we have more than one pv
        QString tooltip;
        tooltip.append(ToolTipPrefix);

        text = stripplotWidget->getPVS();
        reaffectText(map, &text);
        stripplotWidget->setPVS(text);
        QStringList vars = text.split(";", QString::SkipEmptyParts);

        int NumberOfCurves = min(vars.count(), caStripPlot::MAXCURVES);

        // go through the defined curves and add monitor

        if(NumberOfCurves > 0) stripplotWidget->defineCurves(vars, stripplotWidget->getUnits(), stripplotWidget->getPeriod(),  stripplotWidget->width(),  NumberOfCurves);
        for(int i=0; i< NumberOfCurves; i++) {
            int num;
            pv = vars.at(i).trimmed();
            if(pv.size() > 0) {
                if(i==0) {  // user defaults, will be redefined when limits from channel
                    stripplotWidget->setYscale(stripplotWidget->getYaxisLimitsMin(i), stripplotWidget->getYaxisLimitsMax(i));
                }
                specData[1] = i;            // curve number
                specData[0] = vars.count(); // number of curves
                num = addMonitor(myWidget, &kData, pv, w1, specData, map, &pv);
                nbMon++;
                stripplotWidget->showCurve(i, true);

                tooltip.append(pv);
                tooltip.append("<br>");
                integerList.append(num);
            }
        }

        // finish tooltip
        tooltip.append(ToolTipPostfix);
        stripplotWidget->setToolTip(tooltip);

        title = stripplotWidget->getTitlePlot();
        if(reaffectText(map, &title)) stripplotWidget->setTitlePlot(title);
        title = stripplotWidget->getTitleX();
        if(reaffectText(map, &title)) stripplotWidget->setTitleX(title);
        title = stripplotWidget->getTitleY();
        if(reaffectText(map, &title)) stripplotWidget->setTitleY(title);

        integerList.insert(0, nbMon); /* set property into stripplotWidget */
        stripplotWidget->setProperty("MonitorList", integerList);
        stripplotWidget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caTable* tableWidget = qobject_cast<caTable *>(w1)) {

        //qDebug() << "create caTable" << tableWidget->getPVS();
        w1->setProperty("ObjectType", caTable_Widget);

        QStringList vars = tableWidget->getPVS().split(";", QString::SkipEmptyParts);
        tableWidget->setColumnCount(3);
        tableWidget->setRowCount(vars.count());

        // go through the pvs and add monitor
        for(int i=0; i< vars.count(); i++) {
            pv = vars.at(i);
            if(pv.size() > 0) {
                QTableWidgetItem *item;
                specData[0] = i;            // table row
                addMonitor(myWidget, &kData, pv, w1, specData, map, &pv);
                item = new QTableWidgetItem(pv);
                item->setTextAlignment(Qt::AlignAbsolute | Qt:: AlignLeft);
                tableWidget->setItem(i,0, item);
            }
        }
        tableWidget->setColumnSizes(tableWidget->getColumnSizes());
        tableWidget->setProperty("Taken", true);
        tableWidget->setToolTip("select row or columns, then with Ctrl+C you can copy to the clipboard\ninside X11 you can then do shft+ins\nwhen doubleclicking on a value, you may execute a shell script for that device");

        connect(tableWidget, SIGNAL(TableDoubleClickedSignal(QString)), this, SLOT(Callback_TableDoubleClicked(QString)));

        //==================================================================================================================
    } else if(caWaveTable* wavetableWidget = qobject_cast<caWaveTable *>(w1)) {

        //qDebug() << "create caWaveTable" << wavetableWidget->getPVS();
        w1->setProperty("ObjectType", caWaveTable_Widget);

        if(wavetableWidget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, wavetableWidget->getPV(), w1, specData, map, &pv);
            wavetableWidget->setPV(pv);
            connect(wavetableWidget, SIGNAL(WaveEntryChanged(QString, int)), this,
                    SLOT(Callback_WaveEntryChanged(QString, int)));
        }
        wavetableWidget->setProperty("Taken", true);
        wavetableWidget->setToolTip("select row or columns, then with Ctrl+C you can copy to the clipboard\ninside X11 you can then do shft+ins\nwhen doubleclicking on a value, you can change the value");

        //==================================================================================================================

    } else if(caScan2D* scan2dWidget = qobject_cast<caScan2D *>(w1)) {

        //qDebug() << "create caScan2D";

        QString text;

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
                        text = treatMacro(map, thisString.at(j), &doNothing);
                        if(i==15)addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
                        if(i==15)pvs1.append(pv);
                        if(i==16)pvs2.append(text);
                        if((j<3) && (i==15))pvs1.append(";");
                        if((j<3) && (i==16))pvs2.append(";");
                    }
                }
            }

            if(text.size() > 0 && alpha) {
                text =  treatMacro(map, text, &doNothing);
                if((i!=15) && (i!=16)) addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
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

        // Try adding the monitor for the array data last, to ensure that setPV_YCPT will be called before setPV_Data,
        // to see if this reduces the likelihood of skipped lines in fast scans.
        //text = scan2dWidget->getPV_Data(); if (text.size() > 0) specData[0] = 0;
        //addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
        //scan2dWidget->setPV_Data(pv);

        // finish tooltip
        tooltip.append(ToolTipPostfix);
        scan2dWidget->setToolTip(tooltip);

        scan2dWidget->setProperty("Taken", true);
    }

    //==================================================================================================================

    // search for a QTabWidget as nearest parent and set it as property
    if(className.contains("ca")) {
        QTabWidget *tabWidget = getTabParent(w1);
        w1->setProperty("parentTab",QVariant::fromValue(tabWidget) );
    }

    // make a context menu for object having a monitor
    if(className.contains("ca") && !className.contains("caRel") && !className.contains("caTable") && !className.contains("caShellCommand") && nbMonitors > 0) {

        w1->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(w1, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenu(const QPoint&)));
        w1->setProperty("Connect", false);
        // in order to get the context on tablets
#ifdef MOBILE
        w1->grabGesture(Qt::TapAndHoldGesture);
        w1->installEventFilter(this);
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
    // qDebug() << "update " << file;
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

/**
  * this routine uses macro table to replace inside the pv the macro part
  */
QString CaQtDM_Lib::treatMacro(QMap<QString, QString> map, const QString& text, bool *doNothing)
{
    QString newText = text;
    *doNothing = false;

    // a macro exists and when pv contains a right syntax then replace pv
    if(!map.isEmpty()) {
        if(text.contains("$(") && text.contains(")")) {
            QMapIterator<QString, QString> i(map);
            while (i.hasNext()) {
                i.next();
                QString toReplace = "$(" + i.key() + ")";
                //qDebug() << "replace in" << newText << toReplace << "with" << i.value();
                newText.replace(toReplace, i.value());
            }
        }
    } else {
        if(text.contains("$")) *doNothing = true;
    }
    return newText;
}

ControlsInterface * CaQtDM_Lib::getControlInterface(QString plugininterface)
{
    // find the plugin we are going to use
    if(!controlsInterfaces.isEmpty()) {
        QMapIterator<QString, ControlsInterface *> i(controlsInterfaces);
        while (i.hasNext()) {
            i.next();
            if(i.key().contains(plugininterface)) {
                //qDebug() << "interface returned for requested" << plugininterface ;
                return  i.value();
            }
        }
    }
    return  (ControlsInterface *) 0;
}

void CaQtDM_Lib::FlushAllInterfaces()
{
    // flush all plugins
    if(!controlsInterfaces.isEmpty()) {
        QMapIterator<QString, ControlsInterface *> i(controlsInterfaces);
        while (i.hasNext()) {
            i.next();
            ControlsInterface *plugininterface = i.value();
            if(plugininterface != (ControlsInterface *) 0) plugininterface->FlushIO();
        }
    }
}

/**
 * routine to create an epics monitor
 */
int CaQtDM_Lib::addMonitor(QWidget *thisW, knobData *kData, QString pv, QWidget *w, int *specData, QMap<QString, QString> map, QString *pvRep)
{
    QMutex *mutex;
    struct timeb now;
    bool doNothing = false;
    int cpylen;
    int indx;
    QString pluginName;
    ControlsInterface *plugininterface = (ControlsInterface *) 0;


    ftime(&now);
    w->setProperty("Connect", false);
    int rate = 5;  // default will be 5Hz

    if(pv.size() == 0) return -1;

    //qDebug() << "add monitor for " << pv << "soft=" << kData->soft;

    QString trimmedPV = pv.trimmed();

    // is there a json string ?
    int pos = trimmedPV.indexOf("{");
    if(pos != -1) {
        int status;
        char asc[255];
        QString JSONString = trimmedPV.mid(pos);
        trimmedPV = trimmedPV.mid(0, pos);
        status = parseForDisplayRate(JSONString, rate);
        if(!status) {
            sprintf(asc, "JSON parsing error on %s ,should be like {\"monitor\":{\"maxdisplayrate\":10}}",
                    (char*) pv.trimmed().toLatin1().constData());
        } else {
            sprintf(asc, "pv %s display rate set to maximum %dHz", trimmedPV.toLatin1().constData(), rate);
        }
        postMessage(QtDebugMsg, asc);
    }

    // replace macro with ist value
    QString newPV = treatMacro(map, trimmedPV, &doNothing);
    *pvRep = newPV;

    // find out what kind of interface has to be used for this pv, default is epics3
    pos = newPV.indexOf("://");
    if(pos != -1) {
        pluginName = newPV.mid(0, pos);
        trimmedPV = newPV.mid(pos+3);
    } else {
        pluginName = "epics3";
        trimmedPV = newPV;
        if(kData->soft) pluginName = "intern";
        if(mutexKnobDataP->getSoftPV(trimmedPV, &indx, thisW)) pluginName = "intern";
    }

    *pvRep = trimmedPV;
    strcpy(kData->pluginName, (char*) pluginName.toLatin1().constData());

    cpylen = qMin(trimmedPV.length(), MAXPVLEN-1);
    strncpy(kData->pv, (char*) trimmedPV.toLatin1().constData(), (size_t) cpylen);
    kData->pv[cpylen] = '\0';

    // find the plugin we are going to use
    if(!pluginName.contains("intern")) {
        plugininterface = getControlInterface(pluginName);
        // and set it to the widget and the pointer to the data
        kData->pluginInterface = (void *) plugininterface;
        QVariant plugin = qVariantFromValue(kData->pluginInterface);
        w->setProperty("Interface", plugin);
        if(kData->pluginInterface == (void *) 0) {
            char asc[255];
            sprintf(asc, "could not find a control plugin for %s with name %s\n",
                    (char*) trimmedPV.toLatin1().constData(),  (char*) pluginName.trimmed().toLatin1().constData());
            postMessage(QtCriticalMsg, asc);
        } else {
            //qDebug() << "control interface for" << trimmedPV << "plugin:" << pluginName;
        }
    }

    // for soft channels the rate has perhaps already been defined before
    QVariant oldRate = w->property("RATE");
    if (oldRate.isValid()) {
        rate = w->property("RATE").value<int>();
    }

    if(doNothing) {
        char asc[255];
        sprintf(asc, "Info: malformed pv '%s' (due to macro?)", (char*) trimmedPV.toLatin1().constData());
        postMessage(QtWarningMsg, asc);
    }

    // when we defined already the same cacalc software channel, then do nothing
    if(mutexKnobDataP->getSoftPV(kData->pv, &indx, thisW)) {
        //qDebug() << "software channel already defined" << w;
        knobData *kPtr = mutexKnobDataP->GetMutexKnobDataPtr(indx);  // use pointer
        if(kPtr != (knobData*) 0) {
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
    tooltip.append(ToolTipPostfix);
    w->setToolTip(tooltip);

    memcpy(kData->specData, specData, sizeof(int) * NBSPECS);
    kData->thisW = (void*) thisW;
    kData->dispW = (void*) w;

    mutex = new QMutex;
    kData->mutex = (void*)  mutex;

    // keep actual object name
    cpylen = qMin(w->objectName().length(), MAXDISPLEN-1);
    strncpy(kData->dispName, w->objectName().toLower().toLatin1().constData(), (size_t) cpylen);
    kData->dispName[cpylen] = '\0';

    QString classname = w->metaObject()->className();
    cpylen = qMin(classname.length(), MAXDISPLEN-1);
    strncpy(kData->clasName, classname.toLower().toLatin1().constData(), (size_t) cpylen);
    kData->clasName[cpylen] = '\0';

    // keep actual filename
    cpylen = qMin( savedFile[level].length(), MAXFILELEN-1);
    strncpy(kData->fileName, (char*) savedFile[level].toLatin1().constData(), (size_t) cpylen);
    kData->fileName[cpylen] = '\0';

    if (QLineEdit *lineedit = qobject_cast<QLineEdit *>(w)) {
        lineedit->setText("");
    }

    // get an index in the data list
    int num = mutexKnobDataP->GetMutexKnobDataIndex();
    if(num == -1) {
        qDebug() << "this should never happen";
        return num;
    }

    // insert into the softpv list when we create a soft channel
    if(kData->soft) {
        strcpy(kData->pluginName, "intern");
        mutexKnobDataP->InsertSoftPV(kData->pv, num, thisW);
    }

    // did we use a new soft channel here, then set it
    if(mutexKnobDataP->getSoftPV(kData->pv, &indx, thisW)) kData->soft= true;

    // initialize channels
    //qDebug() << "pv" << kData->pv << "create channel index=" << indx << "soft=" << kData->soft << "plugin=" << kData->pluginName;

    kData->index = num;
    kData->edata.monitorCount = 0;
    kData->edata.displayCount = 0;
    kData->edata.precision = 0; //default
    kData->edata.units[0] = '\0';
    kData->edata.dataB =(void*) 0;
    kData->edata.dataSize = 0;
    kData->edata.initialize = true;
    kData->edata.lastTime = now;
    kData->edata.repRate = rate;   // default 5 Hz

    // update data structure
    mutexKnobDataP->SetMutexKnobData(num, *kData);

    // in case of a soft channel there is nothing to do
    if(kData->soft) {
        memset(kData, 0, sizeof (knobData));
        return num;
    }

    // define data acquisition
    if(plugininterface != (ControlsInterface *) 0) plugininterface->pvAddMonitor(num, kData, rate, false);

    // add for this widget the io info
    QVariant v = qVariantFromValue(kData->edata.info);
    QVariant var=w->property("InfoList");
    QVariantList infoList = var.toList();
    infoList.append(v);
    w->setProperty("InfoList", infoList);

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
    char errorType[1024], errorInfo[1024], asc[3000];

    // get latest python exception info
    PyErr_Fetch(&errObj, &errData, &errTraceback);

    pystring = NULL;
    if (errObj != NULL && (pystring = PyObject_Str(errObj)) != NULL && (PyString_Check(pystring))) {
       strcpy(errorType, PyString_AsString(pystring));
    } else {
       strcpy(errorType, "<unknown exception type>");
    }
    Py_XDECREF(pystring);

    pystring = NULL;
    if (errData != NULL && (pystring = PyObject_Str(errData)) != NULL && (PyString_Check(pystring))) {
       strcpy(errorInfo, PyString_AsString(pystring));
    } else {
       strcpy(errorInfo, "<unknown exception data>");
    }
    Py_XDECREF(pystring);

    sprintf(asc, "%s %s : %s %s", qPrintable(message), qPrintable(w->objectName()), errorType, errorInfo);

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
  */
bool CaQtDM_Lib::CalcVisibility(QWidget *w, double &result, bool &valid)
{
    double valueArray[MAX_CALC_INPUTS];
    char post[256];
    char calcString[256];
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
    }

    // no calc
    if(calcQString.length() < 1) {
        valid = true;
        return true;
    }

    calcQString=calcQString.trimmed();
    strcpy(calcString, calcQString.toLatin1().constData());

    // any monitors ?
    QVariant monitorList=w->property("MonitorList");
    QVariantList MonitorList = monitorList.toList();
    QVariant indexList=w->property("IndexList");
    QVariantList IndexList = indexList.toList();

    if(MonitorList.size() == 0) return true;

    int nbMonitors = MonitorList.at(0).toInt();
    //qDebug() << "number of monitors" << nbMonitors << "calc=" << calcString;
    if(nbMonitors > 0)  {

        setlocale(LC_NUMERIC, "C");

        // Regexp will used when is marked with %/regexp/
        QRegExp checkregexp("%\\/(\\S+)\\/");
        checkregexp.setMinimal(true);
        if (checkregexp.indexIn(calcString) != -1){
            knobData *ptr = mutexKnobDataP->GetMutexKnobDataPtr(MonitorList.at(1).toInt());
            if(ptr != (knobData *) 0) {
                char dataString[1024];
                int caFieldType= ptr->edata.fieldtype;
                QString captured_Calc = checkregexp.cap(1);
                if((caFieldType == caSTRING || caFieldType == caENUM || caFieldType == caCHAR) && ptr->edata.dataB != (void*) 0) {
                    if(ptr->edata.dataSize < 1024) {
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
                                    strcpy(dataString, ba.data());
                                }
                            }
                        }

                    } else  {
                        char asc[100];
                        sprintf(asc, "Invalid channel data type %s", qPrintable(w->objectName()));
                        postMessage(QtDebugMsg, asc);
                        valid = false;
                        return true;
                    }
                }
                QRegExp check_A(captured_Calc);
                check_A.setMinimal(true);
                if (check_A.exactMatch(dataString)){
                    result=1;
                    valid = true;
                    return true;
                } else{
                    result=0;
                    valid = true;
                    return false;
                }
            }
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
                if(ptr == (knobData*) 0) {
                    valid = false;
                    return false;
                }
            }

            PyDict_SetItemString( pGlobal, "__builtins__", PyEval_GetBuiltins() );

            // get rid of %P/ and last / on new line
            calcQString = calcQString.mid(3, calcQString.length()-4);

            char newCalc[2048];
            strcpy(newCalc, calcQString.toLatin1().constData());

            //Create a new module object
            QString myModule("myModule"+w->objectName());
            PyObject *pNewMod = PyModule_New((char*)myModule.toLatin1().constData());

            PyModule_AddStringConstant(pNewMod, "__file__", "");

            //Get the dictionary object from my module
            pLocal = PyModule_GetDict(pNewMod);

            //Define my function in the newly created module, when error then we get a null pointer back
            pValue = PyRun_String(calcQString.toLatin1().constData(), Py_file_input, pGlobal, pLocal);
            if(pValue == (PyObject *) 0) {
                valid = false;
                Py_DECREF(pNewMod);
                return Python_Error(w, "probably a syntax error on the python function (calc will be disabled)");
            }
            Py_DECREF(pValue);

            //Get a pointer to the function I just defined
            pFunc = PyObject_GetAttrString(pNewMod, "PythonCalc");
            if((pFunc == (PyObject *) 0) || (!PyCallable_Check(pFunc))) {
                valid = false;
                Py_DECREF(pNewMod);
                return Python_Error(w, "python function not found, must be called PythonCalc (calc will be disabled)");
            }

            //Build a tuple to hold my arguments (just the number 4 in this case)
            pArgs = PyTuple_New(MAXMONITORS);
            for(int i=0; i< MAXMONITORS; i++) pValueA[i] = PyFloat_FromDouble(0.0);
            for(int i=0; i< nbMonitors; i++) {
                knobData *ptr = mutexKnobDataP->GetMutexKnobDataPtr(MonitorList.at(i+1).toInt());
                if(ptr != (knobData*) 0) {
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
            if (pValue != (PyObject *) 0) {
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
            char asc[100];
            sprintf(asc, "python is not enabled in this caqtdm version(calc will be disabled) %s", qPrintable(w->objectName()));
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
                if(ptr != (knobData*) 0) {
                    //qDebug() << "calculate from index" << i << ptr->index << ptr->pv << ptr->edata.connected << ptr->edata.rvalue << IndexList.at(i+1).toInt();
                    // when connected
                    int j = IndexList.at(i+1).toInt(); // input a,b,c,d
                    if(ptr->edata.connected) {
                        valueArray[j] = ptr->edata.rvalue;
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
                char asc[100];
                sprintf(asc, "Invalid Calc %s for %s (calc will be disabled)", calcString, qPrintable(w->objectName()));
                setCalcToNothing(w);
                postMessage(QtDebugMsg, asc);
                //printf("%s\n", asc);
                valid = false;
                return true;
            }
            // Perform the calculation
            status = calcPerform(valueArray, &result, post);
            if(!status) {
                visible = (result?true:false);
                //qDebug() << "valid result" << result << visible;
                valid = true;
                return visible;
            } else {
                char asc[100];
                sprintf(asc, "invalid calc %s for %s (calc will be disabled)", calcString, qPrintable(w->objectName()));
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
    //qDebug() << "number of monitors" << nbMonitors;
    status = NO_ALARM;
    if(nbMonitors > 0)  {

        // medm uses however only first channel
        knobData *ptr = mutexKnobDataP->GetMutexKnobDataPtr(list.at(1).toInt());
        if(ptr != (knobData *) 0) {
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
                if(!isnan(data.edata.lower_disp_limit)) {
                    widget->setMinValue(data.edata.lower_disp_limit);
                    minval = data.edata.lower_disp_limit;
                } else {
                    widget->setMinValue(0.0);
                    minval = 0.0;
                }
                if(!isnan(data.edata.upper_disp_limit)) {
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
                if(!isnan(data.edata.lower_alarm_limit)) widget->setLowError(data.edata.lower_alarm_limit); else  widget->setLowError(minval);
                if(!isnan(data.edata.upper_alarm_limit)) widget->setHighError(data.edata.upper_alarm_limit); else widget->setHighError(maxval);
            }

            if(data.edata.lower_warning_limit == data.edata.upper_warning_limit) {
                // set limits to extreme values
                widget->setLowWarning(minval);
                widget->setHighWarning(maxval);
            } else {
                if(!isnan(data.edata.lower_warning_limit)) widget->setLowWarning(data.edata.lower_warning_limit); else  widget->setLowWarning(minval);
                if(!isnan(data.edata.upper_warning_limit)) widget->setHighWarning(data.edata.upper_warning_limit); else  widget->setHighWarning(maxval);
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
                if(!isnan(data.edata.lower_disp_limit)) {
                    widget->setMinValue(data.edata.lower_disp_limit);
                } else {
                    widget->setMinValue(0.0);
                }
                if(!isnan(data.edata.upper_disp_limit)) {
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
    if(w == (QWidget*) 0) return;
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

    // calc ==================================================================================================================
    if(caCalc *calcWidget = qobject_cast<caCalc *>(w)) {
        bool valid;
        double result = data.edata.rvalue;
        //qDebug() << "we have a caCalc" << calcWidget->getVariable() << "  " <<  data.pv;

        CalcVisibility(w, result, valid);  // visibility not used, but calculation yes
        if(valid) {
            if (!QString::compare(calcWidget->getVariable(), data.pv, Qt::CaseInsensitive)){
                calcWidget->setValue(result);
                mutexKnobDataP->UpdateSoftPV(data.pv, result, myWidget);
                //qDebug() << "we have a caCalc" << calcWidget->getVariable() << "  " <<  data.pv;
            }
        }

        // caLabel ==================================================================================================================
    } else if(caLabel *labelWidget = qobject_cast<caLabel *>(w)) {
        //qDebug() << "we have a label";

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
        //qDebug() << "we have a label";

        if(data.edata.connected) {
            int colorMode = labelverticalWidget->getColorMode();
            if(colorMode == caLabel::Static) {
                // done at initialisation, we have to set it back after no connect
                if(!labelverticalWidget->property("Connect").value<bool>()) {
                    QColor fg = labelverticalWidget->property("FColor").value<QColor>();
                    QColor bg = labelverticalWidget->property("BColor").value<QColor>();
                    labelverticalWidget->setForeground(fg);
                    labelverticalWidget->setBackground(bg);
                    labelverticalWidget->setProperty("Connect", true);
                }
            } else if(colorMode == caLabel::Alarm) {
                short status = ComputeAlarm(w);
                labelverticalWidget->setAlarmColors(status);
            }

            setObjectVisibility(labelverticalWidget, data.edata.rvalue);

        } else {
            SetColorsNotConnected(labelverticalWidget);
        }

        // caInclude ==================================================================================================================
    } else if(caInclude *includeWidget = qobject_cast<caInclude *>(w)) {
        //qDebug() << "we have an included frame";

        setObjectVisibility(includeWidget, data.edata.rvalue);

        // caFrame ==================================================================================================================
    } else if(caFrame *frameWidget = qobject_cast<caFrame *>(w)) {
        //qDebug() << "we have a frame";

        setObjectVisibility(frameWidget, data.edata.rvalue);

        // caMenu ==================================================================================================================
    } else if (caMenu *menuWidget = qobject_cast<caMenu *>(w)) {
        //qDebug() << "we have a menu";

        if(data.edata.connected) {
            //QStringList stringlist = String.split( ";");
            QStringList stringlist = String.split((QChar)27);
            // set enum strings
            if(data.edata.fieldtype == caENUM) {
                menuWidget->populateCells(stringlist);
                if(menuWidget->getLabelDisplay()) menuWidget->setCurrentIndex(0);
                else menuWidget->setCurrentIndex((int) data.edata.ivalue);

                if (menuWidget->getColorMode() == caMenu::Alarm) {
                    menuWidget->setAlarmColors(data.edata.severity);
                    // case of static mode
                } else {
                    SetColorsBack(menuWidget);
                }
            }
        } else {
            SetColorsNotConnected(menuWidget);
        }
        menuWidget->setAccessW(data.edata.accessW);

        // caChoice ==================================================================================================================
    } else if (caChoice *choiceWidget = qobject_cast<caChoice *>(w)) {
        //qDebug() << "we have a choiceButton" << String << value;

        if(data.edata.connected) {
            //QStringList stringlist = String.split( ";");
            QStringList stringlist = String.split((QChar)27);
            // set enum strings
            if(data.edata.fieldtype == caENUM) {
                choiceWidget->populateCells(stringlist, (int) data.edata.ivalue);
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

        // caThermo ==================================================================================================================
    } else if (caThermo *thermoWidget = qobject_cast<caThermo *>(w)) {
        //qDebug() << "we have a thermometer";

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
            if (thermoWidget->getColorMode() == caThermo::Alarm) {
                if(channelLimitsEnabled) {
                    thermoWidget->setAlarmColors(data.edata.severity);
                } else {
                    thermoWidget->setUserAlarmColors(data.edata.rvalue);
                }

                // case of static mode
            } else {
                SetColorsBack(thermoWidget);
            }
            // set no connection color
        } else {
            SetColorsNotConnected(thermoWidget);
        }

        // caSlider ==================================================================================================================
    } else if (caSlider *sliderWidget = qobject_cast<caSlider *>(w)) {

        bool channelLimitsEnabled = false;

        if(data.edata.connected) {
            if(sliderWidget->getLimitsMode() == caSlider::Channel) channelLimitsEnabled= true;
            // take limits from channel, in case of user limits these should already been set
            if((channelLimitsEnabled) && (data.edata.initialize) ) {
                // when limits are the same, do nothing
                if(data.edata.upper_disp_limit != data.edata.lower_disp_limit) {
                    disconnect(w, SIGNAL(valueChanged (double)), 0, 0);
                    if(sliderWidget->getDirection() == caSlider::Down  || sliderWidget->getDirection() == caSlider::Left) {
                        sliderWidget->setMinValue(data.edata.upper_disp_limit);
                        sliderWidget->setMaxValue(data.edata.lower_disp_limit);
                    } else {
                        sliderWidget->setMaxValue(data.edata.upper_disp_limit);
                        sliderWidget->setMinValue(data.edata.lower_disp_limit);
                    }
                    connect(w, SIGNAL(valueChanged(double)), this, SLOT(Callback_SliderValueChanged(double)));
                }
            }

            // disconnect signal to prevent from firing again
            disconnect(w, SIGNAL(valueChanged (double)), 0, 0);
            sliderWidget->setSliderValue(data.edata.rvalue);
            // reconnect signal
            connect(w, SIGNAL(valueChanged(double)), this, SLOT(Callback_SliderValueChanged(double)));

            sliderWidget->setAccessW((bool) data.edata.accessW);

            // set colors when connected
            // case of alarm mode
            if (sliderWidget->getColorMode() == caSlider::Alarm) {
                if(channelLimitsEnabled) {
                    sliderWidget->setAlarmColors(data.edata.severity);
                } else {
                    sliderWidget->setUserAlarmColors(data.edata.rvalue);
                }

                // case of static, default mode
            } else {
                SetColorsBack(sliderWidget);
            }
            // set no connection color
        } else {
            SetColorsNotConnected(sliderWidget);
        }

        // caClock ==================================================================================================================
    } else if (caClock *clockWidget = qobject_cast<caClock *>(w)) {
        if(data.edata.connected) {
            if(clockWidget->getTimeType() == caClock::ReceiveTime) {
                clockWidget->setAlarmColors(data.edata.severity);
                QDateTime dattim = QDateTime::fromTime_t((uint) (data.edata.actTime.time + data.edata.actTime.millitm/1000.0));
                clockWidget->updateClock(dattim.time());
            }
            // set no connection color
        } else {
            SetColorsNotConnected(clockWidget);
        }

        // linear gauge (like thermometer) ==================================================================================================================
    } else if (caLinearGauge *lineargaugeWidget = qobject_cast<caLinearGauge *>(w)) {
        //qDebug() << "we have a linear gauge" << value;
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
        //qDebug() << "we have a linear gauge" << value;
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
        //qDebug() << "we have a simple meter";

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


        // lineEdit and textEntry ====================================================================================================
    } else if (caLineEdit *lineeditWidget = qobject_cast<caLineEdit *>(w)) {

        //qDebug() << "we have a linedit or textentry" << lineeditWidget << data.edata.rvalue <<  data.edata.ivalue;

        QColor bg = lineeditWidget->property("BColor").value<QColor>();
        QColor fg = lineeditWidget->property("FColor").value<QColor>();

        if(data.edata.connected) {
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
                //list = String.split(";");
                list = String.split((QChar)27);

                //qDebug() << lineeditWidget << String << list << data.pv << (int) data.edata.ivalue << data.edata.valueCount;

                if((data.edata.fieldtype == caENUM)  && (list.count() == 0)) {
                    QString str= QString::number((int) data.edata.ivalue);
                    lineeditWidget->setTextLine(str);
                } else if((data.edata.fieldtype == caENUM)  && ((int) data.edata.ivalue < list.count() ) && (list.count() > 0)) {
                    if(list.at((int) data.edata.ivalue).trimmed().size() == 0)  {  // string seems to empty, give value
                        QString str= QString::number((int) data.edata.ivalue);
                        lineeditWidget->setTextLine(str);
                    } else {
                        lineeditWidget->setTextLine(list.at((int) data.edata.ivalue));
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
                }

                // double
            } else {
                int colorMode = lineeditWidget->getColorMode();
                int precMode = lineeditWidget->getPrecisionMode();
                lineeditWidget->setValueType(true);

                if(colorMode == caLineEdit::Static || colorMode == caLineEdit::Default) { // done at initialisation
                    if(!lineeditWidget->property("Connect").value<bool>()) {                      // but was disconnected before
                        lineeditWidget->setAlarmColors(data.edata.severity, data.edata.rvalue, bg, fg);
                        lineeditWidget->setProperty("Connect", true);
                    }
                } else {
                    lineeditWidget->setAlarmColors(data.edata.severity, data.edata.rvalue, bg, fg);
                }

                if((precMode != caLineEdit::User) && (data.edata.initialize)) {
                    lineeditWidget->setFormat(data.edata.precision);
                }

                lineeditWidget->setValue(data.edata.rvalue, units);
                // access control for textentry
                if (caTextEntry *textentryWidget = qobject_cast<caTextEntry *>(w)) {
                    textentryWidget->setAccessW((bool)data.edata.accessW);
                }
            }

        } else {
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

        // Graphics ==================================================================================================================
    } else if (caGraphics *graphicsWidget = qobject_cast<caGraphics *>(w)) {
        //qDebug() << "caGraphics" << graphicsWidget->objectName() << graphicsWidget->getColorMode() << data.pv;

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
        //qDebug() << "led" << led->objectName();
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
        //qDebug() << "caApplyNumeric" << applynumericWidget->objectName() << data.pv << data.edata.monitorCount;

        if(data.edata.connected) {
            ComputeNumericMaxMinPrec(applynumericWidget, data);
            applynumericWidget->setConnectedColors(true);
            applynumericWidget->silentSetValue(data.edata.rvalue);
            applynumericWidget->setAccessW((bool) data.edata.accessW);
        } else {
            applynumericWidget->setConnectedColors(false);
        }

        // Numeric =====================================================================================================
    } else if (caNumeric *numericWidget = qobject_cast<caNumeric *>(w)) {
        // qDebug() << "caNumeric" << numericWidget->objectName() << data.pv;

        if(data.edata.connected) {
            ComputeNumericMaxMinPrec(numericWidget, data);
            numericWidget->setConnectedColors(true);
            numericWidget->silentSetValue(data.edata.rvalue);
            numericWidget->setAccessW((bool) data.edata.accessW);
        } else {
            numericWidget->setConnectedColors(false);
        }

        // Numeric =====================================================================================================
    } else if (caSpinbox *spinboxWidget = qobject_cast<caSpinbox *>(w)) {
        //qDebug() << "caSpinbox" << spinboxWidget->objectName() << data.pv;

        if(data.edata.connected) {
            ComputeNumericMaxMinPrec(spinboxWidget, data);
            spinboxWidget->setConnectedColors(true);
            spinboxWidget->silentSetValue(data.edata.rvalue);
            spinboxWidget->setAccessW((bool) data.edata.accessW);
        } else {
            spinboxWidget->setConnectedColors(false);
        }

        // Toggle =====================================================================================================
    } else if (caToggleButton *togglebuttonWidget = qobject_cast<caToggleButton *>(w)) {
        //qDebug() << "caToggleButton" << togglebuttonWidget->objectName() << data.pv;
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
        } else {
            SetColorsNotConnected(togglebuttonWidget);
        }

        // cartesian plot ==================================================================================================================
    } else if (caCartesianPlot *cartesianplotWidget = qobject_cast<caCartesianPlot *>(w)) {
        //qDebug() << "caCartesianPlot" << cartesianplotWidget->objectName() << data.pv << data.specData[0] << data.specData[1]  << data.specData[2];

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
                        cartesianplotWidget->setXscaling(caCartesianPlot::Auto);
                    }
                } else if(XorY == caCartesianPlot::CH_Y && cartesianplotWidget->getYscaling() == caCartesianPlot::Channel) {
                    if(data.edata.lower_disp_limit != data.edata.upper_disp_limit) {
                        cartesianplotWidget->setScaleY(data.edata.lower_disp_limit, data.edata.upper_disp_limit);
                    } else {
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
                // qDebug() << "scale monitor from" << data.pv << data.edata.rvalue << "min/max" << data.specData[0] << "scale" << XorY;
            }

            if(!cartesianplotWidget->property("Connect").value<bool>()) {
                cartesianplotWidget->setProperty("Connect", true);
                cartesianplotWidget->setAllProperties();
            }

            // value(s)
            if(XorY == caCartesianPlot::CH_X || XorY == caCartesianPlot::CH_Y) {
                // data from vector
                if(data.edata.valueCount > 0 && data.edata.dataB != (void*) 0) {
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
                    cartesianplotWidget->setData(&p, 1, curvNB, curvType, XorY);
                    cartesianplotWidget->displayData(curvNB, curvType);
                }

                // count channel
            } else if(XorY == caCartesianPlot::CH_Count) {
                //qDebug() << "count channel" << data.edata.rvalue << qRound(data.edata.rvalue);
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
        } else {
            cartesianplotWidget->setCountNumber(0);
            cartesianplotWidget->setWhiteColors();
            cartesianplotWidget->setProperty("Connect", false);
        }

        // waterfall plot ==================================================================================================================
    } else if (caWaterfallPlot *waterfallplotWidget = qobject_cast<caWaterfallPlot *>(w)) {
        //qDebug() << "caWaterfallPlot" << waterfallplotWidget->objectName() << data.pv;

        int pvType = data.specData[0];      // waveform=0; Count=1
        int countRequested = data.specData[1];

        if(data.edata.connected) {

            if(pvType == 1) {
                //qDebug() << "count channel" << data.edata.rvalue << qRound(data.edata.rvalue);
                if(data.edata.rvalue >= 0.5) waterfallplotWidget->setCountNumber(qRound(data.edata.rvalue));
                waterfallplotWidget->setCountReceived(true);
            } else if(waterfallplotWidget->getCountReceived() || !countRequested) {

                // scale first time on first curve
                if(data.edata.initialize) {

                    double min = waterfallplotWidget->getIntensityMin();
                    double max = waterfallplotWidget->getIntensityMax();

                    if((waterfallplotWidget->getIntensityScalingMin() == caWaterfallPlot::Channel) && (waterfallplotWidget->getIntensityScalingMax() == caWaterfallPlot::Channel)) {
                        //qDebug() << "channels" <<  data.edata.lower_disp_limit << data.edata.upper_disp_limit;
                        if(data.edata.lower_disp_limit < data.edata.upper_disp_limit) {
                            waterfallplotWidget->setIntensityMin(data.edata.lower_disp_limit);
                            waterfallplotWidget->setIntensityMax(data.edata.upper_disp_limit);
                        } else {
                            waterfallplotWidget->setIntensityMin(0.0);
                            waterfallplotWidget->setIntensityMax(10.0);
                        }
                    } else if(waterfallplotWidget->getIntensityScalingMin() == caWaterfallPlot::Channel) {
                        //qDebug() << "channel1" ;
                        if(data.edata.lower_disp_limit < max) {
                            waterfallplotWidget->setIntensityMin(data.edata.lower_disp_limit);
                        }
                    }  else if(waterfallplotWidget->getIntensityScalingMax() == caWaterfallPlot::Channel) {
                        //qDebug() << "channel2" ;
                        if(data.edata.upper_disp_limit > min) {
                            waterfallplotWidget->setIntensityMax(data.edata.upper_disp_limit);
                        }
                    }
                    waterfallplotWidget->InitData(data.edata.valueCount);
                }

                // value(s)
                if((data.edata.valueCount > 0) && (data.edata.dataB != (void*) 0)) {
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
                if(actPlot == 0) {
                    double ymin = stripplotWidget->getYaxisLimitsMin(0);
                    double ymax = stripplotWidget->getYaxisLimitsMax(0);
                    stripplotWidget->setYscale(ymin, ymax);
                }
                // do this for redisplaying legend with correct limits
                if((stripplotWidget->getYscalingMin(actPlot) == caStripPlot::Channel) ||
                        (stripplotWidget->getYscalingMax(actPlot) == caStripPlot::Channel)) {
                    stripplotWidget->resize(stripplotWidget->width()+1, stripplotWidget->height()+1);
                    stripplotWidget->resize(stripplotWidget->width()-1, stripplotWidget->height()-1);
                }
            }

            stripplotWidget->setData(data.edata.actTime, data.edata.rvalue, actPlot);

            if(data.edata.initialize) {
                stripplotWidget->startPlot();
            }

        }

        // animated gif ==================================================================================================================
    } else if(caImage *imageWidget = qobject_cast<caImage *>(w)) {

        double valueArray[MAX_CALC_INPUTS];
        char post[256];
        char calcString[256];
        long status;
        short errnum;
        double result;

        //qDebug() << "we have a image" << imageWidget << kPtr->pv << data.edata.fieldtype << data.edata.dataSize << String;

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
            //qDebug() << image << "number of monitors" << nbMonitors;
            if(nbMonitors > 0)  {

                // get calc string
                //printf("get calc string <%s>\n", qPrintable(imageWidget->getImageCalc()));
                strcpy(calcString, (char*) imageWidget->getImageCalc().toLatin1().constData());

                // scan and get the channels
                for(int i=0; i < 4; i++) valueArray[i] = 0.0;
                for(int i=0; i< nbMonitors;i++) {
                    knobData *ptr = mutexKnobDataP->GetMutexKnobDataPtr(list.at(i+1).toInt());
                    if(ptr != (knobData *) 0) {
                        // when connected
                        if(ptr->edata.connected) {
                            valueArray[i] = ptr->edata.rvalue;
                        } else {
                            valueArray[i] = 0.0;
                        }
                    }
                }

                status = postfix(calcString, post, &errnum);
                if(status) {
                    imageWidget->setInvalid(Qt::black);
                    //qDebug() << "invalid calc 1" << calcString;
                }

                // Perform the calculation
                status = calcPerform(valueArray, &result, post);
                if(!status) {
                    // Result is valid, convert to frame number
                    if(result < 0.0) {
                        imageWidget->setInvalid(Qt::black);
                        //printf("no valid frame\n");
                    } else {
                        //printf("frame ok=%d\n", (int)(result +.5));
                        imageWidget->setFrame((int)(result +.5));
                    }
                } else {
                    //printf("invalid calc 2\n");
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

        // table for vwaveform values==========================================================================
    } else if(caWaveTable *wavetableWidget = qobject_cast<caWaveTable *>(w)) {

        if(data.edata.connected) {
            // data from vector
            if(data.edata.valueCount > 0 && data.edata.dataB != (void*) 0) {
                if((wavetableWidget->getPrecisionMode() != caWaveTable::User) && (data.edata.initialize)) {
                    wavetableWidget->setActualPrecision(data.edata.precision);
                }
                WaveTable(wavetableWidget, data);
            } else {
                wavetableWidget->displayText(0, NOTCONNECTED, "????");
            }

        } else {
            for(int i=0; i<wavetableWidget->getNumberOfRows(); i++) {
                int nbCol = wavetableWidget->getNumberOfColumns();
                int nbRow = i*nbCol;
                for(int j=0; j<nbCol; j++) {
                    wavetableWidget->displayText(nbRow+j, NOTCONNECTED, "NC");
                }
            }
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

        //qDebug() << data.pv << data.edata.connected << data.specData[0];
        if(data.edata.connected) {
            if(data.specData[0] == 1) {        // width channel
                cameraWidget->setWidth((int) data.edata.rvalue);
            } else if(data.specData[0] == 2) { // height channel
                cameraWidget->setHeight((int) data.edata.rvalue);
            } else if(data.specData[0] == 3) { // code channel if present
                cameraWidget->setCode((int) data.edata.rvalue);
            } else if(data.specData[0] == 4) { // bpp channel if present
                cameraWidget->setBPP((int) data.edata.rvalue);
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
                cameraWidget->showImage(data.edata.dataSize, (char*) data.edata.dataB);
                datamutex->unlock();
            }
            cameraWidget->setAccessW((bool) data.edata.accessW);
        } else {
            cameraWidget->showDisconnected();
            // todo
        }

        // scan2d =========================================================
    } else if (caScan2D *scan2dWidget = qobject_cast<caScan2D *>(w)) {

        //qDebug() << "Callback_UpdateWidget: caScan2D" << data.pv << data.edata.connected << data.specData[0];
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

        // something else (user defined monitors with non ca imageWidgets ?) ==============================================
    } else {
        //qDebug() << "unrecognized widget" << w->metaObject()->className();
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
    } else if(data.edata.fieldtype == caENUM || data.edata.fieldtype == caSTRING) {
        int trueValue = trueString.toInt(&ok1);
        int falseValue = falseString.toInt(&ok2);
        state = Qt::PartiallyChecked;

        QString str = "";
        QStringList list;
        //list = String.split(";");
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

/**
 * callback will write value to device
 */
void CaQtDM_Lib::Callback_EApplyNumeric(double value)
{
    int32_t idata = (int32_t) value;
    double rdata = value;

    caApplyNumeric *numeric = qobject_cast<caApplyNumeric *>(sender());
    if(!numeric->getAccessW()) return;
    if(numeric->getPV().length() > 0) {
        TreatOrdinaryValue(numeric->getPV(), rdata,  idata, (QWidget*) numeric);
    }
}
/**
 * callback will write value to device
 */
void CaQtDM_Lib::Callback_ENumeric(double value)
{
    int32_t idata = (int32_t) value;
    double rdata = value;

    caNumeric *numeric = qobject_cast<caNumeric *>(sender());
    if(!numeric->getAccessW()) return;
    if(numeric->getPV().length() > 0) {
        TreatOrdinaryValue(numeric->getPV(), rdata,  idata, (QWidget*) numeric);
    }
}

/**
 * callback will write value to device
 */
void CaQtDM_Lib::Callback_Spinbox(double value)
{
    int32_t idata = (int32_t) value;
    double rdata = value;

    caSpinbox *numeric = qobject_cast<caSpinbox *>(sender());
    if(!numeric->getAccessW()) return;
    if(numeric->getPV().length() > 0) {
        TreatOrdinaryValue(numeric->getPV(), rdata,  idata, (QWidget*) numeric);
    }
}

void CaQtDM_Lib::Callback_SliderValueChanged(double value)
{
    int32_t idata = (int32_t) value;
    double rdata = value;

    caSlider *numeric = qobject_cast<caSlider *>(sender());
    if(!numeric->getAccessW()) return;
    if(numeric->getPV().length() > 0) {
        TreatOrdinaryValue(numeric->getPV(), rdata, idata,  (QWidget*) numeric);
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
    char errmess[255];
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
        QStringsToChars(w->getPV(), svalue,  w->objectName().toLower());
        ControlsInterface * plugininterface = (ControlsInterface *) w->property("Interface").value<void *>();
        if(plugininterface != (ControlsInterface *) 0) plugininterface->pvSetValue(param1, rvalue, ivalue, param2, param3, errmess, 0);
    }
}

/**
 * updates the text of a QLineEdit widget given by its name
 */
void CaQtDM_Lib::Callback_UpdateLine(const QString& text, const QString& name)
{
    //qDebug() << "Callback_UpdateLine" << text << name;
    QLineEdit *lineedit = this->findChild<QLineEdit*>(name);
    if (lineedit != NULL) lineedit->setText(text);
}

/**
 * callback will write value to device
 */
void CaQtDM_Lib::Callback_ChoiceClicked(const QString& text)
{
    char errmess[255];

    caChoice *choice = qobject_cast<caChoice *>(sender());

    choice->updateChoice();

    if(!choice->getAccessW()) return;

    if(choice->getPV().length() > 0) {
        //qDebug() << "choice_clicked" << text << choice->getPV();
        QStringsToChars(choice->getPV(), text,  choice->objectName().toLower());
        ControlsInterface * plugininterface = (ControlsInterface *) choice->property("Interface").value<void *>();
        if(plugininterface != (ControlsInterface *) 0) plugininterface->pvSetValue(param1, 0.0, 0, param2, param3, errmess, 0);
    }
}

/**
 * callback will write value to device
 */
void CaQtDM_Lib::Callback_MenuClicked(const QString& text)
{
    char errmess[255];
    caMenu *menu = qobject_cast<caMenu *>(sender());

    if(!menu->getAccessW()) return;

    if(menu->getPV().length() > 0) {
        //qDebug() << "menu_clicked" << text << menu->getPV();
        QStringsToChars(menu->getPV(), text,  menu->objectName().toLower());
        ControlsInterface * plugininterface = (ControlsInterface *) menu->property("Interface").value<void *>();
        if(plugininterface != (ControlsInterface *) 0) plugininterface->pvSetValue(param1, 0.0, 0, param2, param3, errmess, 0);
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
    caTextEntry::FormatType fType;
    QWidget *w1 = qobject_cast<QWidget *>(sender());
    caTextEntry *w = qobject_cast<caTextEntry *>(sender());

    if(!w->getAccessW()) return;

    fType = w->getFormatType();
    TreatRequestedValue(w->getPV(), text, fType, w1);
}

void CaQtDM_Lib::Callback_WaveEntryChanged(const QString& text, int index)
{
    caWaveTable::FormatType fType;
    QWidget *w1 = qobject_cast<QWidget *>(sender());
    caWaveTable *w = qobject_cast<caWaveTable *>(sender());

    if(!w->getAccessW()) return;

    //qDebug() << "should write" << text << "at index" << index;
    fType = w->getFormatType();
    TreatRequestedWave(w->getPV(), text, fType, index, w1);
}

/**
 * callback will call the specified ui file
 */
void CaQtDM_Lib::Callback_RelatedDisplayClicked(int indx)
{
    caRelatedDisplay *w = qobject_cast<caRelatedDisplay *>(sender());
    //qDebug() << "relateddisplaycallback" << indx << w;
    QStringList files = w->getFiles().split(";");
    QStringList args = w->getArgs().split(";");
    QStringList removeParents = w->getReplaceModes().split(";");
    //qDebug() << "files:" << files;
    //qDebug() << "args" <<  w->getArgs() << args;

    // find position of this window
    int xpos = this->pos().x();
    int ypos = this->pos().y();
    QString geometry = "+%1+%2";
    geometry = geometry.arg(xpos).arg(ypos);

    // do we have to remove this window while removeparent was specified
    if(indx < removeParents.count()) {
        QString removeParent = removeParents.at(indx);
        removeParent = removeParent.toLower();
        if(removeParent.contains("true")) {
            this->close();
        } else {
            // in case we do not remove the parent let the window manager position the new window
            geometry = "";
        }
    }

    // open new file and
    if(indx < files.count() && indx < args.count()) {
        emit Signal_OpenNewWFile(files[indx].trimmed(), args[indx].trimmed(), geometry);
    } else if(indx < files.count()) {
        emit Signal_OpenNewWFile(files[indx].trimmed(), "", geometry);
    }

}

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
        if(w->getPressMessage().size() > 0)
            TreatRequestedValue(w->getPV(), w->getPressMessage(), caTextEntry::decimal, w1);
    } else if(type == 1) {  // released
        if(w->getReleaseMessage().size() > 0)
            TreatRequestedValue(w->getPV(), w->getReleaseMessage(), caTextEntry::decimal, w1);
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
        TreatOrdinaryValue(w->getPV(), (double) number, (int32_t) number,  w1);
        // bit not set
    } else {
        number |= 1 << bit;
        TreatOrdinaryValue(w->getPV(), (double) number, (int32_t) number,  w1);
    }
}

/**
 * callback will execute script
 */
void CaQtDM_Lib::Callback_ScriptButton()
{
#ifndef MOBILE
    QString command = "";
    bool displayWindow;
    caScriptButton *w = qobject_cast<caScriptButton *>(sender());
    command.append(w->getScriptCommand());

    if(w->getScriptParam().size() > 0) {
        command.append("\n\"");
        command.append( w->getScriptParam());
        command.append(" \"");
    }
    displayWindow = w->getDisplayShowExecution();

    if(w->getAccessW()) {
        processWindow *t = new processWindow(this, displayWindow, w);
        connect(t, SIGNAL(processClose()), this, SLOT(processTerminated()));
        t->start(command);
        w->setToolTip("process running, to kill use right mouse button !");
        w->setAccessW(false);
        w->setProcess(t);
    }
#endif
}

void CaQtDM_Lib::processTerminated()
{
#ifndef MOBILE
    //qDebug() << "caQtDM -- process terminated callback";
    processWindow *t = qobject_cast<processWindow *>(sender());
    QWidget *w = t->getProcessCaller();
    caScriptButton *w1 = qobject_cast<caScriptButton *>(w);
    if(w1 != (QWidget*) 0) {
        w1->setToolTip("process terminated !");
        w1->setAccessW(true);
        w->setEnabled(true);
    }

    if(t != (processWindow *) 0) t->deleteLater();
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
    command.replace("&T", thisFileShort);
    command.replace("&A", thisFileFull);
#ifdef linux
    int windid = this->winId();
    command.replace("&X", QString::number(windid));
#endif
    command = command.trimmed();
    postMessage(QtDebugMsg, (char*) qPrintable(command.trimmed()));
#ifndef linux
    if(command.endsWith("&")) command.remove(command.size()-1, 1);
    //qDebug() << "execute:" << command;
    proc = new myQProcess( this);
    proc->start(command.trimmed(), QIODevice::ReadWrite);
#else
    // I had too many problems with QProcess start, use standard execl
    if(!command.endsWith("&")) command.append("&");
    //qDebug() << "execute:" << command;
    int status = Execute((char*)command.toLatin1().constData());
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
    close();
}

/**
  * when closing the window, we will clear all associated monitors and free data
  */
void CaQtDM_Lib::closeEvent(QCloseEvent* ce)
{
    Q_UNUSED(ce);

    killTimer(loopTimerID);

    AllowsUpdate = false;

    for(int i=0; i < mutexKnobDataP->GetMutexKnobDataSize(); i++) {

        knobData kData =  mutexKnobDataP->GetMutexKnobData(i);

        if((kData.index != -1) && (myWidget == (QWidget*) kData.thisW)) {
            QString pv = kData.pv;
            QWidget* w = (QWidget*) kData.thisW;
            short soft = kData.soft;
            //qDebug() << pv << "clear monitor at" << i << "index="  << kData.index << "plugin" << kData.pluginName;
            if(soft) {
                mutexKnobDataP->RemoveSoftPV(pv, w, kData.index);
            } else {
               ControlsInterface * plugininterface = getControlInterface(kData.pluginName);
               if(plugininterface != (ControlsInterface *) 0) plugininterface->pvClearMonitor(&kData);
            }
            kData.index = -1;
            kData.pv[0] = '\0';
            mutexKnobDataP->SetMutexKnobData(i, kData);
        }
    }

    Sleep::msleep(200);

    // get rid of memory, that was allocated before for this window.
    // it has not been done previously, while otherwise in the datacallback
    // you will run into trouble
    for(int i=0; i < mutexKnobDataP->GetMutexKnobDataSize(); i++) {
        knobData *kPtr = mutexKnobDataP->GetMutexKnobDataPtr(i);
        if(kPtr != (knobData *) 0) {
            if(myWidget == (QWidget*) kPtr->thisW) {
                if (kPtr->edata.info != (void *) 0) {
                    free(kPtr->edata.info);
                    kPtr->edata.info = (void*) 0;
                    kPtr->thisW = (void*) 0;
                }
                if(kPtr->edata.dataB != (void*) 0) {
                    free(kPtr->edata.dataB);
                    kPtr->edata.dataB = (void*) 0;
                }
                if(kPtr->mutex != (QMutex *) 0) {
                    QMutex *mutex = (QMutex *) kPtr->mutex;
                    delete mutex;
                    kPtr->mutex = (QMutex *) 0;
                }
            }
        }
    }
    mutexKnobDataP->initHighestCountPV();

    // in case of network launcher, close the application when launcher window is closed
    QString thisFileName =  property("fileString").toString().section('/',-1);
    QString launchFile = (QString)  qgetenv("CAQTDM_LAUNCHFILE");
    if(thisFileName.contains(launchFile)) {
        emit Signal_IosExit();
    }
}

/**
  * will display a context menu, composed of the channels associated to the object
  */
void CaQtDM_Lib::DisplayContextMenu(QWidget* w)
{
    QMenu myMenu;
    QPoint cursorPos =QCursor::pos() ;

    bool onMain = false;
    QString pv[20];
    int nbPV = 0;
    int limitsDefault = false;
    int precMode = false;
    int limitsMode = false;
    int Precision = 0;
    const char *caTypeStr[] = {"DBF_STRING", "DBF_INT", "DBF_FLOAT", "DBF_ENUM", "DBF_CHAR", "DBF_LONG", "DBF_DOUBLE"};
    char colMode[20] = {""};
    QString calcString = "";
    QString imageString = "";
    double limitsMax=0.0, limitsMin=0.0;
    bool validExecListItems = false;
    QStringList execListItems;
    QString className = w->metaObject()->className();

    // execution list for context menu defined ?
    QString execList = (QString)  qgetenv("CAQTDM_EXEC_LIST");

    if(!execList.isNull() && execList.size() > 0) {
#ifdef _MSC_VER
        execListItems= execList.split(";", QString::SkipEmptyParts);
#else
        execListItems= execList.split(":", QString::SkipEmptyParts);
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
            execListItems= execList.split(";", QString::SkipEmptyParts);
#else
            execListItems= execList.split(":", QString::SkipEmptyParts);
#endif
            for(int i=0; i<execListItems.count(); i++) {
                validExecListItems = true;
            }
        }
    }

    if(caImage* imageWidget = qobject_cast<caImage *>(w)) {
        getAllPVs(imageWidget);
        GetDefinedCalcString(caImage, imageWidget, calcString);
        imageString =imageWidget->getImageCalc();
    } else if(caFrame* frameWidget = qobject_cast<caFrame *>(w)) {
        getAllPVs(frameWidget);
        GetDefinedCalcString(caFrame, frameWidget, calcString);
    } else if(caInclude* includeWidget = qobject_cast<caInclude *>(w)) {
        getAllPVs(includeWidget);
        GetDefinedCalcString(caInclude, includeWidget, calcString);
    } else if(caLabel* labelWidget = qobject_cast<caLabel *>(w)) {
        getAllPVs(labelWidget);
        GetDefinedCalcString(caLabel, labelWidget, calcString);
    } else if(caLabelVertical* labelverticalWidget = qobject_cast<caLabelVertical *>(w)) {
        getAllPVs(labelverticalWidget);
        GetDefinedCalcString(caLabelVertical, labelverticalWidget, calcString);
    } else if(caGraphics* graphicsWidget = qobject_cast<caGraphics *>(w)) {
        GetDefinedCalcString(caGraphics, graphicsWidget, calcString);
        if(graphicsWidget->getColorMode() == caGraphics::Alarm) strcpy(colMode, "Alarm");
        else strcpy(colMode, "Static");
    } else if(caPolyLine* polylineWidget = qobject_cast<caPolyLine *>(w)) {
        getAllPVs(polylineWidget);
        GetDefinedCalcString(caPolyLine, polylineWidget, calcString);
        if(polylineWidget->getColorMode() == caPolyLine::Alarm) strcpy(colMode, "Alarm");
        else strcpy(colMode, "Static");
    } else if(caCalc* calcWidget = qobject_cast<caCalc *>(w)) {
        pv[0] = calcWidget->getVariable().toLatin1().constData();
        nbPV++;
        getAllPVs(calcWidget);
        calcString = calcWidget->getCalc();
    } else if(caMenu* menuWidget = qobject_cast<caMenu *>(w)) {
        pv[0] = menuWidget->getPV().trimmed();
        nbPV = 1;
    } else if(caChoice* choiceWidget = qobject_cast<caChoice *>(w)) {
        pv[0] = choiceWidget->getPV().trimmed();
        if(choiceWidget->getColorMode() == caChoice::Alarm) strcpy(colMode, "Alarm");
        else strcpy(colMode, "Static");
        nbPV = 1;
    } else if(caTextEntry* textentryWidget = qobject_cast<caTextEntry *>(w)) {
        pv[0] = textentryWidget->getPV().trimmed();
        nbPV = 1;
    } else if(caLineEdit* lineeditWidget = qobject_cast<caLineEdit *>(w)) {
        pv[0] = lineeditWidget->getPV().trimmed();
        if(lineeditWidget->getPrecisionMode() == caLineEdit::User) {
            precMode = true;
            Precision = lineeditWidget->getPrecision();
        }
        if(lineeditWidget->getLimitsMode() == caLineEdit::User) {
            limitsMode = true;
            limitsMax = lineeditWidget->getMaxValue();
            limitsMin = lineeditWidget->getMinValue();
        }
        if(lineeditWidget->getColorMode() == caLineEdit::Alarm_Default) strcpy(colMode, "Alarm");
        else if(lineeditWidget->getColorMode() == caLineEdit::Alarm_Static) strcpy(colMode, "Alarm");
        else strcpy(colMode, "Static");
        nbPV = 1;
    } else if (caApplyNumeric* applynumericWidget = qobject_cast<caApplyNumeric *>(w)) {
        pv[0] = applynumericWidget->getPV().trimmed();
        if(applynumericWidget->getPrecisionMode() == caApplyNumeric::User) {
            precMode = true;
            Precision = applynumericWidget->decDigits();
        } else {
            knobData *kPtr = mutexKnobDataP->getMutexKnobDataPV(w, pv[0]);
            if(kPtr != (knobData *) 0) Precision =  kPtr->edata.precision;
        }
        nbPV = 1;
    } else if (caNumeric* numericWidget = qobject_cast<caNumeric *>(w)) {
        pv[0] = numericWidget->getPV();
        if(numericWidget->getPrecisionMode() == caNumeric::User) {
            precMode = true;
            Precision = numericWidget->decDigits();
        } else {
            knobData *kPtr = mutexKnobDataP->getMutexKnobDataPV(w, pv[0]);
            if(kPtr != (knobData *) 0) Precision =  kPtr->edata.precision;
        }
        nbPV = 1;
    } else if (caSpinbox* spinboxWidget = qobject_cast<caSpinbox *>(w)) {
        pv[0] = spinboxWidget->getPV();
        if(spinboxWidget->getPrecisionMode() == caSpinbox::User) {
            precMode = true;
            Precision = spinboxWidget->decDigits();
        } else {
            knobData *kPtr = mutexKnobDataP->getMutexKnobDataPV(w, pv[0]);
            if(kPtr != (knobData *) 0) Precision =  kPtr->edata.precision;
        }
        nbPV = 1;
    } else if (caMessageButton* messagebuttonWidget = qobject_cast<caMessageButton *>(w)) {
        pv[0] = messagebuttonWidget->getPV().trimmed();
        pv[1] = messagebuttonWidget->getDisablePV().trimmed();
        nbPV = 2;
    } else if(caLed* ledWidget = qobject_cast<caLed *>(w)) {
        pv[0] = ledWidget->getPV().trimmed();
        nbPV = 1;
    } else if(caToggleButton* togglebuttonWidget = qobject_cast<caToggleButton *>(w)) {
        pv[0] = togglebuttonWidget->getPV().trimmed();
        nbPV = 1;
    } else if(caBitnames* bitnamesWidget = qobject_cast<caBitnames *>(w)) {
        pv[0] = bitnamesWidget->getEnumPV().trimmed();
        pv[1] = bitnamesWidget->getValuePV().trimmed();
        nbPV = 2;
    } else if(caSlider* sliderWidget = qobject_cast<caSlider *>(w)) {
        pv[0] = sliderWidget->getPV().trimmed();
        if(sliderWidget->getLimitsMode() == caSlider::User) {
            limitsMode = true;
            limitsMax = sliderWidget->getMaxValue();
            limitsMin = sliderWidget->getMinValue();
        }
        if(sliderWidget->getColorMode() == caSlider::Alarm) strcpy(colMode, "Alarm");
        else strcpy(colMode, "Static");
        nbPV = 1;
    } else if (caClock* clockWidget = qobject_cast<caClock *>(w)) {
        pv[0] = clockWidget->getPV().trimmed();
        nbPV = 1;
    } else if (caMeter* meterWidget = qobject_cast<caMeter *>(w)) {
        pv[0] = meterWidget->getPV().trimmed();
        nbPV = 1;
    } else if(caThermo* thermoWidget = qobject_cast<caThermo *>(w)) {
        pv[0] = thermoWidget->getPV().trimmed();
        if(thermoWidget->getLimitsMode() == caThermo::User) {
            limitsMode = true;
            limitsMax = thermoWidget->maxValue();
            limitsMin = thermoWidget->minValue();
        }
        knobData *kPtr = mutexKnobDataP->getMutexKnobDataPV(w, pv[0]);
        if(kPtr != (knobData *) 0) {
            if(kPtr->edata.lower_disp_limit == kPtr->edata.upper_disp_limit) {
                limitsDefault = true;
                limitsMax = thermoWidget->maxValue();
                limitsMin = thermoWidget->minValue();
            }
        }
        if(thermoWidget->getColorMode() == caThermo::Alarm) strcpy(colMode, "Alarm");
        else strcpy(colMode, "Static");
        nbPV = 1;
    } else if(caLinearGauge* lineargaugeWidget = qobject_cast<caLinearGauge *>(w)) {
        pv[0] = lineargaugeWidget->getPV().trimmed();
        nbPV = 1;
    } else if(caWaveTable* wavetableWidget = qobject_cast<caWaveTable *>(w)) {
        wavetableWidget->clearSelection();
        pv[0] = wavetableWidget->getPV().trimmed();
        nbPV = 1;
    } else if(caCircularGauge* circulargaugeWidget = qobject_cast<caCircularGauge *>(w)) {
        pv[0] = circulargaugeWidget->getPV().trimmed();
        nbPV = 1;
    } else if(caByte* byteWidget = qobject_cast<caByte *>(w)) {
        pv[0] = byteWidget->getPV().trimmed();
        if(byteWidget->getColorMode() == caByte::Alarm) strcpy(colMode, "Alarm");
        else strcpy(colMode, "Static");
        nbPV = 1;
    } else if(caByteController* bytecontrollerWidget = qobject_cast<caByteController *>(w)) {
        pv[0] = bytecontrollerWidget->getPV().trimmed();
        if(bytecontrollerWidget->getColorMode() == caByteController::Alarm) strcpy(colMode, "Alarm");
        else strcpy(colMode, "Static");
        nbPV = 1;
    } else if(caStripPlot* stripplotWidget = qobject_cast<caStripPlot *>(w)) {
        QString pvs = stripplotWidget->getPVS();
        QStringList vars = pvs.split(";", QString::SkipEmptyParts);
        nbPV = min(vars.count(), caStripPlot::MAXCURVES);
        for(int i=0; i<nbPV; i++) {
            pv[i] = vars.at(i).trimmed();
        }
    } else if(caWaterfallPlot* waterfallplotWidget = qobject_cast<caWaterfallPlot *>(w)) {
        nbPV = 0;
        pv[nbPV++] = waterfallplotWidget->getPV().trimmed();
        QString CountPV = waterfallplotWidget->getCountPV();
        if(CountPV.trimmed().length() > 0) pv[nbPV++] = CountPV.trimmed();
    } else if(caCartesianPlot* cartesianplotWidget = qobject_cast<caCartesianPlot *>(w)) {
        nbPV = 0;
        for(int i=0; i < caCartesianPlot::curveCount; i++) {
            QStringList thisString;
            thisString = cartesianplotWidget->getPV(i).split(";");
            if(thisString.count() == 2 && thisString.at(0).trimmed().length() > 0 && thisString.at(1).trimmed().length() > 0) {
                pv[nbPV++] = thisString.at(0).trimmed();
                pv[nbPV++] = thisString.at(1).trimmed();
            } else if(thisString.count() == 2 && thisString.at(0).trimmed().length() > 0 && thisString.at(1).trimmed().length() == 0) {
                pv[nbPV++] = thisString.at(0).trimmed();
            } else if(thisString.count() == 2 && thisString.at(1).trimmed().length() > 0 && thisString.at(0).trimmed().length() == 0) {
                pv[nbPV++] = thisString.at(1).trimmed();
            }
        }
        QString TriggerPV = cartesianplotWidget->getTriggerPV();
        if(TriggerPV.trimmed().length() > 0) pv[nbPV++] = TriggerPV.trimmed();
        QString CountPV = cartesianplotWidget->getCountPV();
        if(CountPV.trimmed().length() > 0) pv[nbPV++] = CountPV.trimmed();
        QString ErasePV = cartesianplotWidget->getErasePV();
        if(ErasePV.trimmed().length() > 0) pv[nbPV++] = ErasePV.trimmed();
    } else if(caCamera* cameraWidget = qobject_cast<caCamera *>(w)) {
        nbPV=0;
        for(int i=0; i< 6; i++) {
            QString text;
            if(i==0) text = cameraWidget->getPV_Data();
            if(i==1) text = cameraWidget->getPV_Width();
            if(i==2) text = cameraWidget->getPV_Height();
            if(i==3) text = cameraWidget->getPV_Code();
            if(i==4) text = cameraWidget->getPV_BPP();
            if(i<5 && text.size() > 0) {
                pv[nbPV] = text;
                nbPV++;
            } else if(i==5) {
                for(int j=0; j<2; j++) {
                    QStringList thisString;
                    if(j==0) thisString = cameraWidget->getROIChannelsRead().split(";");
                    if(j==1) thisString = cameraWidget->getROIChannelsWrite().split(";");

                    if(thisString.count() == 4 &&
                            thisString.at(0).trimmed().length() > 0 &&
                            thisString.at(1).trimmed().length() > 0 &&
                            thisString.at(2).trimmed().length() > 0 &&
                            thisString.at(3).trimmed().length() > 0) {
                        for(int k=0; k<4; k++) {
                            text = thisString.at(k);
                            pv[nbPV] = text;
                            nbPV++;
                        }
                    }
                }
            }
        }

    } else if(caScriptButton* scriptbuttonWidget =  qobject_cast< caScriptButton *>(w)) {
        // add acion : kill associated process if running
        if(!scriptbuttonWidget->getAccessW()) myMenu.addAction("Kill Process");

    } else if(caScan2D* scan2dWidget = qobject_cast<caScan2D *>(w)) {
        nbPV=0;
        for(int i=0; i< 5; i++) {
            QString text;
            if(i==0) text = scan2dWidget->getPV_Data();
            if(i==1) text = scan2dWidget->getPV_Width();
            if(i==2) text = scan2dWidget->getPV_Height();
            if(text.size() > 0) {
                pv[nbPV] = text;
                nbPV++;
            }
        }

    } else if(className.contains("QE")) {
        qDebug() << "treat" << w;

        // must be mainwindow
    } else if(w==myWidget->parent()->parent()) {
        //qDebug() << "must be mainwindow?" << w << myWidget->parent()->parent();
        onMain = true;
        myMenu.addAction("Print");
        myMenu.addAction("Raise main window");
        myMenu.addAction("Include files");
    }

    // add some more actions
    if(caScriptButton* scriptbuttonWidget =  qobject_cast< caScriptButton *>(w)) {
        Q_UNUSED(scriptbuttonWidget);

        // for the camera cameraWidget
    } else if(caCamera * cameraWidget = qobject_cast< caCamera *>(w)) {
        QAction *menuAction;
        menuAction = myMenu.addAction("Toggle fit to size");
        menuAction->setCheckable(true);
        if(cameraWidget->getFitToSize() == caCamera::Yes) menuAction->setChecked(true);
        else  menuAction->setChecked(false);
        addColorTableActions;

        // for the scan2d scan2dWidget
    } else if(caScan2D * scan2dWidget = qobject_cast< caScan2D *>(w)) {
        QAction *menuAction;
        menuAction = myMenu.addAction("Toggle fit to size");
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
        myMenu.addAction("Get Info");
        myMenu.addAction("Change Increment/Value");

        // all other widgets
    } else if(!onMain){
        // construct info for the pv we are pointing at
        myMenu.addAction("Get Info");
    }

    // for stripplot add one more action
    if(caStripPlot* stripplotWidget = qobject_cast<caStripPlot *>(w)) {
        Q_UNUSED(stripplotWidget);
        myMenu.addAction("Change Axis");
    }

    // for cartesian plot add more actions
    if(caCartesianPlot* cartesianplotWidget = qobject_cast<caCartesianPlot *>(w)) {
        Q_UNUSED(cartesianplotWidget);
        myMenu.addAction("Change Axis");
        myMenu.addAction(QWhatsThis::createAction());
        myMenu.addAction("Reset zoom");
    }

    // for catextentry add filedialog
    if(caTextEntry* catextentryWidget = qobject_cast<caTextEntry *>(w)) {
        if(catextentryWidget->getAccessW()) {
            knobData *kPtr = mutexKnobDataP->getMutexKnobDataPV(w, pv[0]);  // use pointer for getting all necessary information
            if((kPtr != (knobData *) 0) && (pv[0].length() > 0)) {
                myMenu.addAction("Input Dialog");
                if((kPtr->edata.fieldtype == caSTRING) || (kPtr->edata.fieldtype == caCHAR)) {
                    myMenu.addAction("File Dialog");
                }
            }
        }
    }

    // for some widgets one more action
    if(caSlider * widget = qobject_cast< caSlider *>(w)) {Q_UNUSED(widget); myMenu.addAction("Change Limits/Precision");}
    if(caLineEdit* widget = qobject_cast<caLineEdit *>(w)) {Q_UNUSED(widget); myMenu.addAction("Change Limits/Precision");}
    if(caThermo* widget = qobject_cast<caThermo *>(w)){Q_UNUSED(widget);  myMenu.addAction("Change Limits/Precision");}
    if(caNumeric* widget = qobject_cast<caNumeric *>(w)) {Q_UNUSED(widget); myMenu.addAction("Change Limits/Precision");}
    if(caApplyNumeric* widget = qobject_cast<caApplyNumeric *>(w)) {Q_UNUSED(widget); myMenu.addAction("Change Limits/Precision");}
    if(caSpinbox* widget = qobject_cast<caSpinbox *>(w)) {Q_UNUSED(widget); myMenu.addAction("Change Limits/Precision");}
    if(caLinearGauge* widget = qobject_cast<caLinearGauge *>(w)) {Q_UNUSED(widget); myMenu.addAction("Change Limits/Precision");}
    if(caCircularGauge* widget = qobject_cast<caCircularGauge *>(w)) {Q_UNUSED(widget); myMenu.addAction("Change Limits/Precision");}
    if(caMeter* widget = qobject_cast<caMeter *>(w)) {Q_UNUSED(widget); myMenu.addAction("Change Limits/Precision");}

    // add to context menu, the actions requested by the environment variable caQtDM_EXEC_LIST
    if(validExecListItems) {
        for(int i=0; i<execListItems.count(); i++) {
            QStringList item = execListItems[i].split(";");
            if(item.count() > 1) {
                if(!item[1].contains("&P") && onMain) myMenu.addAction(item[0]);
                else if(item[1].contains("&P") && !onMain && nbPV > 0) myMenu.addAction(item[0]);
            }
        }
    }

    QAction* selectedItem = myMenu.exec(cursorPos);

    if (selectedItem) {
        if(selectedItem->text().contains("Kill Process")) {
            if(caScriptButton* scriptbuttonWidget =  qobject_cast< caScriptButton *>(w)) {
#ifndef MOBILE
                processWindow *t= (processWindow *) scriptbuttonWidget->getProcess();
                t->tryTerminate();
#else
                Q_UNUSED(scriptbuttonWidget);
#endif
            }

        } else  if(selectedItem->text().contains("Raise message window")) {
            QMainWindow *mainWindow = (QMainWindow *) this->parentWidget();
            mainWindow->showNormal();
            if(messageWindowP != (MessageWindow *) 0) messageWindowP->raise();

        } else  if(selectedItem->text().contains("Toggle fit to size")) {
            if(caCamera * cameraWidget = qobject_cast< caCamera *>(w)) {
                if(cameraWidget->getFitToSize() == caCamera::Yes) cameraWidget->setFitToSize(caCamera::No);
                else cameraWidget->setFitToSize(caCamera::Yes);
            }
            else if(caScan2D * scan2dWidget = qobject_cast< caScan2D *>(w)) {
                if(scan2dWidget->getFitToSize() == caScan2D::Yes) scan2dWidget->setFitToSize(caScan2D::No);
                else scan2dWidget->setFitToSize(caScan2D::Yes);
            }

        } else  if(selectedItem->text().contains("Set Spectrum Wavelength")) {
            if(caCamera * cameraWidget = qobject_cast< caCamera *>(w)) cameraWidget->setColormap(caCamera::spectrum_wavelength);
            else if(caScan2D * scan2dWidget = qobject_cast< caScan2D *>(w)) scan2dWidget->setColormap(caScan2D::spectrum_wavelength);
            else if(caWaterfallPlot * waterfallplotWidget = qobject_cast< caWaterfallPlot *>(w)) waterfallplotWidget->setColormap(caWaterfallPlot::spectrum_wavelength);

        } else  if(selectedItem->text().contains("Set Spectrum Hot")) {
            if(caCamera * cameraWidget = qobject_cast< caCamera *>(w)) cameraWidget->setColormap(caCamera::spectrum_hot);
            else if(caScan2D * scan2dWidget = qobject_cast< caScan2D *>(w)) scan2dWidget->setColormap(caScan2D::spectrum_hot);
            else if(caWaterfallPlot * waterfallplotWidget = qobject_cast< caWaterfallPlot *>(w)) waterfallplotWidget->setColormap(caWaterfallPlot::spectrum_hot);

        } else  if(selectedItem->text().contains("Set Spectrum Heat")) {
            if(caCamera * cameraWidget = qobject_cast< caCamera *>(w)) cameraWidget->setColormap(caCamera::spectrum_heat);
            else if(caScan2D * scan2dWidget = qobject_cast< caScan2D *>(w)) scan2dWidget->setColormap(caScan2D::spectrum_heat);
            else if(caWaterfallPlot * waterfallplotWidget = qobject_cast< caWaterfallPlot *>(w)) waterfallplotWidget->setColormap(caWaterfallPlot::spectrum_heat);

        } else  if(selectedItem->text().contains("Set Spectrum Jet")) {
            if(caCamera * cameraWidget = qobject_cast< caCamera *>(w)) cameraWidget->setColormap(caCamera::spectrum_jet);
            else if(caScan2D * scan2dWidget = qobject_cast< caScan2D *>(w)) scan2dWidget->setColormap(caScan2D::spectrum_jet);
            else if(caWaterfallPlot * waterfallplotWidget = qobject_cast< caWaterfallPlot *>(w)) waterfallplotWidget->setColormap(caWaterfallPlot::spectrum_jet);

        } else  if(selectedItem->text().contains("Set Spectrum Custom")) {
            if(caCamera * cameraWidget = qobject_cast< caCamera *>(w)) cameraWidget->setColormap(caCamera::spectrum_custom);
            else if(caScan2D * scan2dWidget = qobject_cast< caScan2D *>(w)) scan2dWidget->setColormap(caScan2D::spectrum_custom);
            else if(caWaterfallPlot * waterfallplotWidget = qobject_cast< caWaterfallPlot *>(w)) waterfallplotWidget->setColormap(caWaterfallPlot::spectrum_custom);

        } else  if(selectedItem->text().contains("Set Greyscale")) {
            if(caCamera * cameraWidget = qobject_cast< caCamera *>(w)) cameraWidget->setColormap(caCamera::grey);
            else if(caScan2D * scan2dWidget = qobject_cast< caScan2D *>(w)) scan2dWidget->setColormap(caScan2D::grey);
            else if(caWaterfallPlot * waterfallplotWidget = qobject_cast< caWaterfallPlot *>(w)) waterfallplotWidget->setColormap(caWaterfallPlot::grey);

        } else  if(selectedItem->text().contains("Include files")) {
            QString info;
            info.append(InfoPrefix);
            info.append(includeFiles);
            info.append(InfoPostfix);
            myMessageBox box(this);
            box.setText("<html>" + info + "</html>");
            box.exec();
        } else  if(selectedItem->text().contains("Get Info")) {
            QString info;
            info.append(InfoPrefix);
            info.append("-----------------------------------------------------------------<br>");

            info.append("Object: ");
            info.append(w->objectName());
            info.append("<br>");
            if(!calcString.isEmpty()) {
                info.append("<br>");
                info.append("VisibilityCalc: ");  
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

            qDebug() << nbPV;

            for(int i=0; i< nbPV; i++) {
                // is there a json string ?
                int pos = pv[i].indexOf("{");
                if(pos != -1) pv[i] = pv[i].mid(0, pos);
                knobData *kPtr = mutexKnobDataP->getMutexKnobDataPV(w, pv[i]);  // use pointer for getting all necessary information

                if((kPtr != (knobData *) 0) && (pv[i].length() > 0)) {
                    char asc[2048] = {'\0'};
                    char timestamp[50] = {'\0'};
                    char description[40] = {'\0'};
                    info.append("<br>");
                    info.append(kPtr->pv);

                    info.append("<br>");
                    info.append("Plugin: ");
                    info.append(kPtr->pluginName);
                    ControlsInterface * plugininterface = getControlInterface(kPtr->pluginName);
                    if(plugininterface == (ControlsInterface *) 0) {
                         if(!kPtr->soft)info.append(" : not loaded");
                    } else {
                         info.append(" : loaded");
                    }
                    if((plugininterface != (ControlsInterface *) 0) || (kPtr->soft)) {
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
                            info.append("<br>");
                            info.append("Description: ");
                            if(plugininterface != (ControlsInterface *) 0) plugininterface->pvGetDescription((char*) pv[i].toLatin1().constData(), description);
                            info.append(description);
                            if(plugininterface != (ControlsInterface *) 0) plugininterface->pvGetTimeStamp((char*) pv[i].toLatin1().constData(), timestamp);
                            info.append("<br>");
                            info.append(timestamp);
                        }
                        info.append("<br>Type: ");
                        info.append(caTypeStr[kPtr->edata.fieldtype]);

                        sprintf(asc,"<br>Count: %d", kPtr->edata.valueCount);
                        info.append(asc);

                        info.append("<br>Value: ");
                        switch (kPtr->edata.fieldtype) {
                        case caCHAR:
                            sprintf(asc,"%ld (0x%lx)", kPtr->edata.ivalue, kPtr->edata.ivalue);
                            info.append(asc);
                            break;
                        case caSTRING:
                            if(kPtr->edata.valueCount <= 1) {
                                info.append((char*)kPtr->edata.dataB);
                            } else {
                                QString States((char*) kPtr->edata.dataB);
                                //QStringList list = States.split(";");
                                QStringList list = States.split((QChar)27);
                                for(int j=0; j<list.count(); j++) {
                                    sprintf(asc, "<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;%d %s", j, qPrintable(list.at(j)));
                                    info.append(asc);
                                }
                            }
                            break;

                        case caENUM:{
                            sprintf(asc,"%ld %s", kPtr->edata.ivalue, kPtr->edata.units);
                            info.append(asc);
                            sprintf(asc,"<br>nbStates: %d", kPtr->edata.enumCount);
                            info.append(asc);
                            info.append("<br>States: ");
                            if(kPtr->edata.enumCount > 0) {
                                QString States((char*) kPtr->edata.dataB);
                                //QStringList list = States.split(";");
                                QStringList list = States.split((QChar)27);
                                for(int j=0; j<list.count(); j++) {
                                    sprintf(asc, "<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;%d %s", j, qPrintable(list.at(j)));
                                    info.append(asc);
                                }
                            }
                            break;
                        }
                        case caINT:
                        case caLONG:
                            sprintf(asc,"%ld (0x%lx) %s", kPtr->edata.ivalue, kPtr->edata.ivalue, kPtr->edata.units);
                            info.append(asc);
                            break;
                        case caFLOAT:
                        case caDOUBLE:
                            sprintf(asc,"%lf %s", kPtr->edata.rvalue, kPtr->edata.units);
                            info.append(asc);
                            break;

                        default:
                            sprintf(asc,"unhandled epics type");
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
                            sprintf(asc,"<br>Precision (channel) :%d ", kPtr->edata.precision);
                            Precision = kPtr->edata.precision;
                        } else {
                            sprintf(asc,"<br>Precision (user) :%d ", Precision);
                        }

                        info.append(asc);
                        if(className.contains("Gauge")) {
                            sprintf(asc, "not used for scale") ;
                            info.append(asc);
                        }

                        // limits
                        if(limitsMode) {
                            sprintf(asc,"<br>User alarm: MIN:%g  MAX:%g ", limitsMin, limitsMax);
                            info.append(asc);
                        }
                        if(limitsDefault) {
                            sprintf(asc,"<br>Default limits: MIN:%g  MAX:%g ",limitsMin, limitsMax);
                            info.append(asc);
                        }
                        sprintf(asc,"<br>LOPR:%g  HOPR:%g ", kPtr->edata.lower_disp_limit, kPtr->edata.upper_disp_limit);
                        info.append(asc);
                        sprintf(asc,"<br>LOLO:%g  HIHI:%g ", kPtr->edata.lower_alarm_limit, kPtr->edata.upper_alarm_limit);
                        info.append(asc);
                        sprintf(asc,"<br>LOW :%g  HIGH:%g ", kPtr->edata.lower_warning_limit, kPtr->edata.upper_warning_limit);
                        info.append(asc);
                        sprintf(asc,"<br>DRVL:%g  DRVH:%g ",kPtr->edata.lower_ctrl_limit, kPtr->edata.upper_ctrl_limit);
                        info.append(asc);
                        info.append("<br>");

                        //colormode
                        if(strlen(colMode) > 0) {
                            sprintf(asc, "Colormode: %s", colMode);
                            info.append(asc);
                            info.append("<br>");
                        }

                        // access
                        if(kPtr->edata.accessR==1 && kPtr->edata.accessW==1) sprintf(asc, "Access: ReadWrite");
                        else if(kPtr->edata.accessR==1 && kPtr->edata.accessW==0) sprintf(asc, "Access: ReadOnly");
                        else if(kPtr->edata.accessR==0 && kPtr->edata.accessW==1) sprintf(asc, "Access: WriteOnly"); // not possible
                        else sprintf(asc, "Access: NoAccess");
                        info.append(asc);
                        info.append("<br>");
                    }
                    }
                }
            }
            info.append(InfoPostfix);

            myMessageBox box(this);
            box.setText("<html>" + info + "</html>");
            box.exec();

        // add a file dialog to simplify user path+file input
        } else if(selectedItem->text().contains("File Dialog")) {
            QFileDialog dialog(this);
            dialog.setFileMode(QFileDialog::DirectoryOnly);
            if (dialog.exec()) {
                QStringList fileNames = dialog.selectedFiles();
                if(!fileNames[0].isEmpty()) {
                    if(caTextEntry* textentryWidget = qobject_cast<caTextEntry *>(w)) {
                        caTextEntry::FormatType fType = textentryWidget->getFormatType();
                        TreatRequestedValue(textentryWidget->getPV(), fileNames[0], fType, w);
                    }
                }
            }

        } else if(selectedItem->text().contains("Input Dialog")) {
            bool ok;
            QString text = QInputDialog::getText(this, tr("Input data"), tr("Input:"), QLineEdit::Normal,"", &ok);
            if (ok && !text.isEmpty()) {
                if(caTextEntry* textentryWidget = qobject_cast<caTextEntry *>(w)) {
                   caTextEntry::FormatType fType = textentryWidget->getFormatType();
                   TreatRequestedValue(textentryWidget->getPV(), text, fType, w);
                }
            }

        } else if(selectedItem->text().contains("Print")) {
            print();
        } else if(selectedItem->text().contains("Change Axis")) {
            if(caStripPlot* stripplotWidget = qobject_cast<caStripPlot *>(w)) {
                limitsStripplotDialog dialog(stripplotWidget, mutexKnobDataP, "stripplot modifications", this);
                dialog.exec();
            } else if(caCartesianPlot* cartesianplotWidget = qobject_cast<caCartesianPlot *>(w)) {
                limitsCartesianplotDialog dialog(cartesianplotWidget, mutexKnobDataP, "cartesianplot modifications", this);
                dialog.exec();
            }

        } else if(selectedItem->text().contains("Reset zoom")) {
            if(caCartesianPlot* cartesianplotWidget = qobject_cast<caCartesianPlot *>(w)) {
                cartesianplotWidget->resetZoom();

                /*************************************************/
                // oops, case of axis defines by channel forgotten
                if(cartesianplotWidget->getXscaling() == caCartesianPlot::Channel) {

                    QString pvs =cartesianplotWidget->getPV(0);
                    QStringList vars = pvs.split(";");
                    if((vars.size()== 2) || (vars.at(1).trimmed().length() > 0)) {
                        knobData *kPtr =  mutexKnobDataP->getMutexKnobDataPV(w, vars.at(0).trimmed());
                        if(kPtr != (knobData *) 0) {
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

        } else if(selectedItem->text().contains("Change Increment/Value")) {
            if(caSlider* sliderWidget = qobject_cast<caSlider *>(w)) {
                sliderDialog dialog(sliderWidget, mutexKnobDataP, "slider Increment/Value change", this);
                dialog.exec();
            }
        } else if(selectedItem->text().contains("Change Limits/Precision")) {
            limitsDialog dialog(w, mutexKnobDataP, "Limits/Precision change", this);
            dialog.exec();
        } else {
            // any action from environment ?
            if(validExecListItems) {
                for(int i=0; i<execListItems.count(); i++) {
                    QStringList item = execListItems[i].split(";");
                    if(item.count() > 1) {
                        if(selectedItem->text().contains(item[0])) {
                            QString command = item[1];
                            command.replace("&P", pv[0]);
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
    DisplayContextMenu(qobject_cast<QWidget *>(sender()));
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
            int pos = trimmedPV.indexOf("{");  // jason string
            if(pos != -1) JSONString = trimmedPV.mid(pos);
            if(pos != -1) trimmedPV = trimmedPV.mid(0, pos);
            strng[i] = treatMacro(map, trimmedPV, &doNothing);
            if(i==4) {
                char asc[256];
                QString pv = calcWidget->getVariable();
                calcWidget->setVariable(strng[i]);  // update variable name when macro used
                if(pos != -1) {
                    int status = parseForDisplayRate(JSONString, rate);
                    if(!status) {
                        sprintf(asc, "JSON parsing error on %s ,should be like {\"monitor\":{\"maxdisplayrate\":10}}",
                                (char*) pv.trimmed().toLatin1().constData());
                    } else {
                        sprintf(asc, "pv %s display rate set to maximum %dHz", trimmedPV.toLatin1().constData(), rate);
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
        qDebug() << "widget has not been defined for visibility";
        return 0;
    }

    /* add monitors for this if any */
    for(int i=0; i<4; i++) {
        if((num = addMonitor(myWidget, kData, strng[i], widget, specData, map, &pv)) >= 0) {
            if (caCalc *calcWidget = qobject_cast<caCalc *>(widget)) {
                replaceVisibilityChannels(calcWidget);
            } else if (caImage *imageWidget = qobject_cast<caImage *>(widget)) {
                replaceVisibilityChannels(imageWidget);
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
    if(nbMon> 0) widget->setToolTip(tooltip);

    // replace macros for imagecalc
    if (caImage *imageWidget = qobject_cast<caImage *>(widget)) {
        text =  treatMacro(map, imageWidget->getImageCalc(), &doNothing);
        imageWidget->setImageCalc(text);
    }

    /* replace also some macro values in the visibility calc string */
    text =  treatMacro(map, visibilityCalc, &doNothing);

    monitorList.insert(0, nbMon);
    indexList.insert(0,nbMon);

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
            caMode = caNumeric::Channel;
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
                prec = data.edata.precision;
                if(prec < 0) prec = 0;
                if(prec > 4) prec = 4;
                maxAbsHoprLopr= qMax(fabs(maxValue), fabs(minValue));
                if(maxAbsHoprLopr > 1.0) {
                    width = (int)log10(maxAbsHoprLopr) + 2 + prec;
                } else {
                    width = 2 + prec;
                }

                if (caApplyNumeric *applynumericWidget = qobject_cast<caApplyNumeric *>(widget)) {
                    //printf("%d %d\n", width-prec-1, prec);
                    applynumericWidget->setIntDigits(width-prec-1);
                    applynumericWidget->setDecDigits(prec);
                } else if (caNumeric *numericWidget = qobject_cast<caNumeric *>(widget)) {
                    //printf("%d %d\n", width-prec-1, prec);
                    numericWidget->setIntDigits(width-prec-1);
                    numericWidget->setDecDigits(prec);
                } else if (caSpinbox *spinboxWidget = qobject_cast<caSpinbox *>(widget)) {
                    //printf("%d %d\n", width-prec-1, prec);
                    spinboxWidget->setIntDigits(width-prec-1);
                    spinboxWidget->setDecDigits(prec);
                }

            } else {
                maxAbsHoprLopr= qMax(fabs(maxValue), fabs(minValue));

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
    if(messageWindowP == (MessageWindow *) 0) return;
    messageWindowP->postMsgEvent(type, msg);
}

/**
  * execute an application on linux
  */
#ifdef linux
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

void CaQtDM_Lib::TreatOrdinaryValue(QString pv, double value, int32_t idata,  QWidget *w)
{
    char errmess[255];
    int indx;

    //qDebug() << "treatordinary value" << pv << w;
    knobData *kPtr = mutexKnobDataP->getMutexKnobDataPV(w, pv);
    if(kPtr != (knobData *) 0) {
        //qDebug() << "try to find out if this is a soft pv" << pv;
        // when softpv treat it and get out
        if(mutexKnobDataP->getSoftPV(pv, &indx, (QWidget*) kPtr->thisW)) {
            if(kPtr->soft) {
                //qDebug() << "write softpv";
                kPtr = mutexKnobDataP->GetMutexKnobDataPtr(indx);  // use pointer
                kPtr->edata.rvalue = value;
                // set value also into widget, will be overwritten when driven from other channels
                caCalc * ww = (caCalc*) kPtr->dispW;
                ww->setValue(value);
                return;
            };
        }
    } else {
        qDebug() << "internal error; return while pv not found";
        return;
    }

    QString text(" ");
    QStringsToChars(pv, text, w->objectName().toLower());
    ControlsInterface * plugininterface = getControlInterface(kPtr->pluginName);
    if(plugininterface != (ControlsInterface *) 0) plugininterface->pvSetValue(param1, value, idata, param2, param3, errmess, 0);
}

/**
  * this routine will get a value from a string with hex and octal representation
  */
long CaQtDM_Lib::getValueFromString(char *textValue, formatsType fType, char **end)
{
    if(fType == octal) {
        return strtoul(textValue, end, 8);
    } else if(fType == hexadecimal) {
        return strtoul(textValue, end, 16);
    } else {
        if((strlen(textValue) > (size_t) 2) && (textValue[0] == '0') && (textValue[1] == 'x' || textValue[1] == 'X')) {
            return strtoul(textValue, end, 16);
        } else {
            return strtol(textValue, end, 10);
        }
    }
}

/**
  * this routine will treat the string, command, value to write to the pv
  */
void CaQtDM_Lib::TreatRequestedValue(QString pv, QString text, caTextEntry::FormatType fType, QWidget *w)
{
    char errmess[255];
    double value;
    long longValue;
    char *end = NULL, textValue[255];
    bool match;
    int indx;
    ControlsInterface * plugininterface = (ControlsInterface *) 0;

    formatsType fTypeNew;

    if(fType == caTextEntry::octal) fTypeNew = octal;
    else if(fType == caTextEntry::hexadecimal) fTypeNew = hexadecimal;
    else if(fType == caTextEntry::string) fTypeNew = string;
    else fTypeNew = decimal;

    knobData *kPtr = mutexKnobDataP->getMutexKnobDataPV(w, pv);
    if(kPtr == (knobData *) 0) return;
    knobData *auxPtr = kPtr;

    // when softpv get index to where it is defined
    if(mutexKnobDataP->getSoftPV(kPtr->pv, &indx, (QWidget*) kPtr->thisW)) {
        kPtr = mutexKnobDataP->GetMutexKnobDataPtr(indx);  // use pointer
        if(kPtr == (knobData *) 0) return;
    } else {
        plugininterface = (ControlsInterface *) w->property("Interface").value<void *>();
        if(plugininterface == (ControlsInterface *) 0) return;
    }

    if(!kPtr->soft) {
        if(plugininterface == (ControlsInterface *) 0) return;
    }

    //qDebug() << "fieldtype:" << kPtr->edata.fieldtype;
    switch (kPtr->edata.fieldtype) {
    case caSTRING:
        //qDebug() << "set string" << text;
        plugininterface->pvSetValue(kPtr->pv, 0.0, 0, (char*) text.toLatin1().constData(), (char*) w->objectName().toLower().toLatin1().constData(), errmess, 0);
        break;

    case caENUM:
    case caINT:
    case caLONG:
        strcpy(textValue, text.toLatin1().constData());
        // Check for an enum text
        match = false;
        if(kPtr->edata.dataB != (void*)0 && kPtr->edata.enumCount > 0) {
            QString strng((char*) kPtr->edata.dataB);
            //QStringList list = strng.split(";", QString::SkipEmptyParts);
            QStringList list = strng.split((QChar)27, QString::SkipEmptyParts);
            for (int i=0; i<list.size(); i++) {
                if(!text.compare(list.at(i).trimmed())) {
                    //qDebug() << "set enum text" << textValue;
                    plugininterface->pvSetValue((char*) kPtr->pv, 0.0, 0, textValue, (char*) w->objectName().toLower().toLatin1().constData(), errmess, 0);
                    match = true;
                    break;
                }
            }
        }

        if(!match) {
            //qDebug() << "assume it is a number";
            // Assume it is a number
            longValue = getValueFromString(textValue, fTypeNew, &end);

            // number must be between the enum possibilities
            if(kPtr->edata.fieldtype == caENUM) {
                if(*end == 0 && end != textValue && longValue >= 0 && longValue <= kPtr->edata.enumCount) {
                    //qDebug() << "decode value *end=0, set a longvalue to enum" << longValue;
                    plugininterface->pvSetValue((char*) kPtr->pv, 0.0, (int32_t) longValue, textValue, (char*) w->objectName().toLower().toLatin1().constData(), errmess, 0);
                } else {
                    char asc[100];
                    sprintf(asc, "Invalid value: pv=%s value= \"%s\"\n", kPtr->pv, textValue);
                    postMessage(QtDebugMsg, asc);
                    if(caTextEntry* widget = qobject_cast<caTextEntry *>((QWidget*) auxPtr->dispW)) {
                        Q_UNUSED(widget);
                    }
                }
                // normal int or long
            } else
                //qDebug() << "set normal longvalue" << longValue;
                plugininterface->pvSetValue((char*) kPtr->pv, 0.0, (int32_t) longValue, textValue, (char*) w->objectName().toLower().toLatin1().constData(), errmess, 0);
        }

        break;

    case caCHAR:
        if(fType == caTextEntry::string) {
            if(kPtr->edata.nelm > 1) {
               //qDebug() << "set string" << text;
               plugininterface->pvSetValue((char*) kPtr->pv, 0.0, 0, (char*) text.toLatin1().constData(), (char*) w->objectName().toLower().toLatin1().constData(), errmess, 0);
            } else {  // single char written through its ascii code while character entered
               QChar c = text.at(0);
               plugininterface->pvSetValue((char*) kPtr->pv, 0.0, (int)c.toLatin1(), (char*)  "", (char*) w->objectName().toLower().toLatin1().constData(), errmess, 2);
            }
            break;
        }
        //qDebug() << "fall through default case";

    default:
        match = false;
        //qDebug() << "assume it is a double";
        strcpy(textValue, text.toLatin1().constData());
        value = (double) getValueFromString(textValue, fTypeNew, &end);
        if(*end == '\0' && end != textValue) {        // decoded as long
            match = true;
        } else {
            value = strtod(textValue, &end);  // decoded as double
            if(*end == '\0' && end != textValue) {
                match = true;
            }
        }
        if(match) {
            //qDebug() << "decoded as double, and set as double" << value;
            if(kPtr->soft) {
                kPtr->edata.rvalue = value;
                // set value also into widget, will be overwritten when driven from other channels
                caCalc * ww = (caCalc*) kPtr->dispW;
                ww->setValue(value);
            } else {
                plugininterface->pvSetValue((char*) kPtr->pv, value, 0, textValue, (char*) w->objectName().toLower().toLatin1().constData(), errmess, 1);
            }

        } else {
            char asc[100];
            sprintf(asc, "Invalid value: pv=%s value= \"%s\"\n", kPtr->pv, textValue);
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
void CaQtDM_Lib::TreatRequestedWave(QString pv, QString text, caWaveTable::FormatType fType, int index, QWidget *w)
{
    char    errmess[255], sdata[40];
    int32_t data32[1];
    int16_t data16[1];
    float   fdata[1];
    double  value, ddata[1];
    long    longValue;
    char    *end = NULL, textValue[255];
    bool    match;

    ControlsInterface * plugininterface = (ControlsInterface *) w->property("Interface").value<void *>();
    if(plugininterface == (ControlsInterface *) 0) return;

    formatsType fTypeNew;

    if(fType == caWaveTable::octal) fTypeNew = octal;
    else if(fType == caWaveTable::hexadecimal) fTypeNew = hexadecimal;
    else if(fType == caWaveTable::string) fTypeNew = string;
    else fTypeNew = decimal;

    //qDebug() << "treat requested wave";
    knobData *kPtr = mutexKnobDataP->getMutexKnobDataPV(w, pv);
    if(kPtr == (knobData *) 0)return;

    QMutex *datamutex;
    datamutex = (QMutex*) kPtr->mutex;
    datamutex->lock();

    //qDebug() << "fieldtype:" << kPtr->edata.fieldtype;
    switch (kPtr->edata.fieldtype) {

    case caINT:
    case caLONG:
    case caCHAR:
        strcpy(textValue, text.toLatin1().constData());
        longValue = getValueFromString(textValue, fTypeNew, &end);

        if(kPtr->edata.fieldtype == caLONG) {
            int32_t* P = (int32_t*) kPtr->edata.dataB;
            P[index] = (int32_t) longValue;
            plugininterface->pvSetWave((char*) kPtr->pv, fdata, ddata, data16, P, sdata, kPtr->edata.valueCount,
                                 (char*) w->objectName().toLower().toLatin1().constData(), errmess);
        } else if(kPtr->edata.fieldtype == caINT) {
            int16_t* P = (int16_t*) kPtr->edata.dataB;
            P[index] = (int16_t) longValue;
            plugininterface->pvSetWave((char*) kPtr->pv, fdata, ddata, P, data32, sdata, kPtr->edata.valueCount,
                                 (char*) w->objectName().toLower().toLatin1().constData(), errmess);
        } else {
            if(fTypeNew == string) {
                char* P = (char*) kPtr->edata.dataB;
                P[index] = textValue[0];
                plugininterface->pvSetWave((char*) kPtr->pv, fdata, ddata, data16, data32, P, kPtr->edata.valueCount,
                                     (char*) w->objectName().toLower().toLatin1().constData(), errmess);
            } else {
                char* P = (char*) kPtr->edata.dataB;
                P[index] = (char) ((int) longValue);
                plugininterface->pvSetWave((char*) kPtr->pv, fdata, ddata, data16, data32, P, kPtr->edata.valueCount,
                                     (char*) w->objectName().toLower().toLatin1().constData(), errmess);
            }
        }

        break;

    case caFLOAT:
    case caDOUBLE:
        match = false;
        // Treat as a double
        strcpy(textValue, text.toLatin1().constData());
        value = (double) getValueFromString(textValue, fTypeNew, &end);
        if(*end == '\0' && end != textValue) {        // decoded as long
            match = true;
        } else {
            value = strtod(textValue, &end);
            if(*end == '\0' && end != textValue) {   // decoded as double
                match = true;
            }
        }
        if(match) {
            if(kPtr->edata.fieldtype == caFLOAT) {
                float* P = (float*) kPtr->edata.dataB;
                P[index] = (float) value;
                plugininterface->pvSetWave((char*) kPtr->pv, P, ddata, data16, data32, sdata, kPtr->edata.valueCount,
                                     (char*) w->objectName().toLower().toLatin1().constData(), errmess);
            } else  {
                double* P = (double*) kPtr->edata.dataB;
                P[index] = value;
                plugininterface->pvSetWave((char*) kPtr->pv, fdata, P, data16, data32, sdata, kPtr->edata.valueCount,
                                     (char*) w->objectName().toLower().toLatin1().constData(), errmess);
            }
        } else {
            char asc[100];
            sprintf(asc, "Invalid value: pv=%s value= \"%s\"\n", kPtr->pv, textValue);
            postMessage(QtDebugMsg, asc);
        }
        break;

    default:
        char asc[100];
        sprintf(asc, "unhandled epics type (%d) in treatRequestedWave\n", kPtr->edata.fieldtype);
        postMessage(QtDebugMsg, asc);
    }

    datamutex->unlock();
}

int CaQtDM_Lib::parseForDisplayRate(QString inputc, int &rate)
{
    // Parse data
    char input[MAXPVLEN];
    int cpylen = qMin(inputc.length(), MAXPVLEN-1);
    strncpy(input, (char*) inputc.toLatin1().constData(), (size_t) cpylen);
    input[cpylen] = '\0';

    JSONValue *value = JSON::Parse(input);

    // Did it go wrong?
    if (value == NULL) {
        //printf("failed to parse <%s>\n", input);
        return false;
    } else {
        // Retrieve the main object
        JSONObject root;
        if(!value->IsObject()) {
            //printf("The root element is not an object");
            delete(value);
            return false;
        } else {

            root = value->AsObject();
            // check for monitor
            if (root.find(L"monitor") != root.end() && root[L"monitor"]->IsObject()) {

                //printf("monitor detected\n");
                // Retrieve nested object
                JSONValue *value1 = JSON::Parse(root[L"monitor"]->Stringify().c_str());
                // Did it go wrong?
                if ((value1 != NULL) && value1->IsObject()) {
                    JSONObject root;
                    root = value1->AsObject();
                    if (root.find(L"maxdisplayrate") != root.end() && root[L"maxdisplayrate"]->IsNumber()) {
                        int status;
                        //printf("maxdisplayrate detected\n");
                        status = swscanf(root[L"maxdisplayrate"]->Stringify().c_str(), L"%d", &rate);
                        if(status != 1) return false;
                        //printf("%d decode value=%d\n", status, rate);
                        delete(value1);
                        delete(value);
                        return true;
                    } else {
                        delete(value1);
                        delete(value);
                        return false;
                    }
                } else {
                    delete(value);
                    return false;
                }
            }
        }
    }
    return false;
}

void CaQtDM_Lib::allowResizing(bool allowresize)
{
    allowResize = allowresize;
}

void CaQtDM_Lib::resizeSpecials(QString className, QWidget *widget, QVariantList list, double factX, double factY)
{
    // for horizontal or vertical line we still have to set the linewidth
    if(!className.compare("QFrame")) {
        double linewidth;
        QFrame * line = (QFrame *) widget;
        if(line->frameShape() == QFrame::HLine || line->frameShape() == QFrame::VLine) {
            if(line->frameShape() != QFrame::HLine) {
                linewidth = (double) list.at(4).toInt() * factY;
            } else {
                linewidth = (double) list.at(4).toInt() * factX;
            }
            int width = qRound(linewidth);
            if(width < 1) width = 1;
            line->setLineWidth(width);
        }
    }

    else if(!className.compare("caTable")) {
        caTable *table = (caTable *) widget;
        QFont f = table->font();
        qreal fontSize = qMin(factX, factY) * (double) list.at(4).toInt();
        if(fontSize < MIN_FONT_SIZE) fontSize = MIN_FONT_SIZE;
        f.setPointSizeF(fontSize);
        table->setUpdatesEnabled(false);
        for(int i = 0; i < table->rowCount(); ++i) {
            for(int j = 0; j < table->columnCount(); ++j) {
                QTableWidgetItem* selectedItem = table->item(i, j);
                if(selectedItem != (QTableWidgetItem*) 0) selectedItem->setFont(f);
            }
        }
        table->setValueFont(f);
        table->verticalHeader()->setDefaultSectionSize((int) (qMin(factX, factY)*20));
        table->setUpdatesEnabled(true);
    }

    else if(!className.compare("caWaveTable")) {
        caWaveTable *table = (caWaveTable *) widget;
        QFont f = table->font();
        qreal fontSize = qMin(factX, factY) * (double) list.at(4).toInt();
        if(fontSize < MIN_FONT_SIZE) fontSize = MIN_FONT_SIZE;
        f.setPointSizeF(fontSize);
        table->setUpdatesEnabled(false);
        for(int i = 0; i < table->rowCount(); ++i) {
            for(int j = 0; j < table->columnCount(); ++j) {
                QTableWidgetItem* selectedItem = table->item(i, j);
                if(selectedItem != (QTableWidgetItem*) 0) selectedItem->setFont(f);
            }
        }
        table->setValueFont(f);
        table->verticalHeader()->setDefaultSectionSize((int) (qMin(factX, factY)*20));
        table->setUpdatesEnabled(true);
    }

    else if(!className.compare("QLabel")) {
        QLabel *label = (QLabel *) widget;
        className = label->parent()->metaObject()->className();
        if(!className.contains("Numeric") ) {  // would otherwise interfere with our wheelswitch
            if(list.at(4).toInt() < 0) return; // on android I got -1 for these fonts at initialization, i.e pixelsize
            qreal fontSize = qMin(factX, factY) * (double) list.at(4).toInt();
            if(fontSize < MIN_FONT_SIZE) fontSize = MIN_FONT_SIZE;
            QFont f = label->font();
            f.setPointSizeF(fontSize);
            label->setFont(f);
        }
    }

    else if((!className.compare("caMenu")) ||
            (!className.compare("QPlainTextEdit")) ||
            (!className.compare("QTextEdit")) ||
            (!className.compare("QLineEdit")) ) {
        if(list.at(4).toInt() < 0) return; // on android I got -1 for these fonts at initialization, i.e pixelsize
        qreal fontSize = qMin(factX, factY) * (double) list.at(4).toInt();
        if(fontSize < MIN_FONT_SIZE) fontSize = MIN_FONT_SIZE;
        QFont f = widget->font();
        f.setPointSizeF(fontSize);
        widget->setFont(f);
    }

    else if(!className.compare("caStripPlot") || !className.compare("caCartesianPlot")) {
        QwtPlot *plot = (QwtPlot *) widget;

        // change font of axis ticks
        qreal fontSize = qMin(factX, factY) * (double) list.at(4).toInt();
        if(fontSize < MIN_FONT_SIZE) fontSize = MIN_FONT_SIZE;
        QFont f = plot->axisFont(QwtPlot::xBottom);
        f.setPointSizeF(fontSize);
        plot->setAxisFont(QwtPlot::xBottom, f);
        plot->setAxisFont(QwtPlot::yLeft, f);

        // change font of labels and title
        f = plot->title().font();
        QwtText title = plot->title().text();
        QwtText titleX = plot->axisTitle(QwtPlot::xBottom).text();
        QwtText titleY = plot->axisTitle(QwtPlot::yLeft).text();
        fontSize = qMin(factX, factY) * (double) list.at(6).toInt();
        if(fontSize < MIN_FONT_SIZE) fontSize = MIN_FONT_SIZE;
        f.setPointSizeF(fontSize);
        title.setFont(f);
        titleX.setFont(f);
        titleY.setFont(f);
        plot->setTitle(title);
        plot->setAxisTitle(QwtPlot::xBottom, titleX);
        plot->setAxisTitle(QwtPlot::yLeft, titleY);

        // font size of legends with qwt 6.0
        if(!className.compare("caStripPlot")) {
            caStripPlot * stripplotWidget = (caStripPlot *) widget;
            fontSize = qMin(factX, factY) * (double) list.at(7).toInt();
            if(fontSize < MIN_FONT_SIZE) fontSize = MIN_FONT_SIZE;
            f.setPointSizeF(fontSize);
            if(stripplotWidget->getLegendEnabled()) {
                stripplotWidget->setLegendAttribute(stripplotWidget->getScaleColor(), f, caStripPlot::FONT);
            }
        }
        // resize ticks (will not do for the timescale of the castripplot, while new every time)
        plot->axisScaleDraw(QwtPlot::xBottom)->setTickLength(QwtScaleDiv::MajorTick, factY * (double) list.at(8).toInt());
        plot->axisScaleDraw(QwtPlot::xBottom)->setTickLength(QwtScaleDiv::MediumTick, factY * (double) list.at(9).toInt());
        plot->axisScaleDraw(QwtPlot::xBottom)->setTickLength(QwtScaleDiv::MinorTick, factY * (double) list.at(10).toInt());
        plot->axisScaleDraw(QwtPlot::xBottom)->setSpacing(0.0);
        plot->axisScaleDraw(QwtPlot::yLeft)->setTickLength(QwtScaleDiv::MajorTick, factX * (double) list.at(8).toInt());
        plot->axisScaleDraw(QwtPlot::yLeft)->setTickLength(QwtScaleDiv::MediumTick, factX * (double) list.at(9).toInt());
        plot->axisScaleDraw(QwtPlot::yLeft)->setTickLength(QwtScaleDiv::MinorTick, factX * (double) list.at(10).toInt());
        plot->axisScaleDraw(QwtPlot::xBottom)->setSpacing(0.0);
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

    QMainWindow *main = this->findChild<QMainWindow *>();
    // it seems that when mainwindow was fixed by user, then the window stays empty ?
    if(main != (QObject*) 0) {
        main->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    if(!allowResize) return;
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

            // for a horizontal or vertical line get the linewidth
            if(!className.compare("QFrame")) {
                QFrame * line = (QFrame *) widget;
                if(line->frameShape() == QFrame::HLine || line->frameShape() == QFrame::VLine) {
                    integerList.insert(4, line->lineWidth());
                }
                // for plots get the linewidth
            } else if(!className.compare("caStripPlot") || !className.compare("caCartesianPlot")) {
                QwtPlot * plot = (QwtPlot *) widget;
                integerList.insert(4, plot->axisFont(QwtPlot::xBottom).pointSize());         // label of ticks
                integerList.insert(5, 0);                                                    // empty
                integerList.insert(6, plot->axisTitle(QwtPlot::xBottom).font().pointSize()); // titles have the same font

                if(!className.compare("caStripPlot")) {
                    caStripPlot * stripplotWidget = (caStripPlot *) widget;
                    integerList.insert(7, 9);
                    if(stripplotWidget->getLegendEnabled()) {
                        stripplotWidget->setLegendAttribute(stripplotWidget->getScaleColor(), QFont("arial", 9), caStripPlot::FONT);
                    }
                } else {
                    integerList.insert(7, 9);
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
            }
            widget->setProperty("GeometryList", integerList);
        }
        return;
    }

    factX = (double) event->size().width() / (double) origWidth;
    factY = (double) event->size().height() / (double) origHeight;

    QString classNam;

    bool mainlayoutPresent = false;

    if(main == (QObject*) 0) {
        QDialog *dialog = this->findChild<QDialog *>();
        if(dialog == (QObject*) 0) return;  // if not a mainwindow or dialog get out
        if(dialog->layout() != (QObject*) 0) {
            classNam = dialog->layout()->metaObject()->className();
            mainlayoutPresent = true;
        }
    } else {
        if( main->centralWidget()->layout() != (QObject*) 0) {
            classNam = main->centralWidget()->layout()->metaObject()->className();
            mainlayoutPresent = true;
        }
    }

    //qDebug() << className << mainlayoutPresent;

    // if our window is not using a layout, then we we have to do the resizing ourselves
    if(!mainlayoutPresent) {
        //qDebug() << "no main layout present, we should do the work";

        // if our window is using a layout, then Qt has to do the resizing
    } else {
        //qDebug() << "main layout present, Qt should do the work" << className;

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

        if(     !className.contains("QMainWindow")  &&
                !className.contains("QRubberBand")  &&
                !className.contains("Qwt")    &&
                !className.contains("QWidget")    &&
                (className.contains("ca") || className.contains("Q") || className.contains("Line"))
                ) {
            QWidget *w = (QWidget*) widget->parent();
            // if this widget is managed by a layout, do not do anything
            // parent is a layout and must be resized and repositioned

            if((w->layout() != (QObject*) 0) && (w->objectName().contains("layoutWidget"))) {
                QVariant var=w->property("GeometryList");
                QVariantList list = var.toList();
                double x = (double) list.at(0).toInt() * factX;
                double y = (double) list.at(1).toInt() * factY;
                double width = (double) list.at(2).toInt() *factX;
                double height = (double) list.at(3).toInt() *factY;
                QRect rectnew = QRect(qRound(x), qRound(y), qRound(width), qRound(height));
                w->setGeometry(rectnew);
                w->updateGeometry();

                // not a layout, widget has to be resized and repositioned
            } else {
                QVariant var=widget->property("GeometryList");
                QVariantList list = var.toList();

                if(list.size() >= 4) {
                    double x = (double) list.at(0).toInt() * factX;
                    double y = (double) list.at(1).toInt() * factY;
                    double width = (double) list.at(2).toInt() * factX;
                    double height = (double) list.at(3).toInt() * factY;
                    if(width < 1.0) width=1.0;
                    if(height < 1.0) height = 1.0;
                    QRect rectnew = QRect(qRound(x), qRound(y), qRound(width), qRound(height));

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
                    widget->setGeometry(rectnew);
                    resizeSpecials(className, widget, list, factX, factY);
                    widget->updateGeometry();
                }
            }

        }
    }
}

void CaQtDM_Lib::Callback_WriteDetectedValues(QWidget* child)
{
    int x,y,w,h,count=4;
    double values[4];

    QStringList thisString;
    QWidget *widget = (QWidget *) 0;
    QPointF P1, P2;

    // must fit the definitions in cacamera and cascan2d
    enum ROI_type {none=0, xy_only, xy1_xy2, xyUpleft_xyLowright, xycenter_width_height};
    enum ROI_markertype {box=0, line, arrow};

    ROI_type roiType;

    if (caCamera *cameraWidget = qobject_cast<caCamera *>(child)) {
        roiType = (ROI_type) cameraWidget->getROIwriteType();
        if(!cameraWidget->getAccessW()) return;
        widget = (QWidget*) cameraWidget;
        cameraWidget->getROI(P1, P2);
        thisString = cameraWidget->getROIChannelsWrite().split(";");

    } else if (caScan2D *scan2dWidget = qobject_cast<caScan2D *>(child)) {
        roiType = (ROI_type) scan2dWidget->getROIwriteType();
        if(!scan2dWidget->getAccessW()) return;
        widget = (QWidget*) scan2dWidget;
        scan2dWidget->getROI(P1, P2);
        thisString = scan2dWidget->getROIChannelsWrite().split(";");
    } else {
        return;
    }

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
        if((P2.x() < P1.x() ) || (P2.y() < P1.y())) {
            values[0] = P2.x();
            values[1] = P2.y();
            values[2] = P1.x();
            values[3] = P1.y();
        } else {
            values[0] = P1.x();
            values[1] = P1.y();
            values[2] = P2.x();
            values[3] = P2.y();
        }
        break;
    case xycenter_width_height:
    {
        int ROIx = x = P1.x();
        int ROIy = y = P1.y();
        int ROIw = w = P2.x() - P1.x();
        int ROIh = h = P2.y() - P1.y();
        if(ROIw < 0) { x = ROIx + ROIw; w = -ROIw;}
        if(ROIh < 0) { y = ROIy + ROIh; h = -ROIh;}
        values[0] = x+qRound(w/2.0);
        values[1] = y+qRound(h/2.0);
        values[2]=w;
        values[3]=h;
    }
        break;

    default:
        return;
    }

    for(int i=0; i<count; i++) {
        int32_t idata = (int32_t) values[i];
        double rdata = (double) values[i];
        if(thisString.at(i).trimmed().length() > 0) {
            TreatOrdinaryValue(thisString.at(i), rdata,  idata, widget);
        }
    }
}

// initiate drag, one will be able to drop to another Qt-application
void CaQtDM_Lib::mousePressEvent(QMouseEvent *event)
{
    if((event->button() == Qt::LeftButton) ||  (event->button() == Qt::RightButton)) {
        return;
    }
    QWidget *w = static_cast<QWidget*>(childAt(event->pos()));
    if (!w) return;

    QDrag *drag = new QDrag(this);
    QPoint hotSpot = event->pos() - w->pos();
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-hotspot", QByteArray::number(hotSpot.x()) + " " + QByteArray::number(hotSpot.y()));

    if(caCalc *calcWidget = qobject_cast<caCalc *>(w)) {
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
        //qDebug() << "unrecognized widget" << w;
        return;
    }

    // put it also to the clipboard (usefull for microsoft windows)
    const QString text = mimeData->text().toLatin1().constData();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text, QClipboard::Clipboard);

    // build a pixmap from pv text
    QFont f = font();
    QFontMetrics metrics(f);
    int width = metrics.width(mimeData->text() + 20);
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

    Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
    if (dropAction == Qt::MoveAction)w->close();
}


