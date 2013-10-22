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

#ifndef EPUSHBUTTON_H
#define EPUSHBUTTON_H

#include <qtcontrols_global.h>
#include <fontscalingwidget.h>
#include <QPushButton>
#include <QRect>

class QTCON_EXPORT EPushButton : public QPushButton, public FontScalingWidget
{
  Q_OBJECT
  Q_ENUMS(ScaleMode)
  Q_PROPERTY(ScaleMode fontScaleMode READ fontScaleMode WRITE setFontScaleMode)
  Q_PROPERTY(double fontScaleFactor READ fontScaleFactor WRITE setFontScaleFactor  DESIGNABLE false)
  
  public:
	EPushButton(QWidget *parent);
	EPushButton(const QString &, QWidget *);
	EPushButton(const QIcon &, const QString &, QWidget *);
	
	/** \brief the font scaling modality
	 *
	 * Please refer to the ESimpleLabel::ScaleMode documentation.
	 */
        enum ScaleMode { None, Height, WidthAndHeight };
	
    virtual void setFontScaleMode(ScaleMode m) { d_scaleMode = (int) m; FontScalingWidget::setScaleMode((int) m);}
    ScaleMode fontScaleMode() { return (ScaleMode) d_scaleMode; }
	
	/** \brief sets the text on the push button triggering a font scale.
	 */
	virtual void setText(const QString& text);
	
	QString text() const { return QPushButton::text(); }
	
  protected:
	/* the following two implementations trigger font scaling */
	virtual void resizeEvent(QResizeEvent *e);
	virtual void showEvent(QShowEvent *);
	
	/** \brief provides sizeHint for the widgets inserted into a layout
	 *
	 * The size hint is used by the layout system to resize the widgets according to the 
	 * resize policy. The implementation of size hint returns a size whose width is wide
	 * enough to contain the current text() with the current font with size imposed to 10.
	 * If font scaling is disabled, returns the super class sizeHint.
	 */
	virtual QSize sizeHint() const;
	
	/** \brief returns the minimumSizeHint.
	 *
	 * The implementation returns sizeHint() if font scaling is enabled, super class 
	 * minimumSizeHint() if disabled.
	 */
	virtual QSize minimumSizeHint() const;
	
  /* the useful rectangle is the label rectangle inside the QPushButton and
   * this is style dependent.
   */
  QSize calculateTextSpace();
  QSize d_savedLabelSize;
};


#endif
