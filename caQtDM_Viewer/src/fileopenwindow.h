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

#ifndef CALCULATORFORM_H
#define CALCULATORFORM_H

#define NoValue         0x0000
#define XValue          0x0001
#define YValue          0x0002
#define WidthValue      0x0004
#define HeightValue     0x0008
#define AllValues       0x000F
#define XNegative       0x0010
#define YNegative       0x0020

#include <QMainWindow>
#include <QSharedMemory>
#include <QTableWidget>
#include <QScrollBar>
#include <QFile>

#ifdef MOBILE
#include <QGuiApplication>
#endif

#ifndef MOBILE
#include "myQProcess.h"
#endif
#include "MessageWindow.h"
#include "dbrString.h"
#include "knobDefines.h"
#include "knobData.h"
#include "mutexKnobData.h"
#include "caqtdm_lib.h"
#include "ui_main.h"
#include <stdio.h>

#include "epicsExternals.h"
#if defined(_MSC_VER)
    int setenv(const char *name, const char *value, int overwrite);
#endif

#define RingSize 50
#define BlopSize 4096
    struct _blop {
        char blop[BlopSize];
    };

#ifdef linux
#  include <unistd.h>
#endif
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

 class FileOpenWindow : public QMainWindow
 {
     Q_OBJECT

 public:
     FileOpenWindow(QMainWindow *parent = 0,  QString filename = "", QString macroString = "",
                    bool attach = false, bool minimize = false, QString geometry = "", bool printscreen = false, bool resizing = true,
                    QMap<QString, QString> options = (QMap<QString, QString>()));

     QMainWindow *loadMainWindow(const QPoint &position, const QString &fileS, const QString &macroS, const QString &resizeS,
                                         const bool &printexit, const bool &moveit, const bool &centerwindow);
     bool isRunning();
     bool sendMessage(const QString &message);
     void fillPVtable(int &countPV, int &countnotConnected, int &countDisplayed);
     int ReadInteger(char *string, char **NextString);
     int parseGeometry(const char* string, int* x, int* y, int* width, int* height);
     void parse_and_set_Geometry(QMainWindow *w, QString parsestring);
     void shellCommand(QString command);
     void cycleWindows();

     void setAllEnvironmentVariables(const QString &fileName);
     void parseConfigFile(const QString &filename, QList<QString> &urls, QList<QString> &files);
     void saveConfigFile(const QString &filename, QList<QString> &urls, QList<QString> &files);


     QString getStatusBarContents();
     QString getLogFilePath();


     void MSQ_getPtrs(int &front, int &rear) {
             if (!sharedMemory.isAttached()) return;
             int *ptr1 = (int*) sharedMemory.data();
             front = *ptr1;
             int *ptr2 = ptr1 + 1;
             rear = *ptr2;
         }

         void MSQ_setPtrs(int front, int rear) {
             if (!sharedMemory.isAttached()) return;
             int *ptr1 = (int*) sharedMemory.data();
             *ptr1 = front;
             int *ptr2 = ptr1 + 1;
             *ptr2 = rear;
         }

         void MSQ_init() {
             MSQ_setPtrs(-1, -1);
         }

         bool MSQ_isFull() {
             MSQ_getPtrs(front, rear);

             if(front == 0 && rear == RingSize - 1){
                 return true;
             }
             if(front == rear + 1) {
                 return true;
             }
             return false;
         }

         bool MSQ_isEmpty() {
             MSQ_getPtrs(front, rear);

             if(front == -1) return true;
             else return false;
         }

         void MSQ_enQueue(_blop element) {
             MSQ_getPtrs(front, rear);

             if(MSQ_isFull()){
                 qDebug() << "caQtDM -- attach queue is full";
             } else {
                 if(front == -1) front = 0;
                 rear = (rear + 1) % RingSize;
                 char *ptr = (char*) (((char*) sharedMemory.data()) + (rear * BlopSize) + 2*sizeof(int));
                 memcpy((char*) ptr, &element.blop[0], BlopSize);
                 //qDebug() << "Inserted @" << rear;
                 MSQ_setPtrs(front, rear);
             }
             return;
         }

         _blop MSQ_deQueue() {
             _blop element;

             MSQ_getPtrs(front, rear);
             //qDebug() << front << rear;

             if(MSQ_isEmpty()){
                 return(empty);
             } else {
                 char *ptr = (char*) (((char*) sharedMemory.data()) + (front * BlopSize) + 2*sizeof(int));
                 memcpy(element.blop, (char*) ptr, BlopSize);
                 if(front == rear) {
                     front = -1;
                     rear = -1;
                 }
                 else {
                     front=(front+1) % RingSize;
                 }
                 MSQ_setPtrs(front, rear);
                 return(element);
             }
         }

         void MSQ_display()
         {
             int i = 0;

             if(MSQ_isEmpty()) {
                 qDebug() << "Empty Queue";
             } else {

                 MSQ_getPtrs(front, rear);

                 qDebug() << "Front -> " << front;
                 for(i=front; i!=rear;i=(i+1) % RingSize ) {
                     char *ptr = (char*) (((char*) sharedMemory.data()) + (i * BlopSize) + 2 * sizeof(int));
                     qDebug() << i << ptr;
                 }
                 char *ptr = (char*) (((char*) sharedMemory.data()) + (i * BlopSize) + 2 * sizeof(int));
                 qDebug() << i << ptr;
                 qDebug() << "Rear -> " << rear;
             }
         }

 private slots:
     void Callback_ActionTimed();
     void Callback_ActionDirect();
     void Callback_OpenButton();
     void Callback_ActionAbout();
     void Callback_ActionExit();
     void Callback_ActionHelp();
     void Callback_ActionReload();
     void Callback_ActionUnconnected();
     void Callback_EmptyCache();
     void Callback_OpenNewFile(const QString&, const QString&, const QString&, const QString&);
     void checkForMessage();
     void onReloadTimeout();
     void Callback_PVwindowExit();

#if QT_VERSION > 0x050000
     void onApplicationStateChange(Qt::ApplicationState state);
#endif

public slots:
     void doSomething() {
         printf("About to quit!\n");
#if defined linux || defined TARGET_OS_MAC
         // remove temporary file created by caQtDM for pipe reading
         if(lastFile.contains("qt-tempFile")) {
             QFile::remove(lastFile);
         }
#endif
         sharedMemory.detach();
     }
     void nextWindow();
     void Callback_IosExit();
     void Callback_ReloadWindow(QWidget*);
     void Callback_ReloadAllWindows();

protected:
#ifdef MOBILE
     virtual bool event(QEvent *);
#endif
     virtual void timerEvent(QTimerEvent *e);
     Qt::GestureType fingerSwipeGestureType;
     bool eventFilter(QObject *obj, QEvent *event);

signals:
   void messageAvailable(QString message);

private:
   void setDirectUpdateTypeOnRestart(const QDateTime);
     void closeEvent(QCloseEvent* ce);
     void FlushAllInterfaces();
     void TerminateAllInterfaces();
     void reload(QWidget *w);
     long long getAvailableMemory();

     QMainWindow *lastWindow;
     QString lastMacro, lastFile, lastGeometry, lastResizing;
     Ui::MainWindow ui;
     QSharedMemory sharedMemory;
     bool _isRunning;
     MessageWindow* messageWindow;
     MutexKnobData *mutexKnobData;
     bool userClose;

     QMainWindow *pvWindow;
     QTableWidget* pvTable;
     QTimer *timer;

     bool mustOpenFile;

     QString lastFilePath;

     bool printandexit;
     bool minimizeMessageWindow;

     int activWindow;
     bool debugWindow;
     bool fromIOS;

     QMap<QString, ControlsInterface*> interfaces;
     QMap<QString, QString> OptionList;

     struct Row {QPoint position; QString file; QString macro; QString resize;};

     QList<Row> reloadList;

     double caQtDM_TimeLeft, caQtDM_TimeOut;
     bool caQtDM_TimeOutEnabled;

     QMutex mutex;
     int front;
     int rear;
     _blop empty;

     QDateTime lastReloadTime;
 };

 #endif
