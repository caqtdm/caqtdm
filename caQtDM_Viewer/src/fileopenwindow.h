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
     void shellCommand(QString command);
     void cycleWindows();

     void setAllEnvironmentVariables(const QString &fileName);
     void parseConfigFile(const QString &filename, QList<QString> &urls, QList<QString> &files);
     void saveConfigFile(const QString &filename, QList<QString> &urls, QList<QString> &files);

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
     void Callback_OpenNewFile(const QString&, const QString&, const QString&);
     void checkForMessage();
     void Callback_PVwindowExit();

#if QT_VERSION > 0x050000
     void onApplicationStateChange(Qt::ApplicationState state);
#endif

 public slots:
     void doSomething() { printf("About to quit!\n"); sharedMemory.detach();}
     void nextWindow();
     void Callback_IosExit();

 protected:
     virtual void timerEvent(QTimerEvent *e);
     Qt::GestureType fingerSwipeGestureType;

signals:
   void messageAvailable(QString message);

 private:

     void closeEvent(QCloseEvent* ce);
     void FlushAllInterfaces();
     void TerminateAllInterfaces();
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
     bool minimizeMessageWindow;

     int activWindow;
     bool debugWindow;
     bool fromIOS;

     QMap<QString, ControlsInterface*> interfaces;

 };

 #endif
