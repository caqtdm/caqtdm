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

#ifndef caLINEDEMO_H
#define caLINEDEMO_H

#include <QEvent>
#include <qtcontrols_global.h>
#include "fontscalingwidget.h"
#include "caWidgetInterface.h"

class QTCON_EXPORT caLineDraw : public QWidget, public FontScalingWidget, public caWidgetInterface
{
    Q_OBJECT

    Q_PROPERTY(QString channel READ getPV WRITE setPV)
    Q_PROPERTY(Alignment alignment READ getAlignment WRITE setAlignment)
    Q_PROPERTY(Direction direction READ getDirection WRITE setDirection)
    Q_PROPERTY(QColor foreground READ getForeground WRITE setForeground)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground)
    Q_PROPERTY(colMode colorMode READ getColorMode WRITE setColorMode)

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

    Q_PROPERTY(ScaleMode fontScaleMode READ fontScaleMode WRITE setFontScaleModeL)
    Q_PROPERTY(bool unitsEnabled READ getUnitsEnabled WRITE setUnitsEnabled)
    Q_PROPERTY(FormatType formatType READ getFormatType WRITE setFormatType)
    Q_PROPERTY(QString formatString READ getFormatString WRITE setFormatString)
    // this will prevent user interference
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE noStyle DESIGNABLE false)

    Q_ENUMS(colMode)
    Q_ENUMS(Alignment)
    Q_ENUMS(SourceMode)
    Q_ENUMS(ScaleMode)
    Q_ENUMS(alertHandling)
    Q_ENUMS(FormatType)
    Q_ENUMS(Direction)

public:
    void noStyle(QString style) {Q_UNUSED(style);}

    enum FormatType { decimal, exponential, engr_notation, compact, truncated, utruncated,
                      hexadecimal, octal, string, sexagesimal, sexagesimal_hms, sexagesimal_dms, enumeric,user_defined_format}; // enumeric = enum as number
    enum ScaleMode { None, Height, WidthAndHeight};
    enum Alignment { Center, Left, Right};
    enum alertHandling { onForeground = 0, onBackground };
    enum colMode {Default=0, Static, Alarm_Default, Alarm_Static};
    enum SourceMode {Channel = 0, User};
    enum Direction {Horizontal = 0, Up, Down};

    caLineDraw(QWidget *parent = 0);
    ~caLineDraw() {}

    // caWidgetInterface implementation
    void caDataUpdate(const QString& units, const QString& String, const knobData& data);
    void caActivate(CaQtDM_Lib_Interface* lib_interface, QMap<QString, QString> map, knobData* kData, int* specData, QWidget* parent);
    void getWidgetInfo(QString* pv, int& nbPV, int& limitsDefault, int& precMode, int& limitsMode,
                                    int& Precision, char* colMode, double& limitsMax, double& limitsMin);
    void createContextMenu(QMenu& menu);
    QString getDragText() { return getPV();}
    // caWidgetInterface implementation finish

    // other stuff
    alertHandling getAlarmHandling() const { return m_AlarmHandling;}
    void setAlarmHandling(alertHandling alarmHandling) {m_AlarmHandling = alarmHandling;}

    bool getUnitsEnabled() const { return m_UnitMode;}
    void setUnitsEnabled(bool mode) {m_UnitMode = mode;}

    double getMaxValue()  const {return m_Maximum;}
    void setMaxValue(double const &maxim) {m_Maximum = maxim;}

    double getMinValue()  const {return m_Minimum;}
    void setMinValue(double const &minim) {m_Minimum = minim;}

    QString getPV()  const {return thisPV;}
    void setPV(QString const &newPV) {thisPV = newPV;}

    void setAlignment(const Alignment &alignment);
    Alignment getAlignment() const {return m_Alignment;}

    void setFontScaleModeL(ScaleMode m) { FontScalingWidget::setScaleMode((int) m); }
    ScaleMode fontScaleMode() { return (ScaleMode) FontScalingWidget::scaleMode();  }

    void setFormatType(FormatType m) { m_FormatType = m; }
    FormatType getFormatType() { return m_FormatType; }

    bool getFrame() const { return m_FramePresent;}
    void setFrame(bool frame);
    QColor getFrameColor() const {return m_FrameColor;}
    void setFrameColor(QColor c);
    int getLineWidth() const {return m_FrameLineWidth;}
    void setLinewidth(int width);

    QColor getForeground() const {return m_ForeColor;}
    void setForeground(QColor c);

    QColor getBackground() const {return m_BackColor;}
    void setBackground(QColor c);

    colMode getColorMode() const { return m_ColorMode; }
    void setColorMode(colMode colormode) {m_ColorMode = colormode;
                                          setBackground(m_BackColor);
                                          setForeground(m_ForeColor);}

    SourceMode getPrecisionMode() const { return m_PrecMode; }
    void setPrecisionMode(SourceMode precmode) {m_PrecMode = precmode;}

    SourceMode getLimitsMode() const { return m_LimitsMode; }
    void setLimitsMode(SourceMode limitsmode) { m_LimitsMode = limitsmode;}

    int getPrecision() const {return m_Precision;}
    void setPrecision(int prec) {m_Precision = prec;}

    void setColors(QColor bg, QColor fg, QColor frame);
    void setAlarmColors(short status, double value, QColor bgAtInit, QColor fgAtInit);

    void forceForeAndBackground(QColor fg, QColor bg);
    void setForeAndBackground(QColor foreground, QColor background);
    void setText(const QString &txt);
    QString text() const { return m_Text; }
    void setFormat(int prec);
    void setValue(double value, const QString& units);
    void setDatatype(int datatype);

    void setDirection(const Direction &direction);
    Direction getDirection() const {return m_Direction;}

    bool rotateText(float degrees);

    void setFormatString(const QString m) { thisFormatUserString = m; }
    QString getFormatString() {return thisFormatUserString;}
    QString getMarkedText();
    void resetMarking();
    bool getMarkAll() {return m_markAllText;}
    QString getText() {return m_Text;}

