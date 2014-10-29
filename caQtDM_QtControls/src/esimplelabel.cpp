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

#include "esimplelabel.h"
#include <QStyleOptionFrame>
#include <QStyle>
#include <QtDebug>
#include <QEvent>
#include <QResizeEvent>

ESimpleLabel::ESimpleLabel(QWidget *parent) : QLabel(parent), FontScalingWidget(this)
{
  if(fontScaleMode() != None)
	setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
  else
	setAlignment(Qt::AlignRight|Qt::AlignVCenter);
}

ESimpleLabel::ESimpleLabel(const QString& text, QWidget *parent) : QLabel(text, parent), FontScalingWidget(this)
{
  if(fontScaleMode() != None)
	setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
}

void ESimpleLabel::setText(const QString &txt)
{
  QLabel::setText(txt);
  FontScalingWidget::rescaleFont(txt, d_savedTextSpace);
}

bool ESimpleLabel::event(QEvent *e)
{
  if(e->type() == QEvent::Resize || e->type() == QEvent::Show) {
	FontScalingWidget::rescaleFont(text(), calculateTextSpace());
  }
  return QLabel::event(e);
}

QSize ESimpleLabel::calculateTextSpace()
{
  // contentsRect takes into account frame lineWidth and midLineWidth properties
   d_savedTextSpace = contentsRect().size();
   d_savedTextSpace.setWidth(d_savedTextSpace.width() - 2 * frameWidth() - 2 * midLineWidth());
   d_savedTextSpace.setHeight(d_savedTextSpace.height()- 2 * frameWidth() - 2 * midLineWidth());
   //printf("ESimpleLabel: contents rect %dx%d, text space %dx%d\n", contentsRect().width(), contentsRect().height(),
   //              d_savedTextSpace.width(), d_savedTextSpace.height());
  
  return d_savedTextSpace;
}



QSize ESimpleLabel::sizeHint() const
{
  if(!fontScaleEnabled())
  {
	return QLabel::sizeHint();
  }
 QFont f = font();
 f.setPointSize(4);
 QFontMetrics fm(f);
 int w = fm.width(text());
 int h = fm.height();
 /* add some pixels... */
 w += 2 * frameWidth() + 2 * midLineWidth();
 h += 2 * frameWidth() + 2 * midLineWidth();
 QSize size(w, h);
 return size;
}

QSize ESimpleLabel::minimumSizeHint() const
{
  QSize size;
  if(!fontScaleEnabled())
	size = QLabel::minimumSizeHint();
  else
	size = sizeHint();
  return size;
}

