#include "processWindow.h"


processWindow::processWindow(QWidget *parent, bool display, QWidget *caller): QMainWindow(parent)
{
    outputWindow = (QTextEdit *) 0;
    debugWindow = (QTextEdit *) 0;
    splitter = (QSplitter *) 0;
    displayWindow = display;
    thisCaller = caller;
    thisPID = 0;

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
    if(outputWindow != (QTextEdit *) 0) outputWindow->deleteLater();
    if(debugWindow != (QTextEdit *) 0) debugWindow->deleteLater();
    if(splitter != (QSplitter *) 0) splitter->deleteLater();
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
    if(outputWindow == (QTextEdit *) 0) return;
    QByteArray data = termProcess->readAllStandardError();
    QString text = outputWindow->toPlainText() + QString(data);
    outputWindow->setText(text);

    QTextCursor cursor = outputWindow->textCursor(); // retrieve  cursor
    cursor.movePosition(QTextCursor::End);           // move to the end of text
    outputWindow->setTextCursor(cursor);
}

void processWindow::updateText()
{
   if(outputWindow == (QTextEdit *) 0) return;
    QByteArray data = termProcess->readAllStandardOutput();
    QString text = outputWindow->toPlainText() + QString(data);
    outputWindow->setText(text);

    QTextCursor cursor = outputWindow->textCursor(); // retrieve  cursor
    cursor.movePosition(QTextCursor::End);           // move to the end of text
    outputWindow->setTextCursor(cursor);
}

void processWindow::processFinished()
{
    if(outputWindow == (QTextEdit *) 0) {
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
    thisPID = termProcess->pid();
}

void processWindow::processError(QProcess::ProcessError err)
{
    switch(err)
    {
    case QProcess::FailedToStart:
        QMessageBox::information(0,"FailedToStart","FailedToStart");
        break;
    case QProcess::Crashed:
        QMessageBox::information(0,"Crashed","Crashed");
        break;
    case QProcess::Timedout:
        QMessageBox::information(0,"FailedToStart","FailedToStart");
        break;
    case QProcess::WriteError:
        QMessageBox::information(0,"Timedout","Timedout");
        break;
    case QProcess::ReadError:
        QMessageBox::information(0,"ReadError","ReadError");
        break;
    case QProcess::UnknownError:
        QMessageBox::information(0,"UnknownError","UnknownError");
        break;
    default:
        QMessageBox::information(0,"default","default");
        break;
    }
}

