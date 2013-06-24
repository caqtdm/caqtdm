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

#include "calineedit.h"
#include "alarmdefs.h"
#include <QEvent>
#include <QStyleOptionFrame>
#include <QStyle>
#include <QtDebug>

caLineEdit::caLineEdit(QWidget *parent) : QLineEdit(parent), FontScalingWidget(this)
{
    // we want this font, while nice and monospace
    QFont font("Lucida Sans Typewriter");
    // if this font does not exist then try a next one
    QFontInfo info(font);
    QString family = info.family();
    //printf("got font %s\n", family.toAscii().constData());
    if(!family.contains("Lucida Sans Typewriter")) {
        QFont  newfont("Monospace");   // not very nice, while a a dot inside the zero to distinguish from o
        setFont(newfont);
    } else {
       setFont(font);
    }

    oldStyle = "";
    thisStyle = "";
    thisColorMode=Default;
    oldColorMode =Static;
    thisFormatC[0] = '\0';

    setUnitsEnabled(false);

    thisBackColor = Qt::gray;
    thisForeColor = Qt::black;

    oldBackColor = Qt::black;
    oldForeColor = Qt::gray;

    setPrecisionMode(Channel);
    setLimitsMode(Channel);
    setPrecision(0);
    setFormatType(decimal);
    setFormat(0);
    setMinValue(0.0);
    setMaxValue(1.0);
    setFrame(false);

    keepText = "";
    setText(keepText);

    setFontScaleMode(WidthAndHeight);
    setColorMode(Default);
    newFocusPolicy(Qt::NoFocus);

    d_rescaleFontOnTextChanged = true;
    connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(rescaleFont(const QString&)));
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
// while this gives a perfomance problem, limit the use of it by testing changes

void caLineEdit::setColors(QColor bg, QColor fg)
{
    if((bg != oldBackColor) || (fg != oldForeColor) || (thisStyle != oldStyle) || (thisColorMode != oldColorMode)) {
        QColor defColor = QColor(255, 248, 220, thisBackColor.alpha());  // my default color, we do not use the stylesheet anymore
        QColor lc;
        QColor dc;
        thisStyle = "caTextEntry,caLineEdit {background-color: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8); border-radius: 1px; }";
        thisStyle.append("caLineEdit {border: 0px; padding: 0px 0px 0px 2px;} caTextEntry { border: 2px; padding: 0px;}");
        if(thisColorMode == Default || thisColorMode == Alarm_Default) {
            thisStyle = thisStyle.arg(defColor.red()).arg(defColor.green()).arg(defColor.blue()).arg(defColor.alpha()).
                    arg(fg.red()).arg(fg.green()).arg(fg.blue()).arg(fg.alpha());

            lc = defColor.lighter();
            dc = defColor.darker();
        } else {
            thisStyle = "caTextEntry, caLineEdit {background-color: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8); border-radius: 1px; }";
            thisStyle.append("caLineEdit{border: 0px; padding: 0px 0px 0px 2px;} caTextEntry { border: 2px; padding: 0px;} ");
            thisStyle = thisStyle.arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha()).
                    arg(fg.red()).arg(fg.green()).arg(fg.blue()).arg(fg.alpha());
            lc = bg.lighter();
            dc = bg.darker();
        }
        if(thisStyle != oldStyle || thisColorMode != oldColorMode) {
            thisStyle.append(" caTextEntry {border-style:inset; border-color: rgba(%1, %2, %3, %4) rgba(%5, %6, %7, %8)  rgba(%9, %10, %11, %12) rgba(%13, %14, %15, %16);} caTextEntry:focus {padding: 0px; border: 2px groove darkred; border-radius: 1px;} ");
            thisStyle = thisStyle.arg(dc.red()).arg(dc.green()).arg(dc.blue()).arg(dc.alpha()).
                    arg(lc.red()).arg(lc.green()).arg(lc.blue()).arg(lc.alpha()).
                    arg(lc.red()).arg(lc.green()).arg(lc.blue()).arg(lc.alpha()).
                    arg(dc.red()).arg(dc.green()).arg(dc.blue()).arg(dc.alpha());
            setStyleSheet(thisStyle);
            oldStyle = thisStyle;
        }
    }
    oldBackColor = bg;
    oldForeColor = fg;
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
    setColors(thisBackColor, thisForeColor);
}

void caLineEdit::setForeground(QColor c)
{
    thisForeColor = c;
    setColors(thisBackColor, thisForeColor);
}

void caLineEdit::forceForeAndBackground(QColor fg, QColor bg)
{
    colMode aux = thisColorMode;
    thisColorMode = Alarm_Static;
    setColors(fg, bg);
    thisColorMode = aux;
}

void caLineEdit::setForeAndBackground(QColor foreground, QColor background)
{
    thisForeColor = foreground;
    thisBackColor = background;
    setColors(thisBackColor, thisForeColor);
}

