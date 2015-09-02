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

#include "cabytecontroller.h"
#include "alarmdefs.h"
#include <QGridLayout>

caByteController::caByteController(QWidget *parent) : QWidget(parent)
{
    numRows = 32;

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    signalMapper = new QSignalMapper(this);

    setAccessW(true);

    grid = new QGridLayout(this);
    grid->setMargin(0);
    grid->setSpacing(0);
    thisStartBit = 0;
    thisEndBit = 31;
    thisColorMode=Static;
    thisScaleMode = EPushButton::WidthAndHeight;
    setDirection(Down);
    setTrueColor(Qt::blue);
    setFalseColor(Qt::gray);
    setTextColor(Qt::black);
    arrangeCells();

    setElevation(on_top);

    installEventFilter(this);
}

QString caByteController::getPV() const
{
    return thisPV;
}

void caByteController::setPV(QString const &newPV)
{
    thisPV = newPV;
}

void caByteController::arrangeCells()
{
    foreach(EPushButton *l, cells) {
        grid->removeWidget(l);
        l->hide();
        l->deleteLater();
    }
    cells.clear();

    // signalmapper will map signals from all buttons to one signal
    delete signalMapper;
    signalMapper = new QSignalMapper(this);

    for (int i = 0; i < numRows; i++) {
        EPushButton* temp;

        if(thisDirection == Down || thisDirection == Right)  {
           temp = new EPushButton(QString::number(i + thisStartBit), this);
        } else {
           temp = new EPushButton(QString::number(thisEndBit - i), this);
        }

        temp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        if(thisScaleMode == EPushButton::None) {  // in this case we may use font
            temp->setFont(this->font());
        } else {
            temp->setFontScaleMode(thisScaleMode);
        }
        temp->setMinimumSize(2,2); //important for resizing as small as possible
        temp->setBotTopBorderWidth(0);

        if(thisDirection == Up || thisDirection == Down) {
          grid->addWidget(temp, i, 0);
        } else {
          grid->addWidget(temp, 0, i);
        }
        cells.push_back(temp);
        temp->show();

        if(thisDirection == Down || thisDirection == Right) {
            signalMapper->setMapping(temp, i + thisStartBit);
        } else {
            signalMapper->setMapping(temp, thisEndBit - thisStartBit  - i);
        }
        connect(temp, SIGNAL(clicked()), signalMapper, SLOT(map()));
    }
    setValue(0);
    connect(signalMapper, SIGNAL(mapped(int)),this, SIGNAL(clicked(int)));
}

bool caByteController::bitState(long value, int bitNr)
{
    return ((((int) value >> bitNr) & 1) == 1);
}

void caByteController::setValue(long value)
{
    thisValue = value;
    drawByte(thisValue, thisTrueColor, thisFalseColor);
}

long caByteController::getValue()
{
    return thisValue;
}

void caByteController::drawByte(long lvalue, QColor trueColor, QColor falseColor)
{
    if(thisDirection == Down || thisDirection == Right)  {
        for(int i=0; i<= thisEndBit - thisStartBit; i++) {
            if(bitState(lvalue, i + thisStartBit)) {
                setColor(cells[i], trueColor, thisTextColor);
            } else {
                setColor(cells[i], falseColor, thisTextColor);
            }
        }
    } else {
        for(int i=0; i<= thisEndBit - thisStartBit; i++) {
            if(bitState(lvalue, i + thisStartBit)) {
                setColor(cells[thisEndBit - thisStartBit  - i], trueColor, thisTextColor);
            } else {
                setColor(cells[thisEndBit - thisStartBit  - i], falseColor, thisTextColor);
            }
        }
    }
}

void caByteController::setColor(EPushButton *button, QColor c, QColor text)
{
    QColor  highlightColor = c.lighter(120);
    QColor  pressedColor = c.darker(120);

    //set colors and style
    QString style = "QPushButton {background-color: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8);";
    style = style.arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha()).arg(text.red()).arg(text.green()).arg(text.blue()).arg(text.alpha());
    style.append("border-radius: 3px; padding: 1px; border-width: 1px; border-style: outset;} ");

    QString hover = "QPushButton:hover {background-color: rgba(%1, %2, %3, %4);}  QPushButton:pressed {background-color: rgba(%5, %6, %7, %8)};";
    hover = hover.arg(highlightColor.red()).arg(highlightColor.green()).arg(highlightColor.blue()).arg(highlightColor.alpha())
            .arg(pressedColor.red()).arg(pressedColor.green()).arg(pressedColor.blue()).arg(pressedColor.alpha());
    style.append(hover);

    button->setStyleSheet(style);
}

void caByteController::setTrueColor(QColor c)
{
    thisTrueColor = c;
    arrangeCells();
}

void caByteController::setFalseColor(QColor c)
{
    thisFalseColor = c;
    arrangeCells();
}

void caByteController::setTextColor(QColor c)
{
    thisTextColor = c;
    arrangeCells();
}

void caByteController::setAlarmColors(short status)
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
      for(int i=0; i<= thisEndBit - thisStartBit; i++)  setColor(cells[i], c, thisTextColor);
    } else {
        drawByte(thisValue, c, thisFalseColor);
    }

}

void caByteController::setStartBit(int const &bit) {
    thisStartBit = bit;
    numRows = thisEndBit - thisStartBit + 1;
    if(numRows < 1) {
        thisStartBit=0;
        thisEndBit = 0;
        numRows = 1;
    } else if(numRows > 32) {
        thisStartBit=0;
        thisEndBit = 31;
        numRows = 32;
    }
    arrangeCells();
}

void caByteController::setEndBit(int const &bit) {
    thisEndBit = bit;
    numRows = thisEndBit - thisStartBit + 1;
    if(numRows < 1) {
        thisStartBit=0;
        thisEndBit = 0;
        numRows = 1;
    } else if(numRows > 32) {
        thisStartBit=0;
        thisEndBit = 31;
        numRows = 32;
    }
    arrangeCells();
}


void caByteController::setDirection(Direction dir)
{
    thisDirection = dir;
    arrangeCells();
}

void caByteController::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
}


void caByteController::setFontScaleModeL(EPushButton::ScaleMode m)
{
   thisScaleMode = m;
   arrangeCells();
}

EPushButton::ScaleMode caByteController::fontScaleMode()
{
    arrangeCells();
    return thisScaleMode;
}


void caByteController::setAccessW(bool access)
{
    _AccessW = access;
}

bool caByteController::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        if(!_AccessW) {
            QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));
        } else {
            QApplication::restoreOverrideCursor();
        }
    } else if(event->type() == QEvent::Leave) {
        QApplication::restoreOverrideCursor();
    // intercept space key, so that no keyboard spacebar will trigger when button has focus
    }  else if(event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
      QKeyEvent *me = static_cast<QKeyEvent *>(event);
      if(me->key() == Qt::Key_Space) {
        return true;
      }
    }

    return QObject::eventFilter(obj, event);
}




