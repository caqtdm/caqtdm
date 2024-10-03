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

#include "calinedraw.h"
#include "alarmdefs.h"
#include "qevent.h"

#include <cmath>
#include <cstdlib>
#include <QPainter>
#include <qnumeric.h>
#include <QDebug>
#if defined(_MSC_VER)
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif

caLineDraw::caLineDraw(QWidget *parent) : QWidget(parent), FontScalingWidget(this), caWidgetInterface()
{
    // we want this font, while nice and monospace
    QFont font("Lucida Sans Typewriter");
    // if this font does not exist then try a next one
    QFontInfo info(font);
    QString family = info.family();
    // printf("got font %s\n", qasc(family));
    if (!family.contains("Lucida Sans Typewriter"))
    {
        QFont newfont("Monospace"); // not very nice, while a a dot inside the zero to distinguish from o
        newfont.setStyleHint(QFont::TypeWriter);
        setFont(newfont);
    }
    else
    {
        setFont(font);
    }

    m_Text = "text";
    m_IsShown = false;
    m_BackColor = QColor(255, 255, 255, 0);
    m_ForeColor = Qt::black;
    m_BackColorOld = Qt::black;
    m_ForeColorOld = Qt::gray;
    m_FrameColor = Qt::black;
    m_FrameLineWidth = 0;
    m_FramePresent = false;

    m_ColorMode = Default;
    m_AlarmHandling = onForeground;
    m_Alignment = Center;
    m_FormatType = decimal;
    m_UnitMode = false;
    m_PrecMode = Channel;
    m_LimitsMode = Channel;
    m_Precision = 0;
    m_FormatType = decimal;

    m_Maximum = 0.0;
    m_Minimum = 0.0;

    m_Format[0] = '\0';
    m_FormatC[0] = '\0';

    setFormat(0);
    setFontScaleModeL(WidthAndHeight);
    setFocusPolicy(Qt::NoFocus);
    setDirection(Horizontal);

    m_AlarmState = 0;

    brush = QBrush(m_BackColor);
    //setText(" ");
}

void caLineDraw::setFrame(bool frame)
{
    m_FramePresent = frame;
    if (!m_FramePresent)
        setLinewidth(0);
}

void caLineDraw::setFrameColor(QColor c)
{
    m_FrameColor = c;
    setColors(m_BackColor, m_ForeColor, m_FrameColor);
}

void caLineDraw::setLinewidth(int width)
{
    if (width < 0)
        m_FrameLineWidth = 0;
    else
        m_FrameLineWidth = width;
    update();
}

void caLineDraw::setAlignment(const Alignment &alignment)
{
    m_Alignment = alignment;
    update();
}

void caLineDraw::setBackground(QColor c)
{
    m_BackColor = c;
    setColors(m_BackColor, m_ForeColor, m_FrameColor);
}

void caLineDraw::setForeground(QColor c)
{
    m_ForeColor = c;
    setColors(m_BackColor, m_ForeColor, m_FrameColor);
}

void caLineDraw::setColors(QColor bg, QColor fg, QColor frame)
{
    if (!m_BackColorDefault.isValid() || !m_ForeColorDefault.isValid())
        return;

    if ((bg != m_BackColorOld) || (fg != m_ForeColorOld) || (m_ColorMode != m_ColorModeOld) || (frame != m_FrameColorOld))
    {

        m_FrameColorTop = frame.darker();
        m_FrameColorBottom = frame.lighter();

        // default = (colors from stylesheet)
        if (m_ColorMode == Default)
        {
            m_BackColor = m_BackColorDefault;
            m_ForeColor = m_ForeColorDefault;

            // alarm default = alarm colors on foreground or background (colors from alarms and stylesheet)
            // when major alarm and background handling take the background from stylesheet (normally would be white)
        }
        else if (m_ColorMode == Alarm_Default)
        {

            if (m_AlarmState == MAJOR_ALARM && m_AlarmHandling == onBackground)
            {
                m_BackColor = bg;
                m_ForeColor = m_BackColorDefault;
            }
            else
            {
                if (m_AlarmHandling == onForeground)
                {
                    m_BackColor = m_BackColorDefault;
                    m_ForeColor = fg;
                }
                else
                {
                    m_BackColor = bg;
                    m_ForeColor = m_ForeColorDefault;
                }
            }

            // alarm alarm colors on foreground or background (colors from color properties)
        }
        else if (m_ColorMode == Alarm_Static)
        {
            m_BackColor = bg;
            m_ForeColor = fg;

            // static (colors from color properties)
        }
        else
        {
            m_BackColor = bg;
            m_ForeColor = fg;
        }
        brush = QBrush(m_BackColor);
        update();
    }
    m_BackColorOld = bg;
    m_ForeColorOld = fg;
    m_ColorModeOld = m_ColorMode;
}

