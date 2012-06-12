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
 
#include <QMainWindow>
#include <QSharedMemory>
#include "MessageWindow.h"
#include "dbrString.h"
#include "knobDefines.h"
#include "knobData.h"
#include "mutexKnobData.h"
#include "ui_main.h"
 
 class FileOpenWindow : public QMainWindow
 {
     Q_OBJECT
 
 public:
     FileOpenWindow(QMainWindow *parent = 0,  QString filename = "", QString macroString = "", bool attach = false);
     bool isRunning();
     bool sendMessage(const QString &message);

 private slots:
     void Callback_OpenButton();
     void Callback_ActionAbout();
     void Callback_ActionExit();
     void Callback_ActionReload();
     void Callback_OpenNewFile(const QString&, const QString&);
     void checkForMessage();


 protected:

         virtual void timerEvent(QTimerEvent *e);

signals:

   void messageAvailable(QString message);
 
 private:
     QMainWindow *lastWindow;
     QString lastMacro, lastFile;
     Ui::MainWindow ui;
     QSharedMemory sharedMemory;
     bool _isRunning;
     MessageWindow* messageWindow;
     MutexKnobData *mutexKnobData;
 };
 
 #endif
