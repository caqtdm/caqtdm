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

#ifndef CAQTDM_LIB_H
#define CAQTDM_LIB_H

#include "caQtDM_Lib_global.h"
#if defined(_MSC_VER)
#define _MATH_DEFINES_DEFINED
#endif

#ifdef epics4
#include "epics4Subs.h"
#endif

#include "dbrString.h"
#include <stdint.h>
#include <QMainWindow>
#include <QTableWidget>
#include <QGroupBox>
#include <QHeaderView>
#include <QVector>
#include <QMutex>
#include <QFile>
#include <QMap>
#include <QtGui>
#include <QtUiTools>
#include <QWhatsThis>
#include <QTextBrowser>

#include <QWidget>
#include <QWaitCondition>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#ifndef MOBILE
#include <QPrinter>
#include <QPrintDialog>
#endif
#include <QClipboard>

#include <QUiLoader>

#ifndef MOBILE
   #include "myQProcess.h"
   #include "processWindow.h"
#endif
#include "mutexKnobData.h"
#include "mutexKnobDataWrapper.h"
#include "MessageWindow.h"
#include "messageWindowWrapper.h"
#include "JSON.h"
#include "limitsStripplotDialog.h"
#include "limitsCartesianplotDialog.h"
#include "limitsDialog.h"
#include "sliderDialog.h"
#include "splashscreen.h"
#include "messageQueue.h"
#include "controlsinterface.h"

#include <QtControls>

#ifdef MOBILE
#include <QGestureEvent>
#include <QTapAndHoldGesture>
#include "fingerswipegesture.h"
#endif

#include <QMenuBar>

namespace Ui {
class CaQtDM_Lib;
}

class CAQTDM_LIBSHARED_EXPORT CaQtDM_Lib : public QMainWindow
{
    Q_OBJECT

public:

    explicit CaQtDM_Lib(QWidget *parent = 0, QString="", QString="", MutexKnobData *mutexKnobData = 0,
                                                                     QMap<QString, ControlsInterface *> interfaces = (QMap<QString, ControlsInterface *>()),
                                                                     MessageWindow *msgWindow = 0,
                                                                     bool willPrint = false,
                                                                     QWidget *parentAS = 0,
                                                                     QMap<QString, QString> options = (QMap<QString, QString>()));
    ~CaQtDM_Lib();

    void allowResizing(bool allowresize);
    int addMonitor(QWidget *thisW, knobData *data, QString pv, QWidget *w, int *specData, QMap<QString, QString> map, QString *pvRep);
    void ComputeNumericMaxMinPrec(QWidget* widget, const knobData &data);
    void UpdateGauge(EAbstractGauge *w, const knobData &data);
    ControlsInterface * getControlInterface(QString plugininterface);

#ifdef MOBILE
    void grabSwipeGesture(Qt::GestureType fingerSwipeGestureTypeID);
#endif

#ifdef linux
    QString getDefaultPrinterFromSystem() {
        QProcess Process;
        QString exec = "lpstat";
        QStringList params;
        params << "-d"  ;
        Process.start(exec, params);
        Process.waitForFinished(); // sets current thread to sleep and waits for Process end
        QString output(Process.readAllStandardOutput());

        QRegExp noDefaultReg("[^:]*no .*default");
        int pos = noDefaultReg.indexIn(output);
        if (pos >= 0) {
            return QString();
        }

        QRegExp defaultReg("default.*: *([a-zA-Z0-9_]+)");
        defaultReg.indexIn(output);
        QString printer = defaultReg.cap(1);
        return printer;
    }
#else
    QString getDefaultPrinterFromSystem() {
        QString printer = "";
        return printer;
    }
#endif