void caLineDraw::forceForeAndBackground(QColor fg, QColor bg)
{
    colMode aux = m_ColorMode;
    m_ColorMode = Alarm_Static;
    setColors(fg, bg, m_FrameColor);
    m_ColorMode = aux;
}

void caLineDraw::setForeAndBackground(QColor foreground, QColor background)
{
    m_ForeColor = foreground;
    m_BackColor = background;
    setColors(m_BackColor, m_ForeColor, m_FrameColor);
}

void caLineDraw::setAlarmColors(short status, double value, QColor bgAtInit, QColor fgAtInit)
{
    QColor c;
    m_AlarmState = 0;

    if (status != NOTCONNECTED)
    {
        if (m_LimitsMode == Channel)
        {
            m_AlarmState = status;
        }
        else if (m_LimitsMode == User)
        {
            if (value > getMaxValue() || value < getMinValue())
            {
                m_AlarmState = MAJOR_ALARM;
            }
            else
            {
                m_AlarmState = NO_ALARM;
            }
        }
        else
        {
            // return;
        }
    }
    else
    {
        m_AlarmState = status;
    }

    switch (m_AlarmState)
    {

    case NO_ALARM:
        // qDebug() << "no alarm" << kPtr->pv;
        if (m_ColorMode == Alarm_Static || m_ColorMode == Alarm_Default)
        {
            c = AL_GREEN;
            if (m_AlarmHandling == onForeground)
                setForeAndBackground(c, bgAtInit);
            else
                setForeAndBackground(fgAtInit, c);
        }
        else
        {
            setForeAndBackground(fgAtInit, bgAtInit);
        }
        break;

    case MINOR_ALARM:
        // qDebug() << "minor alarm";
        if (m_ColorMode == Alarm_Static || m_ColorMode == Alarm_Default)
        {
            c = AL_YELLOW;
            if (m_AlarmHandling == onForeground)
                setForeAndBackground(c, bgAtInit);
            else
                setForeAndBackground(fgAtInit, c);
        }
        else
        {
            setForeAndBackground(fgAtInit, bgAtInit);
        }
        break;

    case MAJOR_ALARM:
        // qDebug() << "serious alarm" << kPtr->pv;
        if (m_ColorMode == Alarm_Static || m_ColorMode == Alarm_Default)
        {
            c = AL_RED;
            if (m_AlarmHandling == onForeground)
                setForeAndBackground(c, bgAtInit);
            else
                setForeAndBackground(fgAtInit, c);
        }
        else
        {
            setForeAndBackground(fgAtInit, bgAtInit);
        }
        break;

    case INVALID_ALARM:
        // qDebug() << "invalid alarm";
        if (m_ColorMode == Alarm_Static)
        {
            c = AL_WHITE;
            if (m_AlarmHandling == onForeground)
                setForeAndBackground(c, bgAtInit);
            else
                setForeAndBackground(fgAtInit, c);
        }
        else
        {
            setForeAndBackground(fgAtInit, bgAtInit);
        }
        break;

    case NOTCONNECTED:
        // qDebug() << "no connection";
        forceForeAndBackground(AL_WHITE, AL_WHITE);
        break;

    default:
        // qDebug() << "Alarm default" << status;
        if (m_ColorMode == Alarm_Static)
        {
            c = AL_DEFAULT;
            if (m_AlarmHandling == onForeground)
                setForeAndBackground(c, bgAtInit);
            else
                setForeAndBackground(fgAtInit, c);
        }
        else
        {
            setForeAndBackground(fgAtInit, bgAtInit);
        }

        break;
    }

    m_bgAtInitLast = bgAtInit;
    m_fgAtInitLast = fgAtInit;
}

