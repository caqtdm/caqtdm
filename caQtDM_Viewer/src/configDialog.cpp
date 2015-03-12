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
    const QString buttonStyle = "background-color: lightgray; border-style: outset; border-width: 3px; border-radius: 10px; border-color: cyan; padding: 6px";
    Specials specials;
    int thisWidth = 600;  // normal for ipad
    int thisHeight = 370;

    Qt::WindowFlags flags = Qt::Dialog;
    setWindowFlags(flags);
    setWindowModality (Qt::WindowModal);

    QSize size = QSize(0,20);

    if(desktopSize.height() < 500) {
        thisWidth=430;  // normal for iphone
        thisHeight=275;
    }

    setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter, desktopSize - size , qApp->desktop()->availableGeometry()));

    // I do not know why this is necessary, but move to center of screen
#ifdef MOBILE_ANDROID
    const QRect screen = QApplication::desktop()->screenGeometry();
    move( screen.center() - this->rect().center() );
#endif

    qDebug() << "size=" << desktopSize << qApp->desktop()->devicePixelRatio() <<  qApp->desktop()->logicalDpiX();

    QPixmap bg(":/caQtDM-BGL-2048.png");

    bg = bg.scaled(desktopSize-QSize(0,10));
    QPalette palette;
    palette.setBrush(QPalette::Background, bg);
    setPalette(palette);

    ClearConfigButtonClicked = false;
    QGridLayout *mainLayout = new QGridLayout();

    QFrame *frame = new QFrame();
    QRect rect((desktopSize.width()-thisWidth)/2, (desktopSize.height()-thisHeight)/2, thisWidth, thisHeight);

    frame->setAutoFillBackground(true);
    frame->setGeometry(rect);
    frame->setFixedSize(thisWidth+30, thisHeight);
    frame->setMaximumSize(rect.width()+30, rect.height());

    specials.setNewStyleSheet(frame, desktopSize, 22, 15, "border:0px solid gray; border-radius: 15px; background: rgba(255,255,255,0.7); ");

    // I did not manage this on android, sizes are too different
#ifndef MOBILE_ANDROID
    mainLayout->addWidget(frame);
#endif

    QVBoxLayout *frameLayout = new QVBoxLayout();
    mainLayout->addLayout(frameLayout,0,0);
    frameLayout->setContentsMargins(5,5,5,0);

    QLabel *title = new QLabel("Start settings");
#ifndef MOBILE_ANDROID
    specials.setNewStyleSheet(title, desktopSize, 22, 15, "", 4);
#else
    specials.setNewStyleSheet(title, desktopSize, 30, 15, "", 4);
#endif
    title->setFixedWidth(thisWidth-20);
    title->setStyleSheet("QLabel {background-color : #aaffff; color : black; }");
    title->setAlignment(Qt::AlignCenter);
    frameLayout->addWidget(title, 0, Qt::AlignCenter);

    QGridLayout *clearLayout = new QGridLayout;
    clearLayout->setSpacing(2);
    clearLayout->setContentsMargins(3, 3, 3, 3);

    QGroupBox* clearBox = new QGroupBox("Local ui/prc/graphic files");
    specials.setNewStyleSheet(clearBox, desktopSize, 22, 15, "");
    QString str= QString::number((int) NumberOfFiles());
    QLabel *label = new QLabel("Number:");

    fileCountLabel = new QLabel(str);
    clearLayout->addWidget(label, 0, 0);
    clearLayout->addWidget(fileCountLabel, 0, 1);

    QPushButton* clearConfigButton = new QPushButton("Clear config files");
    clearLayout->addWidget(clearConfigButton, 0, 2);
    connect(clearConfigButton, SIGNAL(clicked()), this, SLOT(clearConfigClicked()) );

#ifdef MOBILE_ANDROID
    specials.setNewStyleSheet(clearConfigButton, desktopSize, 22, 15, buttonStyle, 2);
    int height = clearConfigButton->fontMetrics().boundingRect(clearConfigButton->text()).height() * 1.5;
    clearConfigButton->setFixedHeight(height);
