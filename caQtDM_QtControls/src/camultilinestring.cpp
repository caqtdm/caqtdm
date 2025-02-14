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

#include "camultilinestring.h"
#include "alarmdefs.h"
#include <QEvent>
#include <QStyleOptionFrame>
#include <QStyle>
#include <QtDebug>
#include <QMouseEvent>
#include <QScrollBar>

#include <QApplication>
#include <QClipboard>

caMultiLineString::caMultiLineString(QWidget *parent) : QPlainTextEdit(parent), FontScalingWidget(this)
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

    isShown = false;

    oldStyle = "";
    thisStyle = "";

    setColorMode(Default);
    setAlarmHandling(onForeground);

    thisBackColor = Qt::gray;
    thisForeColor = Qt::black;
    oldBackColor = Qt::black;
    oldForeColor = Qt::gray;
    thisFrameColor = Qt::gray;
    oldFrameColor = Qt::black;

    Alarm = 0;

    // default colors will be defined in my event handler by taking them from the palette defined by stylesheet definitions
    //defBackColor = QColor(255, 248, 220, 255);
    //defForeColor = Qt::black;
    defSelectColor = Qt::red; // this does not appear in the palette

    setLineWrapMode(NoWrap);
    setWordWrapMode(QTextOption::NoWrap);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    keepText = "";
    setTextLine(keepText);

    setFontScaleModeL(WidthAndHeight);
    setFocusPolicy(Qt::NoFocus);
    setLinewidth(0);
    setFrame(false);

    d_rescaleFontOnTextChanged = true;

    installEventFilter(this);

    setFocusPolicy(Qt::ClickFocus);
}

QString caMultiLineString::getPV() const
{
    return thisPV;
}

void caMultiLineString::setPV(QString const &newPV)
{
    thisPV = newPV;
}

// this routine sets the correct styles for the calinedit and catextentry (inheriting from calinedit)
// the styles are now defined here and not in the style sheet any more
// while this gives a perfomance problem, limit the use of it by testing changes
void caMultiLineString::setColors(QColor bg, QColor fg, QColor frame, int lineWidth)
{
    if(!defBackColor.isValid() || !defForeColor.isValid()) return;

    if((bg != oldBackColor) || (fg != oldForeColor) || (thisColorMode != oldColorMode) || (frame != oldFrameColor) || lineWidth != oldFrameLineWidth) {

        thisStyle = "caMultiLineString {background-color: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8); border-radius: 1px;} ";
        thisStyle.append("caMultiLineString {border: %9px; border-style:solid; padding: 0px 0px 0px 2px; border-color: rgba(%10, %11, %12, %13);}");

        // alarm default = (colors from stylesheet)
        if(thisColorMode == Default) {
            thisStyle = thisStyle.arg(defBackColor.red()).arg(defBackColor.green()).arg(defBackColor.blue()).arg(defBackColor.alpha()).
                    arg(defForeColor.red()).arg(defForeColor.green()).arg(defForeColor.blue()).arg(defForeColor.alpha()).
                    arg(lineWidth).
                    arg(frame.red()).arg(frame.green()).arg(frame.blue()).arg(frame.alpha());

          // alarm default = alarm colors on foreground or background (colors from alarms and stylesheet)
          // when major alarm and background handling take the background from stylesheet (normally would be white)
        } else if(thisColorMode == Alarm_Default) {
            if(Alarm == MAJOR_ALARM && thisAlarmHandling == onBackground) {
                thisStyle = thisStyle.arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha()).
                        arg(defBackColor.red()).arg(defBackColor.green()).arg(defBackColor.blue()).arg(defBackColor.alpha()).
                        arg(lineWidth).
                        arg(frame.red()).arg(frame.green()).arg(frame.blue()).arg(frame.alpha());
            } else {
                if(thisAlarmHandling == onForeground) {
                    thisStyle = thisStyle.
                            arg(defBackColor.red()).arg(defBackColor.green()).arg(defBackColor.blue()).arg(defBackColor.alpha()).
                            arg(fg.red()).arg(fg.green()).arg(fg.blue()).arg(fg.alpha()).
                            arg(lineWidth).
                            arg(frame.red()).arg(frame.green()).arg(frame.blue()).arg(frame.alpha());
                } else {
                    thisStyle = thisStyle.
                            arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha()).
                            arg(defForeColor.red()).arg(defForeColor.green()).arg(defForeColor.blue()).arg(defForeColor.alpha()).
                            arg(lineWidth).
                            arg(frame.red()).arg(frame.green()).arg(frame.blue()).arg(frame.alpha());
                }
            }

            // alarm static = alarm colors on foreground or background (colors from color properties)
        } else if(thisColorMode == Alarm_Static) {
            if(thisAlarmHandling == onForeground) {
                thisStyle = thisStyle.
                        arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha()).
                        arg(fg.red()).arg(fg.green()).arg(fg.blue()).arg(fg.alpha()).
                        arg(lineWidth).
                        arg(frame.red()).arg(frame.green()).arg(frame.blue()).arg(frame.alpha());
            } else {
                thisStyle = thisStyle.
                        arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha()).
                        arg(fg.red()).arg(fg.green()).arg(fg.blue()).arg(fg.alpha()).
                        arg(lineWidth).
                        arg(frame.red()).arg(frame.green()).arg(frame.blue()).arg(frame.alpha());
            }

            // static (colors from color properties)
        } else {
            thisStyle = thisStyle.arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha()).
                    arg(fg.red()).arg(fg.green()).arg(fg.blue()).arg(fg.alpha()).
                    arg(lineWidth).
                    arg(frame.red()).arg(frame.green()).arg(frame.blue()).arg(frame.alpha());
        }

        if(thisStyle != oldStyle) {
                setStyleSheet(thisStyle);
        }
        oldStyle = thisStyle;
    }
    oldBackColor = bg;
    oldForeColor = fg;
    oldFrameColor = frame;
    oldFrameLineWidth = lineWidth;
    oldColorMode = thisColorMode;
}