void caLineDraw::setDirection(const Direction &direction)
{
    switch (direction)
    {
    case Horizontal:
        rotateText(0.0);
        setVerticalLabel(false);
        break;
    case Up:
        rotateText(270.0);
        setVerticalLabel(true);
        break;
    case Down:
    default:
        rotateText(90.0);
        setVerticalLabel(true);
        break;
    }
    m_Direction = direction;
    update();
}

bool caLineDraw::rotateText(float degrees)
{
    if (degrees >= 0 && degrees <= 360)
    {
        rotation = degrees;
        update();
        return true;
    }
    return false;
}

void caLineDraw::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() == Qt::LeftButton){
        isMarked = QList<bool>();
        startPointMarker = transformCoordinates(event->pos());
        update();
    }
}

void caLineDraw::mouseMoveEvent(QMouseEvent *event){
    if(event->buttons() == Qt::LeftButton){
        QPoint position = event->pos();

        // Setup Marking-List
        for(int i = 0; i < m_Text.size(); i++){
            isMarked << false;
        }

        handleMarking(position);
        update();
    }
}

/**
 * @brief Handles Marking of Text when clicking and moving the mouse
 * @param position -> Current Mouse Position
 */
void caLineDraw::handleMarking(QPoint position){
    // Reset Marking-List
    for(int i = 0; i < isMarked.size(); i++){
        isMarked[i] = false;
    }

    for(int i = 0; i < isMarked.count(); i++){

        QRect rect = BoundingRects[i];

        // Harmonise Coordinates in relation to Direction
        QPoint p = position;
        p = transformCoordinates(position);
        // Ignore Y-Axis for Marking
        p.setY(rect.center().y());

        // Get starting index (mouseclick) and current index (mousemoveposition)
        int startIndex = getIndexOfMarkedRect(startPointMarker);

        int currentIndex = getIndexOfMarkedRect(p);
        int start;
        int end;

        // Define Correct Startingindex
        switch(startIndex < currentIndex){
        case false:
            start = currentIndex;
            end = startIndex;
            break;
        case true:
        default:
            start = startIndex;
            end = currentIndex;
            break;
        }



        // Set Marking-List
        for(int j = start; j <= end; j++){
            if(start >= 0 && end >= 0){
                isMarked[j] = true;
            }else{
                isMarked[j] = false;
            }
        }

        // Is Starting Point within Text
        bool startWithinBounds = (startPointMarker.x() > BoundingRects[0].x() && startPointMarker.x() <= BoundingRects[m_Text.size()-1].right());
        // Is End Point within Text
        bool endWithinBounds = (p.x() > BoundingRects[0].x() && p.x() <= BoundingRects[m_Text.size()-1].right());

        // Is Any Text between both Points, i.e. are both points on the same side outside of the text
        bool isTextBetween = ((startPointMarker.x() < BoundingRects[0].x() && p.x() > BoundingRects[m_Text.size()-1].right()) || (startPointMarker.x() > BoundingRects[0].x() && p.x() < BoundingRects[m_Text.size()-1].right()));


        if(!startWithinBounds && !endWithinBounds && !isTextBetween){
            for(int i = 0; i < isMarked.size(); i++){
               isMarked[i] = false;
            }
        }
    }
}

/**
 * @brief Return Index of Currently Marked Rectangle
 * @param position -> Current Position of Mouse
 * @return index -> index of marked Rectangle for the "isMarked" list
 */
int caLineDraw::getIndexOfMarkedRect(QPoint position){
    int index = 0;

    for(int i = 0; i <= isMarked.size(); i++){
        if(BoundingRects[i].contains(position)){
            index = i;
            break;
        }
    }
    int lastIdx = m_Text.size()-1;

    // If Mouse leaves Text and index is bigger than text, set to length of text

    if(position.x() > BoundingRects[lastIdx].x()){
        index = m_Text.size()-1;
    }

    return index;
}

