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
#include "themecolors.h"
#include <QTimer>
#include <QEvent>
#include <QStyleOptionFrame>
#include <QStyle>
#include <QtDebug>
#include <QMouseEvent>
#include <QScrollBar>

#include <QApplication>
#include <QClipboard>

Q_LOGGING_CATEGORY(caMultiLineStringLog, "caqtdm.widgets.camultilinestring")

// colour set of caMultiLineString (level 2), values come from themecolors.h
namespace caMultiLineStringColor {
    constexpr const ThemeColor &AlarmNoAlarm = ThemeColors::AlarmNoAlarm;
    constexpr const ThemeColor &AlarmMinor   = ThemeColors::AlarmMinor;
    constexpr const ThemeColor &AlarmMajor   = ThemeColors::AlarmMajor;
    constexpr const ThemeColor &AlarmInvalid = ThemeColors::AlarmInvalid;
    constexpr const ThemeColor &AlarmDefault = ThemeColors::AlarmDefault;
    constexpr const ThemeColor &NotConnected = ThemeColors::AlarmInvalid;
    constexpr const ThemeColor &FocusFrame   = ThemeColors::FocusFrame;
}

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
    qCDebug(caMultiLineStringLog) << "got font" << family;
    if(!family.contains("Lucida Sans Typewriter")) {
        QFont  newfont("Monospace");   // not very nice, while a a dot inside the zero to distinguish from o
        newfont.setStyleHint(QFont::TypeWriter);
        setFont(newfont);
    } else {
       setFont(font);
    }

    applyingColors = false;
    refreshPending = false;
    defaultsShown = false;
    refreshDefaultColors();

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
    defSelectColor = ThemeColors::color(caMultiLineStringColor::FocusFrame); // not in the palette

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
// default colours: Base/Text as the panel palette and external stylesheets define
// them for this widget (own stylesheet cleared while reading, see themecolors.h)
void caMultiLineString::refreshDefaultColors()
{
    applyingColors = true;            // clearing the sheet re-enters changeEvent
    ThemeColors::defaultBaseAndText(this, defBackColor, defForeColor);
    applyingColors = false;
    oldDefBackColor = oldDefForeColor = QColor();         // force a new stylesheet
    oldStyle.clear();
}

// deferred after an external palette/style change (not from inside the event)
void caMultiLineString::applyDefaultColors()
{
    refreshPending = false;
    // re-apply what was set last (author colours, alarm or not connected) in the mode of
    // that call: a palette change must not undo the dispatcher's state
    const colMode lastMode = oldColorMode;
    const colMode mode = thisColorMode;
    refreshDefaultColors();
    thisColorMode = lastMode;
    setColors(oldBackColor, oldForeColor, thisFrameColor, thisFrameLineWidth);
    thisColorMode = mode;
}

// dark scheme: the (pair-adapted) text colour must stay readable on the alarm colour
static QColor textOnAlarm(QColor text, const QColor &alarm, const QWidget *widget)
{
    const ThemeColors::Scheme scheme = ThemeColors::schemeOf(widget);
    if (scheme == ThemeColors::Dark) ThemeColors::ensureContrast(text, alarm, scheme);
    return text;
}

void caMultiLineString::setColors(QColor bg, QColor fg, QColor frame, int lineWidth)
{
    if(!defBackColor.isValid() || !defForeColor.isValid()) return;
    if(applyingColors) return;        // setStyleSheet/setPalette re-enter through changeEvent
    applyingColors = true;

    if((bg != oldBackColor) || (fg != oldForeColor) || (thisColorMode != oldColorMode) || (frame != oldFrameColor) || lineWidth != oldFrameLineWidth
            || (defBackColor != oldDefBackColor) || (defForeColor != oldDefForeColor)) {

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
                    const QColor alarmText = textOnAlarm(defForeColor, bg, this);
                    thisStyle = thisStyle.
                            arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha()).
                            arg(alarmText.red()).arg(alarmText.green()).arg(alarmText.blue()).arg(alarmText.alpha()).
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
    oldDefBackColor = defBackColor;
    oldDefForeColor = defForeColor;
    applyingColors = false;
}