void caMultiLineString::setColorMode(colMode colormode)
{
    thisColorMode = colormode;
    setBackground(thisBackColor);
    setForeground(thisForeColor);
    oldColorMode = thisColorMode;
}

void caMultiLineString::setBackground(QColor c)
{
    thisBackColor = c;
    setColors(thisBackColor, thisForeColor, thisFrameColor, thisFrameLineWidth);
}

void caMultiLineString::setForeground(QColor c)
{
    thisForeColor = c;
    setColors(thisBackColor, thisForeColor, thisFrameColor, thisFrameLineWidth);
}

void caMultiLineString::forceForeAndBackground(QColor fg, QColor bg, QColor fr)
{
    colMode aux = thisColorMode;
    thisColorMode = Alarm_Static;
    setColors(fg, bg, fr, thisFrameLineWidth);
    thisColorMode = aux;
}

void caMultiLineString::setForeAndBackground(QColor foreground, QColor background, QColor frame)
{
    thisForeColor = foreground;
    thisBackColor = background;
    thisFrameColor = frame;
    setColors(thisBackColor, thisForeColor, thisFrameColor, thisFrameLineWidth);
}

void caMultiLineString::setFrame(bool frame) {
    thisFramePresent = frame;
    if(!thisFramePresent) setLinewidth(0);
}

void caMultiLineString::setFrameColor(QColor c) {
    thisFrameColor = c;
    setColors(thisBackColor, thisForeColor, thisFrameColor, thisFrameLineWidth);
}

void caMultiLineString::setLinewidth(int width)
{
    if(width < 0) thisFrameLineWidth = 0;
    else thisFrameLineWidth = width;
    setColors(thisBackColor, thisForeColor, thisFrameColor, thisFrameLineWidth);
}

bool caMultiLineString::event(QEvent *e)
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
#if QT_VERSION< QT_VERSION_CHECK(4, 8, 0)
        if(ev->button() == Qt::MidButton) {
#else
        if(ev->button() == Qt::MiddleButton) {
#endif
            setEnabled(false);
        }

    } else if(e->type() == QEvent::KeyPress) {

        /*
        QKeyEvent *key_event = static_cast<QKeyEvent*>(e);
        if (key_event->matches(QKeySequence::Copy)) {
            emit copy();
        }
        */
    }
    return QPlainTextEdit::event(e);
}

void caMultiLineString::setAlarmColors(short status, double value, QColor bgAtInit, QColor fgAtInit)
{
    QColor c;

    Alarm = status;

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

void caMultiLineString::setTextLine(const QString &txt)
{
    if(keepText == txt) {  // accelerate things
        return;
    }
    QPlainTextEdit::setPlainText(txt);
    if(keepText.size() != txt.size()) {
       FontScalingWidget::rescaleFont(text(), d_savedTextSpace);
    }

    keepText = txt;
}


QSize caMultiLineString::calculateTextSpace()
{
    int innerWidth = size().width();
    int innerHeight = size().height();

    if (verticalScrollBar()) innerWidth -= verticalScrollBar()->width();

    d_savedTextSpace.setWidth(innerWidth - frameWidth() - 4);
    d_savedTextSpace.setHeight(innerHeight - frameWidth() - 6);
    return d_savedTextSpace;
}

// will now be used only for catextentry (performance)
void caMultiLineString::rescaleFont(const QString& newText)
{
    if(d_rescaleFontOnTextChanged) {
        FontScalingWidget::rescaleFont(newText, d_savedTextSpace);
    }
}

QSize caMultiLineString::sizeHint() const
{
    if(!fontScaleEnabled()) return QPlainTextEdit::sizeHint();
    QFont f = font();
    f.setPointSize(10);
    QFontMetrics fm(f);
    int w = QMETRIC_QT456_FONT_WIDTH(fm,text());
    int h = fm.height();
    QSize size(w, h);
    //printf("ESimpleLabel \e[1;33msizeHint\e[0m \"%s\" returning size w %d h %d\n", objectName(), size.width(), size.height());
    return size;
}

QSize caMultiLineString::minimumSizeHint() const
{
    QSize size;
    if(!fontScaleEnabled())
        size = QPlainTextEdit::minimumSizeHint();
    else
        size = sizeHint();
    //printf("ESimpleLabel \e[0;33mminimumSizeHint\e[0m \"%s\" returning size w %d h %d\n", objectName(), size.width(), size.height());
    return size;
}

void caMultiLineString::copy()
{
    QString s = textCursor().selectedText();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(s);
}