/**
 * @brief Returns Direction of Mouse
 * @param startPosition -> Starting (1st) Position of Mouse
 * @param endPosition -> Ending (2nd) Position of Mouse
 * @return difference -> Direction is returned as difference of distance
 *
 *  IF diff > 0 -> RIGHT
 *
 *  IF diff < 0 -> LEFT
 *
 *  IF diff = 0 -> VERTICAL or NONE
 */
int caLineDraw::getDirectionOfMouseMove(QPoint startPosition, QPoint endPosition){
    int endLocation = 0;
    int startLocation = 0;

    switch(m_Direction){
    case Down:
        startLocation = startPosition.y();
        endLocation = endPosition.y();
        break;
    case Up:
        endLocation = startPosition.y();
        startLocation = endPosition.y();
        break;
    case Horizontal:
        startLocation = startPosition.x();
        endLocation = endPosition.x();
        break;
    }

    int difference = endLocation - startLocation;

    return difference;
    // positive: right,
    // negative: left,
    // equal to zero: vertical or no at all
}

/**
 * @brief Returns text that is currently marked
 * @return text -> Currently Marked Text
 */
QString caLineDraw::getMarkedText(){
    QString text;
    for(int i = 0; i < m_Text.size(); i++){
        if(isMarked[i]){
            text += m_Text[(i % (m_Text.size() + 1))];
        }
    }
    // Replace Spaces and null Values
    text = text.replace(QString(" "), "");
    text = text.replace(QString(1, QChar('\0')), "");
    qDebug() << text;
    return text;
}

/**
 * @brief Returns all Currently Marked Rectangles
 * @return list -> List of marked Rectangles
 */
QList<QRect> caLineDraw::getMarkedRects(){
    QList<QRect> list;
    for(int i = 0; i < isMarked.count(); i++){
        if(isMarked[i] && !list.contains(BoundingRects[i])){
            list.append(BoundingRects[i]);
        }
    }
    return list;
}

/**
 * @brief Converts coordinates of a point to same kind of coordinates across all directions and alignments *
 * @param point -> The Point that will be transformed
 * @return point -> The same point but with transformed Coordinates
 */
QPoint caLineDraw::transformCoordinates(QPoint point){
    // Unifies Coordinates so that the same ones can be used regardeless of orientation.
    int x;
    int y;
    switch(m_Direction){
    case Up:
        x = (-point.y() + m_textRect.right());
        y = point.x();
        break;
    case Down:
        x = point.y();
        y = (-point.x() + m_textRect.height());
        break;
    case Horizontal:
    default:
        x = point.x();
        y = point.y();
        break;
    }
    return QPoint(x, y);
}

/**
 * @brief Returns Sum Of Coordinate List.
 * @param list -> List of Coordinates
 * @return sum -> Sum Of Coordinates
 */
int caLineDraw::getSumOfCoords(QList<int> list, int end)
{
    if(end == -1){
        end = list.size();
    }else{
        end += 1;
    }
    // Return Sum of Coordinates to get Beginning Point for Marking
    int sum = 0;
    for(int i = 0; i < end; i++){
        sum += list[i];
    }
    return sum;
}

