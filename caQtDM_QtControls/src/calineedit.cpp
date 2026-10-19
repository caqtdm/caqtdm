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
#include "calineedit.h"
#include "alarmdefs.h"
#include "themecolors.h"
#include <QTimer>
#include <QEvent>
#include <QStyleOptionFrame>
#include <QStyle>
#include <QtDebug>
#include <QMouseEvent>
#include <qnumeric.h>
#include "knobDefines.h"
#include <QtMath>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    #include <QRegExp>
#else
    #include <QRegularExpression>
#endif
#if defined(_MSC_VER)
    #ifndef snprintf
     #define snprintf _snprintf
    #endif
#endif

Q_LOGGING_CATEGORY(caLineEditLog, "caqtdm.widgets.calineedit")

// colour set of caLineEdit (level 2), values come from themecolors.h
namespace caLineEditColor {
    constexpr const ThemeColor &AlarmNoAlarm = ThemeColors::AlarmNoAlarm;
    constexpr const ThemeColor &AlarmMinor   = ThemeColors::AlarmMinor;
    constexpr const ThemeColor &AlarmMajor   = ThemeColors::AlarmMajor;
    constexpr const ThemeColor &AlarmInvalid = ThemeColors::AlarmInvalid;
    constexpr const ThemeColor &AlarmDefault = ThemeColors::AlarmDefault;
    constexpr const ThemeColor &NotConnected = ThemeColors::AlarmInvalid;
    constexpr const ThemeColor &FocusFrame   = ThemeColors::FocusFrame;
}

caLineEdit::caLineEdit(QWidget *parent) : QLineEdit(parent), FontScalingWidget(this)
{
    // to start with, clear the stylesheet, so that playing around
    // is not possible.
    setStyleSheet("");

    // we want this font, while nice and monospace
    QFont font("Lucida Sans Typewriter");
    // if this font does not exist then try a next one
    QFontInfo info(font);
    //font.setStyleStrategy(QFont::NoAntialias);
    QString family = info.family();
    qCDebug(caLineEditLog) << "got font" << family;
    if(!family.contains("Lucida Sans Typewriter")) {
        QFont  newfont("Monospace");   // not very nice, while a a dot inside the zero to distinguish from o
        newfont.setStyleHint(QFont::TypeWriter);
        setFont(newfont);
    } else {
       setFont(font);
    }

    specialUnitsAppend = false;
    specialUnitsString = "";

    applyingColors = false;
    refreshPending = false;
    defaultsShown = false;
    refreshDefaultColors();

    oldStyle = "";
    thisStyle = "";

    thisFormatC[0] = '\0';

    setUnitsEnabled(false);

    thisBackColor = Qt::gray;
    thisForeColor = Qt::black;

    oldBackColor = Qt::black;
    oldForeColor = Qt::gray;

    thisFrameColor = Qt::black;
    oldFrameColor = Qt::gray;

    thisFramePresent = false;

    thisFrameLineWidth = 0;
    oldFrameLineWidth = 0;

    setColorMode(Default);
    setAlarmHandling(onForeground);

    Alarm = 0;

    thisDatatype = caDOUBLE;
    // default colors will be defined in my event handler by taking them from the palette defined by stylesheet definitions
    defSelectColor = ThemeColors::color(caLineEditColor::FocusFrame); // not in the palette

    setPrecisionMode(Channel);
    setLimitsMode(Channel);
    setPrecision(0);
    setFormatType(decimal);
    setFormat(0);
    setMinValue(0.0);
    setMaxValue(0.0);
    setFrame(false);

    keepText = " ";
    unitsLast = "";
    setTextLine(keepText);
    setValueType(false);
    //thisDatatype = caDOUBLE;

    setFontScaleModeL(WidthAndHeight);
    newFocusPolicy(Qt::NoFocus);
#ifdef MOBILE
    setCompleter(Q_NULLPTR);
#endif
    d_rescaleFontOnTextChanged = true;

    installEventFilter(this);
}

void caLineEdit::setFromTextEntry()
{
    connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(rescaleFont(const QString&)));
}

