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


#ifndef MYMESSAGEBOX_H
#define MYMESSAGEBOX_H

#include <QDialogButtonBox>
#include <QDialog>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QEventLoop>
#include <QStyle>
#include <QDesktopWidget>
#include <QApplication>
#include <QPainter>

#include "specialFunctions.h"

class myMessageBox : public QWidget
{
public:
    explicit myMessageBox(QWidget *parent = 0);
    void setText(QString strng) const;
    void exec();

protected:
    virtual void closeEvent(QCloseEvent *event);
    virtual void paintEvent(QPaintEvent *e);

private:
    QTextEdit *thisText;
    QDialogButtonBox *buttonBox;
    QEventLoop loop;
};



#endif