void caLineDraw::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(m_ForeColor);
    painter.setBackground(brush);
    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.fillRect(rect(), brush);
    painter.save();
    QRect textRect;

    switch (m_Direction)
    {
    case Horizontal:
        // Create a rectangle to draw text in, make it slightly smaller than the container rect and take frame width into account
        textRect = QRect(QPoint(1 + m_FrameLineWidth, 1 + m_FrameLineWidth), QPoint(width() - 1 - m_FrameLineWidth, height() - 1 - m_FrameLineWidth));
        break;
    case Up:
        // Rectangle is slightly different than for Horizontal, because it needs to be rotated
        textRect = QRect(QPoint(1 + m_FrameLineWidth, 1 + m_FrameLineWidth), QPoint(height() - 1 - m_FrameLineWidth, width() - 1 - m_FrameLineWidth));
        // Move the coordinate system and rotate it, so the text is displayed upwards
        painter.translate(QPoint(0, height()));
        painter.rotate(rotation);
        break;
    case Down:
        // Rectangle is slightly different than for Horizontal, because it needs to be rotated
        textRect = QRect(QPoint(1 + m_FrameLineWidth, 1 + m_FrameLineWidth), QPoint(height() - 1 - m_FrameLineWidth, width() - 1 - m_FrameLineWidth));
        // Move the coordinate system and rotate it, so the text is displayed downwards
        painter.translate(QPoint(width(), 0));
        painter.rotate(rotation);
    }

    int widthTextLess = 0;
    const QFontMetrics fm = painter.fontMetrics();

    // Now that textrect and rotation/translation is set, draw the text aligned correctly
    switch (m_Alignment)
    {
    case Left:
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, m_Text);
        break;
    case Right:
        painter.drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, m_Text);
        widthTextLess = textRect.width() - fm.horizontalAdvance(m_Text);
        break;
    case Center:
    default:
        painter.drawText(textRect, Qt::AlignCenter | Qt::AlignVCenter, m_Text);
        widthTextLess = (textRect.width() / 2) - (fm.horizontalAdvance(m_Text)/2);
        break;
    }

    QList<int> letterCoordinates;
    letterCoordinates << widthTextLess;
    m_textRect = textRect;

    int x;
    int y = textRect.height();
    painter.setPen(brush.color());

    for(int i = 0; i < m_Text.size(); i++){

        QRect r = fm.boundingRect(m_Text[i]);
        x = getSumOfCoords(letterCoordinates) + m_FrameLineWidth;

        // Get accurate width of bounding rectangle
        int horizontalAdvance = fm.horizontalAdvance(m_Text[i]);

        // Calculate and set Startingpoint from previous letters
        y += m_FrameLineWidth;
        letterCoordinates << horizontalAdvance;
        r.setX(x);
        r.setY(y);
        r.setHeight(-y);
        r.setWidth(horizontalAdvance);

        painter.setPen(m_ForeColor);
        BoundingRects << r;
        painter.drawRect(r);
    }

    // MARKING

    for(int i = 0; i < getMarkedRects().size(); i++){
        QRect marked = getMarkedRects()[i];
        QString markedText = getMarkedText()[i];

        // Calculate starting position for the rectangles
        if(m_Alignment == Center || m_Alignment == Right){
            int sumUntilCurrentIndex = getIndexOfMarkedRect(marked.center());
            int x = getSumOfCoords(letterCoordinates, sumUntilCurrentIndex) + m_FrameLineWidth;
            int width = marked.width();

            marked.setX(x);
            marked.setWidth(width);
        }


        // Invert normal colors
        QColor invForeColor = invertColor(m_ForeColor);
        QColor invBrushColor = invertColor(brush.color());

        painter.setPen(invForeColor);
        painter.setBackground(invBrushColor);
        painter.setBackgroundMode(Qt::OpaqueMode);

        // Draw Text on top of old one
        painter.drawText(marked,Qt::AlignCenter | Qt::AlignVCenter,  markedText);

    }

    painter.setPen(m_ForeColor);
    painter.setBrush(brush);

    painter.restore();

    if (m_FramePresent)
    {
        painter.setPen(QPen(m_FrameColorBottom, m_FrameLineWidth));
        painter.drawLine(QPoint(0, height() - m_FrameLineWidth / 2), QPoint(width(), height() - m_FrameLineWidth / 2));
        painter.drawLine(QPoint(width() - m_FrameLineWidth / 2, m_FrameLineWidth / 2), QPoint(width() - m_FrameLineWidth / 2, height() - m_FrameLineWidth / 2));
        painter.setPen(QPen(m_FrameColorTop, m_FrameLineWidth));
        painter.drawLine(QPoint(0, m_FrameLineWidth / 2), QPoint(width(), m_FrameLineWidth / 2));
        painter.drawLine(QPoint(m_FrameLineWidth / 2, m_FrameLineWidth / 2), QPoint(m_FrameLineWidth / 2, height() - m_FrameLineWidth / 2));
    }

}

