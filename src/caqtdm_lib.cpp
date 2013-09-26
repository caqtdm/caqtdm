//******************************************************************************
// Copyright (c) 2012 Paul Scherrer Institut PSI), Villigen, Switzerland
// Disclaimer: neither  PSI, nor any of their employees makes any warranty
// or assumes any legal liability or responsibility for the use of this software
//******************************************************************************
//******************************************************************************
//
//     Author : Anton Chr. Mezger
//
//******************************************************************************

#include "caQtDM_Lib_global.h"
#include "caqtdm_lib.h"
#include "dmsearchfile.h"
#include "parsepepfile.h"
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <sys/timeb.h>
#include <postfix.h>
#include <QObject>
#include <iostream>
#ifdef linux
#  include <sys/wait.h>
#  include <unistd.h>
#endif

#define PRC 1

#include "myMessageBox.h"

#include "alarmdefs.h"

#define PRINT(x)
#define min(x,y)   (((x) < (y)) ? (x) : (y))

#define ToolTipPrefix "<p style='background-color:yellow'><font color='#000000'>"
#define ToolTipPostfix "</font></font></p>"

#define InfoPrefix "<p style='background-color:lightyellow'><font color='#000000'>"
#define InfoPostfix "</font></font></p>"

// used for calculating visibility for several types of widgets
#define ComputeVisibility(x)  {  \
    switch(widget->getVisibility()) { \
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
    visible = CalcVisibility(widget, result, valid);\
    break;\
    }}

// used for preparing the vectors of the cartesian plot
#define BuildVector(x) {\
    switch(data.edata.fieldtype) { \
    case caFLOAT: { \
    float* P = (float*) data.edata.dataB; \
    for(int i=0; i< data.edata.valueCount; i++) y.append(P[i]); \
    } \
    break; \
    case caDOUBLE: { \
    double* P = (double*) data.edata.dataB; \
    for(int i=0; i< data.edata.valueCount; i++) y.append(P[i]); \
    } \
    break; \
    case caLONG: { \
    int32_t* P = (int32_t*) data.edata.dataB; \
    for(int i=0; i< data.edata.valueCount; i++) y.append(P[i]);\
    } \
    break; \
    case caINT: { \
    int16_t* P = (int16_t*) data.edata.dataB; \
    for(int i=0; i< data.edata.valueCount; i++) y.append(P[i]); \
    } \
    break; \
    case caENUM: { \
    int16_t* P = ( int16_t*) data.edata.dataB; \
    for(int i=0; i< data.edata.valueCount; i++) y.append(P[i]); \
    } \
    break; \
    }}

// used for interfacing epics routines with (pv, text, ...)
#define QStringsToChars(x,y,z) \
    char param1[MAXPVLEN], param2[255], param3[80]; \
    int size1, size2, size3; \
    QByteArray Parameter_1 = x.toAscii().constData(); \
    QByteArray Parameter_2 = y.toAscii().constData(); \
    QByteArray Parameter_3 = z.toAscii().constData(); \
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
#define SetColorsBack                            \
    if(!widget->property("Connect").value<bool>()) { \
    widget->setNormalColors();                   \
    widget->setProperty("Connect", true);        \
    }

// colors back after no connect
#define SetColorsNotConnected                    \
    widget->setAlarmColors(NOTCONNECTED);        \
    widget->setProperty("Connect", false);

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

//===============================================================================================

Q_DECLARE_METATYPE(QList<int>)

extern "C" int CreateAndConnect(int index, knobData *data, int rate, int skip);
extern "C" void ClearMonitor(knobData *kData);
extern "C" void PrepareDeviceIO();
extern "C" int EpicsSetValue(char *pv, float rdata, long idata, char *sdata, char *object, char *errmess, int forceType);
extern "C" void TerminateDeviceIO();

MutexKnobData *mutexKnobData;
MessageWindow *messageWindow;

class Sleep
{
public:
    static void msleep(unsigned long msecs)
    {
        QMutex mutex;
        mutex.lock();
        QWaitCondition waitCondition;
        waitCondition.wait(&mutex, msecs);
        mutex.unlock();
    }
};

/**
 * CaQtDM_Lib destructor
 */
CaQtDM_Lib::~CaQtDM_Lib()
{
    //qDebug() << "nb elements:" << includeWidgetList.count();
    for (int i = includeWidgetList.count()-1; i >= 0; --i)
    {
        QWidget *widget;
        widget= includeWidgetList.at(i);
        //qDebug() << "delete" << widget;
        delete widget;
    }
    delete myWidget;
    includeWidgetList.clear();
}

/**
 * CaQtDM_Lib constructor
 */
CaQtDM_Lib::CaQtDM_Lib(QWidget *parent, QString filename, QString macro, MutexKnobData *mKnobData, MessageWindow *msgWindow, bool pepprint) : QMainWindow(parent)
{

    mutexKnobData = mKnobData;
    messageWindow = msgWindow;
    pepPrint = pepprint;

    firstResize = true;

#ifdef epics4
    qDebug() << "epics4 init";
    Epics4 = new epics4Subs(mKnobData);
#endif

    //qDebug() << "open file" << filename << "with macro" << macro;
    setAttribute(Qt::WA_DeleteOnClose);

    QUiLoader loader;

    // define a layout
    QGridLayout *layout = new QGridLayout;

    // define the file to use
    QFile *file = new QFile;
    file->setFileName(filename);

    // treat ui file */
    QFileInfo fi(filename);
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
        qDebug() << "caQtDM -- should never happen ??";
    }

    // set window title without the whole path
    QString title(file->fileName().section('/',-1));
    thisFileShort = file->fileName().section('/',-1);
    thisFileFull = fi.absoluteFilePath ();
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

    qRegisterMetaType<knobData>("knobData");

    // connect signals to slots for exchanging data
    connect(mutexKnobData, SIGNAL(Signal_QLineEdit(const QString&, const QString&)), this,
            SLOT(Callback_UpdateLine(const QString&, const QString&)));

    connect(mutexKnobData,
            SIGNAL(Signal_UpdateWidget(int, QWidget*, const QString&, const QString&, const QString&, knobData)), this,
            SLOT(Callback_UpdateWidget(int, QWidget*, const QString&, const QString&, const QString&, knobData)));

    connect(this, SIGNAL(Signal_OpenNewWFile(const QString&, const QString&, const QString&)), parent,
            SLOT(Callback_OpenNewFile(const QString&, const QString&, const QString&)));

    // initialize IO
    PrepareDeviceIO();

    level=0;

    // say for all widgets that they have to be treated, will be set to true when treated to avoid multiple use
    // by findChildren
    QList<QWidget *> all = this->findChildren<QWidget *>();
    foreach(QWidget* widget, all) {
        widget->setProperty("Taken", false);
    }

    initTry = true;
    // get from the display all the calc widgets
    QList<QWidget *> widgets1 = this->findChildren<QWidget *>();
    foreach(QWidget *w1, widgets1) {
        savedFile[0] = fi.baseName();
        savedMacro[0] = macro;
        // open and load file
        HandleWidget(w1, savedMacro[0], true);
    }
    // get from the display all the widgets having a monitor associated
    QList<QWidget *> widgets2 = this->findChildren<QWidget *>();
    foreach(QWidget *w1, widgets2) {
        savedFile[0] = fi.baseName();
        savedMacro[0] = macro;
        // open and load file
        HandleWidget(w1, savedMacro[0], false);
    }

    // build a list for getting all soft pv
    mutexKnobData->BuildSoftPVList(myWidget);

    // start a timer
    startTimer(2000);
}

/**
 * timer event, not used
 */
void CaQtDM_Lib::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
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

/**
 * this routine handles the initialization of all widgets
 */
