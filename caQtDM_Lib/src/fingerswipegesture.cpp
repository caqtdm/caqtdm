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

QGestureRecognizer::Result FingerSwipeGestureRecognizer::recognize(QGesture *state, QObject *, QEvent *event)
{
    FingerSwipeGesture *q = static_cast<FingerSwipeGesture *>(state);
    const QTouchEvent *ev = static_cast<const QTouchEvent *>(event); // only use ev after checking event->type()
    QGestureRecognizer::Result result = QGestureRecognizer::Ignore;
    switch (event->type()) {
    case QEvent::TouchBegin: {
        result = QGestureRecognizer::MayBeGesture;
        if (ev->touchPoints().size() >= 1) {
            QTouchEvent::TouchPoint p1 = ev->touchPoints().first();
            q->m_startPos = p1.startScenePos();
        }
        break;
    }
    case QEvent::TouchEnd: {
        if (q->m_cancelled) {
            result = QGestureRecognizer::Ignore;
        } else if (q->m_triggered) {
            result = QGestureRecognizer::FinishGesture;
        } else {
            result = QGestureRecognizer::CancelGesture;
        }
        break;
    }
    case QEvent::TouchUpdate: {
        if (!q->m_cancelled && ev->touchPoints().size() >= 1) {
            QTouchEvent::TouchPoint p1 = ev->touchPoints().first();
            q->m_currentPos = p1.screenPos();
            // update the hot-spot to be in the middle between start and current point
            q->setHotSpot(QPointF(p1.screenPos().x() - (q->m_currentPos.x() - q->m_startPos.x()) / 2,
                                  p1.screenPos().y() - (q->m_currentPos.y() - q->m_startPos.y()) / 2));
            if (!q->m_triggered) {
                q->m_triggered = true;
                result = QGestureRecognizer::TriggerGesture;
            }
            break;
        }
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
    q->m_triggered = false;
    q->m_cancelled = false;
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