bool caLineDraw::event(QEvent *e)
{
    if (e->type() == QEvent::Resize || e->type() == QEvent::Show)
    {
        FontScalingWidget::rescaleFont(m_Text, calculateTextSpace());

        // we try to get the default color for the background set through the external stylesheets
        if (!m_IsShown)
        {
            QString c = palette().color(QPalette::Base).name();
            m_BackColorDefault = QColor(c);
            // printf("default back color %s %s\n", qasc(c), qasc(this->objectName()));
            c = palette().color(QPalette::Text).name();
            m_ForeColorDefault = QColor(c);
            // printf("default fore color %s %s\n", qasc(c), qasc(this->objectName()));
            if (!m_BackColorDefault.isValid())
                m_BackColorDefault = QColor(255, 248, 220, 255);
            if (!m_ForeColorDefault.isValid())
                m_ForeColorDefault = Qt::black;

            setColors(m_BackColor, m_ForeColor, m_FrameColor);
            m_IsShown = true;
        }
    }

    return QWidget::event(e);
}

/**
 * @brief Inverts Color to opposite of the color wheel
 * @param color -> Color to be inverted
 * @return Inverted Color
 */
QColor caLineDraw::invertColor(QColor color){
    return QColor(255 - color.red(), 255 - color.green(), 255 - color.blue());
}


QSize caLineDraw::calculateTextSpace()
{
    d_savedTextSpace = contentsRect().size();
    d_savedTextSpace.setWidth(d_savedTextSpace.width() - 2 * m_FrameLineWidth);
    d_savedTextSpace.setHeight(d_savedTextSpace.height() - 2 * m_FrameLineWidth);
    return d_savedTextSpace;
}


QSize caLineDraw::sizeHint() const
{
    if (!fontScaleEnabled())
    {
        return QWidget::sizeHint();
    }
    QFont f = font();
    f.setPointSize(4);
    QFontMetrics fm(f);
    int w = QMETRIC_QT456_FONT_WIDTH(fm, m_Text);
    int h = fm.height();
    /* add some pixels... */
    w += 4;
    h += 4;
    QSize size(w, h);
    return size;
}

QSize caLineDraw::minimumSizeHint() const
{
    QSize size;
    if (!fontScaleEnabled())
        size = QWidget::minimumSizeHint();
    else
        size = sizeHint();

    return size;
}

void caLineDraw::setText(const QString &txt)
{
    if (m_Text == txt)
        return;
    if (m_Text.size() != txt.size())
    {
        FontScalingWidget::rescaleFont(txt, d_savedTextSpace);
    }
    m_Text = txt;
    update();
}

void caLineDraw::setDatatype(int datatype)
{
    thisDatatype = datatype;
}

void caLineDraw::setFormat(int prec)
{
    int precision = prec;
    if (precision > 17)
        precision = 17;
    if (m_PrecMode == User)
    {
        precision = getPrecision();
    }
    switch (m_FormatType)
    {
    case string:
    case decimal:
        if (precision >= 0)
        {
            sprintf(m_Format, "%s.%dlf", "%", precision);
        }
        else
        {
            sprintf(m_Format, "%s.%dle", "%", -precision);
        }
        break;
    case compact:
        sprintf(m_Format, "%s.%dle", "%", qAbs(precision));
        sprintf(m_FormatC, "%s.%dlf", "%", qAbs(precision));
        break;
    case exponential:
    case engr_notation:
        sprintf(m_Format, "%s.%dle", "%", qAbs(precision));
        break;
    case truncated:
    case enumeric:
        if (thisDatatype == caDOUBLE)
            qstrncpy(m_Format, "%lld", MAX_STRING_LENGTH);
        else
            qstrncpy(m_Format, "%d", MAX_STRING_LENGTH);
        break;
    case utruncated:
        if (thisDatatype == caDOUBLE)
            qstrncpy(m_Format, "%llu", MAX_STRING_LENGTH);
        else
            qstrncpy(m_Format, "%u", MAX_STRING_LENGTH);
        break;
    case hexadecimal:
        if (thisDatatype == caDOUBLE)
            qstrncpy(m_Format, "0x%llx", MAX_STRING_LENGTH);
        else
            qstrncpy(m_Format, "0x%x", MAX_STRING_LENGTH);
        break;
    case octal:
        if (thisDatatype == caDOUBLE)
            qstrncpy(m_Format, "O%llo", MAX_STRING_LENGTH);
        else
            qstrncpy(m_Format, "O%o", MAX_STRING_LENGTH);
        break;
    case sexagesimal:
    case sexagesimal_hms:
    case sexagesimal_dms:
        break;
    case user_defined_format:
    {
        qstrncpy(m_Format, thisFormatUserString.toLatin1().data(), MAX_STRING_LENGTH);
        break;
    }
    }
}

