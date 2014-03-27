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

#include "cascriptbutton.h"
#include "alarmdefs.h"
#include <QtDebug>
#include <QApplication>

caScriptButton::caScriptButton(QWidget *parent) : QWidget(parent)
{

    QGridLayout *l = new QGridLayout;
    l->setMargin(0);
    displayScript = new QCheckBox();
    displayScript->setText("");
    displayScript->setGeometry(0,0,15,15);
    displayScript->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    buttonScript = new EPushButton( "Action", this );
    buttonScript->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setFontScaleMode(EPushButton::WidthAndHeight);

    l->addWidget(buttonScript, 0, 0);
    l->addWidget(displayScript, 0, 1);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setLayout(l);

    thisForeColor = Qt::black;
    setBackground(Qt::gray);

    thisShowExecution = false;

    connect(displayScript, SIGNAL(clicked()), this, SLOT(buttonToggled()) );
    connect(buttonScript, SIGNAL(clicked()), this, SLOT(scriptButtonClicked()) );

    installEventFilter(this);
    setAccessW(true);

}

void caScriptButton::buttonToggled()
{
  //QMessageBox::information( this, "Toggled!", QString("The button is %1!").arg(isChecked()?"pressed":"released") );
   thisShowExecution = displayScript->isChecked();
}

void caScriptButton::scriptButtonClicked()
{
    emit scriptButtonSignal();
}


void caScriptButton::setLabel(QString const &label)
{
    thisLabel = label;
    buttonScript->setText(thisLabel);
}

void caScriptButton::setColors(QColor bg, QColor fg, QColor hover, QColor border)
{
    //set colors and style filled
    if((bg != oldBackColor) || (fg != oldForeColor) || (hover != oldHoverColor)) {
       QString style = "QPushButton{ background-color: rgba(%1, %2, %3, %4); color: rgb(%5, %6, %7); border-color: rgb(%8, %9, %10);";
       style = style.arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(255).
             arg(fg.red()).arg(fg.green()).arg(fg.blue()).
             arg(border.red()).arg(border.green()).arg(border.blue());
       style.append("border-radius: 1px; padding: 0px; border-width: 3px;"
                  "border-style: outset; margin:0px;}");
       QString hoverC = "QPushButton:hover {background-color: rgb(%1, %2, %3);} QPushButton:pressed {background-color: rgb(%4, %5, %6)};";
       hoverC = hoverC.arg(hover.red()).arg(hover.green()).arg(hover.blue()).arg(thisBorderColor.red()).arg(thisBorderColor.green()).arg(thisBorderColor.blue());
       style.append(hoverC);
       setStyleSheet(style);

       oldBackColor = bg;
       oldForeColor = fg;
       oldHoverColor = hover;
     }
}

void caScriptButton::setBackground(QColor c)
{
    thisBackColor = c;
    thisHoverColor = c.light(120);
    thisBorderColor = c.dark(150);
    setColors(thisBackColor, thisForeColor, thisHoverColor, thisBorderColor);
}

void caScriptButton::setForeground(QColor c)
{
    thisForeColor = c;
    setColors(thisBackColor, thisForeColor, thisHoverColor, thisBorderColor);
}
void caScriptButton::setFontScaleMode(EPushButton::ScaleMode m)
{
   thisScaleMode = m;
   if(thisScaleMode == EPushButton::None) {  // in this case we may use font
       buttonScript->setFont(this->font());
   } else {
       buttonScript->setFontScaleMode(thisScaleMode);
   }
}

EPushButton::ScaleMode caScriptButton::fontScaleMode()
{
    return thisScaleMode;
}

void caScriptButton::changeEvent(QEvent *e) {
    Q_UNUSED(e);
    setFontScaleMode(thisScaleMode);
}

void caScriptButton::setScriptDisplay(defaultDisplay m)
{
    thisDefaultDisplay = m;
    if(m == Visible) {
       displayScript->setCheckState(Qt::Checked);
    } else {
       displayScript->setCheckState(Qt::Unchecked);
    }
    thisShowExecution = displayScript->isChecked();
}

bool caScriptButton::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        if(!_AccessW) {
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        } else {
            QApplication::restoreOverrideCursor();
        }
    } else if(event->type() == QEvent::Leave) {
        QApplication::restoreOverrideCursor();
    }
    return QObject::eventFilter(obj, event);
}



