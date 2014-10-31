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
#include "qstandardpaths.h"

//#include <QMacStyle>

configDialog::configDialog(const bool debugWindow, const QList<QString> &urls, const QList<QString> &files, QWidget *parent): QWidget(parent)
{
    int thisWidth = 600;
    int thisHeight = 350;
    Qt::WindowFlags flags = Qt::Dialog;
    setWindowFlags(flags);
    setWindowModality (Qt::WindowModal);

    setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter, qApp->desktop()->size(), qApp->desktop()->availableGeometry()));

    QPixmap bg(":/caQtDM-BGL-2048.png");
    QMatrix rotated;
    //rotated.rotate(90);
    //bg = bg.transformed(rotated);
    bg = bg.scaled(qApp->desktop()->size());
    QPalette palette;
    palette.setBrush(QPalette::Background, bg);
    setPalette(palette);

    QFrame *frame = new QFrame();

    QRect rect((qApp->desktop()->size().width()-thisWidth)/2,
               (qApp->desktop()->size().height()-thisHeight)/2, thisWidth, thisHeight);
    frame->setFrameStyle(QFrame::StyledPanel || QFrame::Raised);
    frame->setLineWidth(5);

    QPalette framePalette = frame->palette();
    framePalette.setColor(backgroundRole(), QColor(210,210,210));
    frame->setPalette(framePalette);
    frame->setAutoFillBackground(true);
    frame->setGeometry(rect);
    frame->setFixedSize(thisWidth+30, thisHeight);
    frame->setMaximumSize(rect.width()+30, rect.height());

    ClearConfigButtonClicked = false;
    QGridLayout *mainLayout = new QGridLayout();
    mainLayout->addWidget(frame);

    QVBoxLayout *frameLayout = new QVBoxLayout();
    mainLayout->addLayout(frameLayout,0,0);
    frameLayout->setContentsMargins(15,15,15,15);

    QLabel *title = new QLabel("<h1>Start settings</h1>");
    title->setFixedWidth(thisWidth-20);
    title->setStyleSheet("QLabel {background-color : #aaffff; color : black; }");
    title->setAlignment(Qt::AlignCenter);
    frameLayout->addWidget(title, 0, Qt::AlignCenter);

    QGridLayout *clearLayout = new QGridLayout;
    QGroupBox* clearBox = new QGroupBox("Local ui/prc files");

    QString str= QString::number((int) NumberOfFiles());
    QLabel *label = new QLabel("Number:");
    fileCountLabel = new QLabel(str);
    clearLayout->addWidget(label, 0, 0);
    clearLayout->addWidget(fileCountLabel, 0, 1);

    QPushButton* clearConfigButton = new QPushButton("Clear config files");
    clearLayout->addWidget(clearConfigButton, 0, 2);
    connect(clearConfigButton, SIGNAL(clicked()), this, SLOT(clearConfigClicked()) );

    QPushButton* clearUiButton = new QPushButton("Clear ui files");
    clearLayout->addWidget(clearUiButton, 0, 3);
    connect(clearUiButton, SIGNAL(clicked()), this, SLOT(clearUiClicked()) );

    QLabel *debugLabel = new QLabel(" Message window:");
    clearLayout->addWidget(debugLabel, 0, 4);

    debugComboBox = new QComboBox();
    debugComboBox->setEditable(false);
    debugComboBox->addItem("No");
    debugComboBox->addItem("Yes");
    debugComboBox->setCurrentIndex(0);
    clearLayout->addWidget(debugComboBox, 0, 5);

    clearBox->setLayout(clearLayout);
    frameLayout->addWidget(clearBox);

    QGridLayout* urlLayout = new QGridLayout;
    QGroupBox* urlBox = new QGroupBox("Choose your url where your config file is located");
    urlComboBox = new QComboBox();
    urlComboBox->setEditable(true);
    urlComboBox->setInsertPolicy(QComboBox::InsertAtCurrent);
    for(int i=0; i< 5; i++) {
            if(i>=urls.length()) urlComboBox->addItem(QString::number(i+1));
            else  if(urls.at(i).trimmed().length() < 1) urlComboBox->addItem(QString::number(i+1));
            else urlComboBox->addItem(urls.at(i));
    }
    urlComboBox->setCurrentIndex(0);

    urlLayout->addWidget(urlComboBox, 0, 0);
    urlBox->setLayout(urlLayout);
    frameLayout->addWidget(urlBox);

    QGridLayout* fileLayout = new QGridLayout;
    QGroupBox* fileBox = new QGroupBox("Choose your config file at the above url");
    fileComboBox = new QComboBox();
    fileComboBox->setEditable(true);
    fileComboBox->setInsertPolicy(QComboBox::InsertAtCurrent);

    for(int i=0; i< 5; i++) {
        if(i>=files.length()) fileComboBox->addItem(QString::number(i+1));
        else  if(files.at(i).trimmed().length() < 1) fileComboBox->addItem(QString::number(i+1));
        else fileComboBox->addItem(files.at(i));
    }
    fileComboBox->setCurrentIndex(0);

    fileLayout->addWidget(fileComboBox, 0, 0);
    fileBox->setLayout(fileLayout);
    frameLayout->addWidget(fileBox);

    QPushButton *startButton = new QPushButton(QIcon(":/caQtDM.ico"), "Start");
    connect(startButton, SIGNAL(clicked()), this, SLOT(startClicked()) );

    frameLayout->addWidget(startButton);

    setLayout(mainLayout);
    showNormal();
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
    QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    path.append("/");
    QDir dir(path);
    dir.setNameFilters(QStringList() << "*.ui" << "*.prc");
    dir.setFilter(QDir::Files);
    foreach(QString dirFile, dir.entryList()) dir.remove(dirFile);

    QString str= QString::number((int) NumberOfFiles());
    fileCountLabel->setText(str);
}

void configDialog::clearConfigClicked()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
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
    QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    path.append("/");
    QDir dir(path);
    dir.setNameFilters(QStringList() << "*.ui" << "*.prc");
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
