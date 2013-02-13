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
    Q_PROPERTY(EPushButton::ScaleMode fontScaleMode READ fontScaleMode WRITE setFontScaleMode)

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

    void setMouseTracking(bool en);

    QString getLabel() const {return thisLabel;}
    void setLabel(QString const &label);

    void setImage(QString const &image);

    void setFontScaleMode(EPushButton::ScaleMode m);
    EPushButton::ScaleMode fontScaleMode();

protected:

       void populateCells();
       void resizeEvent(QResizeEvent *e);

signals:

    void clicked(int indx);
    void triggered(int indx);

private:

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
};

#endif
