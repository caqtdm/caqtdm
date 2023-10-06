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

#include "processWindow.h"

processWindow::processWindow(QWidget *parent, bool display, QWidget *caller): QMainWindow(parent)
{
    outputWindow = (QTextEdit *) Q_NULLPTR;
    debugWindow = (QTextEdit *) Q_NULLPTR;
    splitter = (QSplitter *) Q_NULLPTR;
    displayWindow = display;
    thisCaller = caller;
    thisPID =(Q_PID) Q_NULLPTR;

    termProcess = new QProcess;
    if(!display) return;

    QVBoxLayout *l = new QVBoxLayout;

    QDialogButtonBox *box = new QDialogButtonBox( Qt::Horizontal );
    QPushButton *button = new QPushButton( "Close (will also kill process if running)" );
    connect( button, SIGNAL(clicked()), this, SLOT(closeButtonClicked()) );
    box->addButton(button, QDialogButtonBox::RejectRole );

    splitter= new QSplitter(Qt::Vertical);
    outputWindow = new QTextEdit(splitter);
    debugWindow = new QTextEdit(splitter);
    outputWindow->setReadOnly(true);
    debugWindow->setReadOnly(true);

    l->addWidget(splitter);
    l->addWidget(button);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(l);
    centralWidget->layout()->setContentsMargins(0,0,0,0);
    setCentralWidget(centralWidget);
    resize(640, 480);
    show();
}

processWindow::~processWindow()
{
    if(outputWindow != (QTextEdit *) Q_NULLPTR) outputWindow->deleteLater();
    if(debugWindow != (QTextEdit *) Q_NULLPTR) debugWindow->deleteLater();
    if(splitter != (QSplitter *) Q_NULLPTR) splitter->deleteLater();
    termProcess->deleteLater();
}

QWidget* processWindow::getProcessCaller() {
     return thisCaller;
}

Q_PID processWindow::getProcessID()
{
    return thisPID;
}

void processWindow::closeButtonClicked()
{
    tryTerminate();
}

bool processWindow::tryTerminate()
{
    if(termProcess && termProcess->state() == QProcess::Running) {
        termProcess->terminate();
        termProcess->waitForFinished(500);
        if( termProcess->state() == QProcess::Running) {
            qDebug() << "process still running, I will kill it";
            termProcess->kill();
        }
    }
    emit processClose();
    return true;
}


void processWindow::closeEvent(QCloseEvent *e)
{
    if (!tryTerminate()) {
        qDebug() << "processWindow -- Warning: cannot terminate process";
    } else {
        //qDebug() << "processWindow -- Process terminated";
    }
    e->accept();
}


bool processWindow::isRunning()
{
    return termProcess && termProcess->state() == QProcess::Running;
}

void processWindow::start(QString command)
{
    connect(termProcess, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(processFinished()));
    connect(termProcess, SIGNAL(started()), SLOT(processStarted()));
    connect(termProcess, SIGNAL(error(QProcess::ProcessError)), SLOT(processError(QProcess::ProcessError)));

    if(displayWindow) {
       connect(termProcess, SIGNAL(readyReadStandardError()), this, SLOT(updateError()));
       connect(termProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(updateText()));
       debugWindow->setText(command);
    }

    termProcess->start(command);
}

void processWindow::updateError()
{
    if(outputWindow == (QTextEdit *) Q_NULLPTR) return;
    QByteArray data = termProcess->readAllStandardError();
    QString text = outputWindow->toPlainText() + QString(data);
    outputWindow->setText(text);

    QTextCursor cursor = outputWindow->textCursor(); // retrieve  cursor
    cursor.movePosition(QTextCursor::End);           // move to the end of text
    outputWindow->setTextCursor(cursor);
}

void processWindow::updateText()
{
   if(outputWindow == (QTextEdit *) Q_NULLPTR) return;
    QByteArray data = termProcess->readAllStandardOutput();
    QString text = outputWindow->toPlainText() + QString(data);
    outputWindow->setText(text);

    QTextCursor cursor = outputWindow->textCursor(); // retrieve  cursor
    cursor.movePosition(QTextCursor::End);           // move to the end of text
    outputWindow->setTextCursor(cursor);
}

void processWindow::processFinished()
{
    if(outputWindow == (QTextEdit *) Q_NULLPTR) {
        tryTerminate();
        return;
    }
    QString text = outputWindow->toPlainText() + QString("process terminated");
    outputWindow->setText(text);

    QTextCursor cursor = outputWindow->textCursor(); // retrieve  cursor
    cursor.movePosition(QTextCursor::End);           // move to the end of text
    outputWindow->setTextCursor(cursor);
}

void processWindow::processStarted()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    thisPID = termProcess->pid();
#else
    thisPID = termProcess->processId();
#endif

}

void processWindow::processError(QProcess::ProcessError err)
{
    switch(err)
    {
    case QProcess::FailedToStart:
        QMessageBox::information(Q_NULLPTR,"FailedToStart","FailedToStart");
        break;
    case QProcess::Crashed:
        QMessageBox::information(Q_NULLPTR,"Crashed","Crashed");
        break;
    case QProcess::Timedout:
        QMessageBox::information(Q_NULLPTR,"FailedToStart","FailedToStart");
        break;
    case QProcess::WriteError:
        QMessageBox::information(Q_NULLPTR,"Timedout","Timedout");
        break;
    case QProcess::ReadError:
        QMessageBox::information(Q_NULLPTR,"ReadError","ReadError");
        break;
    case QProcess::UnknownError:
        QMessageBox::information(Q_NULLPTR,"UnknownError","UnknownError");
        break;
    default:
        QMessageBox::information(Q_NULLPTR,"default","default");
        break;
    }
}