void caLineEdit::triggertextChange(bool send)
{
    if (send){
        emit textChanged(text());
    }
}

void caLineEdit::setValueType(bool isvalue)
{
    isValue = isvalue;
}

void caLineEdit::newFocusPolicy(Qt::FocusPolicy f){
    setFocusPolicy(f);
}

QString caLineEdit::getPV() const
{
    return thisPV;
}

void caLineEdit::setPV(QString const &newPV)
{
    thisPV = newPV;
}

// this routine sets the correct styles for the calinedit and catextentry (inheriting from calinedit)
// the styles are now defined here and not in the style sheet any more
// while this gives a performance problem, limit the use of it by testing changes

// default colours: Base/Text as the panel palette and external stylesheets define
// them for this widget (own stylesheet cleared while reading, see themecolors.h)
void caLineEdit::refreshDefaultColors()
{
    applyingColors = true;            // clearing the sheet re-enters changeEvent
    ThemeColors::defaultBaseAndText(this, defBackColor, defForeColor);
    applyingColors = false;
    oldDefBackColor = oldDefForeColor = QColor();         // force a new stylesheet
    oldStyle.clear();
}

// deferred after an external palette/style change (not from inside the event)
void caLineEdit::applyDefaultColors()
{
    refreshPending = false;
    // re-apply what was set last (author colours, alarm or not connected) in the mode of
    // that call: a palette change must not undo the dispatcher's state
    const QColor lastBack = oldBackColor;
    const QColor lastFore = oldForeColor;
    const colMode lastMode = oldColorMode;
    const colMode mode = thisColorMode;
    refreshDefaultColors();
    thisColorMode = lastMode;
    setColors(oldBackColor, oldForeColor, thisFrameColor, thisFrameLineWidth);
    thisColorMode = mode;
}

