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

#ifndef sliderDialog_H
#define sliderDialog_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QList>
#include <QLabel>
#include <QPushButton>
#include <QPair>
#include <QString>
#include "caslider.h"
#include "dbrString.h"
#include "knobDefines.h"
#include "knobData.h"
#include "mutexKnobData.h"
#include "specialFunctions.h"

#include <QEventLoop>
#include <QDesktopWidget>
#include <QApplication>
#include <QGroupBox>
#include <QPainter>

 class QDialogButtonBox;
 class QLabel;
 class QLineEdit;
 class QTextEdit;
 class QWidget;

 class sliderDialog : public QWidget
{
     Q_OBJECT

 public:
     sliderDialog(caSlider *w, MutexKnobData *data, const QString &title, QWidget *parent);
     void exec();

 private slots:
     void cancelClicked();
     void applyClicked();

 protected:
     virtual void closeEvent(QCloseEvent *event);
     virtual void paintEvent(QPaintEvent *e);

 private:
     QString thisPV;

     QDialogButtonBox *buttonBox;

     QEventLoop loop;

     caSlider *slider;
     QLineEdit  *incrementEdit, *valueEdit;
 };


 #endif
