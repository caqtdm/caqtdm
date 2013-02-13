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

#ifndef CAIMAGE_H
#define CAIMAGE_H

#include <QVBoxLayout>
#include <QLabel>
#include <QMovie>
#include <QMenu>
#include <QMouseEvent>
#include <qtcontrols_global.h>

class QTCON_EXPORT caImage : public QWidget
{

    Q_OBJECT

    Q_PROPERTY(QString imageCalc READ getImageCalc WRITE setImageCalc)
    Q_PROPERTY(QString filename READ getFileName WRITE setFileName)
    Q_PROPERTY(int frame READ getFrame WRITE setFrame)
    Q_PROPERTY(int delayMilliseconds READ getDelay WRITE setDelay)

#include "caVisib.h"

public:

#include "caVisibPublic.h"

    caImage( QWidget *parent = 0 );

    QString getFileName() const {return thisFileName;}
    void setFileName(QString filename);

    int getFrame() const {return thisFrame;}
    void setFrame(int frame);

    int getDelay() const {return thisDelay;}
    void setDelay(int delay) {thisDelay = delay;}

    QString getImageCalc() const {return thisImageCalc;}
    void setImageCalc(QString const &calc) {thisImageCalc = calc;}

    int getFrameCount();
    void startMovie();
    void setInvalid(QColor c);
    void setValid();

virtual ~caImage();


private slots:


protected:

    virtual void timerEvent(QTimerEvent *e);

private:

#include "caVisibPrivate.h"

    void init(const QString& filename);

    QPointer<QLabel> _container;
    QPointer<QMovie> _animation;
    QVBoxLayout* _layout;
    QString thisFileName;
    int thisFrame, thisDelay;
    QString thisImageCalc;
    int timerId;
    QColor oldColor;
};

#endif
