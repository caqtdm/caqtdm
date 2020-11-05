/****************************************************************************
**
** Copyright © 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
** Copyright © 2011 Evgeniy Degtyarev <degtep@gmail.com>
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "fingerswipegesture.h"
#include <QGesture>
#include <QWidget>
#include <QGraphicsObject>
#include <QEvent>
#include <QTouchEvent>
#include <QtDebug>
#include <QDesktopWidget>
#include <QApplication>

#include <QtControls>

FingerSwipeGestureRecognizer::FingerSwipeGestureRecognizer()
{
}

QGesture *FingerSwipeGestureRecognizer::create(QObject *target)
{
    if (target && target->isWidgetType()) {
        QWidget *widget = static_cast<QWidget *>(target);
        widget->setAttribute(Qt::WA_AcceptTouchEvents);
    } else if (target) {
        QGraphicsObject *go = qobject_cast<QGraphicsObject*>(target);
        if (go) {
            go->setAcceptTouchEvents(true);
        }
    }
    return new FingerSwipeGesture();
}

QGestureRecognizer::Result FingerSwipeGestureRecognizer::recognize(QGesture *state, QObject *obj, QEvent *event)
{
    FingerSwipeGesture *q = static_cast<FingerSwipeGesture *>(state);
    const QTouchEvent *ev = static_cast<const QTouchEvent *>(event); // only use ev after checking event->type()
    QGestureRecognizer::Result result = QGestureRecognizer::Ignore;

    QMainWindow *w1 = (QMainWindow *) obj;
    if(w1 == (QWidget*) nullptr) return result;

    switch (event->type()) {
    case QEvent::TouchBegin: {
        //qDebug() << "TouchBegin" << obj;
        result = QGestureRecognizer::MayBeGesture;
        if (ev->touchPoints().size() >= 1) {
            QTouchEvent::TouchPoint p1 = ev->touchPoints().first();
            q->m_startPos = p1.startScenePos();
            q->m_actPos = p1.startScenePos();
            q->m_touchupdate = false;

                QWidget *w2 = w1->childAt ((int)q->m_startPos.x(), (int)q->m_startPos.y());
                if(w2 != (QWidget*) nullptr) {
#ifdef MOBILE_ANDROID
                    if(caNumeric* numericWidget = qobject_cast<caNumeric *>(w2)) {
                        return QGestureRecognizer::Ignore;
                    } else if(caApplyNumeric* numericWidget = qobject_cast<caApplyNumeric *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    } else if(caSlider* sliderWidget = qobject_cast<caSlider *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    } else if(caMenu* sliderWidget = qobject_cast<caMenu *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    } else if(caChoice* sliderWidget = qobject_cast<caChoice *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    } else if(caRelatedDisplay* sliderWidget = qobject_cast<caRelatedDisplay *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    } else if(caTextEntry* sliderWidget = qobject_cast<caTextEntry *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    } else if(caMessageButton* sliderWidget = qobject_cast<caMessageButton *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    } else if(caToggleButton* sliderWidget = qobject_cast<caToggleButton *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    } else if(caSpinbox* sliderWidget = qobject_cast<caSpinbox *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    } else if(caByteController* sliderWidget = qobject_cast<caByteController *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    }
#else
                    if(caNumeric* foundWidget = qobject_cast<caNumeric *>(w2)) {
                        return QGestureRecognizer::Ignore;
                    } else if(caApplyNumeric* foundWidget = qobject_cast<caApplyNumeric *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    } else if(caSlider* foundWidget = qobject_cast<caSlider *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    } else if(caMenu* foundWidget = qobject_cast<caMenu *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    } else if(caChoice* foundWidget = qobject_cast<caChoice *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    } else if(caRelatedDisplay* foundWidget = qobject_cast<caRelatedDisplay *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    } else if(caTextEntry* foundWidget = qobject_cast<caTextEntry *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    } else if(caMessageButton* foundWidget = qobject_cast<caMessageButton *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    } else if(caToggleButton* foundWidget = qobject_cast<caToggleButton *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    } else if(caSpinbox* foundWidget = qobject_cast<caSpinbox *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    } else if(caByteController* foundWidget = qobject_cast<caByteController *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    }else if(QwtPlotCanvas* foundWidget = qobject_cast<QwtPlotCanvas *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    }else if(EPushButton* foundWidget = qobject_cast<EPushButton *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    }else if(QPushButton* foundWidget = qobject_cast<QPushButton *>(w2)) {
                        q->m_cancelled = true;
                        return QGestureRecognizer::Ignore;
                    }
#endif
                }
                q->m_begin = true;
                q->m_cancelled = false;
        }
        break;
    }
    case QEvent::TouchEnd: {
        //qDebug() << "TouchEnd" <<q->m_startPos.toPoint().x() << q->m_startPos.toPoint().y();
        QMainWindow *w1 = (QMainWindow *) obj;
        if (q->m_cancelled) {
            //qDebug() << "TouchEnd ignore";
            result = QGestureRecognizer::Ignore;
            return result;
        } else if (q->m_triggered) {
            //qDebug() << "TouchEnd finish";
            result = QGestureRecognizer::FinishGesture;
        } else {
            //qDebug() << "TouchEnd cancel";
            result = QGestureRecognizer::CancelGesture;
        }
        if (q->m_touchupdate)   {
           //qDebug() << "move";
           w1->move(q->m_startPos.toPoint().x(), q->m_startPos.toPoint().y());
           w1->show();
           if(!q->m_begin) w1->setCentralWidget(q->m_central);
        }
        break;
    }
    case QEvent::TouchUpdate: {
        if (!q->m_cancelled && ev->touchPoints().size() >= 1) {
            //qDebug() << "touchupdate true";
            QTouchEvent::TouchPoint p1 = ev->touchPoints().first();
            q->m_currentPos = p1.screenPos();
            //qDebug() << "touchupdate" <<q->m_currentPos.toPoint().x() << q->m_currentPos.toPoint().y();
            // update the hot-spot to be in the middle between start and current point
            q->setHotSpot(QPointF(p1.screenPos().x() - (q->m_currentPos.x() - q->m_startPos.x()) / 2,
                                  p1.screenPos().y() - (q->m_currentPos.y() - q->m_startPos.y()) / 2));
            if (!q->m_triggered) {
                q->m_triggered = true;
                result = QGestureRecognizer::TriggerGesture;
            }
            // here we move the window, but first replace the centralwidget in order to have faster interaction
            if((qAbs(q->m_currentPos.x() - q->m_actPos.x()) > 10) || (qAbs(q->m_currentPos.y() - q->m_actPos.y()) > 10)) {
                QMainWindow *w1 = (QMainWindow *) obj;
                if(q->m_begin) {

                    // get snaphot
                    //q->m_snapshot = QPixmap::grabWidget(w1->centralWidget());
                    #ifdef MOBILE_ANDROID
                        q->m_snapshot = w1->centralWidget()->grab();
                    #else
                        q->m_snapshot = w1->centralWidget()->grab(QGuiApplication::primaryScreen()->availableGeometry());
                    #endif
                    // save actual centralwidget
                    q->m_central = w1->centralWidget();
                    q->m_central->setParent(nullptr);  //now it is saved

                    // prepare new central widget with a label
                    QWidget *centralwidget = new QWidget(w1);
                    //centralwidget->setFixedSize((sSize));
                    QLabel *label = new QLabel(centralwidget);
                    //label->setStyleSheet(QStringLiteral("QLabel { border: 1px solid red;}"));
                    label->setScaledContents(false);
                    label->setPixmap(q->m_snapshot);
                    // replace centralwidget
                    w1->setCentralWidget(centralwidget);
                    w1->showNormal();
                    q->m_begin = false;
                }
                if(qAbs(q->m_currentPos.x() - q->m_actPos.x()) > 10) {
                    w1->move(q->m_currentPos.toPoint().x()- q->m_startPos.toPoint().x(), 10);
                } else {
                    w1->move(10, q->m_currentPos.toPoint().y()- q->m_startPos.toPoint().y());
                }
                // save last position
                q->m_actPos = q->m_currentPos;
                q->m_touchupdate = true;
            }
        }
        break;
    }
    default:
        break;
    }
    return result;
}
void FingerSwipeGestureRecognizer::reset(QGesture *state)
{
    FingerSwipeGesture *q = static_cast<FingerSwipeGesture *>(state);
    q->m_startPos = QPoint();
    q->m_lastPos = QPoint();
    q->m_currentPos = QPoint();
    q->m_decisionPos = QPoint();
    q->m_triggered = false;
    q->m_cancelled = false;
    q->m_decision = false;
    QGestureRecognizer::reset(state);
}

FingerSwipeGesture::FingerSwipeGesture(QObject *parent): QGesture(parent), m_triggered(false), m_cancelled(false)
{ }

FingerSwipeGesture::~FingerSwipeGesture()
{ }

//#define movelength 300

bool FingerSwipeGesture::isLeftToRight() const
{
    QSize size = qApp->desktop()->size();
    int movelength = size.width() / 3;
    //printf("horiz movelenght=%d\n", movelength);
    return m_startPos.x()+movelength < m_currentPos.x();
}
bool FingerSwipeGesture::isRightToLeft() const
{
    QSize size = qApp->desktop()->size();
    int movelength = size.width() / 3;
    //printf("horiz movelenght=%d\n", movelength);
    return m_startPos.x() > m_currentPos.x()+movelength;
}
bool FingerSwipeGesture::isBottomToTop() const
{
    QSize size = qApp->desktop()->size();
    int movelength = size.height() / 2;
    //printf("vert movelenght=%d\n", movelength);
    return m_startPos.y()+movelength < m_currentPos.y();
}
bool FingerSwipeGesture::isTopToBottom() const
{
    QSize size = qApp->desktop()->size();
    int movelength = size.height() / 2;
    //printf("vert movelenght=%d\n", movelength);
    return m_startPos.y() > m_currentPos.y()+movelength;
}

bool FingerSwipeGesture::isVertical() const
{
    QSize size = qApp->desktop()->size();
    int movelength = size.height() / 20;
    //printf("vert movelenght=%d\n", movelength);
    return m_startPos.y() > m_currentPos.y()+qAbs(movelength);

}

bool FingerSwipeGesture::isHorizontal() const
{
    QSize size = qApp->desktop()->size();
    int movelength = size.width() / 20;
    //printf("horiz movelenght=%d\n", movelength);
    return m_startPos.x() > m_currentPos.x()+qAbs(movelength);

}