bool caLineEdit::event(QEvent *e)
{
    if(e->type() == QEvent::Resize || e->type() == QEvent::Show) {
        FontScalingWidget::rescaleFont(text(), calculateTextSpace());
    }

    return QLineEdit::event(e);
}

void caLineEdit::setFormat(int prec)
{
    int precision = prec;
    if(thisPrecMode == User) {
        precision = getPrecision();
    }
    switch (thisFormatType) {
    case string:
    case decimal:
        sprintf(thisFormat, "%s.%dlf", "%", precision);
        break;
    case compact:
        sprintf(thisFormat, "%s.%dle", "%", precision);
        sprintf(thisFormatC, "%s.%dlf", "%", precision);
        break;
    case exponential:
    case engr_notation:
        sprintf(thisFormat, "%s.%dle", "%", precision);
        break;
    case truncated:
        break;
    case hexadecimal:
        strcpy(thisFormat, "0x%x");
        break;
    case octal:
        strcpy(thisFormat, "O%o");
        break;
    case sexagesimal:
    case sexagesimal_hms:
    case sexagesimal_dms:
        break;
    }
}

void caLineEdit::setValue(double value, const QString& units)
{
    char asc[256];

    if(thisFormatType == compact) {
      if ((value < 1.e4 && value > 1.e-4) || (value > -1.e4 && value < -1.e-4) || value == 0.0) {
        sprintf(asc, thisFormatC, value);
      } else {
        sprintf(asc, thisFormat, value);
      }
    } else if(thisFormatType == hexadecimal || thisFormatType == octal)  {
        sprintf(asc, thisFormat, (int) value);
    } else {
        sprintf(asc, thisFormat, value);
    }
    if(thisUnitMode) {
        strcat(asc, " ");
        strcat(asc, units.toAscii().constData());
    }
    setText(asc);
    setCursorPosition(0);
}

void caLineEdit::setAlarmColors(short status, double value, QColor bgAtInit, QColor fgAtInit)
{
    QColor c;
    short Alarm = 0;

    if(status != NOTCONNECTED) {
        if(thisLimitsMode == Channel) {
            Alarm = status;
        } else if(thisLimitsMode == User) {
            if(value > getMaxValue() || value < getMinValue()) {
                Alarm = 2;
            } else {
                Alarm = 0;
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
            setForeAndBackground(c, bgAtInit);
        } else {
            setForeAndBackground(fgAtInit, bgAtInit);
        }
        break;

    case MINOR_ALARM:
        //qDebug() << "minor alarm";
        if(thisColorMode == Alarm_Static || thisColorMode == Alarm_Default) {
            c = AL_YELLOW;
            setForeAndBackground(c, bgAtInit);
        } else {
            setForeAndBackground(fgAtInit, bgAtInit);
        }
        break;

    case MAJOR_ALARM:
        //qDebug() << "serious alarm" << kPtr->pv;

        if(thisColorMode == Alarm_Static || thisColorMode == Alarm_Default) {
            c = AL_RED;
            setForeAndBackground(c, bgAtInit);
        } else {
            setForeAndBackground(fgAtInit, bgAtInit);
        }
        break;

    case ALARM_INVALID:
        //qDebug() << "invalid alarm";

        if(thisColorMode == Alarm_Static) {
            c =AL_WHITE;
            setForeAndBackground(c, bgAtInit);
        } else {
            setForeAndBackground(fgAtInit, bgAtInit);
        }
        break;

    case NOTCONNECTED:
        //qDebug() << "no connection";
        forceForeAndBackground(AL_WHITE, AL_WHITE);
        break;

    default:
        //qDebug() << "Alarm default" << status;
        if(thisColorMode == Alarm_Static) {
            c = AL_DEFAULT;
            setForeAndBackground(c, bgAtInit);
        } else {
            setForeAndBackground(fgAtInit, bgAtInit);
        }

        break;
    }
}

void caLineEdit::setText(const QString &txt)
{
    if(keepText == txt) {  // accelerate things
        return;
    }
    //printf("settext: %s <%s> <%s>\n", thisPV.toAscii().constData(),  txt.toAscii().constData(), keepText.toAscii().constData());
    QLineEdit::setText(txt);

    if(keepText.size() != txt.size()) {
       FontScalingWidget::rescaleFont(text(), d_savedTextSpace);
    }

    keepText = txt;
}

void caLineEdit::forceText(const QString &txt)
{
   // printf("forcetext: <%s>\n", txt.toAscii().constData());
    QLineEdit::setText(txt);
    FontScalingWidget::rescaleFont(text(), d_savedTextSpace);
}

QSize caLineEdit::calculateTextSpace()
{
    QStyleOptionFrame labelStyleOption;
    initStyleOption(&labelStyleOption);
    d_savedTextSpace = style()->subElementRect(QStyle::SE_LineEditContents, &labelStyleOption, this).size();
    return d_savedTextSpace;
}

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
    int w = fm.width(text());
    int h = fm.height();
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