void caLineDraw::setValue(double value, const QString &units)
{
    char asc[MAX_STRING_LENGTH];
    if (m_FormatType == compact)
    {
        if ((value < 1.e4 && value > 1.e-4) || (value > -1.e4 && value < -1.e-4) || value == 0.0)
        {
            snprintf(asc, MAX_STRING_LENGTH, m_FormatC, value);
        }
        else
        {
            snprintf(asc, MAX_STRING_LENGTH, m_Format, value);
        }
    }
    else if (m_FormatType == hexadecimal || m_FormatType == octal || m_FormatType == user_defined_format)
    {
        if (thisDatatype == caDOUBLE)
            snprintf(asc, MAX_STRING_LENGTH, m_Format, (long long)value);
        else
            snprintf(asc, MAX_STRING_LENGTH, m_Format, (int)value);
    }
    else if (m_FormatType == truncated)
    {
        if (thisDatatype == caDOUBLE)
            snprintf(asc, MAX_STRING_LENGTH, m_Format, (long long)value);
        else
            snprintf(asc, MAX_STRING_LENGTH, m_Format, (int)value);
    }
    else if (m_FormatType == enumeric)
    {
        if (thisDatatype == caDOUBLE)
            snprintf(asc, MAX_STRING_LENGTH, m_Format, (long long)value);
        else
            snprintf(asc, MAX_STRING_LENGTH, m_Format, (int)value);
    }
    else if (m_FormatType == utruncated)
    {
        if (thisDatatype == caDOUBLE)
            snprintf(asc, MAX_STRING_LENGTH, m_Format, (unsigned long long)value);
        else
            snprintf(asc, MAX_STRING_LENGTH, m_Format, (uint)value);
    }
    else
    {
        snprintf(asc, MAX_STRING_LENGTH, m_Format, value);
    }

    if (qIsNaN(value))
    {
        snprintf(asc, MAX_STRING_LENGTH, "nan");
    }
    if (m_UnitMode)
    {
        strcat(asc, " ");
        strcat(asc, units.toUtf8().constData());
    }
    setText(asc);
    emit textChanged(QString(asc));
}

