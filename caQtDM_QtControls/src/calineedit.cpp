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
#include <QEvent>
#include <QStyleOptionFrame>
#include <QStyle>
#include <QtDebug>
#include <QMouseEvent>
#include <qnumeric.h>
#include "knobDefines.h"

#if defined(_MSC_VER)
    #ifndef snprintf
     #define snprintf _snprintf
    #endif
#endif


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
    //printf("got font %s\n", qasc(family));
    if(!family.contains("Lucida Sans Typewriter")) {
        QFont  newfont("Monospace");   // not very nice, while a a dot inside the zero to distinguish from o
        newfont.setStyleHint(QFont::TypeWriter);
        setFont(newfont);
    } else {
       setFont(font);
    }

    specialUnitsAppend = false;
    specialUnitsString = "";

    isShown = false;

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
    defSelectColor = Qt::red; // this does not appear in the palette

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

void caLineEdit::setColors(QColor bg, QColor fg, QColor frame, int lineWidth)
{
    if(!defBackColor.isValid() || !defForeColor.isValid()) return;

    if((bg != oldBackColor) || (fg != oldForeColor) || (thisColorMode != oldColorMode) || (frame != oldFrameColor) || lineWidth != oldFrameLineWidth) {
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
    if(e->type() == QEvent::Resize || e->type() == QEvent::Show) {
        FontScalingWidget::rescaleFont(text(), calculateTextSpace());
        // we try to get the default color for the background set through the external stylesheets

        if(!isShown) {
          setStyleSheet("");
          QString c=  palette().color(QPalette::Base).name();
          defBackColor = QColor(c);
          //printf("default back color %s %s\n", qasc(c), qasc(this->objectName()));
          c=  palette().color(QPalette::Text).name();
          defForeColor = QColor(c);
          //printf("default fore color %s %s\n", qasc(c), qasc(this->objectName()));

          if(!defBackColor.isValid()) defBackColor = QColor(255, 248, 220, 255);
          if(!defForeColor.isValid()) defForeColor = Qt::black;

          setColors(thisBackColor, thisForeColor, thisFrameColor, thisFrameLineWidth);
          isShown = true;
        }

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
    case engr_notation:
        sprintf(thisFormat, "%s.%dle", "%", qAbs(precision));
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
    } else if(thisFormatType == hexadecimal || thisFormatType == octal || thisFormatType == user_defined_format)  {
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
        //qDebug() << "no alarm" << kPtr->pv;
        if(thisColorMode == Alarm_Static || thisColorMode == Alarm_Default) {
            c = AL_GREEN;
            if(thisAlarmHandling == onForeground) setForeAndBackground(c, bgAtInit, thisFrameColor);
            else setForeAndBackground(fgAtInit, c, thisFrameColor);
        } else {
            setForeAndBackground(fgAtInit, bgAtInit, thisFrameColor);
        }
        break;

    case MINOR_ALARM:
        //qDebug() << "minor alarm";
        if(thisColorMode == Alarm_Static || thisColorMode == Alarm_Default) {
            c = AL_YELLOW;
            if(thisAlarmHandling == onForeground) setForeAndBackground(c, bgAtInit, thisFrameColor);
            else setForeAndBackground(fgAtInit, c, thisFrameColor);
        } else {
            setForeAndBackground(fgAtInit, bgAtInit, thisFrameColor);
        }
        break;

    case MAJOR_ALARM:
        //qDebug() << "serious alarm" << kPtr->pv;
        if(thisColorMode == Alarm_Static || thisColorMode == Alarm_Default) {
            c = AL_RED;
            if(thisAlarmHandling == onForeground) setForeAndBackground(c, bgAtInit, thisFrameColor);
            else setForeAndBackground(fgAtInit, c, thisFrameColor);
        } else {
            setForeAndBackground(fgAtInit, bgAtInit, thisFrameColor);
        }
        break;

    case INVALID_ALARM:
        //qDebug() << "invalid alarm";
        if(thisColorMode == Alarm_Static) {
            c =AL_WHITE;
            if(thisAlarmHandling == onForeground) setForeAndBackground(c, bgAtInit, thisFrameColor);
            else setForeAndBackground(fgAtInit, c, thisFrameColor);
        } else {
            setForeAndBackground(fgAtInit, bgAtInit, thisFrameColor);
        }
        break;

    case NOTCONNECTED:
        //qDebug() << "no connection";
        forceForeAndBackground(AL_WHITE, AL_WHITE, thisFrameColor);
        break;

    default:
        //qDebug() << "Alarm default" << status;
        if(thisColorMode == Alarm_Static) {
            c = AL_DEFAULT;
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
    QLineEdit::setText(txt);
    setCursorPosition(pos);
    //printf("settext: %s <%s> <%s> cursor@%d\n", qasc(thisPV),  qasc(txt), qasc(keepText), pos);

    if(keepText.size() != txt.size()) {
       FontScalingWidget::rescaleFont(text(), d_savedTextSpace);
    }

    keepText = txt;
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
    //printf("ESimpleLabel \e[1;33msizeHint\e[0m \"%s\" returning size w %d h %d\n", objectName(), size.width(), size.height());
    return size;
}

QSize caLineEdit::minimumSizeHint() const
{
    QSize size;
    if(!fontScaleEnabled())
        size = QLineEdit::minimumSizeHint();
    else
        size = sizeHint();
    //printf("ESimpleLabel \e[0;33mminimumSizeHint\e[0m \"%s\" returning size w %d h %d\n", objectName(), size.width(), size.height());
    return size;
}

void caLineEdit::setUnitsEnabled(bool check)
{
    thisUnitMode = check;
}

