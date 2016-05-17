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

#ifndef __processWindow_H__
#define __processWindow_H__

#include <QWidget>
#include <QDialogButtonBox>
#include <QTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QMainWindow>
#include <QProcess>
#include <QtGui>
#include <QVBoxLayout>
#include <QMessageBox>

#include "caQtDM_Lib_global.h"


class CAQTDM_LIBSHARED_EXPORT processWindow : public QMainWindow
{
    Q_OBJECT

public:
    processWindow(QWidget * = 0, bool display=true, QWidget * = 0);
    ~processWindow();

    bool isRunning();
    void start(QString command);
    QWidget *getProcessCaller();
    Q_PID getProcessID();

public slots:

    bool tryTerminate();

protected slots:
    void processFinished();
    void processStarted();
    void processError(QProcess::ProcessError err);
    void closeButtonClicked();
    void updateError();
    void updateText();

signals:

    void processClose();

protected:
    void closeEvent(QCloseEvent *);

private:
    QTextEdit *outputWindow, *debugWindow;
    QProcess *termProcess;
    QSplitter* splitter;
    bool displayWindow;
    QWidget *thisCaller;
    Q_PID thisPID;
};

#endif