void CaQtDM_Lib::HandleWidget(QWidget *w1, QString macro, bool firstPass)
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

    if(className.contains("ca")) {
        PRINT(printf("\n%*c %s macro=<%s>", 15 * level, '+', qPrintable(w1->objectName()), qPrintable(macro)));
        map = createMap(macro);
    }

    QColor bg = w1->property("background").value<QColor>();
    QColor fg = w1->property("foreground").value<QColor>();

    // keep original colors
    w1->setProperty("BColor", bg);
    w1->setProperty("FColor", fg);

    // when first pass specified, treat only caCalc
    //==================================================================================================================
    if(firstPass) {
        if(caCalc* widget = qobject_cast<caCalc *>(w1)) {

            // soft channel
            kData.soft = true;

            // add soft channel
            addMonitor(myWidget, &kData, widget->getVariable().toAscii().constData(), w1, specData, map, &pv);

            // other channels if any
            kData.soft = false;
            nbMonitors = InitVisibility(w1, &kData, map, specData, widget->getVariable().toAscii().constData());

            // when no monitors then inititalize value
            if(nbMonitors == 0) {
                //qDebug() << "update " << widget->getVariable().toAscii().constData()<< "initial value" << widget->getInitialValue();
                widget->setValue(widget->getInitialValue());
                mutexKnobData->UpdateSoftPV(widget->getVariable().toAscii().constData(), widget->getInitialValue(), myWidget);
            }

            w1->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(w1, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenu(const QPoint&)));
            w1->setProperty("Connect", false);

            widget->setProperty("Taken", true);
        }

        return;
    }

    // the different widgets to be handled
    //==================================================================================================================
    if(caImage* widget = qobject_cast<caImage *>(w1)) {

        //qDebug() << "create caImage";

        nbMonitors = InitVisibility(w1, &kData, map, specData, "");

        // empty calc string, set animation
        if(widget->getImageCalc().size() == 0) {
            //qDebug() <<  "no calc for image";
            widget->setFrame(0);
            widget->startMovie();
        }

        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caRelatedDisplay* widget = qobject_cast<caRelatedDisplay *>(w1)) {

        //qDebug() << "create caRelatedDisplay" << widget << widget->getLabels() << widget->getArgs() <<  widget->getFiles();

        QString text;

        text = widget->getLabels();
        if(reaffectText(map, &text))  widget->setLabels(text);

        text = widget->getArgs();
        if(reaffectText(map, &text))  widget->setArgs(text);

        text = widget->getFiles();
        if(reaffectText(map, &text))  widget->setFiles(text);

        text = widget->getLabel();
        if(reaffectText(map, &text))  widget->setLabel(text);

        connect(widget, SIGNAL(clicked(int)), this, SLOT(Callback_RelatedDisplayClicked(int)));
        connect(widget, SIGNAL(triggered(int)), this, SLOT(Callback_RelatedDisplayClicked(int)));

        widget->raise();

        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caShellCommand* widget = qobject_cast<caShellCommand *>(w1)) {

        //qDebug() << "create caShellCommand";

        QString text;
        text= widget->getLabels();
        if(reaffectText(map, &text))  widget->setLabels(text);

        text = widget->getArgs();
        if(reaffectText(map, &text))  widget->setArgs(text);

        text = widget->getFiles();
        if(reaffectText(map, &text)) widget->setFiles(text);

        text = widget->getLabel();
        if(reaffectText(map, &text))  widget->setLabel(text);

        connect(widget, SIGNAL(clicked(int)), this, SLOT(Callback_ShellCommandClicked(int)));
        connect(widget, SIGNAL(triggered(int)), this, SLOT(Callback_ShellCommandClicked(int)));

        widget->raise();

        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caMenu* widget = qobject_cast<caMenu *>(w1)) {

        //qDebug() << "create caMenu";

        QString text = widget->getPV();
        if(text.size() > 0) {
            text =  treatMacro(map, text, &doNothing);
            addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
            connect(widget, SIGNAL(activated(QString)), this, SLOT(Callback_MenuClicked(QString)));
            widget->setPV(text);
        }

        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caCamera* widget = qobject_cast<caCamera *>(w1)) {

        //qDebug() << "create caCamera";

        // addmonitor normally will add a tooltip to show the pv; however here we have more than one pv
        QString tooltip;
        QString pvs= "";
        tooltip.append(ToolTipPrefix);

        for(int i=0; i< 8; i++) {
            QString text;
            bool alpha = true;
            if(i==0) text = widget->getPV_Data();
            if(i==1) text = widget->getPV_Width();
            if(i==2) text = widget->getPV_Height();
            if(i==3) text = widget->getPV_Code();
            if(i==4) text = widget->getPV_BPP();
            // for spectrum pseudo levels
            if(i==5) {
                alpha = widget->isAlphaMinLevel();
                text = widget->getMinLevel();
            }
            if(i==6) {
                alpha = widget->isAlphaMaxLevel();
                text = widget->getMaxLevel();
            }
            // for dataprocessing data x,y,w,h
            if(i==7) {
                QStringList thisString = widget->getDataProcChannels().split(";");
                if(thisString.count() == 4 &&
                        thisString.at(0).trimmed().length() > 0 &&
                        thisString.at(1).trimmed().length() > 0  &&
                        thisString.at(2).trimmed().length() > 0 &&
                        thisString.at(3).trimmed().length() > 0) {
                    for(int j=0; j<4; j++) {
                        specData[0] = i+j;   // x,y,w,h
                        text =  treatMacro(map, thisString.at(j), &doNothing);
                        addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
                        pvs.append(pv);
                        if(j<3)pvs.append(";");
                    }
                }
            }

            if(text.size() > 0 && alpha) {
                specData[0] = i;   // pv type
                text =  treatMacro(map, text, &doNothing);
                if(i!=7) addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
                if(i==0) widget->setPV_Data(pv);
                if(i==1) widget->setPV_Width(pv);
                if(i==2) widget->setPV_Height(pv);
                if(i==3) widget->setPV_Code(pv);
                if(i==4) widget->setPV_BPP(pv);
                if(i==5) widget->setMinLevel(pv);
                if(i==6) widget->setMaxLevel(pv);
                if(i==7) widget->setDataProcChannels(pvs);
                if(i>0) tooltip.append("<br>");
                if(i<=6) tooltip.append(pv); else tooltip.append(pvs);
            } else if (i==3) {  // code missing (assume 1 for Helge)
                widget->setCode(1);
            } else if (i==4) {  // bpp missing (assume 3 for Helge)
                widget->setBPP(3);
            }
        }
        // finish tooltip
        tooltip.append(ToolTipPostfix);
        widget->setToolTip(tooltip);

        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caChoice* widget = qobject_cast<caChoice *>(w1)) {

        //qDebug() << "create caChoice";

        QString text = widget->getPV();
        if(text.size() > 0) {
            text =  treatMacro(map, text, &doNothing);
            addMonitor(myWidget, &kData, text, w1, specData, map, &pv);
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(Callback_ChoiceClicked(QString)));
            widget->setPV(text);
        }

        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caLabel* widget = qobject_cast<caLabel *>(w1)) {

        //qDebug() << "create caLabel";

        nbMonitors = InitVisibility(w1, &kData, map, specData, "");

        QString text =  treatMacro(map, widget->text(), &doNothing);
        text.replace(QString::fromWCharArray(L"\u00A6"), " ");    // replace ¦ with a blanc (was used in macros for creating blancs)
        widget->setText(text);

        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caTextEntry* widget = qobject_cast<caTextEntry *>(w1)) {

        //qDebug() << "create caTextEntry";

        if(widget->getPV().size() > 0) {
            widget->setEnabled(true);
            addMonitor(myWidget, &kData, widget->getPV(), w1, specData, map, &pv);
            widget->setPV(pv);
            connect(widget, SIGNAL(TextEntryChanged(const QString&)), this,
                    SLOT(Callback_TextEntryChanged(const QString&)));
        }
        // default format, format from ui file will be used normally except for channel precision
        widget->setFormat(1);
        widget->clearFocus();

        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caLineEdit* widget = qobject_cast<caLineEdit *>(w1)) {

        //qDebug() << "create caLineEdit";

        if(widget->getPV().size() > 0) {
            widget->setCursor(QCursor());
            widget->setReadOnly(true);
            widget->setDragEnabled(true);

            widget->setAlignment(widget->alignment());
            addMonitor(myWidget, &kData, widget->getPV(), w1, specData, map, &pv);
            widget->setPV(pv);
        }

        // default format, format from ui file will be used normally except for channel precision
        widget->setFormat(1);

        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caGraphics* widget = qobject_cast<caGraphics *>(w1)) {

        //qDebug() << "create caGraphics";

        nbMonitors = InitVisibility(w1, &kData, map, specData, "");
        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caPolyLine* widget = qobject_cast<caPolyLine *>(w1)) {

        //qDebug() << "create caPolyLine";

        nbMonitors = InitVisibility(w1, &kData, map, specData, "");
        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if (caApplyNumeric* widget = qobject_cast<caApplyNumeric *>(w1)){

        //qDebug() << "create caAppyNumeric";

        if(widget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, widget->getPV(), w1, specData, map, &pv);
            widget->setPV(pv);
            connect(widget, SIGNAL(clicked(double)), this, SLOT(Callback_EApplyNumeric(double)));
        }
        widget->raise();

        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if (caNumeric* widget = qobject_cast<caNumeric *>(w1)){

        if(widget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, widget->getPV(), w1, specData, map, &pv);
            widget->setPV(pv);
            connect(widget, SIGNAL(valueChanged(double)), this, SLOT(Callback_ENumeric(double)));
        }
        widget->raise();

        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if (caMessageButton* widget = qobject_cast<caMessageButton *>(w1)) {

        QString text;
        //qDebug() << "create caMessageButton" <<  widget->getPV();
        if(widget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, widget->getPV(), w1, specData, map, &pv);
            widget->setPV(pv);
        }
        connect(widget, SIGNAL(messageButtonSignal(int)), this, SLOT(Callback_MessageButton(int)));

        text = widget->getLabel();
        if(reaffectText(map, &text))  widget->setLabel(text);

        widget->raise();

        widget->setProperty("Taken", true);


        //==================================================================================================================
    } else if(caToggleButton* widget = qobject_cast<caToggleButton *>(w1)) {

        //qDebug() << "create caToggleButton";

        connect(widget, SIGNAL(toggleButtonSignal(bool)), this, SLOT(Callback_ToggleButton(bool)));

        addMonitor(myWidget, &kData, widget->getPV(), w1, specData, map, &pv);
        widget->setPV(pv);

        widget->raise();

        widget->setProperty("Taken", true);


        //==================================================================================================================
    } else if(caScriptButton* widget = qobject_cast<caScriptButton *>(w1)) {

        //qDebug() << "create caToggleButton";

        connect(widget, SIGNAL(scriptButtonSignal()), this, SLOT(Callback_ScriptButton()));

        widget->raise();
        widget->setToolTip("process never started !");
        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caLed* widget = qobject_cast<caLed *>(w1)) {

        //qDebug() << "create caLed";

        if(widget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, widget->getPV(), w1, specData, map, &pv);
            widget->setPV(pv);
        }

        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caBitnames* widget = qobject_cast<caBitnames *>(w1)) {

        //qDebug() << "create caBitnames";

        if(widget->getEnumPV().size() > 0 && widget->getValuePV().size() > 0) {
            addMonitor(myWidget, &kData, widget->getEnumPV(), w1, specData, map, &pv);
            widget->setEnumPV(pv);
            addMonitor(myWidget, &kData, widget->getValuePV(), w1, specData, map, &pv);
            widget->setValuePV(pv);
        }

        widget->setProperty("Taken", true);
        //==================================================================================================================
    } else if(caSlider* widget = qobject_cast<caSlider *>(w1)) {

        //qDebug() << "create caSlider";

        if(widget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, widget->getPV(), w1, specData, map, &pv);
            widget->setPV(pv);
            connect(widget, SIGNAL(sliderMoved(double)), this, SLOT(Callback_Slider(double)));
        }

        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caThermo* widget = qobject_cast<caThermo *>(w1)) {

        //qDebug() << "create caThermo";

        if(widget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, widget->getPV(), w1, specData, map, &pv);
            widget->setPV(pv);
        }
        // for an opposite direction, invert maximum and minimum

        if(widget->getDirection() == caThermo::Down || widget->getDirection() == caThermo::Left) {
            double max = widget->maxValue();
            double min = widget->minValue();
            widget->setMinValue(max);
            widget->setMaxValue(min);
        }

        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caLinearGauge* widget = qobject_cast<caLinearGauge *>(w1)) {

        //qDebug() << "create lineargauge for" << widget->getPV();

        if(widget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, widget->getPV(), w1, specData, map, &pv);
            widget->setPV(pv);
        }

        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caCircularGauge* widget = qobject_cast<caCircularGauge *>(w1)) {

        //qDebug() << "create circulargauge for" << widget->getPV();

        if(widget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, widget->getPV(), w1, specData, map, &pv);
            widget->setPV(pv);
        }

        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caByte* widget = qobject_cast<caByte *>(w1)) {

        //qDebug() << "create caByte" << w1;

        if(widget->getPV().size() > 0) {
            addMonitor(myWidget, &kData, widget->getPV(), w1, specData, map, &pv);
            widget->setPV(pv);
        }

        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caInclude* widget = qobject_cast<caInclude *>(w1)) {
        QWidget *thisW;
        QFile *file = new QFile;
        QUiLoader loader;
        bool prcFile = false;

        // define a layout
        QGridLayout *layout = new QGridLayout;
        layout->setContentsMargins(0,0,0,0);

        //qDebug() << "treat caInclude" << w1 << "level=" << level;

        nbMonitors = InitVisibility(w1, &kData, map, specData, "");

        widget->setProperty("Taken", true);

        QString macroS = widget->getMacro();
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
        QString fileName = widget->getFileName().trimmed();
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

        searchFile *s = new searchFile(fileName);
        QString fileNameFound = s->findFile();
        if(fileNameFound.isNull()) {
            qDebug() << "file" << fileName << "does not exist";
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
#ifdef PRC
                ParsePepFile *parsefile = new ParsePepFile(fileName, pepPrint);
                thisW = parsefile->load(this);
                delete parsefile;
#else
                thisW = (QWidget*) 0;
#endif
            } else {
                // open and load ui file
                file->setFileName(fileName);
                file->open(QFile::ReadOnly);
                thisW = loader.load(file, this);
                file->close();
            }

            // some error with loading
            if (!thisW) {
                postMessage(QtDebugMsg, (char*) tr("could not load include file %1").arg(fileName).toAscii().constData());
                // seems to be ok
            } else {

                includeWidgetList.append(thisW);

                // add widget to the gui
                layout->addWidget(thisW);
                widget->setLayout(layout);
                widget->setLineSize(0);

                // go through its childs
                QList<QWidget *> childs = thisW->findChildren<QWidget *>();
                level++;

                // keep actual filename
                savedFile[level] = fi.baseName();

                foreach(QWidget *child, childs) {
                    HandleWidget(child, macroS, false);
                }
                level--;
            }

        } else {
            postMessage(QtDebugMsg, (char*) tr("sorry, could not load include file %1").arg(fileName).toAscii().constData());
            qDebug() << "sorry, file" << fileName << " does not exist";
        }

        delete file;

        macroS = savedMacro[level];

        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caFrame* widget = qobject_cast<caFrame *>(w1)) {

        //qDebug() << "treat caFrame" << w1;

        nbMonitors = InitVisibility(w1, &kData, map, specData, "");

        widget->setProperty("Taken", true);

        QString macroS = widget->getMacro();

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

        QList<QWidget *> childs = widget->findChildren<QWidget *>();
        level++;

        // get actual filename from previous level
        savedFile[level] = savedFile[level-1];

        foreach(QWidget *child, childs) {
            HandleWidget(child, macroS, false);
        }
        level--;

        macroS= savedMacro[level];

        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caCartesianPlot* widget = qobject_cast<caCartesianPlot *>(w1)) {

        QString triggerChannel, countChannel, eraseChannel, title;

        // addmonitor normally will add a tooltip to show the pv; however here we have more than one pv
        QString tooltip;
        tooltip.append(ToolTipPrefix);

        // do this while the order has to be correct
        widget->setForeground(widget->getForeground());
        widget->setBackground(widget->getBackground());
        widget->setScaleColor(widget->getScaleColor());
        widget->setGridColor(widget->getGridColor());

        // go through all possible curves and add monitor
        for(int i=0; i< caCartesianPlot::curveCount; i++) {
            QString pvs ="";
            QStringList thisString = widget->getPV(i).split(";");

            widget->setColor(widget->getColor(i), i);
            widget->setStyle(widget->getStyle(i), i);
            widget->setSymbol(widget->getSymbol(i), i);

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
            widget->setPV(pvs, i);
        }

        // handle trigger channel if any
        triggerChannel = widget->getTriggerPV();
        if(triggerChannel.trimmed().length() > 0) {
            specData[2] = caCartesianPlot::CH_Trigger; // Trigger
            addMonitor(myWidget, &kData, triggerChannel, w1, specData, map, &pv);
            tooltip.append(pv);
            tooltip.append("<br>");
            widget->setTriggerPV(pv);
        }

        // handle count channel if any
        int Number;
        if(!widget->hasCountNumber(&Number)) {
            countChannel = widget->getCountPV();
            if(countChannel.trimmed().length() > 0) {
                specData[2] = caCartesianPlot::CH_Count; // Count
                addMonitor(myWidget, &kData, countChannel, w1, specData, map, &pv);
                tooltip.append(pv);
                tooltip.append("<br>");
                widget->setCountPV(pv);
            }
        } else {
            //qDebug() << "count=" << Number;
        }

        // handle erase channel if any
        eraseChannel = widget->getErasePV();
        if(eraseChannel.trimmed().length() > 0) {
            specData[2] = caCartesianPlot::CH_Erase; // Count
            addMonitor(myWidget, &kData, eraseChannel, w1, specData, map, &pv);
            tooltip.append(pv);
            tooltip.append("<br>");
            widget->setErasePV(pv);
        }

        // handle user scale
        if(widget->getXscaling() == caCartesianPlot::User) {
            double xmin, xmax;
            int ok=widget->getXLimits(xmin, xmax);
            if(ok) widget->setScaleX(xmin, xmax);
        } else if(widget->getXscaling() == caCartesianPlot::Auto) {
            widget->setXscaling(caCartesianPlot::Auto);
            // in case of channel the limits will be defined later by the hopr and lopr
            // however in case of channel, it is possible to get dynamic limits through monitors
        } else if(widget->getXscaling() == caCartesianPlot::Channel) {
            QString pvs ="";
            QStringList thisString = widget->getXaxisLimits().split(";");
            if(thisString.count() == 2 && thisString.at(0).trimmed().length() > 0 && thisString.at(1).trimmed().length() > 0) {
                double xmin, xmax;
                int ok=widget->getXLimits(xmin, xmax);
                if(ok) {
                    widget->setScaleX(xmin, xmax);
                    // when channel was specified for x, but no channel was defined for the plot then set to auto
                    QStringList thisString = widget->getPV(0).split(";");
                    if(thisString.count() == 2 && thisString.at(0).trimmed().length() == 0)  widget->setXscaling(caCartesianPlot::Auto);
                } else {
                    specData[0] = 0;
                    specData[2] = caCartesianPlot::CH_Xscale;
                    addMonitor(myWidget, &kData, thisString.at(0), w1, specData, map, &pv);
                    tooltip.append(pv);
                    pvs = pv;
                    specData[0] = 1;
                    specData[2] = caCartesianPlot::CH_Xscale;
                    addMonitor(myWidget, &kData, thisString.at(1), w1, specData, map, &pv);
                    tooltip.append(",");
                    tooltip.append(pv);
                    pvs.append(";");
                    pvs.append(pv);
                    tooltip.append("<br>");
                    widget->setXaxisLimits(pvs);
                }
            }
        }
        if(widget->getYscaling() == caCartesianPlot::User) {
            double ymin, ymax;
            int ok=widget->getYLimits(ymin, ymax);
            if(ok) widget->setScaleY(ymin, ymax);
        } else if(widget->getYscaling() == caCartesianPlot::Auto){
            widget->setYscaling(caCartesianPlot::Auto);
            // in case of channel the limits will be defined later by the hopr and lopr
            // however in case of channel, it is possible to get dynamic limits through monitors
        } else if(widget->getYscaling() == caCartesianPlot::Channel) {
            QString pvs ="";
            QStringList thisString = widget->getYaxisLimits().split(";");
            if(thisString.count() == 2 && thisString.at(0).trimmed().length() > 0 && thisString.at(1).trimmed().length() > 0) {
                double ymin, ymax;
                int ok=widget->getYLimits(ymin, ymax);
                if(ok) {
                    widget->setScaleY(ymin, ymax);
                    // when channel was specified for x, but no channel was defined for the plot then set to auto
                    QStringList thisString = widget->getPV(0).split(";");
                    if(thisString.count() == 2 && thisString.at(1).trimmed().length() == 0)  widget->setYscaling(caCartesianPlot::Auto);
                } else {
                    specData[0] = 0;
                    specData[2] = caCartesianPlot::CH_Yscale;
                    addMonitor(myWidget, &kData, thisString.at(0), w1, specData, map, &pv);
                    tooltip.append(pv);
                    pvs = pv;
                    specData[0] = 1;
                    specData[2] = caCartesianPlot::CH_Yscale;
                    addMonitor(myWidget, &kData, thisString.at(1), w1, specData, map, &pv);
                    tooltip.append(",");
                    tooltip.append(pv);
                    pvs.append(";");
                    pvs.append(pv);
                    tooltip.append("<br>");
                    widget->setYaxisLimits(pvs);
                }
            }
        }

        // finish tooltip
        tooltip.append(ToolTipPostfix);
        widget->setToolTip(tooltip);

        // reaffect titles
        title = widget->getTitlePlot();
        if(reaffectText(map, &title)) widget->setTitlePlot(title);
        title = widget->getTitleX();
        if(reaffectText(map, &title)) widget->setTitleX(title);
        title = widget->getTitleY();
        if(reaffectText(map, &title)) widget->setTitleY(title);

        widget->setWhiteColors();

        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caStripPlot* widget = qobject_cast<caStripPlot *>(w1)) {

        //qDebug() << "create caStripPlot";

        QString text, title;
        QList<QVariant> integerList;
        int nbMon = 0;

        // addmonitor normally will add a tooltip to show the pv; however here we have more than one pv
        QString tooltip;
        tooltip.append(ToolTipPrefix);

        text = widget->getPVS();
        reaffectText(map, &text);
        widget->setPVS(text);
        QStringList vars = text.split(";", QString::SkipEmptyParts);

        int NumberOfCurves = min(vars.count(), caStripPlot::MAXCURVES);

        // go through the defined curves and add monitor

        if(NumberOfCurves > 0) widget->defineCurves(vars, widget->getUnits(), widget->getPeriod(),  widget->width(),  NumberOfCurves);
        for(int i=0; i< NumberOfCurves; i++) {
            int num;
            QString pv = vars.at(i).trimmed();
            if(pv.size() > 0) {
                if(i==0) {  // user defaults, will be redefined when limits from channel
                    widget->setYscale(widget->getYaxisLimitsMin(i), widget->getYaxisLimitsMax(i));
                }
                specData[1] = i;            // curve number
                specData[0] = vars.count(); // number of curves
                num = addMonitor(myWidget, &kData, pv, w1, specData, map, &pv);
                nbMon++;
                widget->showCurve(i, true);

                tooltip.append(pv);
                tooltip.append("<br>");
                integerList.append(num);
            }
        }

        // finish tooltip
        tooltip.append(ToolTipPostfix);
        widget->setToolTip(tooltip);

        title = widget->getTitlePlot();
        if(reaffectText(map, &title)) widget->setTitlePlot(title);
        title = widget->getTitleX();
        if(reaffectText(map, &title)) widget->setTitleX(title);
        title = widget->getTitleY();
        if(reaffectText(map, &title)) widget->setTitleY(title);

        integerList.insert(0, nbMon); /* set property into widget */
        widget->setProperty("MonitorList", integerList);
        widget->setProperty("Taken", true);

        //==================================================================================================================
    } else if(caTable* widget = qobject_cast<caTable *>(w1)) {

        //qDebug() << "create caTable" << widget->getPVS();

        QStringList vars = widget->getPVS().split(";", QString::SkipEmptyParts);
        widget->setColumnCount(3);
        widget->setRowCount(vars.count());

        // go through the pvs and add monitor
        for(int i=0; i< vars.count(); i++) {
            QString pv = vars.at(i);
            if(pv.size() > 0) {
                QTableWidgetItem *item;
                specData[0] = i;            // table row
                addMonitor(myWidget, &kData, pv, w1, specData, map, &pv);
                item = new QTableWidgetItem(pv);
                item->setTextAlignment(Qt::AlignAbsolute | Qt:: AlignLeft);
                widget->setItem(i,0, item);
            }
        }
        widget->setColumnSizes(widget->getColumnSizes());
        widget->setProperty("Taken", true);
        widget->setToolTip("with Ctrl+C you can copy selected items to the clipboard\ninside X11 you can then do shft+ins");
    }

    // make a context menu for object having a monitor
    if(className.contains("ca") && !className.contains("caRel") && !className.contains("caTable") && nbMonitors > 0) {
        w1->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(w1, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenu(const QPoint&)));
        w1->setProperty("Connect", false);
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

