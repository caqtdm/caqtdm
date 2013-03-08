//******************************************************************************
// Copyright (c) 2012 Paul Scherrer Institut PSI), Villigen, Switzerland
// Disclaimer: neither  PSI, nor any of their employees makes any warranty
// or assumes any legal liability or responsibility for the use of this software
//******************************************************************************
//******************************************************************************
//
//     Author : Anton Chr. Mezger
//
//******************************************************************************

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

    Q_PROPERTY(int precision READ getPrecision WRITE setPrecision)
    Q_PROPERTY(SourceMode precisionMode READ getPrecisionMode WRITE setPrecisionMode)

    Q_PROPERTY(SourceMode limitsMode READ getLimitsMode WRITE setLimitsMode)
    Q_ENUMS(SourceMode)

    Q_PROPERTY(double maxValue READ getMaxValue WRITE setMaxValue)
    Q_PROPERTY(double minValue READ getMinValue WRITE setMinValue)

    Q_PROPERTY(bool fontScaleEnabled READ fontScaleEnabled DESIGNABLE false)
    Q_PROPERTY(ScaleMode fontScaleMode READ fontScaleMode WRITE setFontScaleMode)
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

    enum SourceMode {Channel = 0, User};
    SourceMode getPrecisionMode() const { return thisPrecMode; }
    void setPrecisionMode(SourceMode precmode) {thisPrecMode = precmode;}
    SourceMode getLimitsMode() const { return thisLimitsMode; }
    void setLimitsMode(SourceMode limitsmode) { thisLimitsMode = limitsmode;}

    int getPrecision() const {return thisPrecision;}
    void setPrecision(int prec) {thisPrecision = prec;}

    enum FormatType { decimal, exponential, engr_notation, compact, truncated,
                      hexadecimal, octal, string, sexagesimal, sexagesimal_hms, sexagesimal_dms};

    enum ScaleMode { None, Height, WidthAndHeight};
    virtual void setText(const QString&);
    void forceText(const QString&);
    QString text() const { return QLineEdit::text(); }

    void setFontScaleMode(ScaleMode m) { FontScalingWidget::setScaleMode((int) m);}
    ScaleMode fontScaleMode() { return (ScaleMode) FontScalingWidget::scaleMode(); }

    bool rescaleFontOnTextChanged() { return d_rescaleFontOnTextChanged; }
    void setRescaleFontOnTextChanged(bool en) { d_rescaleFontOnTextChanged = en; }

    void setFormat(int prec);
    void setValue(double value, const QString& units);

    void setFormatType(FormatType m) { thisFormatType = m; }
    FormatType getFormatType() { return thisFormatType; }

    void setForeAndBackground(QColor foreground, QColor background);
    void forceForeAndBackground(QColor foreground, QColor background);
    void setAlarmColors(short status, double value, QColor bgAtInit, QColor fgAtInit);
    void setColors(QColor bg, QColor fg);
    void newFocusPolicy(Qt::FocusPolicy f);

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
    QPalette thisPalette;
    colMode thisColorMode;
    colMode oldColorMode;

    int thisPrecision;
    SourceMode thisPrecMode;
    SourceMode thisLimitsMode;

    int thisUnitMode;
    QString keepText;
    char thisFormat[20];
    char thisFormatC[20];
    bool d_rescaleFontOnTextChanged;
    double thisMaximum, thisMinimum;
    FormatType thisFormatType;
    QString thisStyle, oldStyle;
};

#endif
