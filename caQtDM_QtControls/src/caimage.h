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

#ifndef CAIMAGE_H
#define CAIMAGE_H

#include <QVBoxLayout>
#include <QLabel>
#include <QMovie>
#include <QMenu>
#include <QMouseEvent>
#include <qtcontrols_global.h>
#include "messageQueue.h"

class QTCON_EXPORT caImage : public QWidget
{

    Q_OBJECT

    Q_PROPERTY(QString filename READ getFileName WRITE setFileName)
    Q_PROPERTY(QString imageCalc READ getImageCalc WRITE setImageCalc)
    Q_PROPERTY(int frame READ getFrame WRITE setFrame)
    Q_PROPERTY(int delayMilliseconds READ getDelay WRITE setDelay)

#include "caVisibProps.h"
#include "caVisibDefs.h"

public:

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
    QString getMessages();
    bool anyMessages();



virtual ~caImage();


private slots:


protected:

    virtual void timerEvent(QTimerEvent *e);

private:

    void init(const QString& filename);

    messageQueue *messagequeue;
    QPointer<QLabel> _container;
    QPointer<QMovie> _animation;
    QVBoxLayout* _layout;
    QString thisFileName;
    int thisFrame, thisDelay;
    int prevFrame;
    QString thisImageCalc;
    int timerId;
    QColor oldColor;
};

#endif
