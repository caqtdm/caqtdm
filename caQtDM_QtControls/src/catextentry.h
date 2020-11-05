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

#ifndef caTextEntry_H
#define caTextEntry_H

#include <QGridLayout>
#include <QFrame>
#include <QEvent>
#include <QMimeData>

#include <qtcontrols_global.h>
#include <caLineEdit>


class QTCON_EXPORT caTextEntry : public caLineEdit
{
    Q_OBJECT
    Q_PROPERTY(bool unitsEnabled READ getUnitsEnabled WRITE setUnitsEnabled DESIGNABLE false)

    Q_PROPERTY(bool framePresent READ getFrame WRITE setFrame  DESIGNABLE false)
    Q_PROPERTY(QColor frameColor READ getFrameColor WRITE setFrameColor  DESIGNABLE false)
    Q_PROPERTY(int frameLineWidth READ getLineWidth WRITE setLinewidth  DESIGNABLE false)
    Q_PROPERTY(alertHandling alarmHandling READ getAlarmHandling WRITE setAlarmHandling DESIGNABLE false )
    Q_PROPERTY(bool keepFocus READ getKeepFocus WRITE setKeepFocus)

#include "caElevation.h"

public:
    caTextEntry( QWidget *parent = 0 );
    bool getAccessW() const {return _AccessW;}
    void setAccessW(bool access);
    void updateText(const QString &text);

    bool getKeepFocus() const { return keepFocusOnLeave; }
    void setKeepFocus(bool focusBehavior) {keepFocusOnLeave=focusBehavior;}

public slots:
    void animation(QRect p) {
#include "animationcode.h"
    }

    void hideObject(bool hideit) {
#include "hideobjectcode.h"
    }

    void setValue(double v);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void dataInput();

signals:
    void TextEntryChanged(QString);

private:
    bool eventFilter(QObject *obj, QEvent *event);

    bool  keepFocusOnLeave;
    bool _AccessW;
    QString startText;
};
#endif