// caWidgetInterface implementation
void caLineDraw::caDataUpdate(const QString &units, const QString &String, const knobData &data)
{
    QColor bg = property("BColor").value<QColor>();
    QColor fg = property("FColor").value<QColor>();

    if (data.edata.connected)
    {
        setDatatype(data.edata.fieldtype);
        // enum string
        if (data.edata.fieldtype == caENUM || data.edata.fieldtype == caSTRING || data.edata.fieldtype == caCHAR)
        {
            QStringList list;
            if (m_ColorMode == Static || m_ColorMode == Default)
            { // done at initialisation
                if (!property("Connect").value<bool>())
                { // but was disconnected before
                    setAlarmColors(data.edata.severity, (double)data.edata.ivalue, bg, fg);
                    setProperty("Connect", true);
                }
            }
            else
            {
                setAlarmColors(data.edata.severity, (double)data.edata.ivalue, bg, fg);
            }
            list = String.split((QChar)27);

            if ((data.edata.fieldtype == caENUM) && (list.count() == 0))
            {
                QString str = QString::number((int)data.edata.ivalue);
                setText(str); // no string, but number
            }
            else if ((data.edata.fieldtype == caENUM) && ((int)data.edata.ivalue < list.count()) && (list.count() > 0))
            {
                if (list.at((int)data.edata.ivalue).trimmed().size() == 0)
                { // string seems to empty, give value
                    QString str = QString::number((int)data.edata.ivalue);
                    setText(str);
                }
                else
                { // we have a string, we want as string
                    if (m_FormatType != enumeric)
                    {
                        setText(list.at((int)data.edata.ivalue));
                    }
                    else
                    {
                        QString str = QString::number((int)data.edata.ivalue); // we have  astring, we want as number
                        setText(str);
                    }
                }
            }
            else if ((data.edata.fieldtype == caENUM) && ((int)data.edata.ivalue >= list.count()) && (list.count() > 0))
            {
                QString str = QString::number((int)data.edata.ivalue);
                setText(str);
            }
            else if (data.edata.fieldtype == caENUM)
            {
                setText("???");

                // just one char (display as character when string format is specified, otherwise as number in specified format
            }
            else if ((data.edata.fieldtype == caCHAR) && (data.edata.nelm == 1))
            {
                if (m_FormatType == string)
                {
                    QString str = QString(QChar((int)data.edata.ivalue));
                    setText(str);
                }
                else
                {
                    setValue(data.edata.ivalue, "");
                }

                // one or more strings, or a char array
            }
            else
            {
                if (data.edata.nelm == 1)
                {
                    setText(String);
                }
                else if (list.count() > 0)
                {
                    setText(list.at(0));
                }
            }

            // double
        }
        else
        {

            if (m_ColorMode == Static || m_ColorMode == Default)
            { // done at initialisation
                if (!property("Connect").value<bool>())
                { // but was disconnected before
                    setAlarmColors(data.edata.severity, data.edata.rvalue, bg, fg);
                    setProperty("Connect", true);
                }
            }
            else
            {
                setAlarmColors(data.edata.severity, data.edata.rvalue, bg, fg);
            }

            if ((m_PrecMode != User) && (data.edata.initialize))
            {
                setFormat(data.edata.precision);
            }
            setValue(data.edata.rvalue, units);
        }
    }
    else
    {
        setText("");
        setAlarmColors(NOTCONNECTED, 0.0, bg, fg);
        setProperty("Connect", false);
    }
}

void caLineDraw::caActivate(CaQtDM_Lib_Interface *lib_interface, QMap<QString, QString> map, knobData *kData, int *specData, QWidget *parent)
{
    if (getPV().size() > 0)
    {
        QString pv;
        lib_interface->addMonitor(parent, kData, getPV(), (QWidget *)this, specData, map, &pv);
        setPV(pv);
    }
    setFormat(1);
    setProperty("Taken", true);
    caDataInterface = lib_interface;
}

void caLineDraw::createContextMenu(QMenu &menu)
{
    // construct info for the pv we are pointing at
    menu.addAction("Get Info");
}

void caLineDraw::getWidgetInfo(QString *pv, int &nbPV, int &limitsDefault, int &precMode, int &limitsMode,
                               int &Precision, char *colMode, double &limitsMax, double &limitsMin)
{
    Q_UNUSED(limitsDefault);

    pv[0] = getPV().trimmed();
    nbPV = 0;
    if (pv[0].length()>0){ // only when something is inside the PV it could be something
        nbPV = 1;
    }

    if(getPrecisionMode() == User) {
        precMode = true;
        Precision = getPrecision();
    }
    if (getLimitsMode() == User)
    {
        limitsMode = true;
        limitsMax = getMaxValue();
        limitsMin = getMinValue();
    }
    if (getColorMode() == Alarm_Default)
        strcpy(colMode, "Alarm");
    else if (getColorMode() == Alarm_Static)
        strcpy(colMode, "Alarm");
    else
        strcpy(colMode, "Static");

}
