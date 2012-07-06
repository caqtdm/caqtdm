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

#include <QtGui>

#include "fileopenwindow.h"
#include "caqtdm_lib.h"
#include "dmsearchfile.h"

#include <iostream>
#include <string>

#include <QFileDialog>
#include <QString>
#include "messagebox.h"

#include <sys/time.h>

#ifdef linux
#include <sys/resource.h>
#endif

#ifdef Q_WS_X11
        #include <QX11Info>
        #include <X11/Xutil.h>
        #include <X11/Xlib.h>
        #include <X11/Xatom.h>

        #define MESSAGE_SOURCE_OLD            0
        #define MESSAGE_SOURCE_APPLICATION    1
        #define MESSAGE_SOURCE_PAGER          2
#endif //Q_WS_X11


/**
 * our main window (form) constructor
 */
FileOpenWindow::FileOpenWindow(QMainWindow* parent,  QString filename, QString macroString, bool attach): QMainWindow(parent)
{
    // definitions for last opened file
    lastWindow = (QMainWindow*) 0;
    lastMacro ="";
    lastFile = "";
    userClose = false;

    // set window title without the whole path
    QString title("caQtDM ");
    title.append(BUILDVERSION);
    title.append(" Build=");
    title.append(BUILDDATE);
    title.append(" ");
    title.append(BUILDTIME);

    // create a class for exchanging data
    mutexKnobData = new MutexKnobData();
    MutexKnobDataWrapperInit(mutexKnobData);

    // create form
    ui.setupUi(this);
    setGeometry(0,0, 300, 150);
    this->statusBar()->show();

    // connect action buttons
    connect( this->ui.btnOpen, SIGNAL( clicked() ), this, SLOT(Callback_OpenButton()) );
    connect( this->ui.aboutAction, SIGNAL( triggered() ), this, SLOT(Callback_ActionAbout()) );
    connect( this->ui.exitAction, SIGNAL( triggered() ), this, SLOT(Callback_ActionExit()) );
    connect( this->ui.reloadAction, SIGNAL( triggered() ), this, SLOT(Callback_ActionReload()) );

    setWindowTitle(title);

    // message window used by library and here
    messageWindow = new MessageWindow();
    messageWindow->setGeometry(305,0, 500, 150);

    sharedMemory.setKey ("caQtDM shared memory");

    if (sharedMemory.attach()) {
        _isRunning = true;
        if(attach) {
            qDebug() << "another instance of caQtDM detected ==> attach to it";
            QString message(filename);
            message.append(";");
            message.append(macroString);
            //qDebug() << "send a message with file and macro to it and exit "<< message;
            sendMessage(message);
            sharedMemory.detach();
            exit(0);
        } else {
            qDebug() << "another instance of caQtDM detected, but no attach specified ==> standalone";
        }
    } else {
        QByteArray byteArray("0");
        _isRunning = false;
        // create shared memory with a default value to note that no message is available.
        if (!sharedMemory.create(100)) {
            qDebug("Unable to create single instance.");
        } else {
            qDebug() << "created share memory";
            sharedMemory.lock();
            char *to = (char*)sharedMemory.data();
            const char *from = byteArray.data();
            memcpy(to, from, qMin(sharedMemory.size(), byteArray.size()));
            sharedMemory.unlock();
            // start checking for messages of other instances.
            QTimer *timer = new QTimer(this);
            connect(timer, SIGNAL(timeout()), this, SLOT(checkForMessage()));
            timer->start(1000);
        }
    }

    // when file was specified, open it
    if(filename.size() > 0) Callback_OpenNewFile(filename, macroString);

    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint);

    // start a timer
    startTimer(1000);
}

