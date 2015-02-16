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

#include "epushbutton.h"
#include <QStyleOptionButton>
#include <QStyle>
#include <QtDebug>

EPushButton::EPushButton(const QString &text, QWidget * parent) : QPushButton(text, parent), FontScalingWidget(this)
{
    installEventFilter(this);
}

EPushButton::EPushButton(QWidget * parent) : QPushButton(parent), FontScalingWidget(this)
{
    installEventFilter(this);
}

EPushButton::EPushButton(const QIcon &icon, const QString &text, QWidget *parent) : 
	QPushButton(icon, text, parent), FontScalingWidget(this)
{
    installEventFilter(this);
}

void EPushButton::setText(const QString& text)
{
  QPushButton::setText(text);
  FontScalingWidget::rescaleFont(text, d_savedLabelSize);
}

void EPushButton::resizeEvent(QResizeEvent *e)
{
  QPushButton::resizeEvent(e);
  rescaleFont(text(), calculateTextSpace());
}

void EPushButton::showEvent(QShowEvent *e)
{
  QPushButton::showEvent(e);
  rescaleFont(text(), calculateTextSpace());
}

QSize EPushButton:: sizeHint() const
{
  if(fontScaleEnabled())
  {
	QFont f = font();
    f.setPointSize(10);
	QFontMetrics fm(f);
	int width = fm.width(text());
        width += 8; /* add some extra space */
    return QSize(width, fm.height() + 4);
  }
  return QPushButton::sizeHint();
}
 
QSize  EPushButton::minimumSizeHint() const
{
  if(fontScaleEnabled())
	return EPushButton::sizeHint();
  return QPushButton::minimumSizeHint();
}

QSize EPushButton::calculateTextSpace()
{
  //qDebug() << "EPushButton::calculateTextSpace(): button size " << size();
  QStyleOptionButton button;
  initStyleOption(&button);
  /* Use QStyle subControlRect() to retrieve the area containing the button label.
   * QStyle::SE_PushButtonContents is the area containing the label (icon with text or pixmap).
   * http://doc.trolltech.com/4.4/qstyle.html#SubElement-enum
   */
  d_savedLabelSize = style()->subElementRect(QStyle::SE_PushButtonContents, &button, this).size();
  return d_savedLabelSize;
}

// intercept space key, so that no keyboard spacebar will trigger when button has focus
bool EPushButton::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        QKeyEvent *me = static_cast<QKeyEvent *>(event);
        if(me->key() == Qt::Key_Space) {
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}

