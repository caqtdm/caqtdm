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

#if defined(_MSC_VER)
   #include <windows.h>
#endif

#include "dmsearchfile.h"

#include <QtGui>

#include "fileopenwindow.h"
#include "caqtdm_lib.h"

#include <iostream>
#include <string>

#include <QFileDialog>
#include <QString>
#include "messagebox.h"

#ifdef linux
#include <sys/resource.h>
#include <sys/time.h>
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

class Sleep
{
public:
    static void msleep(unsigned long msecs)
    {
        QMutex mutex;
        mutex.lock();
        QWaitCondition waitCondition;
        waitCondition.wait(&mutex, msecs);
        mutex.unlock();
    }
};

/**
 * our main window (form) constructor
 */
FileOpenWindow::FileOpenWindow(QMainWindow* parent,  QString filename, QString macroString,
                               bool attach, bool minimize, QString geometry, bool printscreen, bool resizing): QMainWindow(parent)
{
    // definitions for last opened file
    lastWindow = (QMainWindow*) 0;
    lastMacro ="";
    lastFile = "";
    lastGeometry = geometry;
    userClose = false;
    printandexit = printscreen;
    allowResize = resizing;

    if(minimize) showMinimized ();

    // set window title without the whole path
    QString title("caQtDM ");
    title.append(BUILDVERSION);
    title.append(" Build=");
    title.append(__DATE__);
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
    //connect( this->ui.btnOpen, SIGNAL( clicked() ), this, SLOT(Callback_OpenButton()) );
    connect( this->ui.fileAction, SIGNAL( triggered() ), this, SLOT(Callback_OpenButton()) );
    connect( this->ui.aboutAction, SIGNAL( triggered() ), this, SLOT(Callback_ActionAbout()) );
    connect( this->ui.exitAction, SIGNAL( triggered() ), this, SLOT(Callback_ActionExit()) );
    connect( this->ui.reloadAction, SIGNAL( triggered() ), this, SLOT(Callback_ActionReload()) );
    connect( this->ui.unconnectedAction, SIGNAL( triggered() ), this, SLOT(Callback_ActionUnconnected()) );

    setWindowTitle(title);

    // message window used by library and here

/*  this was for an independant window */
#if INDEPENDENTWINDOW
    messageWindow = new MessageWindow(widget);
    if(minimize) messageWindow->showMinimized();
    messageWindow->setGeometry(305,0, 500, 150);
    messageWindow->setWindowIcon (QIcon(":/caQtDM.ico"));
    messageWindow->show();
    messageWindow->move(messageWindow->x(), 0);
#else
    // message window is now integrated in main window
    QWidget *widget =new QWidget();
    messageWindow = new MessageWindow(widget);
    messageWindow->setAllowedAreas(Qt::TopDockWidgetArea);
    QGridLayout *gridLayoutCentral = new QGridLayout(this->ui.centralwidget);
    QGridLayout *gridLayout = new QGridLayout();
    gridLayoutCentral->addLayout(gridLayout, 0, 0, 1, 1);
    gridLayout->addWidget(messageWindow, 0, 0, 1, 1);
    messageWindow->show();
#endif

    sharedMemory.setKey ("caQtDM shared memory");

    if (sharedMemory.attach()) {
        _isRunning = true;
        if(attach) {
            qDebug() << "caQtDM -- another instance of caQtDM detected ==> attach to it";
            QString message(filename);
            message.append(";");
            message.append(macroString);
            message.append(";");
            message.append(geometry);
            //qDebug() << "send a message with file, macro and geometry to it and exit "<< message;
            sendMessage(message);
            sharedMemory.detach();
            exit(0);
        } else {
            qDebug() << "caQtDM -- another instance of caQtDM detected, but no attach specified ==> standalone";
        }
    } else {
        QByteArray byteArray("0");
        _isRunning = false;
        // create shared memory with a default value to note that no message is available.
        if (!sharedMemory.create(255)) {
            qDebug("caQtDM -- Unable to create single instance.");
        } else {
            qDebug() << "caQtDM -- created shared memory";
            sharedMemory.lock();
            char *to = (char*)sharedMemory.data();
            const char *from = byteArray.data();
            memcpy(to, from, qMin(sharedMemory.size(), byteArray.size()));
            sharedMemory.unlock();
            // start checking for messages of other instances.
            timer = new QTimer(this);
            connect(timer, SIGNAL(timeout()), this, SLOT(checkForMessage()));
            timer->start(1000);
        }
    }

    // when file was specified, open it
    // when called from here on Windows, the actual size of the window
    // is not found => defer opening
    mustOpenFile = false;
    if(filename.size() > 0) {
        lastMacro = macroString;
        lastFile = filename;
        lastGeometry = geometry;
        mustOpenFile = true;
    }

    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint);

    // start a timer
    startTimer(1000);

    pvWindow = (QMainWindow*) 0;
    pvTable = (QTableWidget*) 0;
}

