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

#include "configDialog.h"

configDialog::configDialog(const bool debugWindow, const QList<QString> &urls, const QList<QString> &files, QSize desktopSize, QWidget *parent): QWidget(parent)
{ 
    Specials specials;
    int height;
    Q_UNUSED(debugWindow);

#ifndef MOBILE
    float COMBOHEIGHTFACTOR = 1.3;
#else
    float COMBOHEIGHTFACTOR = 1.0;
#endif

    Qt::WindowFlags flags = Qt::Dialog;
    setWindowFlags(flags);
    setWindowModality (Qt::WindowModal);

#ifndef MOBILE
    installEventFilter(this);
#endif

#ifdef MOBILE_ANDROID
    const QString buttonStyle = "background-color: lightgray; border-style: outset; border-width: 3px; border-radius: 10px; border-color: cyan; padding: 6px";
#endif
    // layout for the window, with margins as % of the display size
    QVBoxLayout* windowlayout = new QVBoxLayout;

#ifdef MOBILE
  #ifdef MOBILE_IOS
    setGeometry(0,0, desktopSize.width(), desktopSize.height());
    if(qApp->desktop()->heightMM() > 100) { // probably an ipad
        windowlayout->setContentsMargins(desktopSize.width() * 0.15, desktopSize.height() * 0.25, desktopSize.width() * 0.15, desktopSize.height() * 0.25);
    } else { // probably an iphone
        windowlayout->setContentsMargins(desktopSize.width() * 0.1, desktopSize.height() * 0.05, desktopSize.width() * 0.1, desktopSize.height() * 0.07);
    }
  #else
    setGeometry(0,0, desktopSize.width(), desktopSize.height());
    windowlayout->setContentsMargins(desktopSize.width() * 0.2, desktopSize.height() * 0.25, desktopSize.width() * 0.2, desktopSize.height() * 0.25);
  #endif
#else
    QDesktopWidget * Desktop = QApplication::desktop();
    QRect defscreengeo = Desktop->availableGeometry(-1);
    setGeometry(0,0, defscreengeo.width(), defscreengeo.height());
    desktopSize=defscreengeo.size();
    windowlayout->setContentsMargins(defscreengeo.width() * 0.2, defscreengeo.height() * 0.2, defscreengeo.width() * 0.2, defscreengeo.height() * 0.2);
#endif

    setLayout(windowlayout);

    QPixmap bg(":/caQtDM-BGL-2048.png");

    bg = bg.scaled(desktopSize-QSize(0,10));
    QPalette palette;
    palette.setBrush(QPalette::Background, bg);
    setPalette(palette);

    ClearConfigButtonClicked = false;
    StartButtonClicked = false;
    EscapeButtonClicked = false;
    QGridLayout *mainLayout = new QGridLayout();

    QFrame *frame = new QFrame();

    frame->setAutoFillBackground(true);
    frame->setObjectName("topFrame");
    frame->setStyleSheet("QFrame#topFrame {border:0px solid gray; border-radius: 15px; background: rgba(255,255,255,150);}");

    windowlayout->addWidget(frame); // add frame to layout

    // framelayout containing the other widgets
    QVBoxLayout *frameLayout = new QVBoxLayout();
    mainLayout->addLayout(frameLayout,0,0);
    frameLayout->setContentsMargins(5,5,5,0);

    // title
    QLabel *title = new QLabel("Start settings");
    specials.setNewStyleSheet(title, desktopSize, 22, 15, "background-color : #aaffff; color : black; ", 4);

    title->setAlignment(Qt::AlignCenter);
    frameLayout->addWidget(title, 0, Qt::AlignCenter);

    // first layout for first group
    QGridLayout *clearLayout = new QGridLayout;
    clearLayout->setSpacing(2);
    clearLayout->setContentsMargins(3, 3, 3, 3);

    // first group
    QGroupBox* clearBox = new QGroupBox("Local ui/prc/graphic files");
    specials.setNewStyleSheet(clearBox, desktopSize, 22, 13, 0);

    // first group, labels
    QString str= QString::number((int) NumberOfFiles());
    QLabel *label = new QLabel("Number:");
    fileCountLabel = new QLabel(str);
    clearLayout->addWidget(label, 0, 0);
    clearLayout->addWidget(fileCountLabel, 0, 1);

    // first group, clear config button
    QPushButton* clearConfigButton = new QPushButton("Clear config files");
#ifdef MOBILE_ANDROID
    specials.setNewStyleSheet(clearConfigButton, desktopSize, 22, 15, buttonStyle, 2);
#endif
    clearLayout->addWidget(clearConfigButton, 0, 2);
    connect(clearConfigButton, SIGNAL(clicked()), this, SLOT(clearConfigClicked()) );

    // adjust height
    height = clearConfigButton->minimumSizeHint().height();
    clearConfigButton->setMinimumHeight(height*COMBOHEIGHTFACTOR);

    // first group, clear ui button
    QPushButton* clearUiButton = new QPushButton("Clear ui files");
#ifdef MOBILE_ANDROID
    specials.setNewStyleSheet(clearUiButton, desktopSize, 22, 15, buttonStyle, 2);
#endif
    clearLayout->addWidget(clearUiButton, 0, 3);
    connect(clearUiButton, SIGNAL(clicked()), this, SLOT(clearUiClicked()) );

    // adjust height
    height = clearUiButton->minimumSizeHint().height();
    clearUiButton->setMinimumHeight(height*COMBOHEIGHTFACTOR);

    // first group, messages label
    QLabel *debugLabel = new QLabel(" Messages:");
    clearLayout->addWidget(debugLabel, 0, 4);

    // first group, combo yes/no
    debugComboBox = new QComboBox();
    debugComboBox->setEditable(false);
    debugComboBox->addItem("No");
    debugComboBox->addItem("Yes");
    debugComboBox->setCurrentIndex(0);
    clearLayout->addWidget(debugComboBox, 0, 5);

    // adjust height
    height = debugComboBox->minimumSizeHint().height();
    debugComboBox->setMinimumHeight(height*COMBOHEIGHTFACTOR);

    // add it
    clearBox->setLayout(clearLayout);
    frameLayout->addWidget(clearBox);

    // second layout for second group
    QGridLayout* urlLayout = new QGridLayout;
    urlLayout->setSpacing(2);
    urlLayout->setContentsMargins(3, 3, 3, 3);

    // second group
    QGroupBox* urlBox = new QGroupBox("Choose your url where your config file is located");
    specials.setNewStyleSheet(urlBox, desktopSize, 22, 13, "");

    // second group, second combo
    urlComboBox = new QComboBox();
    urlComboBox->setEditable(true);
    urlComboBox->setInsertPolicy(QComboBox::InsertAtCurrent);
    urlComboBox->setFocusPolicy(Qt::StrongFocus);
    for(int i=0; i< 5; i++) {
            if(i>=urls.length()) urlComboBox->addItem(QString::number(i+1));
            else  if(urls.at(i).trimmed().length() < 1) urlComboBox->addItem(QString::number(i+1));
            else urlComboBox->addItem(urls.at(i));
    }
    urlComboBox->setCurrentIndex(0);
    urlLayout->addWidget(urlComboBox, 0, 0);

    // adjust height
    height = urlComboBox->minimumSizeHint().height();
#ifdef MOBILE_ANDROID
    urlComboBox->setMinimumHeight(height*COMBOHEIGHTFACTOR*1.2);
#else
    urlComboBox->setMinimumHeight(height*COMBOHEIGHTFACTOR);
#endif

    // add it
    urlBox->setLayout(urlLayout);
    frameLayout->addWidget(urlBox);

    // third layout for third group
    QGridLayout* fileLayout = new QGridLayout;
    fileLayout->setSpacing(2);
    fileLayout->setContentsMargins(3, 3, 3, 3);

    // third group
    QGroupBox* fileBox = new QGroupBox("Choose your config file at the above url");
    specials.setNewStyleSheet(fileBox, desktopSize, 22, 13, "");

    // third group, third combo
    fileComboBox = new QComboBox();
    fileComboBox->setEditable(true);
    fileComboBox->setInsertPolicy(QComboBox::InsertAtCurrent);
    fileComboBox->setFocusPolicy(Qt::StrongFocus);

    for(int i=0; i< 5; i++) {
        if(i>=files.length()) fileComboBox->addItem(QString::number(i+1));
        else  if(files.at(i).trimmed().length() < 1) fileComboBox->addItem(QString::number(i+1));
        else fileComboBox->addItem(files.at(i));
    }
    fileComboBox->setCurrentIndex(0);
    fileLayout->addWidget(fileComboBox, 0, 0);

    // adjust height
    height = fileComboBox->minimumSizeHint().height();
#ifdef MOBILE_ANDROID
    fileComboBox->setMinimumHeight(height*COMBOHEIGHTFACTOR*1.2);
#else
    fileComboBox->setMinimumHeight(height*COMBOHEIGHTFACTOR);
#endif

    // add it
    fileBox->setLayout(fileLayout);
    frameLayout->addWidget(fileBox);

    // start button
    QPushButton *startButton = new QPushButton(QIcon(":/caQtDM.ico"), "Start");
#ifdef MOBILE_ANDROID
    specials.setNewStyleSheet(startButton, desktopSize, 22, 15, buttonStyle, 2);
#else
    specials.setNewStyleSheet(startButton, desktopSize, 22, 13, "");
#endif
    connect(startButton, SIGNAL(clicked()), this, SLOT(startClicked()) );
    frameLayout->addWidget(startButton);

    // adjust height
    height = startButton->minimumSizeHint().height();
    startButton->setMinimumHeight(height*COMBOHEIGHTFACTOR);

    // add message
    QString message = QString("Qt-based Epics Display Manager Version %1 (%2)  ");

    #if defined(_MSC_VER)
     message = message.arg(BUILDVERSION).arg(__DATE__);
    #else
     message = message.arg(BUILDVERSION).arg(BUILDDATE);
    #endif
    QLabel *version = new QLabel(message);
    version->setAlignment(Qt::AlignRight);
    specials.setNewStyleSheet(version, desktopSize, 22, 13, "background-color: transparent;", -4);

    // add it
    frameLayout->addWidget(version);

    // all should expand
    clearBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    urlBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    fileBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    frame->setLayout(mainLayout);
    show();
}

