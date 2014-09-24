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

#include <QWidget>
#include <QWaitCondition>
#include <QMessageBox>
#ifndef Q_OS_IOS
 #include <QPrinter>
 #include <QPrintDialog>
#endif
#include <QClipboard>

#include <QUiLoader>

#include "mutexKnobData.h"
#include "mutexKnobDataWrapper.h"
#include "MessageWindow.h"
#include "JSON.h"
#include "limitsStripplotDialog.h"
#include "limitsCartesianplotDialog.h"
#include "sliderDialog.h"
#include "processWindow.h"
#include "splashscreen.h"

#include <QtControls>

#ifdef Q_OS_IOS
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

    explicit CaQtDM_Lib(QWidget *parent = 0, QString="", QString="", MutexKnobData *mutexKnobData = 0, MessageWindow *msgWindow = 0, bool willPrint = false, QWidget *parentAS = 0);
    ~CaQtDM_Lib();

    void allowResizing(bool allowresize);
    int addMonitor(QWidget *thisW, knobData *data, QString pv, QWidget *w, int *specData, QMap<QString, QString> map, QString *pvRep);
#ifdef Q_OS_IOS
    void grabSwipeGesture(Qt::GestureType fingerSwipeGestureTypeID);
#endif

    void print()
    {
#ifndef Q_OS_IOS
        QPrinter *printer = new QPrinter;
        QPrintDialog *printDialog = new QPrintDialog(printer, this);
        if (printDialog->exec() == QDialog::Accepted) {

            QPainter painter(printer);
            printer->setOrientation(QPrinter::Landscape);
            printer->setResolution(300);
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
#ifndef Q_OS_IOS
        QPrinter *printer = new QPrinter;
        printer->setOrientation(QPrinter::Portrait);
#if QT_VERSION< QT_VERSION_CHECK(5, 0, 0)
        printer->setOutputFormat(QPrinter::PostScriptFormat);
#else
        printer->setOutputFormat(QPrinter::NativeFormat);
        printf("caQtDM_Lib -- seems that postscript is not supoorted any more in Qt5\n");
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
#endif
    }

protected:
    virtual void timerEvent(QTimerEvent *e);
    void resizeEvent ( QResizeEvent * event );
    void mousePressEvent(QMouseEvent *event);

signals:
    void clicked(QString);
    void Signal_QLineEdit(const QString&, const QString&);
    void Signal_OpenNewWFile(const QString&, const QString&, const QString&);
    void Signal_ContextMenu(QWidget*);
    void clicked();
    void Signal_NextWindow();
    void Signal_IosExit();

private:
    bool bitState(int value, int bitNr);
    QString treatMacro(QMap<QString, QString> map, const QString& pv, bool *doNothing);
    void HandleWidget(QWidget *w, QString macro, bool firstPass);
    void closeEvent(QCloseEvent* ce);
    bool CalcVisibility(QWidget *w, double &result, bool &valid);
    int ComputeAlarm(QWidget *w);
    int setObjectVisibility(QWidget *w, double value);
    bool reaffectText(QMap<QString, QString> map, QString *text);
    int InitVisibility(QWidget* widget, knobData *kData, QMap<QString, QString> map,  int *specData, QString info);
    void ComputeNumericMaxMinPrec(QWidget* widget, const knobData &data);
    void postMessage(QtMsgType type, char *msg);
    int Execute(char *command);
    void TreatRequestedValue(QString text, caTextEntry::FormatType fType, QWidget *w);
    void TreatOrdinaryValue(QString pv, double value, int32_t idata, QWidget *w);
    bool getSoftChannel(QString pv, knobData &data);
    int parseForDisplayRate(QString input, int &rate);
    void UpdateGauge(EAbstractGauge *w, const knobData &data);
    void getStatesToggleAndLed(QWidget *widget, const knobData &data, const QString &String, Qt::CheckState &state);

    void resizeSpecials(QString className, QWidget *widget, QVariantList list, double factX, double factY);
    void shellCommand(QString command);

    void WaterFall(caWaterfallPlot *widget, const knobData &data);
    void Cartesian(caCartesianPlot *widget, int curvNB, int curvType, int XorY, const knobData &data);
    void WaveTable(caWaveTable *widget, const knobData &data);

#ifdef Q_OS_IOS
    bool eventFilter(QObject *obj, QEvent *event);
    bool gestureEvent(QObject *obj, QGestureEvent *event);
    void tapAndHoldTriggered(QObject *obj, QTapAndHoldGesture* tapAndHold);
    void fingerswipeTriggered(FingerSwipeGesture *gesture);
    Qt::GestureType fingerSwipeGestureType;
#endif

    QWidget *myWidget;
    QList<QWidget*> includeWidgetList;
    QList<QWidget*> topIncludesWidgetList;
    QwtPlotCurve curve[3];
    QVector<double> xx, yy;

    int level;
    // 50 levels of includes should do it
    QString savedMacro[50];
    QString savedFile[50];

    QProcess *proc;
    QMap<QString, QString> createMap(const QString&);

    QByteArray byteArray;
    bool initTry;

    QString thisFileShort;
    QString thisFileFull;

    bool firstResize;
    bool allowResize;
    bool pepPrint;

    int origWidth, origHeight;

    QString includeFiles;

    SplashScreen *splash;

    int nbIncludes;
    int splashCounter;

    bool AllowsUpdate;
    bool fromAS;

#ifdef epics4
    epics4Subs *Epics4;
#endif

private slots:

    void processError(QProcess::ProcessError err);
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

    void ShowContextMenu(const QPoint&);
    void DisplayContextMenu(QWidget* w);
    void Callback_TextEntryChanged(const QString &);

    void processTerminated();
    void closeWindow();
};

#endif // CaQtDM_Lib_H