#endif

    QPushButton* clearUiButton = new QPushButton("Clear ui files");
    clearLayout->addWidget(clearUiButton, 0, 3);
    connect(clearUiButton, SIGNAL(clicked()), this, SLOT(clearUiClicked()) );

#ifdef MOBILE_ANDROID
    specials.setNewStyleSheet(clearUiButton, desktopSize, 22, 15, buttonStyle, 2);
    clearUiButton->setFixedHeight(height);
#endif

    QLabel *debugLabel = new QLabel(" Messages:");
    clearLayout->addWidget(debugLabel, 0, 4);

    debugComboBox = new QComboBox();
    debugComboBox->setEditable(false);
    debugComboBox->addItem("No");
    debugComboBox->addItem("Yes");
    debugComboBox->setCurrentIndex(0);

#ifdef MOBILE_ANDROID
    specials.setNewStyleSheet(debugComboBox, desktopSize, 22, 15, "");
    height = debugComboBox->fontMetrics().boundingRect(debugComboBox->currentText()).height() * 1.0;
    debugComboBox->setFixedHeight(height);
#endif

    clearLayout->addWidget(debugComboBox, 0, 5);

    clearBox->setLayout(clearLayout);
    frameLayout->addWidget(clearBox);

    QGridLayout* urlLayout = new QGridLayout;
    urlLayout->setSpacing(2);
    urlLayout->setContentsMargins(3, 3, 3, 3);

    QGroupBox* urlBox = new QGroupBox("Choose your url where your config file is located");
    specials.setNewStyleSheet(urlBox, desktopSize, 22, 15, "");

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
#ifdef MOBILE_ANDROID
    specials.setNewStyleSheet(urlComboBox, desktopSize, 22, 15, "");
    height = urlComboBox->fontMetrics().boundingRect(urlComboBox->currentText()).height() * 2.0;
    urlComboBox->setFixedHeight(height);
#endif

    urlLayout->addWidget(urlComboBox, 0, 0);
    urlBox->setLayout(urlLayout);
    frameLayout->addWidget(urlBox);

    QGridLayout* fileLayout = new QGridLayout;
    fileLayout->setSpacing(2);
    fileLayout->setContentsMargins(3, 3, 3, 3);

    QGroupBox* fileBox = new QGroupBox("Choose your config file at the above url");
    specials.setNewStyleSheet(fileBox, desktopSize, 22, 15, "");

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

#ifdef MOBILE_ANDROID
    specials.setNewStyleSheet(fileComboBox, desktopSize, 22, 15, "");
    height = fileComboBox->fontMetrics().boundingRect(fileComboBox->currentText()).height() * 2.0;
    fileComboBox->setFixedHeight(height);
#endif

    fileLayout->addWidget(fileComboBox, 0, 0);
    fileBox->setLayout(fileLayout);
    frameLayout->addWidget(fileBox);

    QPushButton *startButton = new QPushButton(QIcon(":/caQtDM.ico"), "Start");
    connect(startButton, SIGNAL(clicked()), this, SLOT(startClicked()) );

#ifdef MOBILE_ANDROID
    specials.setNewStyleSheet(startButton, desktopSize, 22, 15, buttonStyle, 2);
    height = startButton->fontMetrics().boundingRect(startButton->text()).height() * 1.5;
    startButton->setFixedHeight(height);
#endif

    frameLayout->addWidget(startButton);
    QString message = QString("Qt-based Epics Display Manager Version %1 (%2)  ");
    message = message.arg(BUILDVERSION).arg(BUILDDATE);

    QLabel *version = new QLabel(message);
    version->setAlignment(Qt::AlignRight);
    specials.setNewStyleSheet(version, desktopSize, 22, 15, "background-color: transparent;", -4);

    frameLayout->addWidget(version);

    setLayout(mainLayout);
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
    QString path = specials.stdpathdoc;

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
    QString path = specials.stdpathdoc;

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
    close();
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
    QString path = specials.stdpathdoc;

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
    loop.quit();
}