void caLineEdit::setColors(QColor bg, QColor fg, QColor frame, int lineWidth)
{
    if(!defBackColor.isValid() || !defForeColor.isValid()) return;
    if(applyingColors) return;        // setStyleSheet/setPalette re-enter through changeEvent
    applyingColors = true;

    if((bg != oldBackColor) || (fg != oldForeColor) || (thisColorMode != oldColorMode) || (frame != oldFrameColor) || lineWidth != oldFrameLineWidth
            || (defBackColor != oldDefBackColor) || (defForeColor != oldDefForeColor)) {
        QColor lc, dc;
        QColor blc = frame.lighter();
        QColor bdc = frame.darker();

        thisStyle = "caTextEntry,caLineEdit {background-color: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8); border-radius: 1px;} ";
        thisStyle.append("caLineEdit {border: %9px; border-style:outset; padding: 0px 0px 0px 2px; border-color: rgba(%10, %11, %12, %13) rgba(%14, %15, %16, %17)  rgba(%18, %19, %20, %21) rgba(%22, %23, %24, %25);} caTextEntry { border: 2px; padding: 0px;}");

        setBotTopBorderWidth((double) lineWidth+1);
        setLateralBorderWidth((double) lineWidth+1);

        // alarm default = (colors from stylesheet)
        if(thisColorMode == Default) {
            thisStyle = thisStyle.arg(defBackColor.red()).arg(defBackColor.green()).arg(defBackColor.blue()).arg(defBackColor.alpha()).
                    arg(defForeColor.red()).arg(defForeColor.green()).arg(defForeColor.blue()).arg(defForeColor.alpha()).
                    arg(lineWidth).
                    arg(bdc.red()).arg(bdc.green()).arg(bdc.blue()).arg(bdc.alpha()).
                    arg(blc.red()).arg(blc.green()).arg(blc.blue()).arg(blc.alpha()).
                    arg(blc.red()).arg(blc.green()).arg(blc.blue()).arg(blc.alpha()).
                    arg(bdc.red()).arg(bdc.green()).arg(bdc.blue()).arg(bdc.alpha());

            lc = defBackColor.lighter();
            dc = defBackColor.darker();

          // alarm default = alarm colors on foreground or background (colors from alarms and stylesheet)
          // when major alarm and background handling take the background from stylesheet (normally would be white)
        } else if(thisColorMode == Alarm_Default) {
            if(Alarm == MAJOR_ALARM && thisAlarmHandling == onBackground) {
                thisStyle = thisStyle.arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha()).
                        arg(defBackColor.red()).arg(defBackColor.green()).arg(defBackColor.blue()).arg(defBackColor.alpha()).
                        arg(lineWidth).
                        arg(bdc.red()).arg(bdc.green()).arg(bdc.blue()).arg(bdc.alpha()).
                        arg(blc.red()).arg(blc.green()).arg(blc.blue()).arg(blc.alpha()).
                        arg(blc.red()).arg(blc.green()).arg(blc.blue()).arg(blc.alpha()).
                        arg(bdc.red()).arg(bdc.green()).arg(bdc.blue()).arg(bdc.alpha());
            } else {
                if(thisAlarmHandling == onForeground) {
                    thisStyle = thisStyle.
                            arg(defBackColor.red()).arg(defBackColor.green()).arg(defBackColor.blue()).arg(defBackColor.alpha()).
                            arg(fg.red()).arg(fg.green()).arg(fg.blue()).arg(fg.alpha()).
                            arg(lineWidth).
                            arg(bdc.red()).arg(bdc.green()).arg(bdc.blue()).arg(bdc.alpha()).
                            arg(blc.red()).arg(blc.green()).arg(blc.blue()).arg(blc.alpha()).
                            arg(blc.red()).arg(blc.green()).arg(blc.blue()).arg(blc.alpha()).
                            arg(bdc.red()).arg(bdc.green()).arg(bdc.blue()).arg(bdc.alpha());
                } else {
                    thisStyle = thisStyle.
                            arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha()).
                            arg(defForeColor.red()).arg(defForeColor.green()).arg(defForeColor.blue()).arg(defForeColor.alpha()).
                            arg(lineWidth).
                            arg(bdc.red()).arg(bdc.green()).arg(bdc.blue()).arg(bdc.alpha()).
                            arg(blc.red()).arg(blc.green()).arg(blc.blue()).arg(blc.alpha()).
                            arg(blc.red()).arg(blc.green()).arg(blc.blue()).arg(blc.alpha()).
                            arg(bdc.red()).arg(bdc.green()).arg(bdc.blue()).arg(bdc.alpha());
                }
            }

            lc = defBackColor.lighter();
            dc = defBackColor.darker();

            // alarm static = alarm colors on foreground or background (colors from color properties)
        } else if(thisColorMode == Alarm_Static) {
            if(thisAlarmHandling == onForeground) {
                thisStyle = thisStyle.
                        arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha()).
                        arg(fg.red()).arg(fg.green()).arg(fg.blue()).arg(fg.alpha()).
                        arg(lineWidth).
                        arg(bdc.red()).arg(bdc.green()).arg(bdc.blue()).arg(bdc.alpha()).
                        arg(blc.red()).arg(blc.green()).arg(blc.blue()).arg(blc.alpha()).
                        arg(blc.red()).arg(blc.green()).arg(blc.blue()).arg(blc.alpha()).
                        arg(bdc.red()).arg(bdc.green()).arg(bdc.blue()).arg(bdc.alpha());
            } else {
                thisStyle = thisStyle.
                        arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha()).
                        arg(fg.red()).arg(fg.green()).arg(fg.blue()).arg(fg.alpha()).
                        arg(lineWidth).
                        arg(bdc.red()).arg(bdc.green()).arg(bdc.blue()).arg(bdc.alpha()).
                        arg(blc.red()).arg(blc.green()).arg(blc.blue()).arg(blc.alpha()).
                        arg(blc.red()).arg(blc.green()).arg(blc.blue()).arg(blc.alpha()).
                        arg(bdc.red()).arg(bdc.green()).arg(bdc.blue()).arg(bdc.alpha());
            }
            lc = defBackColor.lighter();
            dc = defBackColor.darker();

            // static (colors from color properties)
        } else {
            thisStyle = thisStyle.arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha()).
                    arg(fg.red()).arg(fg.green()).arg(fg.blue()).arg(fg.alpha()).
                    arg(lineWidth).
                    arg(bdc.red()).arg(bdc.green()).arg(bdc.blue()).arg(bdc.alpha()).
                    arg(blc.red()).arg(blc.green()).arg(blc.blue()).arg(blc.alpha()).
                    arg(blc.red()).arg(blc.green()).arg(blc.blue()).arg(blc.alpha()).
                    arg(bdc.red()).arg(bdc.green()).arg(bdc.blue()).arg(bdc.alpha());

            lc = bg.lighter();
            dc = bg.darker();
        }

        if(thisStyle != oldStyle || thisColorMode != oldColorMode) {
            thisStyle.append(" caTextEntry {border-style:inset; border-color: rgba(%1, %2, %3, %4) rgba(%5, %6, %7, %8)  rgba(%9, %10, %11, %12) rgba(%13, %14, %15, %16);} caTextEntry:focus {padding: 0px; border: 2px groove rgba(%17, %18, %19, %20); border-radius: 1px;} ");
            thisStyle = thisStyle.arg(dc.red()).arg(dc.green()).arg(dc.blue()).arg(dc.alpha()).
                    arg(lc.red()).arg(lc.green()).arg(lc.blue()).arg(lc.alpha()).
                    arg(lc.red()).arg(lc.green()).arg(lc.blue()).arg(lc.alpha()).
                    arg(dc.red()).arg(dc.green()).arg(dc.blue()).arg(dc.alpha()).
                    arg(defSelectColor.red()).arg(defSelectColor.green()).arg(defSelectColor.blue()).arg(defSelectColor.alpha());

            if(thisStyle != oldStyle) {
                setStyleSheet(thisStyle);
            }
            oldStyle = thisStyle;
        }
    }
    oldBackColor = bg;
    oldForeColor = fg;
    oldFrameColor = frame;
    oldFrameLineWidth = lineWidth;
    oldColorMode = thisColorMode;
    oldDefBackColor = defBackColor;
    oldDefForeColor = defForeColor;
    applyingColors = false;
}