void FileOpenWindow::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    char asc[255];
    int countPV=0;
    int countNotConnected=0;
    int countDisplayed = 0;
    static int printIt = 0;
    static int timeout = 0;

    if(mustOpenFile) {
        mustOpenFile = false;
        Callback_OpenNewFile(lastFile, lastMacro, lastGeometry);
    }

    asc[0] = '\0';

#ifdef linux
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    sprintf(asc, "memory: %ld kB", usage.ru_maxrss);
#endif

    // any open windows ?
    // we want to ask with timeout if the application has to be closed. 23-jan-2013 no yust exit
    if(this->findChildren<CaQtDM_Lib *>().count() <= 0 && userClose) {
        if (sharedMemory.isAttached()) sharedMemory.detach();
        exit(0);
    } else if(this->findChildren<CaQtDM_Lib *>().count() > 0){
        userClose = true;
    }

    // any non connected pv's to display ?

    fillPVtable(countPV, countNotConnected, countDisplayed);

    sprintf(asc, "%s - PV=%d (%d NC)", asc, countPV, countNotConnected);
    statusBar()->showMessage(asc);

    // we wanted a print, do it when acquired, then exit
    if(printandexit) {
        if(countPV > 0 && countNotConnected == 0) {
            if(this->findChildren<CaQtDM_Lib *>().count() == 1) {
                CaQtDM_Lib * widget = this->findChild<CaQtDM_Lib *>();
                if(countDisplayed > 0 && countDisplayed == countPV) {
                    printIt++;
                    if(printIt > 2) {
                        widget->printPS("caQtDM.ps");
                        qDebug() << "caQtDM -- file has been printed to caQtDM.ps";
                        exit(1);
                    }
                }
            }
        }
        if(timeout++ > 4) {    // seems we did not get everything
            CaQtDM_Lib * widget = this->findChild<CaQtDM_Lib *>();
            widget->printPS("caQtDM.ps");
            qDebug() << "caQtDM -- file has been printed to caQtDM.ps, probably with errors";
            exit(1);
        }
    }
}

/**
 * slot for opening file by button
 */
void FileOpenWindow::Callback_OpenButton()
{
    //get a filename to open
    QString path = (QString)  getenv("CAQTDM_DISPLAY_PATH");
    if(path.size() == 0 && lastFilePath.size()==0) path.append(".");
    else path = lastFilePath;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open ui or prc file"), path, tr("ui/prc Files (*.ui *.prc)"));
    //std::cout << "Got filename: " << fileName.toStdString() << std::endl;

    if(!fileName.isNull()) {
        char asc[1000];
        QFileInfo fi(fileName);
        lastFilePath = fi.absolutePath();
        if(fi.exists()) {
            CaQtDM_Lib *newWindow = new CaQtDM_Lib(this, fileName, "", mutexKnobData, messageWindow);
            if (fileName.contains("prc")) {
                allowResize = false;
            }
            newWindow->allowResizing(allowResize);
            QMainWindow *mainWindow = newWindow;
            mainWindow->show();
            mainWindow->raise();
            mainWindow->setMinimumSize(0, 0);
            mainWindow->setMaximumSize(16777215, 16777215);
            mainWindow->setWindowFlags(mainWindow->windowFlags() );
            mainWindow->setProperty("fileString", fileName);
            mainWindow->setProperty("macroString", "");

            sprintf(asc, "last file: %s", fileName.toAscii().constData());
            messageWindow->postMsgEvent(QtDebugMsg, asc);

            if (fileName.contains("prc")) {
                mainWindow->resize(mainWindow->minimumSizeHint());
            }

        } else {
            QTDMMessageBox(QMessageBox::Warning, "file open error", "does not exist", QMessageBox::Close, this, Qt::Popup, true);
        }
    }
}

