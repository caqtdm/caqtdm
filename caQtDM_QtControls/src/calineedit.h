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

#ifndef CALINEEDIT_H
#define CALINEEDIT_H

#include <QLineEdit>
#include <qtcontrols_global.h>
#include <fontscalingwidget.h>

class QTCON_EXPORT caLineEdit : public QLineEdit, public FontScalingWidget
{
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

    Q_PROPERTY(int precision READ getPrecision WRITE setPrecision)
    Q_PROPERTY(SourceMode precisionMode READ getPrecisionMode WRITE setPrecisionMode)

    Q_PROPERTY(SourceMode limitsMode READ getLimitsMode WRITE setLimitsMode)
    Q_ENUMS(SourceMode)

    Q_PROPERTY(double maxValue READ getMaxValue WRITE setMaxValue)
    Q_PROPERTY(double minValue READ getMinValue WRITE setMinValue)

    Q_PROPERTY(bool fontScaleEnabled READ fontScaleEnabled DESIGNABLE false)
    Q_PROPERTY(ScaleMode fontScaleMode READ fontScaleMode WRITE setFontScaleModeL)
    Q_PROPERTY(double fontScaleFactor READ fontScaleFactor WRITE setFontScaleFactor DESIGNABLE false)

    Q_PROPERTY(bool unitsEnabled READ getUnitsEnabled WRITE setUnitsEnabled)

    Q_PROPERTY(FormatType formatType READ getFormatType WRITE setFormatType)

    Q_ENUMS(ScaleMode)
    Q_ENUMS(FormatType)

    Q_OBJECT

public:

    caLineEdit( QWidget *parent = 0 );
     ~caLineEdit(){}

    bool getUnitsEnabled() const { return thisUnitMode; }
    void setUnitsEnabled(bool thisUnitMode);

    double getMaxValue()  const {return thisMaximum;}
    void setMaxValue(double const &maxim) {thisMaximum = maxim;}

    double getMinValue()  const {return thisMinimum;}
    void setMinValue(double const &minim) {thisMinimum = minim;}

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

    enum SourceMode {Channel = 0, User};
    SourceMode getPrecisionMode() const { return thisPrecMode; }
    void setPrecisionMode(SourceMode precmode) {thisPrecMode = precmode;}
    SourceMode getLimitsMode() const { return thisLimitsMode; }
    void setLimitsMode(SourceMode limitsmode) { thisLimitsMode = limitsmode;}

    int getPrecision() const {return thisPrecision;}
    void setPrecision(int prec) {thisPrecision = prec;}

    enum FormatType { decimal, exponential, engr_notation, compact, truncated,
                      hexadecimal, octal, string, sexagesimal, sexagesimal_hms, sexagesimal_dms, enumeric}; // enumeric = enum as number

    enum ScaleMode { None, Height, WidthAndHeight};
    void setTextLine(const QString&);
    void forceText(const QString&);
    QString text() const { return QLineEdit::text(); }

    void setFontScaleModeL(ScaleMode m) { FontScalingWidget::setScaleMode((int) m);}
    ScaleMode fontScaleMode() { return (ScaleMode) FontScalingWidget::scaleMode(); }

    void setFormat(int prec);
    void setValue(double value, const QString& units);

    void setFormatType(FormatType m) { thisFormatType = m; }
    FormatType getFormatType() { return thisFormatType; }

    void setForeAndBackground(QColor foreground, QColor background, QColor frame);
    void forceForeAndBackground(QColor foreground, QColor background, QColor frame);
    void setAlarmColors(short status, double value, QColor bgAtInit, QColor fgAtInit);
    void updateAlarmColors();
    void setColors(QColor bg, QColor fg, QColor fr, int lineWidth);
    void newFocusPolicy(Qt::FocusPolicy f);

    void setValueType(bool isvalue);
    void setFromTextEntry();

private slots:
    void rescaleFont(const QString& newText);

protected:
      virtual bool event(QEvent *);
      virtual QSize sizeHint() const;
      virtual QSize minimumSizeHint() const;
      QSize calculateTextSpace();

private:
    QString thisPV;

    QColor thisForeColor, oldForeColor;
    QColor thisBackColor, oldBackColor;
    QColor defBackColor, defForeColor, defSelectColor;
    QPalette thisPalette;
    colMode thisColorMode;
    colMode oldColorMode;

    int thisPrecision;
    SourceMode thisPrecMode;
    SourceMode thisLimitsMode;

    bool thisUnitMode;
    QString keepText;
    char thisFormat[20];
    char thisFormatC[20];
    bool d_rescaleFontOnTextChanged;
    double thisMaximum, thisMinimum;
    FormatType thisFormatType;
    QString thisStyle, oldStyle;

    bool isShown;
    bool isValue;

    bool thisFramePresent;
    QColor thisFrameColor, oldFrameColor;
    int thisFrameLineWidth, oldFrameLineWidth;
    alertHandling thisAlarmHandling;
    short Alarm;

    short statusLast;
    double valueLast;
    QColor bgAtInitLast;
    QColor fgAtInitLast;
    QString unitsLast;
};

#endif