void FileOpenWindow::timerEvent(QTimerEvent *event)
{
#ifdef linux
    char asc[255];
    struct rusage usage;
    int ret = getrusage(RUSAGE_SELF, &usage);
    sprintf(asc, "memory usage: %d kB", usage.ru_maxrss);
    statusBar()->showMessage( asc);
#endif

    // any open windows ?
    //qDebug() << this->findChildren<CaQtDM_Lib *>().count();

    // we want to ask with timeout if the application has to be closed
    if(this->findChildren<CaQtDM_Lib *>().count() <= 0 && userClose) {
        QString message = QString("no more open windows, do you want to exit?");
        MessageBox *m = new MessageBox(QMessageBox::Warning, "Exit", message, QMessageBox::Yes | QMessageBox::No, this, Qt::Dialog, true);
        m->show();
        int selected = m->exec();
        if(selected == QMessageBox::Yes) {
            if (sharedMemory.isAttached()) sharedMemory.detach();
            exit(0);
        } else if(selected == QMessageBox::No){
            m->deleteLater();
            userClose = false;
        } else {  // on timeout, the user does not seem to be interested
            if (sharedMemory.isAttached()) sharedMemory.detach();
            exit(0);
        }

    } else if(this->findChildren<CaQtDM_Lib *>().count() > 0){
        userClose = true;
    }
}

/**
 * slot for opening file by button
 */
void FileOpenWindow::Callback_OpenButton()
{
    //get a filename to open
    QString path = (QString)  getenv("CAQTDM_DISPLAY_PATH");
    if(path.size() == 0) path.append(".");
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open ui file"), path, tr("ui Files (*.ui)"));
    //std::cout << "Got filename: " << fileName.toStdString() << std::endl;

    if(!fileName.isNull()) {
        char asc[255];
        QFileInfo fi(fileName);
        if(fi.exists()) {
            QMainWindow *mainWindow = new CaQtDM_Lib(this, fileName, "", mutexKnobData, messageWindow);
            mainWindow->show();
            mainWindow->raise();

            lastWindow = mainWindow;
            lastMacro = "";
            lastFile = fileName;
            sprintf(asc, "last file: %s", lastFile.toAscii().constData());
            messageWindow->postMsgEvent(QtDebugMsg, asc);

        } else {
            MessageBox(QMessageBox::Warning, "file open error", "does not exist", QMessageBox::Close, this, Qt::Popup, true);
        }
    }
}

/**
 * slot for opening file by signal
 */
void FileOpenWindow::Callback_OpenNewFile(const QString& inputFile, const QString& macroString)
{
    //qDebug() << "*************************************************************************";
    //qDebug() << "callback open new file" << inputFile << "with macro string" << macroString;

    int found1 = inputFile.lastIndexOf(".ui");
    int found2 = inputFile.lastIndexOf(".adl");
    QString openFile = inputFile;
    if (found1 != -1) {
        openFile = inputFile.mid(0, found1);
    }
    if(found2 != -1) {
        openFile = inputFile.mid(0, found2);
    }

    QString FileName = openFile.append(".ui");

    // go through the children of this main window and find out if new or already present
    QList<QWidget *> all = this->findChildren<QWidget *>();
    foreach(QWidget* widget, all) {
        if(QMainWindow* w = qobject_cast<QMainWindow *>(widget)) {
            // if already exists then yust pop it up
            QFile *file = new QFile;
            file->setFileName(FileName);
            QString title(file->fileName().section('/',-1));
            title.append("&");
            title.append(macroString);
            delete file;
            if(QString::compare(w->windowTitle(), title) == 0) {
                w->activateWindow();
                w->raise();
                w->showNormal();
                w->setFocus();
// all these past commands will only give you a notification in the taskbar
// in case of x windows, we will poup the window really up
#ifdef Q_WS_X11
                static Atom  NET_ACTIVE_WINDOW = 0;
                XClientMessageEvent xev;
                if (NET_ACTIVE_WINDOW == 0) {
                    Display *dpy      = QX11Info::display();
                    NET_ACTIVE_WINDOW = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
                }
                xev.type         = ClientMessage;
                xev.window       = w->winId();
                xev.message_type = NET_ACTIVE_WINDOW;
                xev.format       = 32;
                xev.data.l[0]    = MESSAGE_SOURCE_PAGER;
                xev.data.l[1]    = QX11Info::appUserTime();
                xev.data.l[2]    = xev.data.l[3] = xev.data.l[4] = 0;

                XSendEvent(QX11Info::display(), QX11Info::appRootWindow(), False, SubstructureNotifyMask | SubstructureRedirectMask, (XEvent*)&xev);
#endif //Q_WS_X11

                return;
            }
        }
    }

    // open file
    dmsearchFile *s = new dmsearchFile(FileName);

    if(s->findFile().isNull()) {
        QString message = QString(FileName);
        message.append(" does not exist");
        MessageBox *m = new MessageBox(QMessageBox::Warning, "file open error", message, QMessageBox::Close, this, Qt::Dialog, true);
        m->show();
        //qDebug() << "sorry -- file" << FileName << "does not exist";
    } else {
        char asc[255];
        //qDebug() << "file" << s->findFile() << "will be loaded" << "macro=" << macroString;
        QMainWindow *mainWindow = new CaQtDM_Lib(this, s->findFile(), macroString, mutexKnobData, messageWindow);
        mainWindow->show();
        mainWindow->raise();

        lastWindow = mainWindow;
        lastMacro = macroString;
        lastFile = s->findFile();
        sprintf(asc, "last file: %s", lastFile.toAscii().constData());
        messageWindow->postMsgEvent(QtDebugMsg, asc);
    }
    delete s;
}

