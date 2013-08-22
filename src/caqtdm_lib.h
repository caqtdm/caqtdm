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

#include <QWidget>
#include <QWaitCondition>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>

#include <QUiLoader>

#include "mutexKnobData.h"
#include "mutexKnobDataWrapper.h"
#include "MessageWindow.h"
#include "JSON.h"
#include "limitsStripplotDialog.h"
#include "limitsCartesianplotDialog.h"
#include "processWindow.h"

#include <QtControls>

namespace Ui {
    class CaQtDM_Lib;
}

class CAQTDM_LIBSHARED_EXPORT CaQtDM_Lib : public QMainWindow
{
    Q_OBJECT

public:

    explicit CaQtDM_Lib(QWidget *parent = 0, QString="", QString="", MutexKnobData *mutexKnobData = 0, MessageWindow *msgWindow = 0, bool willPrint = false);
    ~CaQtDM_Lib();

    bool bitState(int value, int bitNr);
    QString treatMacro(QMap<QString, QString> map, const QString& pv, bool *doNothing);
    int addMonitor(QWidget *thisW, knobData *data, QString pv, QWidget *w, int *specData, QMap<QString, QString> map, QString *pvRep);
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
    void TreatOrdinaryValue(QString pv, float value, int32_t idata, QWidget *w);
    bool getSoftChannel(QString pv, knobData &data);
    int parseForDisplayRate(QString input, int &rate);
    void UpdateGauge(EAbstractGauge *w, const knobData &data);
    void allowResizing(bool allowresize);
    void resizeSpecials(QString className, QWidget *widget, QVariantList list, double factX, double factY);

    void print()
    {
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
    }
    void printPS(QString filename)
    {
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
    }


protected:

    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void timerEvent(QTimerEvent *e);
    void resizeEvent ( QResizeEvent * event );

signals:
    void clicked(QString);
    void Signal_QLineEdit(const QString&, const QString&);
    void Signal_OpenNewWFile(const QString&, const QString&, const QString&);
    void Signal_ContextMenu(QWidget*);
    void clicked();

private:

    QWidget *myWidget;
    QList<QWidget*> includeWidgetList;
    QwtPlotCurve curve[3];
    QVector<double> xx, yy;
    int level;
    QString savedMacro[50];
    QString savedFile[50];
    QProcess *proc;
    QMap<QString, QString> createMap(const QString&);

    QByteArray byteArray;
    bool initTry;

    QString thisFileShort;
    QString thisFileFull;

    boolean firstResize;
    boolean allowResize;

    boolean pepPrint;

    int origWidth, origHeight;


#ifdef epics4
    epics4Subs *Epics4;
#endif

private slots:

    void processError(QProcess::ProcessError err);
    void Callback_EApplyNumeric(double value);
    void Callback_ENumeric(double value);
    void Callback_Slider(double value);
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

    void ShowContextMenu(const QPoint&);
    void DisplayContextMenu(QWidget* w);
    void Callback_TextEntryChanged(const QString &);

    void processTerminated();

};

#endif // CaQtDM_Lib_H