    void print()
    {
#ifndef MOBILE
#ifdef linux
        QString defaultPrinter =  getDefaultPrinterFromSystem();
#endif
        QPrinter *printer = new QPrinter;
#ifdef linux
        printer->setPrinterName(defaultPrinter);
        printer->setOutputFileName(0);
        printer->setPrintProgram("lpr");
        printer->setOrientation(QPrinter::Landscape);
        printer->setResolution(300);
        printer->setOutputFormat(QPrinter::NativeFormat);
#else
        printer->setOrientation(QPrinter::Landscape);
        printer->setResolution(300);
#endif
        QPrintDialog *printDialog = new QPrintDialog(printer, this);

#ifdef linux
        QList<QWidget*> childWidgets = printDialog->findChildren<QWidget*>(QLatin1String("printers"));
        if (childWidgets.count() == 1) {
            QComboBox* comboBox(qobject_cast<QComboBox*>(childWidgets.at(0)));
            comboBox->addItem(defaultPrinter);
        }
#endif
        if (printDialog->exec() == QDialog::Accepted) {

            QPainter painter(printer);
            double xscale = printer->pageRect().width()/double(this->width());
            double yscale = printer->pageRect().height()/double(this->height());
            double scale = qMin(xscale, yscale);
            painter.translate(printer->paperRect().x() + printer->pageRect().width()/2,
                              printer->paperRect().y() + printer->pageRect().height()/2);
            painter.scale(scale, scale);
            painter.translate(-width()/2, -height()/2);
            QPixmap pm = QPixmap::grabWidget(this);
            painter.drawPixmap(0, 0, pm);
        }
#endif
    }

    void printPS(QString filename)
    {
#ifndef MOBILE
        QPrinter *printer = new QPrinter;
        printer->setOrientation(QPrinter::Portrait);
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        printer->setOutputFormat(QPrinter::PostScriptFormat);
#else
        printer->setOutputFormat(QPrinter::NativeFormat);
        printf("caQtDM_Lib -- seems that postscript is not supported any more in Qt5\n");
#endif
        printer->setResolution(300);
        printer->setOutputFileName(filename);

        QPainter painter(printer);
        double xscale = printer->pageRect().width()/double(this->width());
        double yscale = printer->pageRect().height()/double(this->height());
        double scale = qMin(xscale, yscale);
        painter.translate(printer->paperRect().x() + printer->pageRect().width()/2,
                          printer->paperRect().y() + printer->pageRect().height()/2);
        painter.scale(scale, scale);
        painter.translate(-width()/2, -height()/2);
        QPixmap pm = QPixmap::grabWidget(this);
        painter.drawPixmap(0, 0, pm);
#else
        Q_UNUSED(filename);
#endif
    }

protected:
    virtual void timerEvent(QTimerEvent *e);
    void resizeEvent ( QResizeEvent * event );
    void mousePressEvent(QMouseEvent *event);

signals:
    void clicked(QString);
    void clicked(int);
    void clicked(double);
    void Signal_QLineEdit(const QString&, const QString&);
    void Signal_OpenNewWFile(const QString&, const QString&, const QString&, const QString&);
    void Signal_ContextMenu(QWidget*);
    void Signal_NextWindow();
    void Signal_IosExit();
    void fileChanged(const QString&);

private:
    void scanChildren(QList<QWidget*> children, QWidget *tab, int i);
    QWidget* getTabParent(QWidget *w1);
    QString treatMacro(QMap<QString, QString> map, const QString& pv, bool *doNothing);
    void scanWidgets(QList<QWidget*> list, QString macro);
    void HandleWidget(QWidget *w, QString macro, bool firstPass, bool treatPrimaries);
    void closeEvent(QCloseEvent* ce);
    bool CalcVisibility(QWidget *w, double &result, bool &valid);
    short ComputeAlarm(QWidget *w);
    int setObjectVisibility(QWidget *w, double value);
    bool reaffectText(QMap<QString, QString> map, QString *text);
    int InitVisibility(QWidget* widget, knobData *kData, QMap<QString, QString> map,  int *specData, QString info);
    void postMessage(QtMsgType type, char *msg);
    int Execute(char *command);
    void TreatRequestedValue(QString pv, QString text, caTextEntry::FormatType fType, QWidget *w);
    void TreatRequestedWave(QString pv, QString text, caWaveTable::FormatType fType, int index, QWidget *w);
    void TreatOrdinaryValue(QString pv, double value, int32_t idata, QWidget *w);
    bool getSoftChannel(QString pv, knobData &data);
    int parseForDisplayRate(QString input, int &rate);
    void getStatesToggleAndLed(QWidget *widget, const knobData &data, const QString &String, Qt::CheckState &state);

