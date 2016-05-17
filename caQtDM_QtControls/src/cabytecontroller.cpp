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
    // to start with, clear the stylesheet, so that playing around
    // is not possible.
    setStyleSheet("");

    numRows = 32;

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setAccessW(true);

    grid = new QGridLayout(this);
    grid->setMargin(0);
    grid->setSpacing(0);
    prvStartBit = thisStartBit = 0;
    prvEndBit = thisEndBit = 31;
    thisColorMode=Static;
    thisScaleMode = EPushButton::WidthAndHeight;
    thisDirection = Down;
    thisTrueColor = Qt::blue;
    thisFalseColor = Qt::gray;
    thisTextColor = Qt::black;
    thisValue = 0;

    // for performance reasons create 32 pushbuttons
    signalMapper = new QSignalMapper(this);
    for (int i = 0; i < 32; i++) {
        EPushButton* temp =  new EPushButton(QString::number(i + thisStartBit), this);
        temp->setFontScaleMode(thisScaleMode);
        temp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        temp->setMinimumSize(2,2); //important for resizing as small as possible
        temp->setBotTopBorderWidth(0);
        connect(temp, SIGNAL(clicked()), signalMapper, SLOT(map()));
        cells.push_back(temp);
        prvColor[i] = Qt::white;
        prvTextcolor[i] = Qt::white;
    }

    connect(signalMapper, SIGNAL(mapped(int)),this, SIGNAL(clicked(int)));

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
        signalMapper->removeMappings(l);
    }

    for (int i = 0; i < numRows; i++) {
        if(thisDirection == Down || thisDirection == Right)  {
            cells[i]->setText(QString::number(i + thisStartBit));
            signalMapper->setMapping(cells[i], i + thisStartBit);
        } else {
           cells[i]->setText(QString::number(thisEndBit - i));
           signalMapper->setMapping(cells[i], thisEndBit - thisStartBit  - i);
        }

        if(thisDirection == Up || thisDirection == Down) {
           grid->addWidget(cells[i], i, 0);
        } else {
           grid->addWidget(cells[i], 0, i);
        }
        setColor(i, thisFalseColor, thisTextColor);
        cells[i]->show();
    }
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
                setColor(i, trueColor, thisTextColor);
            } else {
                setColor(i, falseColor, thisTextColor);
            }
        }
    } else {
        for(int i=0; i<= thisEndBit - thisStartBit; i++) {
            if(bitState(lvalue, i + thisStartBit)) {
                setColor(thisEndBit - thisStartBit  - i, trueColor, thisTextColor);
            } else {
                setColor(thisEndBit - thisStartBit  - i, falseColor, thisTextColor);
            }
        }
    }
}

void caByteController::setColor(int indx, QColor c, QColor text)
{
    if(prvColor[indx] == c && prvTextcolor[indx] == text) return;
    //printf("setcolors indx=%d c=%d %d %d prv=%d %d %d text=%d %d %d prv=%d %d %d\n", indx, c.red(), c.green(), c.blue(),
    //                                                                       prvColor[indx].red(), prvColor[indx].green(), prvColor[indx].blue(),
    //                                                                       text.red(), text.green(), text.blue(),
    //                                                                       prvTextcolor[indx].red(), prvTextcolor[indx].green(), prvTextcolor[indx].blue());
    prvColor[indx] = c;
    prvTextcolor[indx] = text;

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

    cells[indx]->setStyleSheet(style);
}

void caByteController::setTrueColor(QColor c)
{
    thisTrueColor = c;
}

void caByteController::setFalseColor(QColor c)
{
    thisFalseColor = c;
    drawByte(thisValue, thisTrueColor, thisFalseColor);
}

void caByteController::setTextColor(QColor c)
{
    thisTextColor = c;
    drawByte(thisValue, thisTrueColor, thisFalseColor);
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
      for(int i=0; i<= thisEndBit - thisStartBit; i++)  setColor(i, c, thisTextColor);
    } else {
        drawByte(thisValue, c, thisFalseColor);
    }
}

void caByteController::setStartBit(int const &bit) {
    if(prvStartBit == bit) return;
    prvStartBit = thisStartBit = bit;
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
    if(prvEndBit == bit) return;
    prvEndBit = thisEndBit = bit;
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

    for (int i = 0; i < numRows; i++) {
        cells[i]->setFontScaleMode(thisScaleMode);
        if(thisScaleMode == EPushButton::None) {  // in this case we may use font
            cells[i]->setFont(this->font());
        }
    }
}

EPushButton::ScaleMode caByteController::fontScaleMode()
{
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




