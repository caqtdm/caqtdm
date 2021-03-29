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

#include "catextlog.h"
#include "alarmdefs.h"
#include <iostream>
#include <sstream>
#include <sys/time.h>
#include <QEvent>
#include <QStyleOptionFrame>
#include <QStyle>
#include <QtDebug>
#include <QMouseEvent>
#include <QScrollBar>

#include <QApplication>
#include <QClipboard>

caTextLog::caTextLog(QWidget *parent) : QPlainTextEdit(parent), FontScalingWidget(this)
{
    std::cerr << "Called caTextLog constructor" << std::endl;
    std::cerr << "Called caTextLog constructor" << std::endl;
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

    setLineWrapMode(WidgetWidth);
    setWordWrapMode(QTextOption::WordWrap);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    keepText = "";
    setTextLine(keepText);

    setFontScaleModeL(WidthAndHeight);
    setFocusPolicy(Qt::NoFocus);
    setLinewidth(2);
    setFrame(true);


    setFontScaleFactor(0.2);
    d_rescaleFontOnTextChanged = false;

    installEventFilter(this);

    setFocusPolicy(Qt::ClickFocus);
}

QString caTextLog::getPV() const
{
    return thisPV;
}

void caTextLog::setPV(QString const &newPV)
{
    thisPV = newPV;
}

// this routine sets the correct styles for the calinedit and catextentry (inheriting from calinedit)
// the styles are now defined here and not in the style sheet any more
// while this gives a perfomance problem, limit the use of it by testing changes
void caTextLog::setColors(QColor bg, QColor fg, QColor frame, int lineWidth)
{
    if(!defBackColor.isValid() || !defForeColor.isValid()) return;

    if((bg != oldBackColor) || (fg != oldForeColor) || (thisColorMode != oldColorMode) || (frame != oldFrameColor) || lineWidth != oldFrameLineWidth) {

        thisStyle = "caTextLog {background-color: rgba(%1, %2, %3, %4); color: rgba(%5, %6, %7, %8); border-radius: 1px;} ";
        thisStyle.append("caTextLog {border: %9px; border-style:solid; padding: 0px 0px 0px 2px; border-color: rgba(%10, %11, %12, %13);}");

        // alarm default = (colors from stylesheet)

        thisStyle = thisStyle.arg(defBackColor.red()).arg(defBackColor.green()).arg(defBackColor.blue()).arg(defBackColor.alpha()).
                arg(defForeColor.red()).arg(defForeColor.green()).arg(defForeColor.blue()).arg(defForeColor.alpha()).
                arg(lineWidth).
                arg(frame.red()).arg(frame.green()).arg(frame.blue()).arg(frame.alpha());

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

void caTextLog::setColorMode(colMode colormode)
{
    thisColorMode = colormode;
    setBackground(thisBackColor);
    setForeground(thisForeColor);
    oldColorMode = thisColorMode;
}

void caTextLog::setBackground(QColor c)
{
    thisBackColor = c;
    setColors(thisBackColor, thisForeColor, thisFrameColor, thisFrameLineWidth);
}

void caTextLog::setForeground(QColor c)
{
    thisForeColor = c;
    setColors(thisBackColor, thisForeColor, thisFrameColor, thisFrameLineWidth);
}

void caTextLog::forceForeAndBackground(QColor fg, QColor bg, QColor fr)
{
    colMode aux = thisColorMode;
    thisColorMode = Alarm_Static;
    setColors(fg, bg, fr, thisFrameLineWidth);
    thisColorMode = aux;
}

void caTextLog::setForeAndBackground(QColor foreground, QColor background, QColor frame)
{
    thisForeColor = foreground;
    thisBackColor = background;
    thisFrameColor = frame;
    setColors(thisBackColor, thisForeColor, thisFrameColor, thisFrameLineWidth);
}

void caTextLog::setFrame(bool frame) {
    thisFramePresent = frame;
    if(!thisFramePresent) setLinewidth(0);
}

void caTextLog::setFrameColor(QColor c) {
    thisFrameColor = c;
    setColors(thisBackColor, thisForeColor, thisFrameColor, thisFrameLineWidth);
}

void caTextLog::setLinewidth(int width)
{
    if(width < 0) thisFrameLineWidth = 0;
    else thisFrameLineWidth = width;
    setColors(thisBackColor, thisForeColor, thisFrameColor, thisFrameLineWidth);
}

bool caTextLog::event(QEvent *e)
{
    if(e->type() == QEvent::Resize || e->type() == QEvent::Show) {
        this->rescaleFont();
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

        QKeyEvent *key_event = static_cast<QKeyEvent*>(e);
        if (key_event->matches(QKeySequence::Copy)) {
            emit copy();
        }

    }
    return QPlainTextEdit::event(e);
}

void caTextLog::setAlarmColors(short status, double value, QColor bgAtInit, QColor fgAtInit)
{
    QColor c;

    Alarm = status;
    // No Alarm for text log!
    if(thisColorMode == Alarm_Static || thisColorMode == Alarm_Default) {
        c = AL_GREEN;
        if(thisAlarmHandling == onForeground) setForeAndBackground(c, bgAtInit, thisFrameColor);
        else setForeAndBackground(fgAtInit, c, thisFrameColor);
    } else {
        setForeAndBackground(fgAtInit, bgAtInit, thisFrameColor);
    }

    statusLast = status;
    valueLast = value;
    bgAtInitLast = bgAtInit;
    fgAtInitLast = fgAtInit;
}

void caTextLog::setTextLine(const QString &txt){

    QPlainTextEdit::setPlainText(keepText + "\n" +txt);
    // FontScalingWidget::rescaleFont(text(), d_savedTextSpace);

    // Timestamp from local time
    const time_t cTime = time(NULL);
    struct tm *timeinfo = localtime(&cTime);
    char ss[40];
    sprintf(ss, "\n[%02d:%02d:%02d] ", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    keepText = keepText + ss + txt;
    QPlainTextEdit::setPlainText(keepText);
}


QSize caTextLog::calculateTextSpace()
{
    int innerWidth = size().width();
    int innerHeight = size().height();

    if (verticalScrollBar()) innerWidth -= verticalScrollBar()->width();

    d_savedTextSpace.setWidth(innerWidth - frameWidth() - 4);
    d_savedTextSpace.setHeight(innerHeight - frameWidth() - 6);
    return d_savedTextSpace;
}

// will now be used only for catextentry (performance)
void caTextLog::rescaleFont(){
    // Constant 40 character width rescaling
    QFont f = QPlainTextEdit::font();
    f.setPointSizeF(QPlainTextEdit::width()/40);
    QPlainTextEdit::setFont(f);
}

QSize caTextLog::sizeHint() const
{
    if(!fontScaleEnabled()) return QPlainTextEdit::sizeHint();
    QFont f = font();
    f.setPointSize(10);
    QFontMetrics fm(f);
    int w = fm.width(text());
    int h = fm.height();
    QSize size(w, h);
    //printf("ESimpleLabel \e[1;33msizeHint\e[0m \"%s\" returning size w %d h %d\n", objectName(), size.width(), size.height());
    return size;
}

QSize caTextLog::minimumSizeHint() const
{
    QSize size;
    if(!fontScaleEnabled())
        size = QPlainTextEdit::minimumSizeHint();
    else
        size = sizeHint();
    //printf("ESimpleLabel \e[0;33mminimumSizeHint\e[0m \"%s\" returning size w %d h %d\n", objectName(), size.width(), size.height());
    return size;
}

void caTextLog::copy()
{
    QString s = textCursor().selectedText();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(s);
}

