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

#ifndef caVertLabel_H
#define caVertLabel_H

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QPropertyAnimation>
#include <qtcontrols_global.h>

/*
class QTCON_EXPORT caVertLabel : public QGraphicsView
{
    Q_OBJECT

public:
    caVertLabel( QWidget *parent = 0 );

protected:
  virtual bool event(QEvent *);
  void timerEvent(QTimerEvent* event) ;

private:
    QLabel *label;
    QGraphicsProxyWidget *proxy;
    QGraphicsScene *scene;
    qreal angle;
*/


#include <qtcontrols_global.h>
#include "fontscalingwidget.h"

class QTCON_EXPORT caLabelVertical : public QWidget, public FontScalingWidget
{
    Q_OBJECT

    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(Direction direction READ getDirection WRITE setDirection)
    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)
    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
    Q_ENUMS(colMode)
    Q_ENUMS(Direction)

#include "caVisibProps.h"
#include "caVisibDefs.h"

public:
    enum ScaleMode { None, Height, WidthAndHeight};
    enum Direction {Up, Down};

    caLabelVertical(QWidget *parent = 0);
    bool rotateText(float degrees);

    void setText(const QString &text);
    QString text() const {return thisText;}

    void setDirection(const Direction &direction);
    Direction getDirection() const {return thisDirection;}

    QSize calculateTextSpace();

    void setFontScaleMode(ScaleMode m) { FontScalingWidget::setScaleMode((int) m); }
    ScaleMode fontScaleMode() { return (ScaleMode) FontScalingWidget::scaleMode();  }

    QColor getForeground() const {return thisForeColor;}
    void setForeground(QColor c);

    QColor getBackground() const {return thisBackColor;}
    void setBackground(QColor c);

    enum colMode {Static, Alarm};
    colMode getColorMode() const { return thisColorMode; }

    void setColors(QColor bg, QColor fg);

    void setAlarmColors(short status);

    void setColorMode(colMode colormode) {thisColorMode = colormode;
                                          setBackground(thisBackColor);
                                          setForeground(thisForeColor);}
protected:
    virtual bool event(QEvent *);
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

private:
    float rotation;
    QString thisText;
    QColor thisForeColor;
    QColor thisBackColor;
    colMode thisColorMode;
    Direction thisDirection;

protected:
    void paintEvent(QPaintEvent *);

};
#endif
