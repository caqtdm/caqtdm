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

#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <QWidget>

class  rectangle : public QWidget
{
    Q_OBJECT

public:

    rectangle( QWidget *parent = 0 );

    QColor readFG() const {return thisForeColor;}
    void writeFG(QColor c);

    QColor readBG() const {return thisBackColor;}
    void writeBG(QColor c);

    int value() const;

    void setLineColor( QColor c );
    QColor getLineColor() { return m_lineColor;}

    void setLineSize( int size );
    int getLineSize() {return m_lineSize;}


public slots:

    void setValue( int );

signals:

    void valueChanged( int );

protected:

    void paintEvent( QPaintEvent* );

private:

    int m_value;
    QColor m_lineColor;
    int m_lineSize;

    QColor thisForeColor;
    QColor thisBackColor;
};

#endif // RECTANGLE