void caLineEdit::setColorMode(colMode colormode)
{
    thisColorMode = colormode;
    setBackground(thisBackColor);
    setForeground(thisForeColor);
    oldColorMode = thisColorMode;
}

void caLineEdit::setBackground(QColor c)
{
    thisBackColor = c;
    setColors(thisBackColor, thisForeColor, thisFrameColor, thisFrameLineWidth);
}

void caLineEdit::setForeground(QColor c)
{
    thisForeColor = c;
    setColors(thisBackColor, thisForeColor, thisFrameColor, thisFrameLineWidth);
}

void caLineEdit::forceForeAndBackground(QColor fg, QColor bg, QColor fr)
{
    colMode aux = thisColorMode;
    thisColorMode = Alarm_Static;
    setColors(fg, bg, fr, thisFrameLineWidth);
    thisColorMode = aux;
}

void caLineEdit::setForeAndBackground(QColor foreground, QColor background, QColor frame)
{
    thisForeColor = foreground;
    thisBackColor = background;
    thisFrameColor = frame;
    setColors(thisBackColor, thisForeColor, thisFrameColor, thisFrameLineWidth);
}

void caLineEdit::setFrame(bool frame) {
    thisFramePresent = frame;
    if(!thisFramePresent) setLinewidth(0);
}

void caLineEdit::setFrameColor(QColor c) {
    thisFrameColor = c;
    setColors(thisBackColor, thisForeColor, thisFrameColor, thisFrameLineWidth);
}

void caLineEdit::setLinewidth(int width)
{
    if(width < 0) thisFrameLineWidth = 0;
    else thisFrameLineWidth = width;
    setColors(thisBackColor, thisForeColor, thisFrameColor, thisFrameLineWidth);
}