    void resizeSpecials(QString className, QWidget *widget, QVariantList list, double factX, double factY);
    void shellCommand(QString command);

    void WaterFall(caWaterfallPlot *widget, const knobData &data);
    void Cartesian(caCartesianPlot *widget, int curvNB, int curvType, int XorY, const knobData &data);
    void WaveTable(caWaveTable *widget, const knobData &data);
    void EnableDisableIO();
    void UpdateMeter(caMeter *widget, const knobData &data);
    bool SoftPVusesItsself(QWidget* widget, QMap<QString, QString> map);
    void setCalcToNothing(QWidget* widget);
    bool Python_Error(QWidget *w, QString message);
    void FlushAllInterfaces();
    void CartesianPlotsVerticalAlign();
    void StripPlotsVerticalAlign();
    qreal fontResize(double factX, double factY, QVariantList list, int usedIndex);

#ifdef MOBILE
    bool eventFilter(QObject *obj, QEvent *event);
    bool gestureEvent(QObject *obj, QGestureEvent *event);
    void tapAndHoldTriggered(QObject *obj, QTapAndHoldGesture* tapAndHold);
    void fingerswipeTriggered(FingerSwipeGesture *gesture);
    Qt::GestureType fingerSwipeGestureType;
#endif

    enum formatsType {decimal, hexadecimal, octal, string};
    long getValueFromString(char *textValue, formatsType fType, char **end);

    QWidget *myWidget;
    QList<QWidget*> includeWidgetList;
    QList<QWidget*> topIncludesWidgetList;
    QList<QTabWidget *> allTabs;
    QList<QStackedWidget *> allStacks;

    int level;
    // 50 levels of includes should do it
    QString savedMacro[50];
    QString savedFile[50];

#ifndef MOBILE
    myQProcess *proc;
#endif

    QMap<QString, QString> createMap(const QString&);

    QString thisFileShort;
    QString thisFileFull;

    bool firstResize;
    bool allowResize;
    bool pepPrint;
    bool prcFile;

    int origWidth, origHeight;

    QString includeFiles;

    SplashScreen *splash;

    int nbIncludes;
    int splashCounter;

    bool AllowsUpdate;
    bool fromAS;

    int loopTimer;
    int loopTimerID;

    QMap<QString, ControlsInterface*> controlsInterfaces;
    MutexKnobData *mutexKnobDataP;
    MessageWindow *messageWindowP;

    QFileSystemWatcher *watcher;

    QMap<int, caCartesianPlot*> cartesianList;  // list of cartesianplots with key group
    QList<int> cartesianGroupList;              // group numbers found

    QMap<int, caStripPlot*> stripList;          // list of stripplots with key group
    QList<int> stripGroupList;                  // group numbers found

#ifdef epics4
    epics4Subs *Epics4;
#endif

    QString defaultPlugin;

private slots:
    void Callback_EApplyNumeric(double value);
    void Callback_ENumeric(double value);
    void Callback_Spinbox(double value);
    void Callback_SliderValueChanged(double);
    void Callback_MessageButton(int type);
    void Callback_ToggleButton(bool type);
    void Callback_ScriptButton();

    void Callback_UpdateWidget(int, QWidget *w, const QString& units,const QString& fec,
                               const QString& statusString, const knobData& data);
    void Callback_UpdateLine(const QString&, const QString&);
    void Callback_MenuClicked(const QString&);
    void Callback_ChoiceClicked(const QString&);
    void Callback_RelatedDisplayClicked(int);
    void Callback_ShellCommandClicked(int);
    void Callback_TableDoubleClicked(const QString&);
    void Callback_ByteControllerClicked(int);

    void Callback_TabChanged(int);

    void ShowContextMenu(const QPoint&);
    void DisplayContextMenu(QWidget* w);
    void Callback_TextEntryChanged(const QString &);
    void Callback_WaveEntryChanged(const QString &, int index);
    void processTerminated();
    void closeWindow();
    void updateTextBrowser();
    void handleFileChanged(const QString&);

    void Callback_WriteDetectedValues(QWidget* w);

    void updateResize();
};

#endif // CaQtDM_Lib_H