signals:
    void textChanged(QString);

public slots:
    void animation(QRect p) {
#include "animationcode.h"
    }

    void hideObject(bool hideit) {
#include "hideobjectcode.h"
    }

protected:
    virtual bool event(QEvent *);
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    QSize calculateTextSpace();
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    int calculateSumOfStartingCoordinates(QList<int> list, int calculateUntilIndex = -1);
    QPoint calculateCoordinates(QPoint point);
    void handleMarking(QPoint position);
    int getIndexofTextRectangle(QPoint position);
    int getDirectionOfMouseMove(QPoint startPosition, QPoint endPosition);
    QColor invertColor(QColor color);
    CaQtDM_Lib_Interface* caDataInterface;

private:
    QString thisPV, thisText;
    float rotation;
    QColor m_ForeColor, m_ForeColorOld;
    QColor m_BackColor, m_BackColorOld;
    QColor m_BackColorDefault, m_ForeColorDefault;
    colMode m_ColorMode, m_ColorModeOld;
    Alignment m_Alignment;
    Direction m_Direction;
    alertHandling m_AlarmHandling;
    bool m_UnitMode;
    double m_Maximum, m_Minimum;
    int m_Precision;
    SourceMode m_PrecMode;
    SourceMode m_LimitsMode;
    FormatType m_FormatType;
    QString m_Text;
    char m_Format[MAX_STRING_LENGTH];
    char m_FormatC[20];
    short m_AlarmState;
    QColor m_bgAtInitLast;
    QColor m_fgAtInitLast;
    bool m_IsShown;
    bool m_FramePresent;
    QColor m_FrameColor, m_FrameColorOld;
    int m_FrameLineWidth, m_FrameLineWidthOld;
    QColor m_FrameColorTop, m_FrameColorBottom;
    QBrush brush;
    int thisDatatype;
    QString thisFormatUserString;
    QList<QRect> m_LettersBoundingRects;
    QList<bool> m_LetterMarkedList;
    QPoint m_MouseClickPosition;
    bool m_markAllText;
    QRect m_caLineDrawRectangle;
};
#endif
