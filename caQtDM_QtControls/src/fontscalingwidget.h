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

#ifndef FONTSCALING_WIDGET_H
#define FONTSCALING_WIDGET_H

#include <QWidget>

class FontScalingWidget
{ 
  public:
	FontScalingWidget(QWidget *parent);
    virtual ~FontScalingWidget() {}
	
	void setLateralBorderWidth(double pixValue);
	double lateralBorderWidth() { return d_lateralBorderWidth; }
	double fontScaleFactor() { return d_fontScaleFactor; }
	void setFontScaleFactor(double factor) { d_fontScaleFactor = factor; }
	bool fontScaleEnabled() const { return d_scaleMode != 0; }
	void setBotTopBorderWidth(double pixValue);
	double botTopBorderWidth() { return d_botTopBorderWidth; }
	void saveTextSpace(const QSize& size) { d_savedTextSpace = size; }
	QSize savedTextSpace() { return d_savedTextSpace; }
	QFont& savedFont() { return d_savedFont; }
	QWidget *widget() { return d_widget; }
	double calculateFontPointSizeF(const QString& text, const QSize & size);
    double calculateVertFontPointSizeF(const QString& text, const QSize & size);
	
	virtual QSize calculateTextSpace() = 0;
    virtual QString text() const = 0;
	
	int scaleMode() { return d_scaleMode; }
	void setScaleMode(int m);
    void setVerticalLabel(const bool &vertical) {d_vertical = vertical;}
	
  protected:
	void rescaleFont(const QString& text, const QSize &);
	
	/* let this accessible by subclasses to improve performance */
	QSize d_savedTextSpace;
	
	int d_scaleMode;
	
  private:
    bool d_vertical;
	double d_lateralBorderWidth;
	double d_botTopBorderWidth;
	double d_fontScaleFactor;
	QWidget *d_widget;
	QFont d_savedFont;
	
	static bool longerThan(const QString& s1, const QString& s2) { return s1.length() > s2.length(); }
};

#endif
