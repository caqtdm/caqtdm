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
#ifndef QFINGERSWIPEGESTURE_H
#define QFINGERSWIPEGESTURE_H
#include <QMainWindow>
#include <QPixmap>
#include <QBoxLayout>
#include <QLabel>
#include <QGesture>
#include <QGestureRecognizer>
#include <QPointF>
#include <QPushButton>
#include <QRectF>
#include <QPainter>
class FingerSwipeGestureRecognizer : public QGestureRecognizer
{
public:
    FingerSwipeGestureRecognizer();
    QGesture *create(QObject *target);
    QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event);
    void reset(QGesture *state);
};
class FingerSwipeGesture : public QGesture
{
public:
    FingerSwipeGesture(QObject *parent = nullptr);
    virtual ~FingerSwipeGesture();
    bool isLeftToRight() const;
    bool isRightToLeft() const;
    bool isBottomToTop() const;
    bool isTopToBottom() const;
    bool isVertical() const;
    bool isHorizontal() const;

private:
    QPointF m_startPos;
    QPointF m_actPos;
    QPointF m_lastPos;
    QPointF m_decisionPos;
    QPointF m_currentPos;
    QWidget *m_central;
    QPixmap m_snapshot;
    bool m_triggered;
    bool m_cancelled;
    bool m_touchupdate;
    bool m_begin;
    bool m_decision;
    friend class FingerSwipeGestureRecognizer;
};
#endif