/**
 * slot for about signal
 */
void FileOpenWindow::Callback_ActionAbout()
{
    QString message = QString("Qt-based Epics Display Manager Version %1 developed at Paul Scherrer Institut, by Anton Mezger\n");
    message = message.arg(BUILDVERSION);
#ifdef ACS
    message.append("ACS enabled\n");
#endif
    MessageBox *m = new MessageBox(QMessageBox::Information, "About", message, QMessageBox::Close, this, Qt::Dialog, true);
    m->show();
}

/**
 * slot for exit signal
 */
void FileOpenWindow::Callback_ActionExit()
{
    QString message = QString("Are you sure to want to exit?");
    MessageBox *m = new MessageBox(QMessageBox::Warning, "Exit", message, QMessageBox::Yes | QMessageBox::No, this, Qt::Dialog, false);
    m->show();
    int selected = m->exec();
    if(selected == QMessageBox::Yes) {
        if (sharedMemory.isAttached()) sharedMemory.detach();
        delete mutexKnobData;
        exit(0);
    }
}

void FileOpenWindow::Callback_ActionReload()
{
    char asc[255];
    if(lastWindow == (QMainWindow*) 0) return;
    if(lastFile.size() <= 0) return;
    // go through the children of the main window and find out if that window still exists
    QList<QWidget *> all = this->findChildren<QWidget *>();
    foreach(QWidget* widget, all) {
        if(QMainWindow* w = qobject_cast<QMainWindow *>(widget)) {
            if(lastWindow == w) {
                lastWindow->close();
                break;
            }
        }
    }

    QMainWindow *mainWindow = new CaQtDM_Lib(this, lastFile, lastMacro, mutexKnobData, messageWindow);
    mainWindow->show();
    mainWindow->raise();

    lastWindow = mainWindow;
    sprintf(asc, "last file: %s", lastFile.toAscii().constData());
    messageWindow->postMsgEvent(QtDebugMsg, asc);
}

void FileOpenWindow::checkForMessage()
{
    // check for message in memory
    sharedMemory.lock();
    QByteArray byteArray = QByteArray((char*)sharedMemory.constData(), sharedMemory.size());
    sharedMemory.unlock();

    if (byteArray.left(1) == "0") return;  // no message, quit
    byteArray.remove(0, 1);                // remove first character
    QString message = QString::fromUtf8(byteArray.constData()); // get and split message in filename and macro
    QStringList vars = message.split(";", QString::SkipEmptyParts);
    if(vars.count() == 2) {
        emit Callback_OpenNewFile(vars.at(0), vars.at(1));
    } else if(vars.count() == 1) {
        emit Callback_OpenNewFile(vars.at(0), "");
    }

    // remove message from shared memory.
    byteArray = "0";
    sharedMemory.lock();
    char *to = (char*)sharedMemory.data();
    const char *from = byteArray.data();
    memcpy(to, from, qMin(sharedMemory.size(), byteArray.size()));
    sharedMemory.unlock();
}

bool FileOpenWindow::isRunning()
{
    return _isRunning;
}

bool FileOpenWindow::sendMessage(const QString &message)
{
    if (!_isRunning) return false;
    QByteArray byteArray("1");
    byteArray.append(message.toUtf8());
    byteArray.append('\0');
    sharedMemory.lock();
    char *to = (char*)sharedMemory.data();
    const char *from = byteArray.data();
    memcpy(to, from, qMin(sharedMemory.size(), byteArray.size()));
    sharedMemory.unlock();
    return true;
}