void configDialog::getChoice(QString &url, QString &file, QList<QString> &urls, QList<QString> &files, bool &debugWindow)
{
    urls.clear();
    files.clear();
    for(int i=0; i< 5; i++) {
        if(urlComboBox->itemText(i).length() > 1) urls.append(urlComboBox->itemText(i));
        url = urlComboBox->currentText();
    }
    for(int i=0; i< 5; i++) {
        if(fileComboBox->itemText(i).length() > 1) files.append(fileComboBox->itemText(i));
        file = fileComboBox->currentText();
    }
    if(debugComboBox->currentIndex() == 1) debugWindow = true;
    else debugWindow = false;
}

void configDialog::clearUiClicked()
{
    // get path for downloaded documents
    Specials specials;
    QString path = specials.getStdPath();

    path.append("/");
    QDir dir(path);
    dir.setNameFilters(QStringList() << "*.ui" << "*.prc" << "*.gif" << "*.jpg" << "*.png");
    dir.setFilter(QDir::Files);
    foreach(QString dirFile, dir.entryList()) dir.remove(dirFile);

    QString str= QString::number((int) NumberOfFiles());
    fileCountLabel->setText(str);
}

void configDialog::clearConfigClicked()
{
    // get path for downloaded documents
    Specials specials;
    QString path =  specials.getStdPath();

    path.append("/");
    QDir dir(path);
    dir.setNameFilters(QStringList() << "*.config" << "*.xml");
    dir.setFilter(QDir::Files);
    foreach(QString dirFile, dir.entryList()) dir.remove(dirFile);
    ClearConfigButtonClicked = true;
    close();
}