/**
 * routine to create an epics monitor
 */
int CaQtDM_Lib::addMonitor(QWidget *thisW, knobData *kData, QString pv, QWidget *w, int *specData, QMap<QString, QString> map, QString *pvRep)
{
    QMutex *mutex;
    // define epics monitors
    bool doNothing = false;
    int cpylen;
    int indx;

    int rate = 5;  // default will be 5Hz

    if(pv.size() == 0) return -1;

    QString trimmedPV = pv.trimmed();
#ifdef epics4
    int EPICS4 = false;
    if(trimmedPV.at( 0 ) == '@' ) {
        trimmedPV.remove( 0, 1 );
        EPICS4 = true;
    }
#endif

    // when we defined already the same software channel, then get back the rate that was specified
    if(mutexKnobData->getSoftPV(pv, &indx, thisW)) {
        knobData *kPtr = mutexKnobData->GetMutexKnobDataPtr(indx);  // use pointer
        rate = kPtr->edata.repRate;
    }

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
                    (char*) pv.trimmed().toAscii().constData());
        } else {
            sprintf(asc, "pv %s display rate set to maximum %dHz", trimmedPV.toAscii().constData(), rate);
        }
        postMessage(QtDebugMsg, asc);
    }

    QString newpv = treatMacro(map, trimmedPV, &doNothing);

    if(doNothing) {
        char asc[255];
        sprintf(asc, "malformed pv '%s' (due to macro?)", (char*) newpv.toAscii().constData());
        postMessage(QtDebugMsg, asc);
    }

    *pvRep = newpv;

    // set pvname as tooltip
    QString tooltip;
    tooltip.append(ToolTipPrefix);
    tooltip.append(newpv);
    tooltip.append(ToolTipPostfix);
    w->setToolTip(tooltip);

    cpylen = qMin(newpv.length(), MAXPVLEN-1);
    strncpy(kData->pv, (char*) newpv.toAscii().constData(), cpylen);
    kData->pv[cpylen] = '\0';

    memcpy(kData->specData, specData, sizeof(int) * NBSPECS);
    kData->thisW = (void*) thisW;
    kData->dispW = (void*) w;

    mutex = new QMutex;
    kData->mutex = (void*)  mutex;

    // keep actual object name

    cpylen = qMin(w->objectName().length(), MAXDISPLEN-1);
    strncpy(kData->dispName, w->objectName().toLower().toAscii().constData(), cpylen);
    kData->dispName[cpylen] = '\0';

    QString classname = w->metaObject()->className();
    cpylen = qMin(classname.length(), MAXDISPLEN-1);
    strncpy(kData->clasName, classname.toLower().toAscii().constData(), cpylen);
    kData->clasName[cpylen] = '\0';

    // keep actual filename
    cpylen = qMin( savedFile[level].length(), MAXFILELEN-1);
    strncpy(kData->fileName, (char*) savedFile[level].toAscii().constData(), cpylen);
    kData->fileName[cpylen] = '\0';

    if (QLineEdit *lineedit = qobject_cast<QLineEdit *>(w)) {
        lineedit->setText("");
    }

    // get an index in the data list
    int num = mutexKnobData->GetMutexKnobDataIndex();
    if(num == -1) {
        qDebug() << "this should never happen";
        return num;
    }

    // insert into the softpv list when we create a soft channel
    if(kData->soft) mutexKnobData->InsertSoftPV(kData->pv, num, thisW);

    // did we use a new soft channel here, then set it
    if(mutexKnobData->getSoftPV(kData->pv, &indx, thisW)) kData->soft= true;

    // update our data
    mutexKnobData->SetMutexKnobData(num, *kData);

    // create data acquisition (C routine for epics and acs)

#ifdef epics4
    if(EPICS4) {
       CreateAndConnect(num, kData, rate, true);
       Epics4->CreateAndConnect4(num, trimmedPV);
    } else {
       CreateAndConnect(num, kData, rate, false);
    }
#else
    CreateAndConnect(num, kData, rate, false);
#endif

    w->setProperty("MonitorIndex", num);
    w->setProperty("Connect", false);

    memset(kData, 0, sizeof (knobData));

    return num;
}

/**
  * return true or false accoprding the bit inside a value
  */
bool CaQtDM_Lib::bitState(int value, int bitNr)
{
    return ((((int) value >> bitNr) & 1) == 1);
}

/**
  * treat visibility of our objects
  */
