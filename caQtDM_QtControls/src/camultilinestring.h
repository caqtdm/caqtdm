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

#ifndef CAMULTILINESTRING_H
#define CAMULTILINESTRING_H

#include <QPlainTextEdit>
#include <qtcontrols_global.h>
#include <fontscalingwidget.h>

class QTCON_EXPORT caMultiLineString : public QPlainTextEdit, public FontScalingWidget
{
    Q_OBJECT

    // things to get rid off
    Q_ENUMS(Shape Shadow WrapMode)
    Q_PROPERTY(Shape frameShape READ getFrameShape  DESIGNABLE false)
    Q_PROPERTY(Shadow frameShadow READ getFrameShadow  DESIGNABLE false)
    Q_PROPERTY(int lineWidth READ getInt  DESIGNABLE false)
    Q_PROPERTY(int midLineWidth READ getInt  DESIGNABLE false)
    Q_PROPERTY(int tabStopWidth READ getInt  DESIGNABLE false)
    Q_PROPERTY(int maximumBlockCount READ getInt  DESIGNABLE false)
    Q_PROPERTY(int cursorWidth READ getInt  DESIGNABLE false)
    Q_PROPERTY(WrapMode lineWrapMode READ getWrapMode  DESIGNABLE false)
    Q_PROPERTY(bool backgroundVisible READ getBool DESIGNABLE false)
    Q_PROPERTY(bool overwriteMode READ getBool DESIGNABLE false)
    Q_PROPERTY(bool readOnly READ getBool DESIGNABLE false)
    Q_PROPERTY(bool tabChangesFocus READ getBool DESIGNABLE false)
    Q_PROPERTY(bool centerOnScroll READ getBool DESIGNABLE false)
    Q_PROPERTY(bool undoRedoEnabled READ getBool DESIGNABLE false)
    Q_PROPERTY(QString documentTitle READ getDocumentTitle DESIGNABLE false)

    // our stuff
    Q_PROPERTY(QString channel READ getPV WRITE setPV)

    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)

    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)
    Q_ENUMS(colMode)

    Q_PROPERTY(bool frame READ getFrame WRITE setFrame DESIGNABLE false)

    Q_PROPERTY(bool framePresent READ getFrame WRITE setFrame)
    Q_PROPERTY(QColor frameColor READ getFrameColor WRITE setFrameColor)
    Q_PROPERTY(int frameLineWidth READ getLineWidth WRITE setLinewidth)

    Q_PROPERTY(alertHandling alarmHandling READ getAlarmHandling WRITE setAlarmHandling)
    Q_ENUMS(alertHandling)

    Q_PROPERTY(bool fontScaleEnabled READ fontScaleEnabled DESIGNABLE false)
    Q_PROPERTY(ScaleMode fontScaleMode READ fontScaleMode WRITE setFontScaleModeL)
    Q_PROPERTY(double fontScaleFactor READ fontScaleFactor WRITE setFontScaleFactor DESIGNABLE false)

    // this will prevent user interference
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE noStyle DESIGNABLE false)

    Q_ENUMS(ScaleMode)


public:
    void noStyle(QString style) {Q_UNUSED(style);}

    enum Shape {NoFrame = 0};
    enum Shadow {Plain = 0x0010};
    enum WrapMode {noWrap = 0};

    caMultiLineString( QWidget *parent = 0 );
     ~caMultiLineString(){}

    QString getDocumentTitle() { return "";}
    Shape getFrameShape() {return NoFrame;}
    Shadow getFrameShadow() {return Plain;}
    WrapMode getWrapMode() {return noWrap;}
    bool getBool() {return false;}
    int getInt() {return (int) 0;}

    QString getPV() const;
    void setPV(QString const &newPV);

    QColor getForeground() const {return thisForeColor;}
    void setForeground(QColor c);
    QColor getBackground() const {return thisBackColor;}
    void setBackground(QColor c);

    enum colMode {Default=0, Static, Alarm_Default, Alarm_Static};
    colMode getColorMode() const { return thisColorMode; }
    void setColorMode(colMode colormode);

    bool getFrame() const { return thisFramePresent;}
    void setFrame(bool frame);

    QColor getFrameColor() const {return thisFrameColor;}
    void setFrameColor(QColor c);
    int getLineWidth() const {return thisFrameLineWidth;}
    void setLinewidth(int width);

    enum alertHandling { onForeground = 0, onBackground };
    alertHandling getAlarmHandling() const { return thisAlarmHandling;}
    void setAlarmHandling(alertHandling alarmHandling) {thisAlarmHandling = alarmHandling;}

    enum ScaleMode { None, Height, WidthAndHeight};
    void setTextLine(const QString&);

    QString text() const { return QPlainTextEdit::toPlainText(); }

    void setFontScaleModeL(ScaleMode m) { FontScalingWidget::setScaleMode((int) m);}
    ScaleMode fontScaleMode() { return (ScaleMode) FontScalingWidget::scaleMode(); }

    void setForeAndBackground(QColor foreground, QColor background, QColor frame);
    void forceForeAndBackground(QColor foreground, QColor background, QColor frame);
    void setAlarmColors(short status, double value, QColor bgAtInit, QColor fgAtInit);
    void updateAlarmColors();
    void setColors(QColor bg, QColor fg, QColor fr, int lineWidth);
    void copy();
    void clearSelection();

public slots:
    void animation(QRect p) {
#include "animationcode.h"
    }

    void hideObject(bool hideit) {
#include "hideobjectcode.h"
    }

private slots:
    void rescaleFont(const QString& newText);

protected:
      virtual bool event(QEvent *);
      virtual QSize sizeHint() const;
      virtual QSize minimumSizeHint() const;
      QSize calculateTextSpace();

private:
    QString thisPV;
    QString keepText;

    QColor thisForeColor, oldForeColor;
    QColor thisBackColor, oldBackColor;
    QColor defBackColor, defForeColor, defSelectColor;
    QPalette thisPalette;
    colMode thisColorMode;
    colMode oldColorMode;

    bool d_rescaleFontOnTextChanged;
    QString thisStyle, oldStyle;

    bool isShown;

    bool thisFramePresent;
    QColor thisFrameColor, oldFrameColor;
    int thisFrameLineWidth, oldFrameLineWidth;

    alertHandling thisAlarmHandling;
    short Alarm;

    short statusLast;
    double valueLast;
    QColor bgAtInitLast;
    QColor fgAtInitLast;
};

#endif
