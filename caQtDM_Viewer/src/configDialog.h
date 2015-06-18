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

#ifndef configDialog_H
#define configDialog_H

#include <QtGui>
#include <QDialog>
#include <QDir>
#include <QUrl>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QTableWidget>
#include <QRadioButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QApplication>

#include "specialFunctions.h"

class configDialog: public QWidget
{
    Q_OBJECT

public:
    configDialog(const bool debugWindow, const QList<QString> &urlString, const QList<QString> &indexFile, QSize desktopSize, QWidget *parent = 0);
    int openUrl();
    void executeQuery();
    void getChoice(QString &url, QString &file, QList<QString> &urls, QList<QString> &files, bool &debugWindow);
    QLineEdit *fileChoice;
    int NumberOfFiles();
    bool isClearConfig();
    bool isStartButtonClicked();
    bool isEscapeButtonClicked();
    void exec();

private slots:

    void clearUiClicked();
    void clearConfigClicked();
    void startClicked();

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    bool eventFilter(QObject *obj, QEvent *event);
    QPushButton *createButton(const QString &text, const char *member);
    void createfilesTable();

    QLabel *deviceLabel;
    QPushButton *cancelButton;
    QComboBox* fileComboBox;
    QComboBox* urlComboBox;

    QLabel *fileCountLabel;
    QComboBox *debugComboBox;
    bool ClearConfigButtonClicked;
    bool StartButtonClicked;
    bool EscapeButtonClicked;
    QEventLoop loop;
};

#endif