bool caLineEdit::event(QEvent *e)
{
    if(e->type() == QEvent::Show && !defaultsShown) {
        defaultsShown = true;
        applyDefaultColors();
    }
    if(e->type() == QEvent::Resize || e->type() == QEvent::Show) {
        FontScalingWidget::rescaleFont(text(), calculateTextSpace());

    // we do this to temporarily disable the widget in order to be able to initiate a drag
    // for context menu it will be enabled again when drag gets initiated (in caQtDM_Lib)
    } else if(e->type() == QEvent::MouseButtonPress) {
        QMouseEvent *ev = (QMouseEvent *) e;
        setFocus();
#if QT_VERSION< QT_VERSION_CHECK(4, 8, 0)
        if(ev->button() == Qt::MidButton) {
#else
        if(ev->button() == Qt::MiddleButton) {
#endif
            setEnabled(false);
        }
    }
    return QLineEdit::event(e);
}

// the panel palette changed (theme switch): regenerate the colours
void caLineEdit::changeEvent(QEvent *e)
{
    QLineEdit::changeEvent(e);
    if(applyingColors || refreshPending) return;
    if(e->type() == QEvent::PaletteChange || e->type() == QEvent::StyleChange) {
        refreshPending = true;
        QTimer::singleShot(0, this, &caLineEdit::applyDefaultColors);
    }
}

void caLineEdit::setDatatype(int datatype)
{
    thisDatatype = datatype;
}

void caLineEdit::setFormat(int prec)
{
    int precision = prec;
    if(precision > 17) precision = 17;
    if(thisPrecMode == User) {
        precision = getPrecision();
    }

    switch (thisFormatType) {
    case string:
    case decimal:
        if(precision >= 0) {
           sprintf(thisFormat, "%s.%dlf", "%", precision);
        } else {
           sprintf(thisFormat, "%s.%dle", "%", -precision);
        }
        break;
    case compact:
        sprintf(thisFormat, "%s.%dle", "%", qAbs(precision));
        sprintf(thisFormatC, "%s.%dlf", "%", qAbs(precision));
        break;
    case exponential:
        sprintf(thisFormat, "%s.%dle", "%", qAbs(precision));
        break;
    case engr_notation:
        // this format is handled by a function
        engr_notationPrecision = precision;
        break;
    case truncated:
    case enumeric:
        if(thisDatatype == caDOUBLE) qstrncpy(thisFormat, "%lld",MAX_STRING_LENGTH);
        else qstrncpy(thisFormat, "%d",MAX_STRING_LENGTH);
        break;
    case utruncated:
        if(thisDatatype == caDOUBLE) qstrncpy(thisFormat, "%llu",MAX_STRING_LENGTH);
        else qstrncpy(thisFormat, "%u",MAX_STRING_LENGTH);
        break;
    case hexadecimal:
        if(thisDatatype == caDOUBLE) qstrncpy(thisFormat, "0x%llx",MAX_STRING_LENGTH);
        else qstrncpy(thisFormat, "0x%x",MAX_STRING_LENGTH);
        break;
    case octal:
        if(thisDatatype == caDOUBLE) qstrncpy(thisFormat, "O%llo",MAX_STRING_LENGTH);
        else qstrncpy(thisFormat, "O%o",MAX_STRING_LENGTH);
        break;
    case sexagesimal:
    case sexagesimal_hms:
    case sexagesimal_dms:
        break;
    case user_defined_format:{
            qstrncpy(thisFormat,thisFormatUserString.toLatin1().data(),MAX_STRING_LENGTH);
            break;
        }
    }
}
/**
 * Converts a double to engineering notation and writes to a char array
 *
 * @param buffer    Destination char array
 * @param bufferLen Size of the buffer (including space for null terminator)
 * @param value     The double value to convert
 * @param precision Number of digits after the decimal point
 * @return          Number of characters written (excluding null), or -1 on error
 */
int caLineEdit::toEngineeringNotation(char* buffer, size_t bufferLen, double value, int eng_precision) {
    // Validate inputs
    if (buffer == nullptr || bufferLen == 0) {
        return -1;
    }

    // Clamp precision to reasonable bounds
    if (eng_precision < 0) eng_precision = 0;
    if (eng_precision > 15) eng_precision = 15;

    // Handle special cases
    if (qIsNaN(value)) {
        if (bufferLen < 4) { buffer[0] = '\0'; return -1; }
        strncpy(buffer, "nan",bufferLen);
        return 3;
    }

    if (qIsInf(value)) {
        const char* result = (value > 0) ? "inf" : "-inf";
        size_t len = strlen(result);
        if (bufferLen <= len) { buffer[0] = '\0'; return -1; }
        strncpy(buffer, result,bufferLen);
        return static_cast<int>(len);
    }

    if (value == 0.0) {
        int written = snprintf(buffer, bufferLen, " %.*fe+00", eng_precision, 0.0);
        if (written < 0 || static_cast<size_t>(written) >= bufferLen) {
            buffer[0] = '\0';
            return -1;
        }
        return written;
    }

    // Handle negative numbers
    bool negative = value < 0;
    double absValue = qAbs(value);

    // Calculate the exponent (power of 10)
    int exponent = static_cast<int>(qFloor(log10(absValue)));

    // Adjust exponent to be a multiple of 3
    int engExponent;
    if (exponent >= 0) {
        engExponent = exponent - (exponent % 3);
    } else {
        // Handle negative exponents correctly
        int mod = exponent % 3;
        engExponent = (mod == 0) ? exponent : exponent - (3 + mod);
    }

    // Calculate the mantissa
    double mantissa = absValue / qPow(10.0, engExponent);

    // Handle floating point precision issues
    // Mantissa should be in range [1, 1000)
    if (mantissa >= 999.9999999999) {
        mantissa /= 1000.0;
        engExponent += 3;
    } else if (mantissa < 1.0) {
        mantissa *= 1000.0;
        engExponent -= 3;
    }
    int written;

    // Apply sign
    // Format the output
    // Format: [-]d.ddde±ee (sign + digits + decimal + precision + 'e' + sign + 2-3 digits)
    if (negative) {
        written = snprintf(buffer, bufferLen, "-%3.*fe%+03d",
                           eng_precision - (exponent - engExponent), mantissa, engExponent);
    }else{
        written = snprintf(buffer, bufferLen, " %3.*fe%+03d",
                           eng_precision - (exponent - engExponent), mantissa, engExponent);
    }


    if (written < 0 || static_cast<size_t>(written) >= bufferLen) {
        buffer[0] = '\0';
        return -1;
    }

    return written;
}

void caLineEdit::setValue(double value, const QString& units)
{
    char asc[MAX_STRING_LENGTH];
    isValue = true;

    if(thisFormatType == compact) {
      if ((value < 1.e4 && value > 1.e-4) || (value > -1.e4 && value < -1.e-4) || value == 0.0) {
        snprintf(asc, MAX_STRING_LENGTH, thisFormatC, value);
      } else {
        snprintf(asc, MAX_STRING_LENGTH, thisFormat, value);
      }
    } else if(thisFormatType == engr_notation and thisDatatype == caDOUBLE)  {
        // proper handling of engineering notation!
        toEngineeringNotation(asc, MAX_STRING_LENGTH, value, engr_notationPrecision);
    } else if (thisFormatType == user_defined_format) {
        QString pattern = QString("%[+\\- 0#]*[0-9]*([.][0-9]+)?[aefgAEFG]");
        bool isDouble=false;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QRegExp rx(pattern);
        isDouble=rx.indexIn(thisFormat)!=-1;
#else
        QRegularExpression rx(pattern);
        QRegularExpressionMatch match = rx.match(thisFormat);
        isDouble=match.hasMatch();
#endif
        if(isDouble) snprintf(asc, MAX_STRING_LENGTH, thisFormat, value);
        else snprintf(asc, MAX_STRING_LENGTH, thisFormat, (int) value);
    } else if(thisFormatType == hexadecimal || thisFormatType == octal)  {
        if(thisDatatype == caDOUBLE) snprintf(asc, MAX_STRING_LENGTH, thisFormat, (long long) value);
        else  snprintf(asc, MAX_STRING_LENGTH, thisFormat, (int) value);
    } else if(thisFormatType == truncated) {
        if(thisDatatype == caDOUBLE) snprintf(asc, MAX_STRING_LENGTH, thisFormat, (long long) value);
        else  snprintf(asc,MAX_STRING_LENGTH,  thisFormat, (int) value);
    } else if(thisFormatType == enumeric) {
        if(thisDatatype == caDOUBLE) snprintf(asc, MAX_STRING_LENGTH, thisFormat, (long long) value);
        else  snprintf(asc, MAX_STRING_LENGTH, thisFormat, (int) value);
    } else if(thisFormatType == utruncated) {
        if(thisDatatype == caDOUBLE) snprintf(asc, MAX_STRING_LENGTH, thisFormat, (unsigned long long) value);
        else  snprintf(asc,MAX_STRING_LENGTH,  thisFormat, (uint) value);
    } else {
        snprintf(asc, MAX_STRING_LENGTH, thisFormat, value);
    }

    if(qIsNaN(value)){
      snprintf(asc, MAX_STRING_LENGTH,  "nan");
    }

    if(thisUnitMode) {
        strcat(asc, " ");
        if(!specialUnitsAppend) {

            QString datastring=asc;
            datastring=datastring+units;

            unitsLast = units;
            setTextLine( datastring.toUtf8().constData());

        } else {
            strcat(asc, specialUnitsString.toUtf8().constData());
            unitsLast = specialUnitsString;
            setTextLine(asc);
        }
    }else{
        valueLast = value;
        setTextLine(asc);
    }
}

void caLineEdit::appendUnits(const QString& units)
{
    specialUnitsAppend = true;
    specialUnitsString = units;
    setValue(valueLast, units);
}

void caLineEdit::setAlarmColors(short status, double value, QColor bgAtInit, QColor fgAtInit)
{
    QColor c;
    Alarm = 0;

    if(status != NOTCONNECTED) {
        if(thisLimitsMode == Channel) {
            Alarm = status;
        } else if(thisLimitsMode == User) {
            if(value > getMaxValue() || value < getMinValue()) {
                Alarm = MAJOR_ALARM;
            } else {
                Alarm = NO_ALARM;
            }
        } else {
             //return;
        }
    } else {
       Alarm = status;
    }

    switch (Alarm) {

    case NO_ALARM:
        qCDebug(caLineEditLog) << "no alarm";
        if(thisColorMode == Alarm_Static || thisColorMode == Alarm_Default) {
            c = ThemeColors::color(caLineEditColor::AlarmNoAlarm);
            if(thisAlarmHandling == onForeground) setForeAndBackground(c, bgAtInit, thisFrameColor);
            else setForeAndBackground(fgAtInit, c, thisFrameColor);
        } else {
            setForeAndBackground(fgAtInit, bgAtInit, thisFrameColor);
        }
        break;

    case MINOR_ALARM:
        qCDebug(caLineEditLog) << "minor alarm";
        if(thisColorMode == Alarm_Static || thisColorMode == Alarm_Default) {
            c = ThemeColors::color(caLineEditColor::AlarmMinor);
            if(thisAlarmHandling == onForeground) setForeAndBackground(c, bgAtInit, thisFrameColor);
            else setForeAndBackground(fgAtInit, c, thisFrameColor);
        } else {
            setForeAndBackground(fgAtInit, bgAtInit, thisFrameColor);
        }
        break;

    case MAJOR_ALARM:
        qCDebug(caLineEditLog) << "serious alarm";
        if(thisColorMode == Alarm_Static || thisColorMode == Alarm_Default) {
            c = ThemeColors::color(caLineEditColor::AlarmMajor);
            if(thisAlarmHandling == onForeground) setForeAndBackground(c, bgAtInit, thisFrameColor);
            else setForeAndBackground(fgAtInit, c, thisFrameColor);
        } else {
            setForeAndBackground(fgAtInit, bgAtInit, thisFrameColor);
        }
        break;

    case INVALID_ALARM:
        qCDebug(caLineEditLog) << "invalid alarm";
        if(thisColorMode == Alarm_Static) {
            c =ThemeColors::color(caLineEditColor::AlarmInvalid);
            if(thisAlarmHandling == onForeground) setForeAndBackground(c, bgAtInit, thisFrameColor);
            else setForeAndBackground(fgAtInit, c, thisFrameColor);
        } else {
            setForeAndBackground(fgAtInit, bgAtInit, thisFrameColor);
        }
        break;

    case NOTCONNECTED:
        qCDebug(caLineEditLog) << "no connection";
        forceForeAndBackground(ThemeColors::color(caLineEditColor::NotConnected), ThemeColors::color(caLineEditColor::NotConnected), thisFrameColor);
        break;

    default:
        qCDebug(caLineEditLog) << "Alarm default" << status;
        if(thisColorMode == Alarm_Static) {
            c = ThemeColors::color(caLineEditColor::AlarmDefault);
            if(thisAlarmHandling == onForeground) setForeAndBackground(c, bgAtInit, thisFrameColor);
            else setForeAndBackground(fgAtInit, c, thisFrameColor);
        } else {
            setForeAndBackground(fgAtInit, bgAtInit, thisFrameColor);
        }

        break;
    }

    statusLast = status;
    valueLast = value;
    bgAtInitLast = bgAtInit;
    fgAtInitLast = fgAtInit;
}

void caLineEdit::updateAlarmColors()
{
    if (isValue) setValue(valueLast, unitsLast);
}

void caLineEdit::setTextLine(const QString &txt)
{
    int pos;
    if(keepText == txt) {  // accelerate things
        return;
    }
    pos = cursorPosition();

    qCDebug(caLineEditLog) << "settext:" << thisPV << "<" << txt << ">" << "<" << keepText << ">" << "cursor@" << pos;

    if(keepText.size() != txt.size()) {
       FontScalingWidget::rescaleFont(txt, d_savedTextSpace);
       QLineEdit::setText(txt);
       repaint();
    }else{
       QLineEdit::setText(txt);
    }

    keepText = txt;
    qCDebug(caLineEditLog) << "settext:" << thisPV << "<" << txt << ">" << "<" << keepText << ">" << "cursor@" << pos;
}

/* attempt to improve performance
void caLineEdit::setTextLine(const QString &txt)
{
    if(keepText == txt) return;
    if(keepText.size() != txt.size()) {
        FontScalingWidget::rescaleFont(txt, d_savedTextSpace);
    }
    keepText = txt;
    repaint();
}
*/

void caLineEdit::forceText(const QString &txt)
{
    int pos = cursorPosition();
    QLineEdit::setText(txt);
    FontScalingWidget::rescaleFont(text(), d_savedTextSpace);
    setCursorPosition(pos);
}

QSize caLineEdit::calculateTextSpace()
{
    QStyleOptionFrame labelStyleOption;
    initStyleOption(&labelStyleOption);
    d_savedTextSpace = style()->subElementRect(QStyle::SE_LineEditContents, &labelStyleOption, this).size();
    return d_savedTextSpace;
}

// will now be used only for catextentry (performance)
void caLineEdit::rescaleFont(const QString& newText)
{
    if(d_rescaleFontOnTextChanged) {
        FontScalingWidget::rescaleFont(newText, d_savedTextSpace);
    }
}

QSize caLineEdit::sizeHint() const
{
    if(!fontScaleEnabled()) return QLineEdit::sizeHint();
    QFont f = font();
    f.setPointSize(10);
    QFontMetrics fm(f);
    int w = QMETRIC_QT456_FONT_WIDTH(fm,text());
    int h = QMETRIC_QT456_FONT_HEIGHT(fm,text());
    QSize size(w, h);
    qCDebug(caLineEditLog) << "ESimpleLabel sizeHint \"" << objectName() << "\" returning size w" << size.width() << "h" << size.height();
    return size;
}

QSize caLineEdit::minimumSizeHint() const
{
    QSize size;
    if(!fontScaleEnabled())
        size = QLineEdit::minimumSizeHint();
    else
        size = sizeHint();
    qCDebug(caLineEditLog) << "ESimpleLabel minimumsizeHint \"" << objectName() << "\" returning size w" << size.width() << "h" << size.height();

    return size;
}

void caLineEdit::setUnitsEnabled(bool check)
{
    thisUnitMode = check;
}

