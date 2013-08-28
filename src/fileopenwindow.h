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
#include "MessageWindow.h"
#include "dbrString.h"
#include "knobDefines.h"
#include "knobData.h"
#include "mutexKnobData.h"
#include "ui_main.h"
#include <stdio.h>

 class FileOpenWindow : public QMainWindow
 {
     Q_OBJECT

 public:
     FileOpenWindow(QMainWindow *parent = 0,  QString filename = "", QString macroString = "",
                    bool attach = false, bool minimize = false, QString geometry = "", bool printscreen = false, bool resizing = true);
     bool isRunning();
     bool sendMessage(const QString &message);
     void fillPVtable(int &countPV, int &countnotConnected, int &countDisplayed);
     int ReadInteger(char *string, char **NextString);
     int parseGeometry(const char* string, int* x, int* y, int* width, int* height);
     void parse_and_set_Geometry(QMainWindow *w, QString parsestring);

 private slots:
     void Callback_OpenButton();
     void Callback_ActionAbout();
     void Callback_ActionExit();
     void Callback_ActionReload();
     void Callback_ActionUnconnected();
     void Callback_OpenNewFile(const QString&, const QString&, const QString&);
     void checkForMessage();
     void Callback_PVwindowExit();

 public slots:
     void doSomething() { printf("About to quit!\n"); sharedMemory.detach();}

 protected:
         virtual void timerEvent(QTimerEvent *e);

signals:
   void messageAvailable(QString message);

 private:
     QMainWindow *lastWindow;
     QString lastMacro, lastFile, lastGeometry;
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
     bool allowResize;
 };

 #endif