int CaQtDM_Lib::setObjectVisibility(QWidget *w, double value)
{
    bool visible = true;
    bool valid = false;
    double result;

    if(caFrame *widget = qobject_cast<caFrame *>(w)) {
        // treat visibility if defined
        ComputeVisibility(caFrame)
        if(widget->getVisibilityMode() == caFrame::Background) {
            if(visible) widget->setAutoFillBackground(true);
            else widget->setAutoFillBackground(false);
            return visible;
        }
    } else if (caInclude *widget = qobject_cast<caInclude *>(w)) {
        // treat visibility if defined
        ComputeVisibility(caInclude)
    } else if (caGraphics *widget = qobject_cast<caGraphics *>(w)) {
        // treat visibility if defined
        ComputeVisibility(caGraphics)
    } else if(caImage *widget = qobject_cast<caImage *>(w)) {
        // treat visibility if defined
        ComputeVisibility(caImage)
    } else if(caPolyLine *widget = qobject_cast<caPolyLine *>(w)) {
        // treat visibility if defined
        ComputeVisibility(caPolyLine)
    } else if(caLabel *widget = qobject_cast<caLabel *>(w)) {
        // treat visibility if defined
        ComputeVisibility(caLabel)
    }

    if(!visible) {
        if(caPolyLine *widget = qobject_cast<caPolyLine *>(w)) widget->setHidden(true);
        else if(caGraphics *widget = qobject_cast<caGraphics *>(w)) widget->setHidden(true);
        else w->hide();
    } else {
        if(caPolyLine *widget = qobject_cast<caPolyLine *>(w)) widget->setHidden(false);
         else if(caGraphics *widget = qobject_cast<caGraphics *>(w)) widget->setHidden(false);
        else w->show();
    }
    return visible;
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
    bool visible;

    if(caFrame *frame = qobject_cast<caFrame *>(w)) {
        strcpy(calcString, frame->getVisibilityCalc().toAscii().constData());
    }
    if(caInclude *frame = qobject_cast<caInclude *>(w)) {
        strcpy(calcString, frame->getVisibilityCalc().toAscii().constData());
    }
    else if(caImage *image = qobject_cast<caImage *>(w)) {
        strcpy(calcString, image->getVisibilityCalc().toAscii().constData());
    }
    else if(caGraphics *image = qobject_cast<caGraphics *>(w)) {
        strcpy(calcString, image->getVisibilityCalc().toAscii().constData());
    }
    else if(caPolyLine *line = qobject_cast<caPolyLine *>(w)) {
        strcpy(calcString, line->getVisibilityCalc().toAscii().constData());
    }
    else if(caLabel *label = qobject_cast<caLabel *>(w)) {
        strcpy(calcString, label->getVisibilityCalc().toAscii().constData());
    }
    else if(caCalc *calc = qobject_cast<caCalc *>(w)) {
        strcpy(calcString, calc->getCalc().toAscii().constData());
    }

    // any monitors ?
    QVariant var=w->property("MonitorList");
    QVariantList list = var.toList();

    if(list.size() == 0) return true;

    int nbMonitors = list.at(0).toInt();
    //qDebug() << "number of monitors" << nbMonitors << "calc=" << calcString;
    if(nbMonitors > 0)  {

        // scan and get the channels
        for(int i=0; i < MAX_CALC_INPUTS; i++) valueArray[i] = 0.0;
        for(int i=0; i< nbMonitors;i++) {
            knobData *ptr = mutexKnobData->GetMutexKnobDataPtr(list.at(i+1).toInt());
            //qDebug() << "calculate from index" << ptr->index << ptr->pv << ptr->edata.connected << ptr->edata.rvalue;
            // when connected
            if(ptr->edata.connected) {
                valueArray[i] = ptr->edata.rvalue;
            } else {
                valueArray[i] = 0.0;
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

        status = postfix(calcString, post, &errnum);
        if(status) {
            char asc[100];
            sprintf(asc, "invalid calc %s for %s", calcString, qPrintable(w->objectName()));
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
            sprintf(asc, "invalid calc %s for %s", calcString, qPrintable(w->objectName()));
            postMessage(QtDebugMsg, asc);
            valid = false;
            return true;
        }
    }
    valid = false;
    return true;
}

/**
  * computes alarm state from the channels
  */
int CaQtDM_Lib::ComputeAlarm(QWidget *w)
{
    int status;
    // any monitors ?
    QVariant var=w->property("MonitorList");
    QVariantList list = var.toList();
    int nbMonitors = list.at(0).toInt();
    //qDebug() << "number of monitors" << nbMonitors;
    status = NO_ALARM;
    if(nbMonitors > 0)  {

        // medm uses however only first channel
        knobData *ptr = mutexKnobData->GetMutexKnobDataPtr(list.at(1).toInt());
        //qDebug() << ptr->pv << ptr->edata.connected << ptr->edata.rvalue;
        // when connected
        if(ptr->edata.connected) {
            status = status | ptr->edata.severity;
        } else {
            return NOTCONNECTED;
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
        if((widget->isExternalEnabled()) && (data.edata.initialize)) {

            // when no limits defined then say no external limits
            if( (data.edata.lower_alarm_limit == data.edata.upper_alarm_limit) &&
                    (data.edata.lower_warning_limit == data.edata.upper_warning_limit) &&
                    (data.edata.upper_disp_limit == data.edata.lower_disp_limit) ) {
                widget->setExternalEnabled(false);
            }

            // set low and high error
            if(data.edata.lower_alarm_limit != data.edata.upper_alarm_limit) {
                widget->setLowError(data.edata.lower_alarm_limit);
                widget->setHighError(data.edata.upper_alarm_limit);
            } else {
                widget->setLowError(data.edata.lower_disp_limit);
                widget->setHighError(data.edata.upper_disp_limit);
            }

            // warning limits, when not specified set alarm limits
            if(data.edata.lower_warning_limit != data.edata.upper_warning_limit) {
                widget->setLowWarning(data.edata.lower_warning_limit);
                widget->setHighWarning(data.edata.upper_warning_limit);
            } else if(data.edata.upper_disp_limit != data.edata.lower_disp_limit) {
                widget->setHighWarning(data.edata.upper_disp_limit);
                widget->setLowWarning(data.edata.lower_disp_limit);
            } else {
                widget->setHighWarning(1000.0);
                widget->setLowWarning(0.0);
            }

            // HOPR and LOPR
            if(data.edata.upper_disp_limit != data.edata.lower_disp_limit) {
                widget->setMaxValue(data.edata.upper_disp_limit);
                widget->setMinValue(data.edata.lower_disp_limit);
            } else {
                widget->setMaxValue(1000.0);
                widget->setMinValue(0.0);
            }
            widget->update();
        }
        widget->setValue(data.edata.rvalue);
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

    // thread mutexknobdata emits to all instances of this class, later we will have to filter on the emit side to enhance performance
    bool thisInstance = false;
    QWidget *widget = w;
      while (widget -> parentWidget()) {
          widget = widget-> parentWidget() ;
          if(widget == myWidget) {
              thisInstance = true;
              break;
          }
      }
      if(!thisInstance) {
          return;
      }

    // calc ==================================================================================================================
    if(caCalc *widget = qobject_cast<caCalc *>(w)) {
        bool valid;
        double result;
       //qDebug() << "we have a caCalc";

        CalcVisibility(w, result, valid);  // visibility not used, but calculation yes
        if(valid) {
            widget->setValue(result);
            mutexKnobData->UpdateSoftPV(data.pv, result, myWidget);
        }

        // frame ==================================================================================================================
    } else if(caLabel *widget = qobject_cast<caLabel *>(w)) {
        //qDebug() << "we have a label";

        if(data.edata.connected) {
            int colorMode = widget->getColorMode();
            if(colorMode == caLabel::Static) {
                // done at initialisation, we have to set it back after no connect
                if(!widget->property("Connect").value<bool>()) {
                    QColor fg = widget->property("FColor").value<QColor>();
                    QColor bg = widget->property("BColor").value<QColor>();
                    widget->setForeground(fg);
                    widget->setBackground(bg);
                    widget->setProperty("Connect", true);
                }
            } else if(colorMode == caLabel::Alarm) {
                int status = ComputeAlarm(w);
                widget->setAlarmColors(status);
            }

            setObjectVisibility(widget, data.edata.rvalue);

        } else {
            SetColorsNotConnected;
        }

        // frame ==================================================================================================================
    } else if(caInclude *widget = qobject_cast<caInclude *>(w)) {
        //qDebug() << "we have an included frame";

        setObjectVisibility(widget, data.edata.rvalue);

        // frame ==================================================================================================================
    } else if(caFrame *widget = qobject_cast<caFrame *>(w)) {
        //qDebug() << "we have a frame";

        setObjectVisibility(widget, data.edata.rvalue);

        // menu ==================================================================================================================
    } else if (caMenu *widget = qobject_cast<caMenu *>(w)) {
        //qDebug() << "we have a menu";

        if(data.edata.connected) {
            QStringList stringlist = String.split( ";");
            // set enum strings
            if(data.edata.fieldtype == caENUM) {
                widget->populateCells(stringlist);
                if(widget->getLabelDisplay()) widget->setCurrentIndex(0);
                else widget->setCurrentIndex((int) data.edata.ivalue);

                if (widget->getColorMode() == caMenu::Alarm) {
                    widget->setAlarmColors(data.edata.severity);
                    // case of static mode
                } else {
                    SetColorsBack;
                }
            }
        } else {
            SetColorsNotConnected;
        }
        widget->setAccessW(data.edata.accessW);

        // choice ==================================================================================================================
    } else if (caChoice *widget = qobject_cast<caChoice *>(w)) {
        //qDebug() << "we have a choiceButton" << String << value;

        if(data.edata.connected) {
            QStringList stringlist = String.split( ";");
            // set enum strings
            if(data.edata.fieldtype == caENUM) {
                widget->populateCells(stringlist, (int) data.edata.ivalue);
                if (widget->getColorMode() == caChoice::Alarm) {
                    widget->setAlarmColors(data.edata.severity);
                    // case of static mode
                } else {
                    SetColorsBack;
                }
            }
        } else {
            SetColorsNotConnected;
        }
        widget->setAccessW(data.edata.accessW);

        // thermometer ==================================================================================================================
    } else if (caThermo *widget = qobject_cast<caThermo *>(w)) {
        //qDebug() << "we have a thermometer";

        if(data.edata.connected) {
            bool channelLimitsEnabled = false;
            if(widget->getLimitsMode() == caThermo::Channel) channelLimitsEnabled= true;
            // take limits from channel, in case of user limits these should already been set
            if((channelLimitsEnabled) && (data.edata.initialize) ) {
                // when limits are the same, do nothing
                if(data.edata.upper_disp_limit != data.edata.lower_disp_limit) {
                    if(widget->getDirection() == caThermo::Down  || widget->getDirection() == caThermo::Left) {
                        widget->setMinValue(data.edata.upper_disp_limit);
                        widget->setMaxValue(data.edata.lower_disp_limit);
                    } else {
                        widget->setMaxValue(data.edata.upper_disp_limit);
                        widget->setMinValue(data.edata.lower_disp_limit);
                    }
                }
            }
            widget->setValue((double) data.edata.rvalue);

            // set colors when connected
            // case of alarm mode
            if (widget->getColorMode() == caThermo::Alarm) {
                if(channelLimitsEnabled) {
                    widget->setAlarmColors(data.edata.severity);
                } else {
                    widget->setUserAlarmColors((double) data.edata.rvalue);
                }

                // case of static mode
            } else {
                SetColorsBack;
            }
            // set no connection color
        } else {
            SetColorsNotConnected;
        }

        // Slider ==================================================================================================================
    } else if (caSlider *widget = qobject_cast<caSlider *>(w)) {

        bool channelLimitsEnabled = false;

        if(data.edata.connected) {
            if(widget->getLimitsMode() == caSlider::Channel) channelLimitsEnabled= true;
            // take limits from channel, in case of user limits these should already been set
            if((channelLimitsEnabled) && (data.edata.initialize) ) {
                // when limits are the same, do nothing
                if(data.edata.upper_disp_limit != data.edata.lower_disp_limit) {
                    if(widget->getDirection() == caSlider::Down  || widget->getDirection() == caSlider::Left) {
                        widget->setMinValue(data.edata.upper_disp_limit);
                        widget->setMaxValue(data.edata.lower_disp_limit);
                    } else {
                        widget->setMaxValue(data.edata.upper_disp_limit);
                        widget->setMinValue(data.edata.lower_disp_limit);
                    }
                }
            }
            widget->setValue((double) data.edata.rvalue);
            widget->setAccessW(data.edata.accessW);

            // set colors when connected
            // case of alarm mode
            if (widget->getColorMode() == caSlider::Alarm) {
                if(channelLimitsEnabled) {
                    widget->setAlarmColors(data.edata.severity);
                } else {
                    widget->setUserAlarmColors((double) data.edata.rvalue);
                }

                // case of static, default mode
            } else {
                SetColorsBack;
            }
            // set no connection color
        } else {
            SetColorsNotConnected;
        }

        // linear gauge (like thermometer) ==================================================================================================================
    } else if (caLinearGauge *widget = qobject_cast<caLinearGauge *>(w)) {
        //qDebug() << "we have a linear gauge" << value;
        Q_UNUSED(widget);
        EAbstractGauge *gauge =  qobject_cast<EAbstractGauge *>(w);
        if(data.edata.connected) {
          if(!gauge->isConnected()) gauge->setConnected(true);
          UpdateGauge(gauge, data);
        } else {
           if(gauge->isConnected()) gauge->setConnected(false);
        }

        // circular gauge  ==================================================================================================================
    } else if (caCircularGauge *widget = qobject_cast<caCircularGauge *>(w)) {
        //qDebug() << "we have a linear gauge" << value;
        Q_UNUSED(widget);
        EAbstractGauge *gauge =  qobject_cast<EAbstractGauge *>(w);
         if(data.edata.connected) {
           if(!gauge->isConnected()) gauge->setConnected(true);
           UpdateGauge(gauge, data);
         } else {
            if(gauge->isConnected()) gauge->setConnected(false);
         }

        // byte ==================================================================================================================
    } else if (caByte *widget = qobject_cast<caByte *>(w)) {

        if(data.edata.connected) {
            int colorMode = widget->getColorMode();
            if(colorMode == caByte::Static) {
                if(!widget->property("Connect").value<bool>()) {
                    widget->setProperty("Connect", true);
                }
                widget->setValue((int) data.edata.ivalue);
            } else if(colorMode == caByte::Alarm) {
                widget->setValue((int) data.edata.ivalue);
                widget->setAlarmColors(data.edata.severity);
            }
        } else {
            SetColorsNotConnected;
        }

        // lineEdit and textEntry ====================================================================================================
    } else if (caLineEdit *widget = qobject_cast<caLineEdit *>(w)) {

        //qDebug() << "we have a linedit or textentry" << widget << data.edata.rvalue <<  data.edata.ivalue;

        QColor bg = widget->property("BColor").value<QColor>();
        QColor fg = widget->property("FColor").value<QColor>();

        if(data.edata.connected) {
            // enum string
            if(data.edata.fieldtype == caENUM || data.edata.fieldtype == caSTRING || data.edata.fieldtype == caCHAR) {
                QStringList list;
                int colorMode = widget->getColorMode();
                if(colorMode == caLineEdit::Static || colorMode == caLineEdit::Default) { // done at initialisation
                    if(!widget->property("Connect").value<bool>()) {                    // but was disconnected before
                        widget->setAlarmColors(data.edata.severity, (double) data.edata.ivalue, bg, fg);
                        widget->setProperty("Connect", true);
                    }
                } else {
                    widget->setAlarmColors(data.edata.severity, (double) data.edata.ivalue, bg, fg);
                }
                list = String.split(";", QString::SkipEmptyParts);
                if((data.edata.fieldtype == caENUM)  && ((int) data.edata.ivalue < list.count())) {
                    widget->setText(list.at((int) data.edata.ivalue).trimmed());
                } else if((data.edata.fieldtype == caENUM)  && ((int) data.edata.ivalue > list.count()) && (data.edata.valueCount == 1)) {
                    widget->setText(list.at(0).trimmed());
                } else if (data.edata.fieldtype == caENUM) {
                    widget->setText("???");
                } else {
                    if(data.edata.valueCount == 1) {
                        widget->setText(String);
                    } else if(list.count() > 0) {
                        widget->setText(list.at(0).trimmed());
                    }
                }
                widget->setCursorPosition(0);
                // access control for textentry
                if (caTextEntry *widget = qobject_cast<caTextEntry *>(w)) {
                    widget->setAccessW(data.edata.accessW);
                }

                // double
            } else {
                int colorMode = widget->getColorMode();
                int precMode = widget->getPrecisionMode();

                if(colorMode == caLineEdit::Static || colorMode == caLineEdit::Default) { // done at initialisation
                    if(!widget->property("Connect").value<bool>()) {                      // but was disconnected before
                        widget->setAlarmColors(data.edata.severity, data.edata.rvalue, bg, fg);
                        widget->setProperty("Connect", true);
                    }
                } else {
                    widget->setAlarmColors(data.edata.severity, data.edata.rvalue, bg, fg);
                }

                if((precMode != caLineEdit::User) && (data.edata.initialize)) {
                    widget->setFormat(data.edata.precision);
                }

                widget->setValue(data.edata.rvalue, units);
                // access control for textentry
                if (caTextEntry *widget = qobject_cast<caTextEntry *>(w)) {
                    widget->setAccessW(data.edata.accessW);
                }
            }

        } else {
            widget->setText("");
            widget->setAlarmColors(NOTCONNECTED, 0.0, bg, fg);        \
            widget->setProperty("Connect", false);
        }
        if (caTextEntry *widget = qobject_cast<caTextEntry *>(w)) {
            widget->updateText(widget->text());
        }

        // Graphics ==================================================================================================================
    } else if (caGraphics *widget = qobject_cast<caGraphics *>(w)) {
        //qDebug() << "caGraphics" << widget->objectName() << widget->getColorMode() << data.pv;

        if(data.edata.connected) {
            int colorMode = widget->getColorMode();
            if(colorMode == caGraphics::Static) {
                // done at initialisation, we have to set it back after no connect
                if(!widget->property("Connect").value<bool>()) {
                    QColor fg = widget->property("FColor").value<QColor>();
                    widget->setForeground(fg);
                    widget->setLineColor(fg);
                    widget->setProperty("Connect", true);
                }
            } else if(colorMode == caGraphics::Alarm) {
                int status = ComputeAlarm(w);
                widget->setAlarmColors(status);
            }

            setObjectVisibility(widget, data.edata.rvalue);

        } else {
            SetColorsNotConnected;
        }

        // Polyline ==================================================================================================================
    } else if (caPolyLine *widget = qobject_cast<caPolyLine *>(w)) {

        if(data.edata.connected) {
            int colorMode = widget->getColorMode();
            if(colorMode == caPolyLine::Static) {
                // done at initialisation, we have to set it back after no connect
                if(!widget->property("Connect").value<bool>()) {
                    QColor fg = widget->property("FColor").value<QColor>();
                    widget->setForeground(fg);
                    widget->setLineColor(fg);
                    widget->setProperty("Connect", true);
                }
            } else if(colorMode == caPolyLine::Alarm) {
                int status = ComputeAlarm(w);
                widget->setAlarmColors(status);
            }

            setObjectVisibility(widget, data.edata.rvalue);

        } else {
            SetColorsNotConnected;
        }

        // Led ==================================================================================================================
    } else if (caLed *widget = qobject_cast<caLed *>(w)) {
        //qDebug() << "led" << led->objectName();

        if(data.edata.connected) {
            int colorMode = widget->getColorMode();
            if(colorMode == caLed::Static) {
                if(bitState((int) data.edata.ivalue, widget->getBitNr())) {
                    widget->setState(true);
                } else {
                    widget->setState(false);
                }
            } else {
                widget->setAlarmColors(data.edata.severity);
            }
        } else {
            widget->setAlarmColors(NOTCONNECTED);
        }

        // ApplyNumeric and Numeric =====================================================================================================
    } else if (caApplyNumeric *widget = qobject_cast<caApplyNumeric *>(w)) {
        //qDebug() << "caApplyNumeric" << wheel->objectName() << kPtr->pv << data.edata.monitorCount;

        if(data.edata.connected) {
            ComputeNumericMaxMinPrec(widget, data);
            widget->setConnectedColors(true);
            widget->setValue(data.edata.rvalue);
            widget->setAccessW(data.edata.accessW);
        } else {
            widget->setConnectedColors(false);
        }

        // Numeric =====================================================================================================
    } else if (caNumeric *widget = qobject_cast<caNumeric *>(w)) {
        //qDebug() << "caNumeric" << widget->objectName() << data.pv;

        if(data.edata.connected) {
            ComputeNumericMaxMinPrec(widget, data);
            widget->setConnectedColors(true);
            widget->silentSetValue(data.edata.rvalue);
            widget->setAccessW(data.edata.accessW);
        } else {
            widget->setConnectedColors(false);
        }

        // Toggle =====================================================================================================
    } else if (caToggleButton *widget = qobject_cast<caToggleButton *>(w)) {
        //qDebug() << "caToggleButton" << widget->objectName() << data.pv;

        if(data.edata.connected) {
            int colorMode = widget->getColorMode();
            if(colorMode == caToggleButton::Alarm) {
                widget->setAlarmColors(data.edata.severity);
            } else {
                SetColorsBack;
            }

            if( data.edata.ivalue > 0) {
                widget->setState(true);
            } else {
                widget->setState(false);
            }

            widget->setAccessW(data.edata.accessW);
        } else {
            SetColorsNotConnected;
        }

        // cartesian plot ==================================================================================================================
    } else if (caCartesianPlot *widget = qobject_cast<caCartesianPlot *>(w)) {
        //qDebug() << "caCartesianPlot" << widget->objectName() << data.pv << data.specData[0] << data.specData[1]  << data.specData[2];

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
                if(XorY == caCartesianPlot::CH_X && widget->getXscaling() == caCartesianPlot::Channel) {
                    if(data.edata.lower_disp_limit != data.edata.upper_disp_limit) {
                        widget->setScaleX(data.edata.lower_disp_limit, data.edata.upper_disp_limit);
                    } else {
                        widget->setXscaling(caCartesianPlot::Auto);
                    }
                } else if(XorY == caCartesianPlot::CH_Y && widget->getYscaling() == caCartesianPlot::Channel) {
                    if(data.edata.lower_disp_limit != data.edata.upper_disp_limit) {
                        widget->setScaleY(data.edata.lower_disp_limit, data.edata.upper_disp_limit);
                    } else {
                        widget->setYscaling(caCartesianPlot::Auto);
                    }
                }
            }

            // monitor for scale will change the above set limits
            if((XorY == caCartesianPlot::CH_Xscale) || (XorY == caCartesianPlot::CH_Yscale)) {
                if(XorY == caCartesianPlot::CH_Xscale) {
                    widget->setScaleXlimits(data.edata.rvalue, curvNB);
                } else {
                    widget->setScaleYlimits(data.edata.rvalue, curvNB);
                }
                // qDebug() << "scale monitor from" << data.pv << data.edata.rvalue << "min/max" << data.specData[0] << "scale" << XorY;
            }

            if(!widget->property("Connect").value<bool>()) {
                widget->setProperty("Connect", true);
                widget->setAllProperties();
            }

            // value(s)
            if(XorY == caCartesianPlot::CH_X || XorY == caCartesianPlot::CH_Y) {
                // data from vector
                if(data.edata.valueCount > 0 && data.edata.dataB != (void*) 0) {
                    QVector<double> y;
                    QMutex *datamutex;
                    datamutex = (QMutex*) data.mutex;
                    datamutex->lock();
                    y.clear();
                    y.reserve(data.edata.valueCount);
                    BuildVector(y);
                    datamutex->unlock();
                    widget->setData(y, curvNB, curvType, XorY);
                    // data from value
                } else {
                    QVector<double> y;
                    y.append(data.edata.rvalue);
                    widget->setData(y, curvNB, curvType, XorY);
                }

                // trigger channel
            } else if(XorY == caCartesianPlot::CH_Trigger) {
                QVector<double> y;
                for(int i=0; i < 4; i++) widget->setData(y, i, curvType, XorY);

                // count channel
            } else if(XorY == caCartesianPlot::CH_Count) {
                //qDebug() << "count channel" << data.edata.rvalue << (int) (data.edata.rvalue + 0.5);
                widget->setCountNumber((int) (data.edata.rvalue + 0.5));

                // erase channel
            } else if(XorY == caCartesianPlot::CH_Erase) {
                if(widget->getEraseMode() == caCartesianPlot::ifnotzero) {
                    if((int) data.edata.rvalue != 0) widget->erasePlots();
                } else if(widget->getEraseMode() == caCartesianPlot::ifzero){
                    if((int) data.edata.rvalue == 0) widget->erasePlots();
                }
            }

            // not connected
        } else {
            widget->setCountNumber(0);
            widget->setWhiteColors();
            widget->setProperty("Connect", false);
        }

        // stripchart ==================================================================================================================
    } else if(caStripPlot *widget = qobject_cast<caStripPlot *>(w)) {

        int actPlot= data.specData[1];
        if(data.edata.connected) {

            // scaling
            if(data.edata.initialize) {
                if(widget->getYscalingMin(actPlot) == caStripPlot::Channel) {
                    widget->setYaxisLimitsMin(actPlot, data.edata.lower_disp_limit);
                }
                if(widget->getYscalingMax(actPlot) == caStripPlot::Channel) {
                    widget->setYaxisLimitsMax(actPlot, data.edata.upper_disp_limit);
                }
                if(actPlot == 0) {
                    double ymin = widget->getYaxisLimitsMin(0);
                    double ymax = widget->getYaxisLimitsMax(0);
                    widget->setYscale(ymin, ymax);
                }
                // do this for redisplaying legend with correct limits
                if((widget->getYscalingMin(actPlot) == caStripPlot::Channel) ||
                        (widget->getYscalingMax(actPlot) == caStripPlot::Channel)) {
                    widget->resize(widget->width()+1, widget->height()+1);
                    widget->resize(widget->width()-1, widget->height()-1);
                }
            }

            widget->setData(data.edata.actTime, data.edata.rvalue, actPlot);

            if(data.edata.initialize) {
                widget->startPlot();
            }

        }

        // animated gif ==================================================================================================================
    } else if(caImage *widget = qobject_cast<caImage *>(w)) {

        double valueArray[MAX_CALC_INPUTS];
        char post[256];
        char calcString[256];
        long status;
        short errnum;
        double result;

        //qDebug() << "we have a image" << widget << kPtr->pv << data.edata.fieldtype << data.edata.dataSize << String;

        if(!data.edata.connected) {
            widget->setInvalid(Qt::white);
            return;
        } else {
            widget->setValid();
        }

        // treat visibility if defined, do nothing when not visible
        if(!setObjectVisibility(widget, data.edata.rvalue)) return; // do nothing when not visible

        // treat image frame number, if non dynamic image, do nothing
        if(widget->getFrameCount() <= 1) {
            return;
            // empty calc string, animation was already set
        } else if(widget->getImageCalc().size() == 0) {
            return;
        } else {
            // any monitors ?
            QVariant var=widget->property("MonitorList");
            QVariantList list = var.toList();
            int nbMonitors = list.at(0).toInt();
            //qDebug() << image << "number of monitors" << nbMonitors;
            if(nbMonitors > 0)  {

                // get calc string
                //printf("get calc string <%s>\n", qPrintable(widget->getImageCalc()));
                strcpy(calcString, (char*) widget->getImageCalc().toAscii().constData());

                // scan and get the channels
                for(int i=0; i < 4; i++) valueArray[i] = 0.0;
                for(int i=0; i< nbMonitors;i++) {
                    knobData *ptr = mutexKnobData->GetMutexKnobDataPtr(list.at(i+1).toInt());
                    //qDebug() << ptr->pv << ptr->edata.connected << ptr->edata.value;
                    // when connected
                    if(ptr->edata.connected) {
                        valueArray[i] = ptr->edata.rvalue;
                    } else {
                        valueArray[i] = 0.0;
                    }
                }

                status = postfix(calcString, post, &errnum);
                if(status) {
                    widget->setInvalid(Qt::black);
                    //qDebug() << "invalid calc 1" << calcString;
                }

                // Perform the calculation
                status = calcPerform(valueArray, &result, post);
                if(!status) {
                    // Result is valid, convert to frame number
                    if(result < 0.0) {
                        widget->setInvalid(Qt::black);
                        //printf("no valid frame\n");
                    } else {
                        //printf("frame ok=%d\n", (int)(result +.5));
                        widget->setFrame((int)(result +.5));
                    }
                } else {
                    //printf("invalid calc 2\n");
                    widget->setInvalid(Qt::black);
                }

                // no monitors
            } else {

            }
        }

        // table with pv name, value and unit==========================================================================
    } else if(caTable *widget = qobject_cast<caTable *>(w)) {

        //qDebug() << "table" << table->objectName() << kPtr->pv << data.edata.value << data.edata.connected;
        int row= data.specData[0];

        if(data.edata.connected) {
            // enum string
            if(data.edata.fieldtype == caENUM || data.edata.fieldtype == caSTRING) {
                int colorMode = widget->getColorMode();
                if(colorMode == caTable::Alarm) {
                    widget->displayText(row, 1, data.edata.severity, String);
                } else {
                    widget->displayText(row, 1, 0, String);
                }
                // double
            } else {
                if(data.edata.initialize) {
                    widget->setFormat(row, data.edata.precision);
                }
                widget->setValue(row, 1, data.edata.severity, data.edata.rvalue, data.edata.units);
            }

        } else {
            widget->displayText(row, 1, NOTCONNECTED, "");
            widget->displayText(row, 2, NOTCONNECTED, "");
        }

        // bitnames table with text and coloring according the value=========================================================
    } else if (caBitnames *widget = qobject_cast<caBitnames *>(w)) {
        if(data.edata.connected) {
            // set enum strings
            if(data.edata.fieldtype == caENUM) {
                widget->setEnumStrings(String);
            } else if(data.edata.fieldtype == caINT || data.edata.fieldtype == caLONG ) {
                widget->setValue(data.edata.ivalue);
            } else if(data.edata.fieldtype == caFLOAT || data.edata.fieldtype == caDOUBLE ) {
                widget->setValue((int) (data.edata.rvalue + 0.5));
            }
        } else {
            // todo
        }

        // camera =========================================================
    } else if (caCamera *widget = qobject_cast<caCamera *>(w)) {

        //qDebug() << data.pv << data.edata.connected << data.specData[0];
        if(data.edata.connected) {
            if(data.specData[0] == 1) {        // width channel
                widget->setWidth((int) data.edata.rvalue);
            } else if(data.specData[0] == 2) { // height channel
                widget->setHeight((int) data.edata.rvalue);
            } else if(data.specData[0] == 3) { // code channel if present
                widget->setCode((int) data.edata.rvalue);
            } else if(data.specData[0] == 4) { // bpp channel if present
                widget->setBPP((int) data.edata.rvalue);
            } else if(data.specData[0] == 5) { // minimum level channel if present
                widget->updateMin((int) data.edata.rvalue);
            } else if(data.specData[0] == 6) { // maximum level channel if present
                widget->updateMax((int) data.edata.rvalue);
            } else if(data.specData[0] == 7) { // x center of mass if present
                widget->dataProcessing((int) data.edata.rvalue, 0);
            } else if(data.specData[0] == 8) { // y center of mass if present
                widget->dataProcessing((int) data.edata.rvalue, 1);
            } else if(data.specData[0] == 9) { // width if present
                widget->dataProcessing((int) data.edata.rvalue, 2);
            } else if(data.specData[0] == 10) { // height if present
                widget->dataProcessing((int) data.edata.rvalue, 3);
            } else if(data.specData[0] == 0) { // data channel
                QMutex *datamutex;
                datamutex = (QMutex*) data.mutex;
                datamutex->lock();
                widget->showImage(data.edata.dataSize, (char*) data.edata.dataB);
                datamutex->unlock();
            }

        } else {
            // todo
        }

        // messagebutton, yust treat access ==========================================================================
    } else if (caMessageButton *widget = qobject_cast<caMessageButton *>(w)) {

        if(data.edata.connected) {
            if (widget->getColorMode() == caMessageButton::Alarm) {
                widget->setAlarmColors(data.edata.severity);
                // case of static mode
            } else {
                SetColorsBack;
            }

        } else {
            SetColorsNotConnected;
        }
        widget->setAccessW(data.edata.accessW);

        // something else (user defined monitors with non ca widgets ?) ==============================================
    } else {
        //qDebug() << "unrecognized widget" << w->metaObject()->className();
    }
}


/**
 * callback will write value to device
 */
void CaQtDM_Lib::Callback_EApplyNumeric(double value)
{
    int32_t idata = (int32_t) value;
    float rdata = (float) value;

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
    float rdata = (float) value;
    caNumeric *numeric = qobject_cast<caNumeric *>(sender());
    if(!numeric->getAccessW()) return;
    if(numeric->getPV().length() > 0) {
        TreatOrdinaryValue(numeric->getPV(), rdata,  idata, (QWidget*) numeric);
    }
}

/**
 * callback will write value to device
 */
void CaQtDM_Lib::Callback_Slider(double value)
{
    int32_t idata = (int32_t) value;
    float rdata = (float) value;
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
    float value;
    int32_t idata;
    QString text;
    char errmess[255];
    caToggleButton *w = qobject_cast<caToggleButton *>(sender());
    if(!w->getAccessW()) return;

    if(!type) {         // toggle cleared
        value=0;
    } else  {           // toggle checked
        value=1;
    }
    idata = (int32_t) value;

    if(w->getPV().length() > 0) {
        QStringsToChars(w->getPV(), text,  w->objectName().toLower());
        EpicsSetValue(param1, value, idata, param2, param3, errmess, 2);
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
        EpicsSetValue(param1, 0.0, 0, param2, param3, errmess, 0);
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
        EpicsSetValue(param1, 0.0, 0, param2, param3, errmess, 0);
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

    TreatRequestedValue(text, fType, w1);
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

    if(type == 0) {         // pressed
        if(w->getPressMessage().size() > 0)
            TreatRequestedValue(w->getPressMessage(), caTextEntry::decimal, w1);
    } else if(type == 1) {  // released
        if(w->getReleaseMessage().size() > 0)
            TreatRequestedValue(w->getReleaseMessage(), caTextEntry::decimal, w1);
    }
}

/**
 * callback will execute script
 */
void CaQtDM_Lib::Callback_ScriptButton()
{
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
}

void CaQtDM_Lib::processTerminated()
{
    // qDebug() << "caQtDM -- process terminated callback";
    processWindow *t = qobject_cast<processWindow *>(sender());
    QWidget *w = t->getProcessCaller();
    caScriptButton *w1 = qobject_cast<caScriptButton *>(w);
    if(w1 != (QWidget*) 0) {
        w1->setToolTip("process terminated !");
        w1->setAccessW(true);
        w->setEnabled(true);
    }
    //qDebug() << "clean up";

    delete t;
}

/**
 * callback will execute a shell command
 */
void CaQtDM_Lib::Callback_ShellCommandClicked(int indx)
{
    QString separator((QChar)27);
#ifndef linux
    proc = new QProcess( this);
    proc->setWorkingDirectory(".");
    proc->setProcessChannelMode(QProcess::MergedChannels);
    QObject::connect( proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
#endif
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
        proc->start(command.trimmed(), QIODevice::ReadWrite);
#else
        // I had too many problems with QProcess start, use standard execl
        if(!command.endsWith("&")) command.append("&");
        //qDebug() << "execute:" << command;
        int status = Execute((char*)command.toAscii().constData());
        if(status != 0) {
            QMessageBox::information(0,"FailedToStart or Error", command);
        }
#endif
    } else if(indx < commands.count()) {
        QString command;
        command.append(commands[indx].trimmed());
        command.replace("&T", thisFileShort);
        command.replace("&A", thisFileFull);
        postMessage(QtDebugMsg, (char*) qPrintable(command.trimmed()));
#ifndef linux
        if(command.endsWith("&")) command.remove(command.size()-1, 1);
        //qDebug() << "execute:" << command;
        proc->start(command.trimmed(), QIODevice::ReadWrite);
#else
        // I had too many problems with QProcess start, use standard execl
        if(!command.endsWith("&")) command.append("&");
        //qDebug() << "execute:" << command;
        int status = Execute((char*)command.toAscii().constData());
        if(status != 0) {
            QMessageBox::information(0,"FailedToStart or error", command);
        }
#endif
    }

}

void CaQtDM_Lib::processError(QProcess::ProcessError err)
{
    switch(err)
    {
    case QProcess::FailedToStart:
        QMessageBox::information(0,"FailedToStart","FailedToStart");
        break;
    case QProcess::Crashed:
        QMessageBox::information(0,"Crashed","Crashed");
        break;
    case QProcess::Timedout:
        QMessageBox::information(0,"FailedToStart","FailedToStart");
        break;
    case QProcess::WriteError:
        QMessageBox::information(0,"Timedout","Timedout");
        break;
    case QProcess::ReadError:
        QMessageBox::information(0,"ReadError","ReadError");
        break;
    case QProcess::UnknownError:
        QMessageBox::information(0,"UnknownError","UnknownError");
        break;
    default:
        QMessageBox::information(0,"default","default");
        break;
    }
}

/**
  * when closing the window, we will clear all associated monitors and free data
  */
void CaQtDM_Lib::closeEvent(QCloseEvent* ce)
{
    Q_UNUSED(ce);
    for(int i=0; i < mutexKnobData->GetMutexKnobDataSize(); i++) {

        knobData kData =  mutexKnobData->GetMutexKnobData(i);

        if((kData.index != -1) && (myWidget == kData.thisW)) {
            QString pv = kData.pv;
            QWidget* w = (QWidget*) kData.thisW;
            short soft = kData.soft;
            //qDebug() << "clear monitor at" << i << "index="  << kData.index;
            if(soft) mutexKnobData->RemoveSoftPV(pv, (QWidget*) w, kData.index);
            ClearMonitor(&kData);
            mutexKnobData->SetMutexKnobData(i, kData);
        }
    }

#ifdef linux
    usleep(200000);
#else
    Sleep::msleep(200);
#endif

    // get rid of memory, that was allocated before for this window.
    // it has not been done previously, while otherwise in the datacallback
    // you will run into trouble
    for(int i=0; i < mutexKnobData->GetMutexKnobDataSize(); i++) {
        knobData *kPtr = mutexKnobData->GetMutexKnobDataPtr(i);
        if(myWidget == kPtr->thisW) {
            if (kPtr->edata.info != (connectInfoShort *) 0) {
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

#ifdef epics4
    delete Epics4;
#endif

}

/**
  * will display a context menu, composed of the channels associated to the object
  */
void CaQtDM_Lib::DisplayContextMenu(QWidget* w)
{
    QMenu myMenu;
    QPoint pos =QCursor::pos() ;

    QString pv[20];
    int nbPV = 1;
    int limitsDefault = false;
    int precMode = false;
    int limitsMode = false;
    int Precision = 0;
    const char *caTypeStr[] = {"DBF_STRING", "DBF_INT", "DBF_FLOAT", "DBF_ENUM", "DBF_CHAR", "DBF_LONG", "DBF_DOUBLE"};
    char colMode[20] = {""};
    double limitsMax=0.0, limitsMin=0.0;

    if(caImage* widget = qobject_cast<caImage *>(w)) {
        pv[0] = widget->getChannelA().trimmed();
        pv[1] = widget->getChannelB().trimmed();
        pv[2] = widget->getChannelC().trimmed();
        pv[3] = widget->getChannelB().trimmed();
        nbPV = 4;
    } else if(caFrame* widget = qobject_cast<caFrame *>(w)) {
        pv[0] = widget->getChannelA().trimmed();
        pv[1] = widget->getChannelB().trimmed();
        pv[2] = widget->getChannelC().trimmed();
        pv[3] = widget->getChannelB().trimmed();
        nbPV = 4;
    } else if(caInclude* widget = qobject_cast<caInclude *>(w)) {
        pv[0] = widget->getChannelA().trimmed();
        pv[1] = widget->getChannelB().trimmed();
        pv[2] = widget->getChannelC().trimmed();
        pv[3] = widget->getChannelB().trimmed();
        nbPV = 4;
    } else if(caLabel* widget = qobject_cast<caLabel *>(w)) {
        pv[0] = widget->getChannelA().trimmed();
        pv[1] = widget->getChannelB().trimmed();
        pv[2] = widget->getChannelC().trimmed();
        pv[3] = widget->getChannelB().trimmed();
        nbPV = 4;
    } else if(caMenu* widget = qobject_cast<caMenu *>(w)) {
        pv[0] = widget->getPV().trimmed();
    } else if(caChoice* widget = qobject_cast<caChoice *>(w)) {
        pv[0] = widget->getPV().trimmed();
        if(widget->getColorMode() == caChoice::Alarm) strcpy(colMode, "Alarm");
        else strcpy(colMode, "Static");
    } else if(caTextEntry* widget = qobject_cast<caTextEntry *>(w)) {
        pv[0] = widget->getPV().trimmed();
    } else if(caLineEdit* widget = qobject_cast<caLineEdit *>(w)) {
        pv[0] = widget->getPV().trimmed();
        if(widget->getPrecisionMode() == caLineEdit::User) {
            precMode = true;
            Precision = widget->getPrecision();
        }
        if(widget->getLimitsMode() == caLineEdit::User) {
            limitsMode = true;
            limitsMax = widget->getMaxValue();
            limitsMin = widget->getMinValue();
        }
        if(widget->getColorMode() == caLineEdit::Alarm_Default) strcpy(colMode, "Alarm");
        else if(widget->getColorMode() == caLineEdit::Alarm_Static) strcpy(colMode, "Alarm");
        else strcpy(colMode, "Static");

    } else if(caGraphics* widget = qobject_cast<caGraphics *>(w)) {
        pv[0] = widget->getChannelA().trimmed();
        pv[1] = widget->getChannelB().trimmed();
        pv[2] = widget->getChannelC().trimmed();
        pv[3] = widget->getChannelB().trimmed();
        nbPV = 4;
        if(widget->getColorMode() == caGraphics::Alarm) strcpy(colMode, "Alarm");
        else strcpy(colMode, "Static");
    } else if(caPolyLine* widget = qobject_cast<caPolyLine *>(w)) {
        pv[0] = widget->getChannelA().trimmed();
        pv[1] = widget->getChannelB().trimmed();
        pv[2] = widget->getChannelC().trimmed();
        pv[3] = widget->getChannelB().trimmed();
        nbPV = 4;
        if(widget->getColorMode() == caPolyLine::Alarm) strcpy(colMode, "Alarm");
        else strcpy(colMode, "Static");
    } else if (caApplyNumeric* widget = qobject_cast<caApplyNumeric *>(w)) {
        pv[0] = widget->getPV().trimmed();
    } else if (caNumeric* widget = qobject_cast<caNumeric *>(w)) {
        pv[0] = widget->getPV();
    } else if (caMessageButton* widget = qobject_cast<caMessageButton *>(w)) {
        pv[0] = widget->getPV().trimmed();
    } else if(caLed* widget = qobject_cast<caLed *>(w)) {
        pv[0] = widget->getPV().trimmed();
    } else if(caBitnames* widget = qobject_cast<caBitnames *>(w)) {
        pv[0] = widget->getEnumPV().trimmed();
        pv[1] = widget->getValuePV().trimmed();
        nbPV = 2;
    } else if(caSlider* widget = qobject_cast<caSlider *>(w)) {
        pv[0] = widget->getPV().trimmed();
        if(widget->getLimitsMode() == caSlider::User) {
            limitsMode = true;
            limitsMax = widget->getMaxValue();
            limitsMin = widget->getMinValue();
        }
        if(widget->getColorMode() == caSlider::Alarm) strcpy(colMode, "Alarm");
        else strcpy(colMode, "Static");
    } else if(caThermo* widget = qobject_cast<caThermo *>(w)) {
        pv[0] = widget->getPV().trimmed();
        if(widget->getLimitsMode() == caThermo::User) {
            limitsMode = true;
            limitsMax = widget->maxValue();
            limitsMin = widget->minValue();
        }
        knobData *kPtr = mutexKnobData->getMutexKnobDataPV(pv[0]);
        if(kPtr->edata.lower_disp_limit == kPtr->edata.upper_disp_limit) {
            limitsDefault = true;
            limitsMax = widget->maxValue();
            limitsMin = widget->minValue();
        }
        if(widget->getColorMode() == caThermo::Alarm) strcpy(colMode, "Alarm");
        else strcpy(colMode, "Static");
    } else if(caLinearGauge* widget = qobject_cast<caLinearGauge *>(w)) {
        pv[0] = widget->getPV().trimmed();
    } else if(caCircularGauge* widget = qobject_cast<caCircularGauge *>(w)) {
        pv[0] = widget->getPV().trimmed();
    } else if(caByte* widget = qobject_cast<caByte *>(w)) {
        pv[0] = widget->getPV().trimmed();
        if(widget->getColorMode() == caByte::Alarm) strcpy(colMode, "Alarm");
        else strcpy(colMode, "Static");
    } else if(caStripPlot* widget = qobject_cast<caStripPlot *>(w)) {
        QString pvs = widget->getPVS();
        QStringList vars = pvs.split(";", QString::SkipEmptyParts);
        nbPV = min(vars.count(), caStripPlot::MAXCURVES);
        for(int i=0; i<nbPV; i++) {
            pv[i] = vars.at(i).trimmed();
        }
    } else if(caCartesianPlot* widget = qobject_cast<caCartesianPlot *>(w)) {
        nbPV = 0;
        for(int i=0; i < caCartesianPlot::curveCount; i++) {
            QStringList thisString;
            thisString = widget->getPV(i).split(";");
            if(thisString.count() == 2 && thisString.at(0).trimmed().length() > 0 && thisString.at(1).trimmed().length() > 0) {
                pv[nbPV++] = thisString.at(0).trimmed();
                pv[nbPV++] = thisString.at(1).trimmed();
            } else if(thisString.count() == 2 && thisString.at(0).trimmed().length() > 0 && thisString.at(1).trimmed().length() == 0) {
                pv[nbPV++] = thisString.at(0).trimmed();
            } else if(thisString.count() == 2 && thisString.at(1).trimmed().length() > 0 && thisString.at(0).trimmed().length() == 0) {
                pv[nbPV++] = thisString.at(1).trimmed();
            }
        }
        QString TriggerPV = widget->getTriggerPV();
        if(TriggerPV.trimmed().length() > 0) pv[nbPV++] = TriggerPV.trimmed();
        QString CountPV = widget->getCountPV();
        if(CountPV.trimmed().length() > 0) pv[nbPV++] = CountPV.trimmed();
        QString ErasePV = widget->getErasePV();
        if(ErasePV.trimmed().length() > 0) pv[nbPV++] = ErasePV.trimmed();
    } else if(caCamera* widget = qobject_cast<caCamera *>(w)) {
        nbPV=0;
        for(int i=0; i< 5; i++) {
            QString text;
            if(i==0) text = widget->getPV_Data();
            if(i==1) text = widget->getPV_Width();
            if(i==2) text = widget->getPV_Height();
            if(i==3) text = widget->getPV_Code();
            if(i==4) text = widget->getPV_BPP();
            if(text.size() > 0) {
                pv[nbPV] = text;
                nbPV++;
            }
        }
    } else if(caCalc* widget = qobject_cast<caCalc *>(w)) {
        pv[0] = widget->getVariable().toAscii().constData();
        pv[1] = widget->getChannelA().trimmed();
        pv[2] = widget->getChannelB().trimmed();
        pv[3] = widget->getChannelC().trimmed();
        pv[4] = widget->getChannelB().trimmed();
        nbPV = 5;


    } else if(caScriptButton* widget =  qobject_cast< caScriptButton *>(w)) {
        Q_UNUSED(widget);
        // add acion : kill associated process if running
        if(!widget->getAccessW()) myMenu.addAction("Kill Process");

    } else  {
        qDebug() << w << "not treated";
        return;
    }

    if(caScriptButton* widget =  qobject_cast< caScriptButton *>(w)) {
        Q_UNUSED(widget);
    } else if(caCamera * widget = qobject_cast< caCamera *>(w)) {
        Q_UNUSED(widget);
        myMenu.addAction("Toggle Zoom");
        myMenu.addAction("Set Spectrum");
        myMenu.addAction("Set Greyscale");
        myMenu.addAction("Get Info");
        myMenu.addAction("Print");
        myMenu.addAction("Raise main window");
    } else {
        // construct info for the pv we are pointing at
        myMenu.addAction("Get Info");
        myMenu.addAction("Print");
        myMenu.addAction("Raise main window");
    }

    if(caStripPlot* widget = qobject_cast<caStripPlot *>(w)) {
        Q_UNUSED(widget);
        myMenu.addAction("Change Axis");
    }
    if(caCartesianPlot* widget = qobject_cast<caCartesianPlot *>(w)) {
        Q_UNUSED(widget);
        myMenu.addAction("Change Axis");
    }

    QAction* selectedItem = myMenu.exec(pos);

    if (selectedItem) {
        if(selectedItem->text().contains("Kill Process")) {
            if(caScriptButton* widget =  qobject_cast< caScriptButton *>(w)) {
                processWindow *t= (processWindow *) widget->getProcess();
                t->tryTerminate();
            }

        } else  if(selectedItem->text().contains("Raise main window")) {
            QMainWindow *mainWindow = (QMainWindow *) this->parentWidget();
            mainWindow->showNormal();
            messageWindow->raise();

        } else  if(selectedItem->text().contains("Toggle Zoom")) {
            if(caCamera * widget = qobject_cast< caCamera *>(w)) {
                if(widget->getZoom() == caCamera::Yes) widget->setZoom(caCamera::No);
                else widget->setZoom(caCamera::Yes);
            }

        } else  if(selectedItem->text().contains("Set Spectrum")) {
            if(caCamera * widget = qobject_cast< caCamera *>(w)) widget->setColormap(caCamera::spectrum);

        } else  if(selectedItem->text().contains("Set Greyscale")) {
           if(caCamera * widget = qobject_cast< caCamera *>(w)) widget->setColormap(caCamera::grey);

        } else  if(selectedItem->text().contains("Get Info")) {
            QString info;
            info.append(InfoPrefix);
            info.append("-------------------------------------------------<br>");
            info.append("Object: ");
            info.append(w->objectName());
            info.append("<br>");
            for(int i=0; i< nbPV; i++) {
                // is there a json string ?
                int pos = pv[i].indexOf("{");
                if(pos != -1) pv[i] = pv[i].mid(0, pos);
                knobData *kPtr = mutexKnobData->getMutexKnobDataPV(pv[i]);  // use pointer for getting all necessary information
                if((kPtr != (knobData*) 0) && (pv[i].length() > 0)) {
                    char asc[255];
                    info.append("<br>");
                    info.append(kPtr->pv);

                    connectInfoShort *tmp = (connectInfoShort *) kPtr->edata.info;
                    if (tmp != (connectInfoShort *) 0) {
                        if(tmp->connected) {
                            info.append(" : connected");
                        } else if(kPtr->soft) {
                            info.append(" : soft channel from caCalc");
                        } else {
                            if(tmp->cs != 1) { // epics
                                info.append(" : not connected");
                            } else {
                                info.append(" : acs device");
                            }
                        }
                    }

                    info.append("<br>=========================<br>Type: ");
                    info.append(caTypeStr[kPtr->edata.fieldtype]);

                    sprintf(asc,"<br>Count: %d", kPtr->edata.valueCount);
                    info.append(asc);

                    info.append("<br>Value: ");
                    switch (kPtr->edata.fieldtype) {
                    case caCHAR:
                        break;
                    case caSTRING:
                        info.append((char*)kPtr->edata.dataB);
                        break;

                    case caENUM:{
                        sprintf(asc,"%ld %s", kPtr->edata.ivalue, kPtr->edata.units);
                        info.append(asc);
                        sprintf(asc,"<br>nbStates: %d", kPtr->edata.enumCount);
                        info.append(asc);
                        info.append("<br>States: ");
                        if(kPtr->edata.enumCount > 0) {
                           QString States((char*) kPtr->edata.dataB);
                           QStringList list = States.split(";");
                           for(int i=0; i<list.count(); i++) {
                            sprintf(asc, "<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;%d %s", i, qPrintable(list.at(i)));
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

                    // limits
                    if(limitsMode) {
                        sprintf(asc,"<br>User alarm: MIN:%.*f  MAX:%.*f ", Precision, limitsMin, Precision, limitsMax);
                        info.append(asc);
                    }
                    if(limitsDefault) {
                        sprintf(asc,"<br>Default limits: MIN:%.*f  MAX:%.*f ", Precision, limitsMin, Precision, limitsMax);
                        info.append(asc);
                    }
                    sprintf(asc,"<br>LOPR:%.*f  HOPR:%.*f ", Precision, kPtr->edata.lower_disp_limit,
                            Precision, kPtr->edata.upper_disp_limit);
                    info.append(asc);
                    sprintf(asc,"<br>LOLO:%.*f  HIHI:%.*f ", Precision,kPtr->edata.lower_alarm_limit,
                            Precision, kPtr->edata.upper_alarm_limit);
                    info.append(asc);
                    sprintf(asc,"<br>LOW :%.*f  HIGH:%.*f ", Precision, kPtr->edata.lower_warning_limit,
                            Precision, kPtr->edata.upper_warning_limit);
                    info.append(asc);
                    sprintf(asc,"<br>DRVL:%.*f  DRVH:%.*f ", Precision,kPtr->edata.lower_ctrl_limit,
                            Precision, kPtr->edata.upper_ctrl_limit);
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
            info.append(InfoPostfix);

            myMessageBox box;
            box.setText("<html>" + info + "</html>");
            box.exec();

        } else if(selectedItem->text().contains("Print")) {
            print();
        } else if(selectedItem->text().contains("Change Axis")) {
            if(caStripPlot* widget = qobject_cast<caStripPlot *>(w)) {
                limitsStripplotDialog dialog(widget, mutexKnobData, "stripplot modifications", this);
                dialog.exec();
            } else if(caCartesianPlot* widget = qobject_cast<caCartesianPlot *>(w)) {
                limitsCartesianplotDialog dialog(widget, mutexKnobData, "cartesianplot modifications", this);
                dialog.exec();
            }
        }
    } else {
        // nothing was chosen
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

void CaQtDM_Lib::mouseReleaseEvent(QMouseEvent *event)
{
    emit customContextMenuRequested(QPoint(event->x(),event->y()));
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

    QList<QVariant> integerList;
    int num, nbMon = 0;
    QString strng[4];
    QString visibilityCalc;
    QString text;

    // caCalc has no visibility, it is a calc entity
    if (caCalc *w = qobject_cast<caCalc *>(widget)) {
        strng[0] = w->getChannelA();
        strng[1] = w->getChannelB();
        strng[2] = w->getChannelC();
        strng[3] = w->getChannelD();
        visibilityCalc = w->getCalc();
    } else if (caImage *w = qobject_cast<caImage *>(widget)) {
        // while caImage uses also ImageCalc, we do not skip this, even when visibility = StaticV
        strng[0] = w->getChannelA();
        strng[1] = w->getChannelB();
        strng[2] = w->getChannelC();
        strng[3] = w->getChannelD();
        visibilityCalc = w->getVisibilityCalc();
    } else if (caGraphics *w = qobject_cast<caGraphics *>(widget)) {
        if(w->getVisibility() == caGraphics::StaticV && w->getColorMode() == caGraphics::Static) return 0;
        strng[0] = w->getChannelA();
        strng[1] = w->getChannelB();
        strng[2] = w->getChannelC();
        strng[3] = w->getChannelD();
        visibilityCalc = w->getVisibilityCalc();
    } else if (caPolyLine *w = qobject_cast<caPolyLine *>(widget)) {
        if(w->getVisibility() == caPolyLine::StaticV && w->getColorMode() == caPolyLine::Static) return 0;
        strng[0] = w->getChannelA();
        strng[1] = w->getChannelB();
        strng[2] = w->getChannelC();
        strng[3] = w->getChannelD();
        visibilityCalc = w->getVisibilityCalc();
    } else if (caInclude *w = qobject_cast<caInclude *>(widget)) {
        if(w->getVisibility() == caInclude::StaticV) return 0;
        strng[0] = w->getChannelA();
        strng[1] = w->getChannelB();
        strng[2] = w->getChannelC();
        strng[3] = w->getChannelD();
        visibilityCalc = w->getVisibilityCalc();
    } else if (caFrame *w = qobject_cast<caFrame *>(widget)) {
        if(w->getVisibility() == caFrame::StaticV) return 0;
        strng[0] = w->getChannelA();
        strng[1] = w->getChannelB();
        strng[2] = w->getChannelC();
        strng[3] = w->getChannelD();
        visibilityCalc = w->getVisibilityCalc();
    } else if (caLabel *w = qobject_cast<caLabel *>(widget)) {
        if(w->getVisibility() == caLabel::StaticV && w->getColorMode() == caLabel::Static) return 0;
        strng[0] = w->getChannelA();
        strng[1] = w->getChannelB();
        strng[2] = w->getChannelC();
        strng[3] = w->getChannelD();
        visibilityCalc = w->getVisibilityCalc();
    } else {
        qDebug() << "widget has not been defined for visibility";
        return 0;
    }

    /* add monitors for this if any */
    for(int i=0; i<4; i++) {
        if((num = addMonitor(myWidget, kData, strng[i], widget, specData, map, &pv)) >= 0) {
            if (caCalc *w = qobject_cast<caCalc *>(widget)) {
                if(i==0) w->setChannelA(pv);  /* replace pv while macro could be used */
                if(i==1) w->setChannelB(pv);  /* replace pv while macro could be used */
                if(i==2) w->setChannelC(pv);  /* replace pv while macro could be used */
                if(i==3) w->setChannelD(pv);  /* replace pv while macro could be used */
            } else if (caImage *w = qobject_cast<caImage *>(widget)) {
                if(i==0) w->setChannelA(pv);  /* replace pv while macro could be used */
                if(i==1) w->setChannelB(pv);  /* replace pv while macro could be used */
                if(i==2) w->setChannelC(pv);  /* replace pv while macro could be used */
                if(i==3) w->setChannelD(pv);  /* replace pv while macro could be used */
            } else if (caGraphics *w = qobject_cast<caGraphics *>(widget)) {
                if(i==0) w->setChannelA(pv);  /* replace pv while macro could be used */
                if(i==1) w->setChannelB(pv);  /* replace pv while macro could be used */
                if(i==2) w->setChannelC(pv);  /* replace pv while macro could be used */
                if(i==3) w->setChannelD(pv);  /* replace pv while macro could be used */
            } else if (caPolyLine *w = qobject_cast<caPolyLine *>(widget)) {
                if(i==0) w->setChannelA(pv);  /* replace pv while macro could be used */
                if(i==1) w->setChannelB(pv);  /* replace pv while macro could be used */
                if(i==2) w->setChannelC(pv);  /* replace pv while macro could be used */
                if(i==3) w->setChannelD(pv);  /* replace pv while macro could be used */
            } else if (caInclude *w = qobject_cast<caInclude *>(widget)) {
                if(i==0) w->setChannelA(pv);  /* replace pv while macro could be used */
                if(i==1) w->setChannelB(pv);  /* replace pv while macro could be used */
                if(i==2) w->setChannelC(pv);  /* replace pv while macro could be used */
                if(i==3) w->setChannelD(pv);  /* replace pv while macro could be used */
            } else if (caFrame *w = qobject_cast<caFrame *>(widget)) {
                if(i==0) w->setChannelA(pv);  /* replace pv while macro could be used */
                if(i==1) w->setChannelB(pv);  /* replace pv while macro could be used */
                if(i==2) w->setChannelC(pv);  /* replace pv while macro could be used */
                if(i==3) w->setChannelD(pv);  /* replace pv while macro could be used */
            } else if (caLabel *w = qobject_cast<caLabel *>(widget)) {
                if(i==0) w->setChannelA(pv);  /* replace pv while macro could be used */
                if(i==1) w->setChannelB(pv);  /* replace pv while macro could be used */
                if(i==2) w->setChannelC(pv);  /* replace pv while macro could be used */
                if(i==3) w->setChannelD(pv);  /* replace pv while macro could be used */
            }

            tooltip.append(pv);
            tooltip.append("<br>");
            integerList.append(num);
            nbMon++;
        }
    }
    tooltip.append(ToolTipPostfix);
    if(nbMon> 0) widget->setToolTip(tooltip);

    // replace macros for imagecalc
    if (caImage *w = qobject_cast<caImage *>(widget)) {
        text =  treatMacro(map, w->getImageCalc(), &doNothing);
        w->setImageCalc(text);
    }

    /* replace also some macro values in the visibility calc string */
    text =  treatMacro(map, visibilityCalc, &doNothing);

    integerList.insert(0, nbMon); /* set property into widget */

    if (caCalc *w = qobject_cast<caCalc *>(widget)) {
        w->setCalc(text);
        w->setProperty("MonitorList", integerList);
    } else if (caImage *w = qobject_cast<caImage *>(widget)) {
        w->setVisibilityCalc(text);
        w->setProperty("MonitorList", integerList);
    } else if (caGraphics *w = qobject_cast<caGraphics *>(widget)) {
        w->setVisibilityCalc(text);
        w->setProperty("MonitorList", integerList);
    } else if (caPolyLine *w = qobject_cast<caPolyLine *>(widget)) {
        w->setVisibilityCalc(text);
        w->setProperty("MonitorList", integerList);
    } else if (caInclude *w = qobject_cast<caInclude *>(widget)) {
        w->setVisibilityCalc(text);
        w->setProperty("MonitorList", integerList);
    } else if (caFrame *w = qobject_cast<caFrame *>(widget)) {
        w->setVisibilityCalc(text);
        w->setProperty("MonitorList", integerList);
    } else if (caLabel *w = qobject_cast<caLabel *>(widget)) {
        w->setVisibilityCalc(text);
        w->setProperty("MonitorList", integerList);
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
    int width, prec, caMode;
    float maxAbsHoprLopr = 0.0;
    bool fixedFormat = false;

    if(data.edata.initialize) {

        if (caApplyNumeric *w = qobject_cast<caApplyNumeric *>(widget)) {
            precMode = w->getPrecisionMode();
            limitsMode = w->getLimitsMode();
            fixedFormat = w->getFixedFormat();
            caMode = caApplyNumeric::Channel;
        } else if (caNumeric *w = qobject_cast<caNumeric *>(widget)) {
            precMode = w->getPrecisionMode();
            limitsMode = w->getLimitsMode();
            fixedFormat = w->getFixedFormat();
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
            if (caApplyNumeric *w = qobject_cast<caApplyNumeric *>(widget)) {
                maxValue = w->getMaxValue();
                minValue = w->getMinValue();
            } else if (caNumeric *w = qobject_cast<caNumeric *>(widget)) {
                maxValue = w->getMaxValue();
                minValue = w->getMinValue();
            }
        }

        if (caApplyNumeric *w = qobject_cast<caApplyNumeric *>(widget)) {
            w->setMaxValue(maxValue);
            w->setMinValue(minValue);
        } else if (caNumeric *w = qobject_cast<caNumeric *>(widget)) {
            w->setMaxValue(maxValue);
            w->setMinValue(minValue);
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

                if (caApplyNumeric *w = qobject_cast<caApplyNumeric *>(widget)) {
                    //printf("%d %d\n", width-prec-1, prec);
                    w->setIntDigits(width-prec-1);
                    w->setDecDigits(prec);
                } else if (caNumeric *w = qobject_cast<caNumeric *>(widget)) {
                    //printf("%d %d\n", width-prec-1, prec);
                    w->setIntDigits(width-prec-1);
                    w->setDecDigits(prec);
                }
            } else {
                float maxAbsHoprLopr= qMax(fabs(maxValue), fabs(minValue));
                if(maxAbsHoprLopr > 1.0) {
                    width = (int)log10(maxAbsHoprLopr) + 2 ;
                } else {
                    width = 2;
                }
                if (caApplyNumeric *w = qobject_cast<caApplyNumeric *>(widget)) {
                    w->setIntDigits(width-1);
                } else if (caNumeric *w = qobject_cast<caNumeric *>(widget)) {
                    w->setIntDigits(width-1);
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
    if(messageWindow == 0) return;
    messageWindow->postMsgEvent(type, msg);
}

/**
  * execute an application on linux
  */
#ifdef linux
int CaQtDM_Lib::Execute(char *command)
{
    int status;
    pid_t pid;
    QTextStream out(stdout);
    out << command << endl << endl;
    out.flush();
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

void CaQtDM_Lib::TreatOrdinaryValue(QString pv, float value, int32_t idata,  QWidget *w)
{
    char errmess[255];
    int indx =  w->property("MonitorIndex").value<int>();
    if(indx < 0) return;

    knobData *kPtr = mutexKnobData->GetMutexKnobDataPtr(indx);  // use pointer

    // when softpv get index to where it is defined
    if(mutexKnobData->getSoftPV(kPtr->pv, &indx, (QWidget*) kPtr->thisW)) {
        if(kPtr->soft) {
            kPtr = mutexKnobData->GetMutexKnobDataPtr(indx);  // use pointer
            kPtr->edata.rvalue = value;
            // set value also into widget, will be overwritten when driven from other channels
            caCalc * ww = (caCalc*) kPtr->dispW;
            ww->setValue(value);
        }
    } else {
#ifdef epics4
        QString from = QString::number(value);
        Epics4->Epics4SetValue(kPtr->index, pv, from);
#else
        QString text(" ");
        QStringsToChars(pv, text, w->objectName().toLower());
        EpicsSetValue(param1, value, idata, param2, param3, errmess, 0);
#endif
    }
}

/**
  * this routine will treat the string, command, value to write to the pv
  */
void CaQtDM_Lib::TreatRequestedValue(QString text, caTextEntry::FormatType fType, QWidget *w)
{
    char errmess[255];
    double value;
    long longValue;
    char *end;
    bool match;
    char textValue[255];

    int indx =  w->property("MonitorIndex").value<int>();
    if(indx < 0) return;

    //qDebug() << "TreatRequestedValue text" << text.toAscii().constData();
    knobData *kPtr = mutexKnobData->GetMutexKnobDataPtr(indx);  // use pointer
    knobData *auxPtr = kPtr;

    // when softpv get index to where it is defined
    if(mutexKnobData->getSoftPV(kPtr->pv, &indx, (QWidget*) kPtr->thisW)) {
        kPtr = mutexKnobData->GetMutexKnobDataPtr(indx);  // use pointer
    }

    //qDebug() << "fieldtype:" << kPtr->edata.fieldtype;
    switch (kPtr->edata.fieldtype) {
    case caSTRING:
        EpicsSetValue(kPtr->pv, 0.0, 0, (char*) text.toAscii().constData(), (char*) w->objectName().toLower().toAscii().constData(), errmess, 0);
        break;

    case caENUM:
    case caINT:
    case caLONG:
        strcpy(textValue, text.toAscii().constData());
        // Check for a match
        match = false;
        if(kPtr->edata.dataB != (void*)0 && kPtr->edata.enumCount > 0) {
            QString strng((char*) kPtr->edata.dataB);
            QStringList list = strng.split(";", QString::SkipEmptyParts);
            for (int i=0; i<list.size(); i++) {
                if(!text.compare(list.at(i).trimmed())) {
                    EpicsSetValue((char*) kPtr->pv, 0.0, 0, textValue, (char*) w->objectName().toLower().toAscii().constData(), errmess, 0);
                    match = true;
                    break;
                }
            }
        }

        if(!match) {
            //qDebug() << "assume it is a number";
            // Assume it is a number
            if(fType == caTextEntry::octal) {
                longValue = strtoul(textValue, &end, 8);
            } else if(fType == caTextEntry::hexadecimal) {
                longValue = strtoul(textValue,&end,16);
            } else {
                if((strlen(textValue) > (size_t) 2) && (textValue[0] == '0') && (textValue[1] == 'x' || textValue[1] == 'X')) {
                    longValue = strtoul(textValue,&end,16);
                } else {
                    longValue = strtol(textValue,&end,10);
                }
            }

            // number must be between the enum possibilities
            if(kPtr->edata.fieldtype == caENUM) {
                if(*end == 0 && end != textValue && longValue >= 0 && longValue <= kPtr->edata.enumCount) {
                    EpicsSetValue((char*) kPtr->pv, 0.0, longValue, textValue, (char*) w->objectName().toLower().toAscii().constData(), errmess, 0);
                } else {
                    char asc[100];
                    sprintf(asc, "Invalid value: pv=%s value= \"%s\"\n", kPtr->pv, textValue);
                    postMessage(QtDebugMsg, asc);
                    if(caTextEntry* widget = qobject_cast<caTextEntry *>((QWidget*) auxPtr->dispW)) {
                        Q_UNUSED(widget);
                        //widget->setText("");
                    }
                }
                // normal int or long
            } else
                EpicsSetValue((char*) kPtr->pv, 0.0, longValue, textValue, (char*) w->objectName().toLower().toAscii().constData(), errmess, 0);
        }

        break;

    case caCHAR:
        if(fType == caTextEntry::string) {
            EpicsSetValue((char*) kPtr->pv, 0.0, 0, (char*) text.toAscii().constData(), (char*) w->objectName().toLower().toAscii().constData(), errmess, 0);
            break;
        }
        //qDebug() << "fall through";


    default:
        //qDebug() << "assume it is a double";
        // Treat as a double
        strcpy(textValue, text.toAscii().constData());
        if(fType == caTextEntry::octal) {
            value = (double) strtoul(textValue, &end, 8);
        } else if(fType == caTextEntry::hexadecimal) {
            value = (double) strtoul(textValue, &end, 16);
        } else {
            if((strlen(textValue) > (size_t) 2) && (textValue[0] == '0')
                    && (textValue[1] == 'x' || textValue[1] == 'X')) {
                value = (double)strtoul(textValue, &end, 16);
            } else {
                value = (double)strtod(textValue, &end);
            }
        }
        if(*end == '\0' && end != textValue) {
            if(kPtr->soft) {
                //char asc[100];
                kPtr->edata.rvalue = value;
                // set value also into widget, will be overwritten when driven from other channels
                caCalc * ww = (caCalc*) kPtr->dispW;
                ww->setValue(value);
                //sprintf(asc, "SoftPV: %s  set to value: \"%s\"\n", kPtr->pv, textValue);
                //postMessage(QtDebugMsg, asc);
            } else {
                //qDebug() << "we should set a double independently if pv has another type ";
                EpicsSetValue((char*) kPtr->pv, value, 0, textValue, (char*) w->objectName().toLower().toAscii().constData(), errmess, 1);
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

int CaQtDM_Lib::parseForDisplayRate(QString inputc, int &rate)
{
    // Parse data
    char input[MAXPVLEN];
    int cpylen = qMin(inputc.length(), MAXPVLEN-1);
    strncpy(input, (char*) inputc.toAscii().constData(), cpylen);
    input[cpylen] = '\0';

    JSONValue *value = JSON::Parse(input);

    // Did it go wrong?
    if (value == NULL) {
        //printf("failed to parse <%s>\n", input);
        return false;
    } else {
        // Retrieve the main object
        JSONObject root;
        if(value->IsObject() == (false)) {
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
                if ((value1 != NULL) && (value1->IsObject() != (false))) {
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
            if(linewidth < 1.0) linewidth = 1.0;
            line->setLineWidth((int) linewidth);
        }
    }

    else if(!className.compare("caTable")) {
        caTable *table = (caTable *) widget;
        QFont f = table->font();
        qreal fontSize = qMin(factX, factY) * (double) list.at(4).toInt();
        f.setPointSizeF(fontSize);
        table->setUpdatesEnabled(false);
 #if QT_VERSION< QT_VERSION_CHECK(5, 0, 0)
        for(int i = 0; i < table->rowCount(); ++i) {
            for(int j = 0; j < table->columnCount(); ++j) {
                QTableWidgetItem* selectedItem = table->item(i, j);
                if(selectedItem != (QTableWidgetItem*) 0) selectedItem->setFont(f);
            }
        }
        table->setValueFont(f);
#else
        printf("caQtDM_Lib -- caTable resizing does not work yet in Qt5\n");
#endif
        table->verticalHeader()->setDefaultSectionSize((int) (qMin(factX, factY)*20));
        table->setUpdatesEnabled(true);
    }

    else if(!className.compare("QLabel")) {
        QLabel *label = (QLabel *) widget;
        className = label->parent()->metaObject()->className();
        if(!className.contains("Numeric") ) {  // would otherwise interfere with our wheelswitch
            qreal fontSize = qMin(factX, factY) * (double) list.at(4).toInt();
            QFont f = label->font();
            f.setPointSizeF(fontSize);
            label->setFont(f);
        }
    }

    else if(!className.compare("caMenu")) {
        caMenu *label = (caMenu *) widget;
        qreal fontSize = qMin(factX, factY) * (double) list.at(4).toInt();
        QFont f = label->font();
        f.setPointSizeF(fontSize);
        label->setFont(f);
    }

    else if(!className.compare("caStripPlot") || !className.compare("caCartesianPlot")) {
        QwtPlot *plot = (QwtPlot *) widget;

        // change font of axis ticks
        qreal fontSize = qMin(factX, factY) * (double) list.at(4).toInt();
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
        f.setPointSizeF(fontSize);
        title.setFont(f);
        titleX.setFont(f);
        titleY.setFont(f);
        plot->setTitle(title);
        plot->setAxisTitle(QwtPlot::xBottom, titleX);
        plot->setAxisTitle(QwtPlot::yLeft, titleY);


        // font size of legends qith qwt 6.0
        if(!className.compare("caStripPlot")) {
            caStripPlot * plot = (caStripPlot *) widget;
            fontSize = qMin(factX, factY) * (double) list.at(7).toInt();
            f.setPointSizeF(fontSize);
            if(plot->getLegendEnabled()) {
                plot->setLegendAttribute(plot->getScaleColor(), f, caStripPlot::FONT);
            }
        } else {
            // todo, color and font size of title for cartesian plot
        }

    }

    // change fonts for next classes, when smaller needed
    else if(!className.compare("QGroupBox")) {
        if(qMin(factX, factY) < 1.0) {
            QGroupBox *box = (QGroupBox *) widget;
            qreal fontSize = qMin(factX, factY) * (double) list.at(4).toInt();
            QFont f;
            f.setPointSizeF(fontSize);
            box->setFont(f);
        }
    }

    else if(!className.compare("caThermo")) {
        if(qMin(factX, factY) < 1.0) {
            caThermo *thermo = (caThermo *) widget;
            qreal fontSize =  qMin(factX, factY) * (double) list.at(4).toInt();
            QFont f = thermo->font();
            f.setPointSizeF(fontSize);
            thermo->setFont(f);
        }
    }

    else if(!className.compare("caSlider")) {
        if(qMin(factX, factY) < 1.0) {
            caSlider *slider = (caSlider *) widget;
            qreal fontSize =  qMin(factX, factY) * (double) list.at(4).toInt();
            QFont f = slider->font();
            f.setPointSizeF(fontSize);
            slider->setFont(f);
        }
    }

}

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
        // keep original width and height
        origWidth = event->size().width();
        origHeight = event->size().height();
        QList<QWidget *> all = this->findChildren<QWidget *>();
        foreach(QWidget* widget, all) {
            QList<QVariant> integerList;
            QString className(widget->metaObject()->className());
            integerList.insert(0, widget->geometry().x());
            integerList.insert(1, widget->geometry().y());
            integerList.insert(2, widget->geometry().width());
            integerList.insert(3, widget->geometry().height());

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
                    caStripPlot * plot = (caStripPlot *) widget;
                    integerList.insert(7, 9);
                    if(plot->getLegendEnabled()) {
                        plot->setLegendAttribute(plot->getScaleColor(), QFont("arial", 9), caStripPlot::FONT);
                    }
                }

            } else {
                integerList.insert(4, widget->font().pointSize());
            }

            //qDebug() << className << integerList;

            widget->setProperty("GeometryList", integerList);
        }
        return;
    }

    factX = (double) event->size().width() / (double) origWidth;
    factY = (double) event->size().height() / (double) origHeight;

    QString className;

    bool mainlayoutPresent = false;

    if(main == (QObject*) 0) {
        QDialog *dialog = this->findChild<QDialog *>();
        if(dialog == (QObject*) 0) return;  // if not a mainwindow or dialog get out
        if(dialog->layout() != (QObject*) 0) {
            className = dialog->layout()->metaObject()->className();
            mainlayoutPresent = true;
        }
    } else {
        if( main->centralWidget()->layout() != (QObject*) 0) {
            className = main->centralWidget()->layout()->metaObject()->className();
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
        if(className.contains("Layout")) {
            // resize some minor stuff before leaving this routine
            QList<QWidget *> all = this->findChildren<QWidget *>();
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
    QList<QWidget *> all = this->findChildren<QWidget *>();
    foreach(QWidget* widget, all) {
        QString className(widget->metaObject()->className());

        if(
                !className.contains("QMainWindow")  &&
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
                QRect rectnew = QRect((int) x, (int) y, (int) width, (int) height);
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
                    QRect rectnew = QRect((int) x, (int) y, (int) width, (int) height);

                    widget->setGeometry(rectnew);
                    resizeSpecials(className, widget, list, factX, factY);
                    widget->updateGeometry();
                }
            }

        }
    }

}