// the panel palette changed (theme switch): regenerate the colours
void caMultiLineString::changeEvent(QEvent *e)
{
    QPlainTextEdit::changeEvent(e);
    if(applyingColors || refreshPending) return;
    if(e->type() == QEvent::PaletteChange || e->type() == QEvent::StyleChange) {
        refreshPending = true;
        QTimer::singleShot(0, this, &caMultiLineString::applyDefaultColors);
    }
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
#if QT_VERSION< QT_VERSION_CHECK(4, 8, 0)
        if(ev->button() == Qt::MidButton) {
#else
        if(ev->button() == Qt::MiddleButton) {
#endif
            setEnabled(false);
        }

    }
    return QPlainTextEdit::event(e);
}

void caMultiLineString::setAlarmColors(short status, double value, QColor bgAtInit, QColor fgAtInit)
{
    QColor c;

    Alarm = status;

    switch (Alarm) {

    case NO_ALARM:
        qCDebug(caMultiLineStringLog) <<  "no alarm";
        if(thisColorMode == Alarm_Static || thisColorMode == Alarm_Default) {
            c = ThemeColors::color(caMultiLineStringColor::AlarmNoAlarm);
            if(thisAlarmHandling == onForeground) setForeAndBackground(c, bgAtInit, thisFrameColor);
            else setForeAndBackground(textOnAlarm(fgAtInit, c, this), c, thisFrameColor);
        } else {
            setForeAndBackground(fgAtInit, bgAtInit, thisFrameColor);
        }
        break;

    case MINOR_ALARM:
        qCDebug(caMultiLineStringLog) <<  "minor alarm";
        if(thisColorMode == Alarm_Static || thisColorMode == Alarm_Default) {
            c = ThemeColors::color(caMultiLineStringColor::AlarmMinor);
            if(thisAlarmHandling == onForeground) setForeAndBackground(c, bgAtInit, thisFrameColor);
            else setForeAndBackground(textOnAlarm(fgAtInit, c, this), c, thisFrameColor);
        } else {
            setForeAndBackground(fgAtInit, bgAtInit, thisFrameColor);
        }
        break;

    case MAJOR_ALARM:
        qCDebug(caMultiLineStringLog) <<  "serious alarm";
        if(thisColorMode == Alarm_Static || thisColorMode == Alarm_Default) {
            c = ThemeColors::color(caMultiLineStringColor::AlarmMajor);
            if(thisAlarmHandling == onForeground) setForeAndBackground(c, bgAtInit, thisFrameColor);
            else setForeAndBackground(textOnAlarm(fgAtInit, c, this), c, thisFrameColor);
        } else {
            setForeAndBackground(fgAtInit, bgAtInit, thisFrameColor);
        }
        break;

    case INVALID_ALARM:
        qCDebug(caMultiLineStringLog) <<  "invalid alarm";
        if(thisColorMode == Alarm_Static) {
            c =ThemeColors::color(caMultiLineStringColor::AlarmInvalid);
            if(thisAlarmHandling == onForeground) setForeAndBackground(c, bgAtInit, thisFrameColor);
            else setForeAndBackground(textOnAlarm(fgAtInit, c, this), c, thisFrameColor);
        } else {
            setForeAndBackground(fgAtInit, bgAtInit, thisFrameColor);
        }
        break;

    case NOTCONNECTED:
        qCDebug(caMultiLineStringLog) <<  "no connection";
        forceForeAndBackground(ThemeColors::color(caMultiLineStringColor::NotConnected), ThemeColors::color(caMultiLineStringColor::NotConnected), thisFrameColor);
        break;

    default:
        qCDebug(caMultiLineStringLog) <<  "Alarm default" << status;
        if(thisColorMode == Alarm_Static) {
            c = ThemeColors::color(caMultiLineStringColor::AlarmDefault);
            if(thisAlarmHandling == onForeground) setForeAndBackground(c, bgAtInit, thisFrameColor);
            else setForeAndBackground(textOnAlarm(fgAtInit, c, this), c, thisFrameColor);
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
    qCDebug(caMultiLineStringLog) << "ESimpleLabel sizeHint \"" << objectName() << "\" returning size w" << size.width() << "h" << size.height();
    return size;
}

QSize caMultiLineString::minimumSizeHint() const
{
    QSize size;
    if(!fontScaleEnabled())
        size = QPlainTextEdit::minimumSizeHint();
    else
        size = sizeHint();
    qCDebug(caMultiLineStringLog) << "ESimpleLabel minimumsizeHint \"" << objectName() << "\" returning size w" << size.width() << "h" << size.height();

    return size;
}

void caMultiLineString::copy()
{
    QString s = textCursor().selectedText();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(s);
}

void caMultiLineString::clearSelection(){
    QTextCursor c = textCursor();
    c.clearSelection();
    setTextCursor(c);
}

