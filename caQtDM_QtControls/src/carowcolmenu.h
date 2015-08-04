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

#ifndef caRowColMenu_H
#define caRowColMenu_H

#include <QString>
#include <QSignalMapper>
#include <qtcontrols_global.h>
#include <epushbutton.h>
#include <imagepushbutton.h>

class QGridLayout;

class QTCON_EXPORT caRowColMenu : public QWidget
{

    Q_OBJECT

    Q_ENUMS(Stacking)
    Q_PROPERTY(QString label READ getLabel WRITE setLabel)
    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)
    Q_PROPERTY(QString labels READ getLabels WRITE setLabels)
    Q_PROPERTY(QString files READ getFiles WRITE setFiles)
    Q_PROPERTY(QString args READ getArgs WRITE setArgs)
    Q_PROPERTY(EPushButton::ScaleMode fontScaleMode READ fontScaleMode WRITE setFontScaleModeL)

public:

    enum Stacking {Menu, Row, Column, RowColumn, Hidden};
    Stacking getStacking() const { return thisStacking; }
    void setStacking(Stacking stacking);

    caRowColMenu(QWidget *parent);

    QColor getForeground() const {return thisForeColor;}
    void setForeground(QColor c);

    QColor getBackground() const {return thisBackColor;}
    void setBackground(QColor c);

    QString getLabels() const {return labels.join(";");}
    void setLabels(QString const &newL);

    QString getFiles() const {return files.join(";");}
    void setFiles(QString const &newL);

    QString getArgs() const {return args.join(";");}
    void setArgs(QString const &newL) ;

    QString getLabel() const {return thisLabel;}
    void setLabel(QString const &label);

    void setImage(QString const &image);

    void setFontScaleModeL(EPushButton::ScaleMode m);
    EPushButton::ScaleMode fontScaleMode();

    void updateLabel();
    void updateColors();
    void updateFontScaleMode();

protected:

       void populateCells();
       void resizeEvent(QResizeEvent *e);

signals:

    void clicked(int indx);
    void triggered(int indx);

private:

    enum {MAXITEMS = 16};
    QList<EPushButton*> cellsP;
    QList<ImagePushButton*> cellsI;
    QGridLayout  *grid;
    Stacking     thisStacking;
    int          numCells;
    QSignalMapper *signalMapper;
    QStringList   args;
    QStringList   labels;
    QStringList   files;
    QStringList   texts;
    QString thisLabel;
    QColor thisForeColor;
    QColor thisBackColor;
    QColor thisBackColorHover;
    QColor thisBorderColor;
    QPalette thisPalette;
    QString thisImage;
    int borderSize;
    EPushButton::ScaleMode thisScaleMode;
    int alpha;
};

#endif
