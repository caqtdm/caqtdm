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

#ifndef limitsDialog_H
#define limitsDialog_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QList>
#include <QPair>
#include <QString>
#include <QLabel>
#include <QEventLoop>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QDesktopWidget>
#endif
#include <QApplication>
#include <QGroupBox>
#include <QPainter>

#include "dbrString.h"
#include "knobDefines.h"
#include "knobData.h"
#include "mutexKnobData.h"
#include "inlines.h"
#include "caqtdm_lib.h"
#include "specialFunctions.h"

 class QCheckBox;
 class QDialogButtonBox;
 class QLabel;
 class QLineEdit;
 class QTableWidget;
 class QTextEdit;
 class QSpinBox;
 class QWidget;

 class limitsDialog : public QWidget
{
     Q_OBJECT

 public:
     limitsDialog(QWidget *w, MutexKnobData *data, const QString &title, QWidget *parent);
     void exec();

 private slots:
     void cancelClicked();
     void applyClicked();
     void indexChanged(int);

 protected:
     virtual void closeEvent(QCloseEvent *event);
     virtual void paintEvent(QPaintEvent *e);

 private:
     enum SourceMode {Channel = 0, User};
     int extractPrecisionFromFormat(const QString &fmt);
     QString getFormatFromPrecision(int prec);

     QWidget *thisWidget;
     QString thisPV;
     QComboBox *limitsComboBox;
     QComboBox *highLimitComboBox;
     QComboBox *lowLimitComboBox;
     QComboBox *precisionComboBox;
     QComboBox *formatComboBox;
     QLineEdit *minimumLineEdit;
     QLineEdit *maximumLineEdit;
     QSpinBox *precisionLineEdit;
     QSpinBox *integerLineEdit;
     QSpinBox *decimalLineEdit;
     MutexKnobData *monData;
     QDialogButtonBox *buttonBox;
     QEventLoop loop;
     double initMin, initMax;
     int initPrecision;
     bool doNothing;
     double channelLowerLimit, channelUpperLimit;
     int channelPrecision;

     static const int PREC_LIMIT_NUMERIC = 15;
 };

 #endif
