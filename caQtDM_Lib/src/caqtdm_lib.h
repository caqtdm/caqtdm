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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QRegExp>
#else
#include <QRegularExpression>
#endif

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

// interface to different controlsystems
#include "controlsinterface.h"

// interface used enabling some widgets doing their own acquisition by calling caQtDM_Lib routines
#include "caqtdm_lib_interface.h"

#include <QtControls>

#ifdef MOBILE
#include <QGestureEvent>
#include <QTapAndHoldGesture>
#include "fingerswipegesture.h"
#endif

#include <QMenuBar>

#if defined(_MSC_VER) || defined(MOBILE_IOS)
#define useElapsedTimer
#else
#endif

// 50 levels of includes should do it
#define CAQTDM_MAX_INCLUDE_LEVEL 50

enum macro_parser{
    parse_simple,parse_withconst
};

namespace Ui {
class CaQtDM_Lib;
}

class CAQTDM_LIBSHARED_EXPORT CaQtDM_Lib : public QMainWindow, public CaQtDM_Lib_Interface
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
    void ComputeNumericMaxMinPrec(QWidget* widget, const knobData &data);
    void UpdateGauge(EAbstractGauge *w, const knobData &data);
    ControlsInterface * getControlInterface(QString plugininterface);

    // interface implementation
    int addMonitor(QWidget *thisW, knobData *data, QString pv, QWidget *w, int *specData, QMap<QString, QString> map, QString *pvRep);
    knobData* GetMutexKnobDataPtr(int index);
    knobData* GetMutexKnobDataPV(QWidget *widget, QString pv);
    void TreatRequestedValue(QString pv, QString text, FormatType fType, QWidget *w);
    // ZHW requested for external integration - allow an external application/object to get the top level ui widget of caQtDM_Lib window
    QWidget* getMyWidget(){ return myWidget; }
    // interface finish (perhaps we need more)

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

        QString noDefaultReg_pattern="[^:]*no .*default";
        QString defaultReg_pattern="default.*: *([a-zA-Z0-9_]+)";

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QRegExp noDefaultReg(noDefaultReg_pattern);
        int pos = noDefaultReg.indexIn(output);
        if (pos >= 0) {
            return QString();
        }

        QRegExp defaultReg(defaultReg_pattern);
        defaultReg.indexIn(output);
        QString printer = defaultReg.cap(1);
        return printer;
#else
        QRegularExpression noDefaultReg(noDefaultReg_pattern);
        QRegularExpressionMatch noDefaultReg_match = noDefaultReg.match(output);
        qsizetype pos=noDefaultReg_match.capturedStart();
        //qDebug() << "Regex output:"<< output;
        if (pos >= 0) {
            return QString();
        }
        QRegularExpression defaultReg(defaultReg_pattern);
        QRegularExpressionMatch defaultReg_match = defaultReg.match(output);
        QString printer = defaultReg_match.captured(1);
        return printer;


#endif
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
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        printer->setOrientation(QPrinter::Landscape);
#else
        printer->setPageOrientation( QPageLayout::Landscape);
#endif
        printer->setResolution(300);
        printer->setOutputFormat(QPrinter::NativeFormat);
        QPrintDialog *printDialog = new QPrintDialog(printer, this);

#ifdef linux
        QList<QWidget*> childWidgets = printDialog->findChildren<QWidget*>(QLatin1String("printers"));
        if (childWidgets.count() == 1) {
            QComboBox* comboBox(qobject_cast<QComboBox*>(childWidgets.at(0)));
            comboBox->addItem(defaultPrinter);
        }
#endif
        if (printDialog->exec() == QDialog::Accepted) {
            print2Painter(printer);
        }