/**
 * slot for opening file by signal
 */
void FileOpenWindow::Callback_OpenNewFile(const QString& inputFile, const QString& macroString, const QString& geometry)
{
    //qDebug() << "*************************************************************************";
    //qDebug() << "callback open new file" << inputFile << "with macro string" << macroString << "geometry=" << geometry;

    int found1 = inputFile.lastIndexOf(".ui");
    int found2 = inputFile.lastIndexOf(".adl");
    int found3 = inputFile.lastIndexOf(".prc");
    QString openFile = inputFile;
    if (found1 != -1) {
        openFile = inputFile.mid(0, found1);
    }
    if(found2 != -1) {
        openFile = inputFile.mid(0, found2);
    }

    QString FileName;
    if(found3 == -1)
        FileName = openFile.append(".ui");
    else
        FileName = inputFile;

    // go through the children of this main window and find out if new or already present
    QList<QWidget *> all = this->findChildren<QWidget *>();
    foreach(QWidget* widget, all) {
        if(QMainWindow* w = qobject_cast<QMainWindow *>(widget)) {
            QString WindowProperty = "";
            // if already exists then yust pop it up
            QFile *file = new QFile;
            file->setFileName(FileName);
            QString title(file->fileName().section('/',-1));
            title.append("&");
            title.append(macroString);
            delete file;

            QVariant fileName = w->property("fileString");
            QVariant macroString = w->property("macroString");
            //qDebug() << "existing filename with macro =" <<  w->windowTitle() << "=" <<  w->property("fileString") << macroString;
            if(!fileName.isNull()) {
                WindowProperty = fileName.toString();
            }
            WindowProperty.append("&");
            if(!macroString.isNull()) {
                WindowProperty.append(macroString.toString());
            }
            //qDebug() << title << WindowProperty;
            if(QString::compare(WindowProperty, title) == 0) {
                w->activateWindow();
                w->raise();
                w->showNormal();
                w->setFocus();
// all these past commands will only give you a notification in the taskbar
// in case of x windows, we will pop the window really up
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
    QString fileNameFound = s->findFile();
    if(fileNameFound.isNull()) {
        QString message = QString(FileName);
        message.append(" does not exist");
        QTDMMessageBox *m = new QTDMMessageBox(QMessageBox::Warning, "file open error", message, QMessageBox::Close, this, Qt::Dialog, true);
        m->show();
        //qDebug() << "sorry -- file" << FileName << "does not exist";
    } else {
        char asc[255];
        bool willPrint = false;
        qDebug() << "file" << fileNameFound << "will be loaded" << "macro=" << macroString << "geometry=" << geometry;

        //QMainWindow *mainWindow = new CaQtDM_Lib(this, fileNameFound, macroString, mutexKnobData, messageWindow);

        if(printandexit) willPrint = true;
        CaQtDM_Lib *newWindow =  new CaQtDM_Lib(this, fileNameFound, macroString, mutexKnobData, messageWindow, willPrint);

        if (FileName.contains("prc")) {
           allowResize = false;
        }
        newWindow->allowResizing(allowResize);
        QMainWindow *mainWindow = newWindow;

        if(printandexit) {
            mainWindow->showMinimized();
        } else {
            mainWindow->show();
        }
        mainWindow->raise();
        mainWindow->setMinimumSize(0, 0);
        mainWindow->setMaximumSize(16777215, 16777215);
        mainWindow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mainWindow->setWindowFlags(mainWindow->windowFlags() );

        // get the filename
        QFile *file = new QFile;
        file->setFileName(fileNameFound);
        QString title(file->fileName().section('/',-1));
        mainWindow->setProperty("fileString", title);
        mainWindow->setProperty("macroString", macroString);
        delete file;

        //qDebug() << "set properties in qmainwindow" << mainWindow << title << macroString;

#ifdef Q_WS_X11
        if(geometry != "") {
            parse_and_set_Geometry(mainWindow, geometry);
        }
#endif
        // for this kind of file we resize to a minimum while the size is not known
        if (FileName.contains("prc")) {
            mainWindow->resize(mainWindow->minimumSizeHint());
        }

        if(macroString.size() > 0) {
          sprintf(asc, "last file: %s, macro: %s", fileNameFound.toAscii().constData(), macroString.toAscii().constData());
        } else {
          sprintf(asc, "last file: %s", fileNameFound.toAscii().constData());
        }
        messageWindow->postMsgEvent(QtDebugMsg, asc);
    }
    delete s;
}

/**
 * slot for icon signal
 */
void FileOpenWindow::Callback_ActionAbout()
{
    QString message = QString("Qt-based Epics Display Manager Version %1 using %2 with data from %3 developed at Paul Scherrer Institut, by Anton Mezger\n");
    message = message.arg(BUILDVERSION, BUILDARCH, SUPPORT);
    QTDMMessageBox *m = new QTDMMessageBox(QMessageBox::Information, "About", message, QMessageBox::Close, this, Qt::Dialog, true);
    m->show();
}

/**
 * slot for exit signal
 */
void FileOpenWindow::Callback_ActionExit()
{
    QString message = QString("Are you sure to want to exit?");
    QTDMMessageBox *m = new QTDMMessageBox(QMessageBox::Warning, "Exit", message, QMessageBox::Yes | QMessageBox::No, this, Qt::Dialog, false);
    m->show();
    int selected = m->exec();
    if(selected == QMessageBox::Yes) {

// we are first going to close all open windows
        // go through the children of the main window and find out if that window still exists
        QList<QWidget *> all = this->findChildren<QWidget *>();
        foreach(QWidget* widget, all) {
            if(QMainWindow* w = qobject_cast<QMainWindow *>(widget)) {
                if(processWindow *w1 =  qobject_cast<processWindow *>(widget)) {
                    Q_UNUSED(w1);
                   // do not close processes
                } else {
                  //qDebug() << "close window" << w;
                  w->close();
                }
            }
        }

// detach shared memory, delete pv container
        if (sharedMemory.isAttached()) sharedMemory.detach();
        delete mutexKnobData;
        exit(0);
    }
}

void FileOpenWindow::Callback_ActionReload()
{
    QPoint position;
    QString macroS = "";
    QString fileS = "";

    // go through all windows, close them and relaod them from files
    QList<QWidget *> all = this->findChildren<QWidget *>();
    foreach(QWidget* widget, all) {
        if(CaQtDM_Lib* w = qobject_cast<CaQtDM_Lib *>(widget)) {
            QVariant fileName = w->property("fileString");
            QVariant macroString = w->property("macroString");
            if(!macroString.isNull()) {
                macroS = macroString.toString();
            }

            position = w->pos();

            w->close();
            if(!fileName.isNull()) {
                fileS = fileName.toString();

                //QMainWindow *mainWindow = new CaQtDM_Lib(this, fileS, macroS, mutexKnobData, messageWindow);

                CaQtDM_Lib *newWindow =  new CaQtDM_Lib(this, fileS, macroS, mutexKnobData, messageWindow);
                newWindow->allowResizing(allowResize);
                QMainWindow *mainWindow = newWindow;

                mainWindow->show();
                mainWindow->move(position);
                mainWindow->raise();
                mainWindow->setMinimumSize(0, 0);
                mainWindow->setMaximumSize(16777215, 16777215);
                mainWindow->setWindowFlags( mainWindow->windowFlags() );
                mainWindow->setProperty("fileString", fileS);
                mainWindow->setProperty("macroString", macroS);

            }
        }
    }
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
    QStringList vars = message.split(";");

    //qDebug() << "received message=" << message;
    //qDebug() << "vars" << vars.count() <<  vars;

    emit Callback_OpenNewFile(vars.at(0), vars.at(1), vars.at(2));

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

/**
 * slot for unconnected channels button
 */
void FileOpenWindow::Callback_ActionUnconnected()
{
    int countPV=0;
    int countNotConnected=0;
    int countDisplayed = 0;

    if(pvWindow != (QMainWindow*) 0) {
        pvWindow->show();
        return;
    }
    pvWindow = new QMainWindow();
    pvWindow->setWindowTitle(QString::fromUtf8("unconnected PV's"));
    pvWindow->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint);

    QVBoxLayout *l = new QVBoxLayout();

    pvWindow->resize(400, 250);

    pvTable = new QTableWidget();

    QPushButton *pushbutton = new QPushButton("close");
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(Callback_PVwindowExit()));

    l->addWidget(pvTable);
    l->addWidget(pushbutton);

    QWidget* widg = new QWidget();
    widg->setLayout(l);

    fillPVtable(countPV, countNotConnected, countDisplayed);

    pvWindow->setCentralWidget(widg);
    pvWindow->show();

    // set width of window
    int w = 0;
    int count = pvTable->columnCount();
    for (int i = 0; i < count; i++) w += pvTable->columnWidth(i);
    int maxW = (w + count + pvTable->verticalHeader()->width() + pvTable->verticalScrollBar()->width());
    pvWindow->setMinimumWidth(maxW+25);
}

void FileOpenWindow::Callback_PVwindowExit()
{
    pvWindow->hide();
}

void FileOpenWindow::fillPVtable(int &countPV, int &countNotConnected, int &countDisplayed)
{
    int count = 0;
    if(pvTable != (QTableWidget*) 0) {
        pvTable->clear();
        pvTable->setColumnCount(3);
        pvTable->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        pvTable->setHorizontalHeaderLabels(QString("unconnected PV;object;filename").split(";"));
        pvTable->setAlternatingRowColors(true);
    }

    for (int i=0; i < mutexKnobData->GetMutexKnobDataSize(); i++) {
        knobData *kPtr = mutexKnobData->GetMutexKnobDataPtr(i);
        if(kPtr->index != -1) {
            if(!kPtr->edata.connected) {
                countNotConnected++;
            } else {
                if(kPtr->edata.displayCount > 0) countDisplayed++;
            }
            countPV++;
        }
    }

    if(pvTable != (QTableWidget*) 0) {
        pvTable->setRowCount(countNotConnected);
        count = 0;
        for (int i=0; i < mutexKnobData->GetMutexKnobDataSize(); i++) {
            knobData *kPtr = mutexKnobData->GetMutexKnobDataPtr(i);
            if(kPtr->index != -1) {
                if(!kPtr->edata.connected) {
                    pvTable->setItem(count,0, new QTableWidgetItem(kPtr->pv));
                    pvTable->setItem(count,1, new QTableWidgetItem(kPtr->dispName));
                    pvTable->setItem(count,2, new QTableWidgetItem(kPtr->fileName));
                    count++;
                }
            }
        }
        pvTable->resizeColumnsToContents();
    }
}

#ifdef Q_WS_X11
/**
 *   in medm geometry is passed through XParseGeometry:
 *    XParseGeometry parses strings of the form
 *   "=<width>x<height>{+-}<xoffset>{+-}<yoffset>", where
 *   width, height, xoffset, and yoffset are unsigned integers.
 *   Example:  "=80x24+300-49"
 *   The equal sign is optional.
 *   It returns a bitmask that indicates which of the four values
 *   were actually found in the string. For each value found,
 *   the corresponding argument is updated;  for each value
 *   not found, the corresponding argument is left unchanged.
 *
 *   here we implement the following routines to implement the
 *   same behaviour
 */

int FileOpenWindow::ReadInteger(char *string, char **NextString)
{
    register int Result = 0;
    int Sign = 1;
    if (*string == '+') string++;
    else if (*string == '-') {
        string++;
        Sign = -1;
    }

    for (; (*string >= '0') && (*string <= '9'); string++) {
        Result = (Result * 10) + (*string - '0');
    }

    *NextString = string;

    if (Sign >= 0) return Result;
    else return -Result;
}

int FileOpenWindow::parseGeometry(const char* string, int* x, int* y, int* width, int* height)
{
    int mask = NoValue;
    register char *strind;
    unsigned int tempWidth=0, tempHeight=0;
    int tempX=0, tempY=0;
    char *nextCharacter;
    if (!string || (*string == '\0')) return mask;
    if (*string == '=') string++;  /* ignore possible '=' at beg of geometry spec */
    strind = const_cast<char *>(string);
    if (*strind != '+' && *strind != '-' && *strind != 'x') {
        tempWidth = ReadInteger(strind, &nextCharacter);
        if (strind == nextCharacter)
            return 0;
        strind = nextCharacter;
        mask |= WidthValue;
    }

    if (*strind == 'x' || *strind == 'X') {
        strind++;
        tempHeight = ReadInteger(strind, &nextCharacter);
        if (strind == nextCharacter) return 0;
        strind = nextCharacter;
        mask |= HeightValue;
    }

    if ((*strind == '+') || (*strind == '-')) {
        if (*strind == '-') {
            strind++;
            tempX = -ReadInteger(strind, &nextCharacter);
            if (strind == nextCharacter) return 0;
            strind = nextCharacter;
            mask |= XNegative;
        } else {
            strind++;
            tempX = ReadInteger(strind, &nextCharacter);
            if (strind == nextCharacter) return 0;
            strind = nextCharacter;
        }
        mask |= XValue;

        if ((*strind == '+') || (*strind == '-')) {
            if (*strind == '-') {
                strind++;
                tempY = -ReadInteger(strind, &nextCharacter);
                if (strind == nextCharacter) return 0;
                strind = nextCharacter;
                mask |= YNegative;
            } else {
                strind++;
                tempY = ReadInteger(strind, &nextCharacter);
                if (strind == nextCharacter) return 0;
                strind = nextCharacter;
            }
            mask |= YValue;
        }
    }

    /* If strind isn't at the end of the string then it's an invalid geometry specification. */

    if (*strind != '\0') return 0;
    if (mask & XValue) *x = tempX;
    if (mask & YValue) *y = tempY;
    if (mask & WidthValue)  *width = tempWidth;
    if (mask & HeightValue) *height = tempHeight;

    return mask;
}

void FileOpenWindow::parse_and_set_Geometry(QMainWindow *widget, QString parsestring) {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    int m = parseGeometry(parsestring.toLatin1().data(), &x, &y, &w, &h);

    QSize minSize = widget->minimumSize();
    QSize maxSize = widget->maximumSize();
    if ((m & XValue) == 0)
        x = widget->geometry().x();

    if ((m & YValue) == 0)
        y = widget->geometry().y();

    if ((m & WidthValue) == 0)
        w = widget->width();

    if ((m & HeightValue) == 0)
        h = widget->height();

    w = qMin(w,maxSize.width());
    h = qMin(h,maxSize.height());
    w = qMax(w,minSize.width());
    h = qMax(h,minSize.height());

    if ((m & XNegative)) {
        x = qApp->desktop()->width()  + x - w;
        x -= (widget->frameGeometry().width() - widget->width()) / 2;
    } else {
        x += (widget->geometry().x() - widget->x());
    }

    if ((m & YNegative)) {
        y = qApp->desktop()->height() + y - h;
    } else {
        y += (widget->geometry().y() - widget->y());
    }
    //qDebug() << "set window" << w << "to" << x << y << w << h;
    widget->setGeometry(x, y, w, h);
}
#endif
