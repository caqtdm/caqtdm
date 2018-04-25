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

#include "catogglebutton.h"
#include "alarmdefs.h"
#include <QtDebug>
#include <QMessageBox>
#include <QApplication>
#include <QStyleOptionFrame>
#include <QKeyEvent>
#include <QStyle>

caToggleButton::caToggleButton(QWidget *parent) : QCheckBox(parent), FontScalingWidget(this)
{
    // to start with, clear the stylesheet, so that playing around
    // is not possible.
    setStyleSheet("");

    setCheckable(true);
    setTristate(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    isShown = false;
    thisPalette = palette();
    thisBackColor = oldBackColor = QColor(200,200,200,0);
    thisForeColor = oldForeColor = Qt::black;
    thisColorMode=Default;
    oldColorMode =Default;
    setColorMode(Default);

    installEventFilter(this);
    connect(this, SIGNAL(clicked()), this, SLOT(buttonToggled()) );

    setTrueValue("1");
    setFalseValue("0");

    setAccessW(true);

    setFontScaleModeL(WidthAndHeight);

    setElevation(on_top);

    setFocusPolicy(Qt::StrongFocus);
}

void  caToggleButton::buttonToggled()
{
  //QMessageBox::information( this, "Toggled!", QString("The button is %1!").arg(isChecked()?"pressed":"released") );
  emit toggleButtonSignal(isChecked());
}

QString caToggleButton::getPV() const
{
    return thisPV;
}

void caToggleButton::setPV(QString const &newPV)
{
    thisPV = newPV;
}

void caToggleButton::setState(Qt::CheckState state)
{
    setCheckState(state);
}

void caToggleButton::setColors(QColor bg, QColor fg)
{
    if((oldBackColor == bg) && (oldForeColor == fg)) return;

    if(!defBackColor.isValid() || !defForeColor.isValid()) return;
    if((bg != oldBackColor) || (fg != oldForeColor) || (thisColorMode != oldColorMode)) {
        if(thisColorMode == Default) {
            thisStyle = "background-color: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8);";
            thisStyle = thisStyle.arg(defBackColor.red()).arg(defBackColor.green()).arg(defBackColor.blue()).arg(defBackColor.alpha()).
                    arg(defForeColor.red()).arg(defForeColor.green()).arg(defForeColor.blue()).arg(defForeColor.alpha());
        } else {
            thisStyle = "background-color: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8);";
            thisStyle = thisStyle.arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha()).
                    arg(fg.red()).arg(fg.green()).arg(fg.blue()).arg(fg.alpha());
        }
        oldBackColor = bg;
        oldForeColor = fg;
    }

    if(thisStyle != oldStyle || thisColorMode != oldColorMode) {
        setStyleSheet(thisStyle);
        oldStyle = thisStyle;
        update();
    }
    oldColorMode = thisColorMode;
}

void caToggleButton::setBackground(QColor c)
{
    thisBackColor = c;
    setColors(thisBackColor, thisForeColor);
}

void caToggleButton::setForeground(QColor c)
{
    thisForeColor = c;
    setColors(thisBackColor, thisForeColor);
}

void caToggleButton::setAlarmColors(short status)
{
    QColor c;
    switch (status) {

    case NO_ALARM:
        c=AL_GREEN;
        break;
    case MINOR_ALARM:
        c=AL_YELLOW;
        break;
    case MAJOR_ALARM:
        c=AL_RED;
        break;
    case INVALID_ALARM:
    case NOTCONNECTED:
        c=AL_WHITE;
        break;
    default:
        c=AL_DEFAULT;
        break;
    }
    if(status == NOTCONNECTED) {
       setColors(c, c);
    } else {
       setColors(thisBackColor, c);
    }
}

void caToggleButton::setNormalColors()
{
    setColors(thisBackColor, thisForeColor);
}

void caToggleButton::setAccessW(bool access)
{
     _AccessW = access;
}

QSize caToggleButton::calculateTextSpace()
{
    QStyleOptionButton option;
    option.initFrom(this);
    d_savedTextSpace = style()->subElementRect(QStyle::SE_CheckBoxContents, &option, this).size();
    return d_savedTextSpace;
}

void caToggleButton::rescaleFont(const QString& newText)
{
        FontScalingWidget::rescaleFont(newText, d_savedTextSpace);
}

bool caToggleButton::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::Resize || event->type() == QEvent::Show) {
        FontScalingWidget::rescaleFont(text(), calculateTextSpace());
    } else if (event->type() == QEvent::Enter) {
        if(!_AccessW) {
            QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));
            setEnabled(false);
        } else {
            QApplication::restoreOverrideCursor();
        }
    } else if(event->type() == QEvent::Leave) {
        QApplication::restoreOverrideCursor();
        setEnabled(true);
    }  else if(event->type() == QEvent::KeyRelease) {
        QKeyEvent *me = static_cast<QKeyEvent *>(event);
         // move cursor with tab focus
        if(me->key() == Qt::Key_Tab) {
            QCursor *cur = new QCursor;
            QPoint p = QWidget::mapToGlobal(QPoint(this->width()/2, this->height()/2));
            cur->setPos( p.x(), p.y());
            setFocus();
        }
    }
    return QObject::eventFilter(obj, event);
}


bool caToggleButton::event(QEvent *e)
{
    if(e->type() == QEvent::Resize || e->type() == QEvent::Show) {
        // we try to get the default color for the background set through the external stylesheets
        if(!isShown) {
          setStyleSheet("");
          QString c=  palette().color(QPalette::Base).name();
          defBackColor = QColor(c);
          //printf("default back color %s %s\n", qasc(c), qasc(this->objectName()));
          c=  palette().color(QPalette::Text).name();
          defForeColor = QColor(c);
          //printf("default fore color %s %s\n", qasc(c), qasc(this->objectName()));

          if(!defBackColor.isValid()) defBackColor = QColor(255, 248, 220, 255);
          if(!defForeColor.isValid()) defForeColor = Qt::black;

          setColors(thisBackColor, thisForeColor);
          isShown = true;
        }
    }
    return QCheckBox::event(e);
}