#endif
    }

    void printPS(QString filename)
    {
#ifndef MOBILE
        QPrinter *printer = new QPrinter;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        printer->setOrientation(QPrinter::Portrait);
#else
        printer->setPageOrientation( QPageLayout::Portrait);
#endif
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        printer->setOutputFormat(QPrinter::PostScriptFormat);
#else
        printer->setOutputFormat(QPrinter::NativeFormat);
        printf("caQtDM_Lib -- seems that postscript is not supported any more in Qt5\n");
#endif
        printer->setResolution(300);
        printer->setOutputFileName(filename);
        print2Painter(printer);
#else
        Q_UNUSED(filename);
#endif
    }

    void save_graphics(QString filename)
    {
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
        QPixmap pm = this->grab();
#else
        QPixmap pm = QPixmap::grabWidget(this);
#endif
        QString text = QDate::currentDate().toString("yyyy-MM-dd");
        text += " " + QTime::currentTime().toString("hh:mm:ss");
        text += ", " + this->thisFileShort;
        QPainter painter( &pm );
        QFont qfont = painter.font();
        qfont.setPointSizeF(8);
        painter.setFont(qfont);
        painter.drawText(QPoint(0, 10), text );
        if (pm.save(filename,"PNG",-1)){
            printf("caQtDM image file saved\n");
        }else{
            printf("caQtDM image file save failed\n");
        }
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
    void Signal_ReloadWindow(QWidget*);
    void Signal_ReloadWindowL();
    void Signal_ReloadAllWindows();
    void fileChanged(const QString&);

private:
#if !defined(useElapsedTimer)
    double rTime();
#endif

#ifndef MOBILE
    void print2Painter(QPrinter *printer)
    {
        QPainter painter(printer);
        QFont qfont = painter.font();
        qfont.setPointSizeF(12);
        painter.setFont(qfont);
        QFontMetrics fm(qfont);
        QFont thisFont = this->font();
        thisFont.setPointSizeF(12);
        QFontMetrics ft(thisFont);

        painter.save();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        double xscale = printer->pageRect().width()/double(this->width());
        double yscale = printer->pageRect().height()/double(this->height() + ft.lineSpacing());
        double scale = qMin(xscale, yscale);
        painter.translate(printer->paperRect().x() + printer->pageRect().width()/2,
                          printer->paperRect().y() + printer->pageRect().height()/2);
#else
        QRectF pageSize=printer->pageLayout().fullRect();
        double xscale = pageSize.width()/double(this->width());
        double yscale = pageSize.height()/double(this->height() + ft.lineSpacing());
        double scale = qMin(xscale, yscale);
        painter.translate(pageSize.x() + pageSize.width()/2,
                          pageSize.y() + pageSize.height()/2);

#endif
        painter.scale(scale, scale);
        painter.translate(-width()/2, -height()/2 + ft.lineSpacing());

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QPixmap pm = QPixmap::grabWidget(this);
#else
        QPixmap pm = this->grab();
#endif

        painter.drawPixmap(0, 0, pm);

        painter.restore();

        QString text = QDate::currentDate().toString("yyyy-MM-dd");
        text += " " + QTime::currentTime().toString("hh:mm:ss");
        text += ", " + this->thisFileShort;

        painter.drawText(0, 0, text);
        painter.drawText(0, fm.height() + fm.descent(), this->thisFileFull);
    }
#endif

    void scanChildren(QList<QWidget*> children, QWidget *tab, int i);
    QWidget* getTabParent(QWidget *w1);
    QString treatMacro(QMap<QString, QString> map, const QString& pv, bool *doNothing, QString widgetName = "");
    void scanWidgets(QList<QWidget*> list, QString macro);
    void HandleWidget(QWidget *w, QString macro, bool firstPass, bool treatPrimaries);
    void closeEvent(QCloseEvent* ce);
    bool CalcVisibility(QWidget *w, double &result, bool &valid);
    short ComputeAlarm(QWidget *w);
    int setObjectVisibility(QWidget *w, double value);
    bool reaffectText(QMap<QString, QString> map, QString *text, QWidget *w);
    int InitVisibility(QWidget* widget, knobData *kData, QMap<QString, QString> map,  int *specData, QString info);
    void postMessage(QtMsgType type, char *msg);
    int Execute(char *command);

    void TreatRequestedWave(QString pv, QString text, caWaveTable::FormatType fType, int index, QWidget *w);
    void TreatOrdinaryValue(QString pv, double value, int32_t idata, QString svalue, QWidget *w);
    bool getSoftChannel(QString pv, knobData &data);
    int parseForDisplayRate(QString &input, int &rate);
    bool checkJsonString(QString &inputc);
    bool parseForQRectConst(QString &input,double* valueArray);
    void getStatesToggleAndLed(QWidget *widget, const knobData &data, const QString &String, Qt::CheckState &state);

    QRect widgetResize(QWidget *w, double factX, double factY);
    void resizeSpecials(QString className, QWidget *widget, QVariantList list, double factX, double factY);
    void shellCommand(QString command);

    void WaterFall(caWaterfallPlot *widget, const knobData &data);
    void Cartesian(caCartesianPlot *widget, int curvNB, int curvType, int XorY, const knobData &data);
    void CameraWaveform(caCamera *widget, int curvNB, int curvType, int XorY, const knobData &data);
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
    ControlsInterface *getPluginInterface(QWidget *w);
    void UndefinedMacrosWindow();

#ifdef MOBILE
    bool eventFilter(QObject *obj, QEvent *event);
    bool gestureEvent(QObject *obj, QGestureEvent *event);
    void tapAndHoldTriggered(QObject *obj, QTapAndHoldGesture* tapAndHold);
    void fingerswipeTriggered(FingerSwipeGesture *gesture);
    Qt::GestureType fingerSwipeGestureType;
#else

    bool eventFilter(QObject *obj, QEvent *event);
#endif

    long getLongValueFromString(char *textValue, FormatType fType, char **end);
    double getDoubleValueFromString(char *textValue, FormatType fType, char **end);

    void ResizeScrollBars(caInclude * includeWidget, int sizeX, int sizeY);

    QWidget *myWidget;
    QList<QWidget*> includeWidgetList;
    QList<QWidget*> topIncludesWidgetList;
    QList<QTabWidget *> allTabs;
    QList<QStackedWidget *> allStacks;
    QList<caCalc *> allCalcs_Vectors;

    QMap<QString, QString> unknownMacrosList;
    QTableWidget* macroTable;
    QDialog *macroWindow;

    int level;
    QString cainclude_path;
    // 50 levels of includes should do it
    QString savedMacro[CAQTDM_MAX_INCLUDE_LEVEL];
    QString savedFile[CAQTDM_MAX_INCLUDE_LEVEL];

    void clearCaLineDraw();
    void clearCaMultiLineString();
    void clearCaWaveTable();
    void clearCaTable();
    void clearcaLineEdit();

#ifndef MOBILE
    myQProcess *proc;
#endif

    QMap<QString, QString> createMap(const QString&);
    QString createMacroStringFromMap(QMap<QString, QString> map);
    QMap<QString, QString> actualizeMacroMap();
    QString actualizeMacroString(QMap<QString, QString> map, QString argument);

    QString thisFileShort;
    QString thisFileFull;

    bool firstResize;
    bool allowResize;
    bool pepPrint;
    bool prcFile;

    int origWidth, origHeight;

    struct includeData {int count; int ms; QString text;};
    QMap<QString, includeData> includeFilesList;

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
    QHash<QString, QString> softvars;                // use a hash list to test if same variable names

    QString defaultPlugin;

    QString handle_single_Macro(QString key, QString value, QString Text);
    QString handle_Macro_withConst(QString key, QString value, QString Text);
    QString handle_Macro_Scan(QString Text, QMap<QString, QString> map, macro_parser parse);
    QString handle_Macro_Constants(QString Text);
private slots:
    void Callback_CaCalc(double value) ;
    void Callback_UndefinedMacrowindowExit();
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
    void showNormalWindow();
    void showMaxWindow();
    void showMinWindow();
    void showFullWindow();
    void resizeFullWindow(QRect& q);

    void updateTextBrowser();
    void handleFileChanged(const QString&);

    void Callback_WriteDetectedValues(QWidget* w);
    void Callback_CopyMarked();

    void Callback_ReloadWindowL() {

        //qDebug() << "================== in caQtDM_Lib::Callback_reloadWindowL";
        // get global macro, replace specified keys from caReplaceMacros when present
        QList<replaceMacro *> all = myWidget->findChildren<replaceMacro *>();
        if(all.count() > 0) {
            QVariant macroString = this->property("macroString");
            if(!macroString.isNull()) {
                QMap<QString, QString> map = actualizeMacroMap();
                QString macro = createMacroStringFromMap(map);
                this->setProperty("macroString", macro);
            }
            //qDebug() << "new macrostring" << macroString;
        }

        emit Signal_ReloadWindow(this);
    }

    void Callback_reloadAllWindows() {
        emit Signal_ReloadAllWindows();
    }

    void Callback_printWindow() {
        print();
    }

    void Callback_ResizeUp() {
        this->resize(this->size()*1.1);
    }

    void Callback_ResizeDown() {
        this->resize(this->size()*0.9);
    }

    void updateResize();
#ifndef MOBILE
    void send_delayed_popup_signal();
#endif

};

#endif // CaQtDM_Lib_H