void configDialog::startClicked()
{
    StartButtonClicked = true;
    close();
}

bool configDialog::isEscapeButtonClicked()
{
    return EscapeButtonClicked;
}

bool configDialog::isStartButtonClicked()
{
    return StartButtonClicked;
}

bool configDialog::isClearConfig()
{
    return ClearConfigButtonClicked;
}

int configDialog::NumberOfFiles()
{
    int count = 0;
    // get path for downloaded documents
    Specials specials;
    QString path =  specials.getStdPath();

    path.append("/");
    QDir dir(path);
    dir.setNameFilters(QStringList() << "*.ui" << "*.prc" << "*.gif" << "*.jpg" << "*.png");
    dir.setFilter(QDir::Files);
    foreach(QString dirFile, dir.entryList()) count++;
    return count;
}

void configDialog::exec()
{
    loop.exec();
    close();
}

void configDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    loop.quit();
}

bool configDialog::eventFilter(QObject *obj, QEvent *event)
{
   if(event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
      QKeyEvent *me = static_cast<QKeyEvent *>(event);
      if(me->key() == Qt::Key_Return) {
           StartButtonClicked = true;
           close();
      } else  if(me->key() == Qt::Key_Escape) {
          EscapeButtonClicked = true;
          close();
      }
    }
    return QObject::eventFilter(obj, event);
}

