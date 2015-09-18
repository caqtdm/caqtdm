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

#include "fontscalingwidget.h"
#include <QEvent>
#include <QtDebug>

#define qslisttoc(x) 			do {}while(0)

#define FONT_SIZE_TOLERANCE_MARGIN 	3 /* pixel */
#define MIN_FONT_SIZE 			4

FontScalingWidget::FontScalingWidget(QWidget *parent)
{
    Q_ASSERT(parent != NULL);
    d_widget = parent;
    d_scaleMode = 0;
    d_vertical = false;
    d_lateralBorderWidth = 2;
    d_botTopBorderWidth = 2;
    d_fontScaleFactor = 1.0;
    d_savedFont = d_widget->font();
}

void FontScalingWidget::setBotTopBorderWidth(double pixValue) 
{ 
    d_botTopBorderWidth = pixValue;
    if(d_scaleMode == 1 || d_scaleMode == 2) /* update */
        rescaleFont(text(), calculateTextSpace());
}

void FontScalingWidget::setLateralBorderWidth(double pixValue)
{ 
    d_lateralBorderWidth = pixValue;
    if(d_scaleMode == 1 || d_scaleMode == 2) /* update */
        rescaleFont(text(), calculateTextSpace());
}

void FontScalingWidget::setScaleMode(int mode)
{
    d_scaleMode = mode;
    if(mode == 1 || mode == 2) {
        rescaleFont(text(), calculateTextSpace());
    } else {
        //d_widget->setFont(d_savedFont); // no, let the designer do that
    }
}

double FontScalingWidget::calculateFontPointSizeF(const QString& text, const QSize &size)
{
    QFontMetrics fmint = d_widget->fontMetrics();
    QFontMetricsF fm(fmint);
    QFont f = d_widget->font();
    QString longestLine;
    double txtWidth;
    double txtHeight;
    int linecnt = text.count("\n") + 1;

    if(linecnt > 1) {
        QStringList lines = text.split("\n");
        qSort(lines.begin(), lines.end(), FontScalingWidget::longerThan);
        qslisttoc(lines);
        longestLine = lines.first();
        txtHeight = fm.lineSpacing() * linecnt;
    } else {
        longestLine = text; /* no newline */
        txtHeight = fm.height();
    }

    double borderH1 = size.height() - d_botTopBorderWidth;
    double borderH2;

     /* scale according to width and height */
    if(d_scaleMode == 2) {
        double borderW1 = size.width() - d_lateralBorderWidth;
        double borderW2 = borderW1 - d_lateralBorderWidth;

         /* first scale according to height (same algorithme as below) and then verify width */
        borderH2 = borderH1;
        if(txtHeight == (borderH1 + 1) || txtHeight == (borderH1)) {
            //printf("good: text for <%s> :text \"%s\" %.2f\e[0m | borderH1: %.2f borderH2: %.2f pointSizeF %.2f, h: %.2f\n",
            //widget()->objectName().toLatin1().constData(), text.toLatin1().constData(), txtHeight, borderH1,borderH2, f.pointSizeF(), borderH1 );

        } else {
            while((txtHeight > borderH1) && f.pointSizeF() > MIN_FONT_SIZE) {
                if(f.pointSizeF() <= 0.0) f.setPointSizeF(1.0);
                f.setPointSizeF(f.pointSizeF() - 0.5);
                //printf(" \e[1;36m -- DECREASING font size for object \"%s\" :text \"%s\"  height %.1f - point size %.2f - h: %.2f\e[0m\n",
                //         d_widget->objectName().toLatin1().constData(), text.toLatin1().constData(),  txtHeight, f.pointSizeF(), borderH1);
                QFontMetricsF tmpFm(f);
                txtHeight = linecnt * tmpFm.lineSpacing();
            }

            while(txtHeight < borderH2) {
                if(f.pointSizeF() <= 0.0) f.setPointSizeF(0.5);
                f.setPointSizeF(f.pointSizeF() + 0.5);
                //printf(" \e[1;35m ++ INCREASING font size for object\"%s\" :text \"%s\" height %.1f - point size %.2f - h: %.2f\e[0m\n",
                //         d_widget->objectName().toLatin1().constData(), text.toLatin1().constData(), txtHeight, f.pointSizeF(), borderH2);
                QFontMetricsF tmpFm(f);
                txtHeight = linecnt * tmpFm.lineSpacing();
            }
        }

        // check if width does not go outside
        QFontMetricsF tmpFm(f);
        txtWidth = tmpFm.width(longestLine);
        while((txtWidth > borderW2) && f.pointSizeF() > MIN_FONT_SIZE) {
            if(f.pointSizeF() <= 0.0) f.setPointSizeF(1.0);
            f.setPointSizeF(f.pointSizeF() - 0.5);
            //printf(" \e[1;36m -- next DECREASING font size \"%s\" :text \"%s\" width %.1f height %.1f - point size %.2f - w: %.2f\e[0m\n",
            //         d_widget->objectName().toLatin1().constData(), text.toLatin1().constData(),  txtWidth, txtHeight, f.pointSizeF(), borderW2);
            QFontMetricsF tmpFm(f);
            txtWidth = tmpFm.width(longestLine);
            //txtHeight = linecnt * tmpFm.lineSpacing();
        }

    /* scale according to height only */
    } else {

        borderH2 = borderH1;
        if(txtHeight == (borderH1 + 1) || txtHeight == (borderH1)) {
        //if(txtHeight == borderH1) {
            //printf("good: text h %.2f\e[0m | borderH1: %.2f borderH2: %.2f pointSizeF %.2f, h: %.2f\n",
            //   txtHeight, borderH1,borderH2, f.pointSizeF(), borderH1 );

        } else {
            while((txtHeight > borderH1) && f.pointSizeF() > MIN_FONT_SIZE) {
                if(f.pointSizeF() <= 0.0) f.setPointSizeF(1.0);
                f.setPointSizeF(f.pointSizeF() - 0.5);
                //printf(" \e[1;36m -- DECREASING font size \"%s\" :text \"%s\"  height %.1f - point size %.2f - h: %.2f\e[0m\n",
                //         widget()->objectName().toLatin1().constData(), text.toLatin1().constData(),  txtHeight, f.pointSizeF(), borderH1);
                QFontMetricsF tmpFm(f);
                txtHeight = linecnt * tmpFm.lineSpacing();
            }

            while(txtHeight < borderH2) {
                if(f.pointSizeF() <= 0.0) f.setPointSizeF(0.5);
                f.setPointSizeF(f.pointSizeF() + 0.5);
                //printf(" \e[1;35m ++ INCREASING font size \"%s\" :text \"%s\" height %.1f - point size %.2f - h: %.2f\e[0m\n",
                //         widget()->objectName().toLatin1().constData(), text.toLatin1().constData(), txtHeight, f.pointSizeF(), borderH2);
                QFontMetricsF tmpFm(f);
                txtHeight = linecnt * tmpFm.lineSpacing();
            }
        }

    }
    return f.pointSizeF();
}

double FontScalingWidget::calculateVertFontPointSizeF(const QString& text, const QSize &size)
{
    QFontMetrics fmint = d_widget->fontMetrics();
    QFontMetricsF fm(fmint);
    QFont f = d_widget->font();
    QString longestLine;
    double txtWidth;
    double txtHeight;
    int linecnt = text.count("\n") + 1;

    if(linecnt > 1) {
        QStringList lines = text.split("\n");
        qSort(lines.begin(), lines.end(), FontScalingWidget::longerThan);
        qslisttoc(lines);
        longestLine = lines.first();
        txtHeight = fm.lineSpacing() * linecnt;
    } else {
        longestLine = text; /* no newline */
        txtHeight = fm.height();
    }

    double borderH1 = size.height() - d_botTopBorderWidth;
    double borderH2 = borderH1;
    double borderW1 = size.width() - d_lateralBorderWidth;
    double borderW2 = borderW1 - d_lateralBorderWidth;
     /* scale according to width and height */
    if(d_scaleMode == 2) {

         /* first scale according to height (same algorithme as below) and then verify width */
        if(txtHeight == (borderW1 + 1) || txtHeight == (borderW1)) {
            //printf("good: text for <%s> :text \"%s\" %.2f\e[0m | borderH1: %.2f borderH2: %.2f pointSizeF %.2f, h: %.2f\n",
            //widget()->objectName().toLatin1().constData(), text.toLatin1().constData(), txtHeight, borderH1,borderH2, f.pointSizeF(), borderH1 );

        } else {
            while((txtHeight > borderW1) && f.pointSizeF() > MIN_FONT_SIZE) {
                if(f.pointSizeF() <= 0.0) f.setPointSizeF(1.0);
                f.setPointSizeF(f.pointSizeF() - 0.5);
                //printf(" \e[1;36m -- DECREASING font size for object \"%s\" :text \"%s\"  height %.1f - point size %.2f - h: %.2f\e[0m\n",
                //         d_widget->objectName().toLatin1().constData(), text.toLatin1().constData(),  txtHeight, f.pointSizeF(), borderH1);
                QFontMetricsF tmpFm(f);
                txtHeight = linecnt * tmpFm.lineSpacing();
            }

            while(txtHeight < borderW2) {
                if(f.pointSizeF() <= 0.0) f.setPointSizeF(0.5);
                f.setPointSizeF(f.pointSizeF() + 0.5);
                //printf(" \e[1;35m ++ INCREASING font size for object\"%s\" :text \"%s\" height %.1f - point size %.2f - h: %.2f\e[0m\n",
                //         d_widget->objectName().toLatin1().constData(), text.toLatin1().constData(), txtHeight, f.pointSizeF(), borderH2);
                QFontMetricsF tmpFm(f);
                txtHeight = linecnt * tmpFm.lineSpacing();
            }
        }

        // check if width does not go outside
        QFontMetricsF tmpFm(f);
        txtWidth = tmpFm.width(longestLine);
        while((txtWidth > borderH2) && f.pointSizeF() > MIN_FONT_SIZE) {
            if(f.pointSizeF() <= 0.0) f.setPointSizeF(1.0);
            f.setPointSizeF(f.pointSizeF() - 0.5);
            //printf(" \e[1;36m -- next DECREASING font size \"%s\" :text \"%s\" width %.1f height %.1f - point size %.2f - w: %.2f\e[0m\n",
            //         d_widget->objectName().toLatin1().constData(), text.toLatin1().constData(),  txtWidth, txtHeight, f.pointSizeF(), borderW2);
            QFontMetricsF tmpFm(f);
            txtWidth = tmpFm.width(longestLine);
            //txtHeight = linecnt * tmpFm.lineSpacing();
        }

    /* scale according to height only */
    } else {

        if(txtHeight == (borderW1 + 1) || txtHeight == (borderW1)) {
            //printf("good: text h %.2f\e[0m | borderH1: %.2f borderH2: %.2f pointSizeF %.2f, h: %.2f\n",
            //   txtHeight, borderH1,borderH2, f.pointSizeF(), borderH1 );

        } else {
            while((txtHeight > borderW1) && f.pointSizeF() > MIN_FONT_SIZE) {
                if(f.pointSizeF() <= 0.0) f.setPointSizeF(1.0);
                f.setPointSizeF(f.pointSizeF() - 0.5);
                //printf(" \e[1;36m -- DECREASING font size \"%s\" :text \"%s\"  height %.1f - point size %.2f - h: %.2f\e[0m\n",
                //         widget()->objectName().toLatin1().constData(), text.toLatin1().constData(),  txtHeight, f.pointSizeF(), borderH1);
                QFontMetricsF tmpFm(f);
                txtHeight = linecnt * tmpFm.lineSpacing();
            }

            while(txtHeight < borderW2) {
                if(f.pointSizeF() <= 0.0) f.setPointSizeF(0.5);
                f.setPointSizeF(f.pointSizeF() + 0.5);
                //printf(" \e[1;35m ++ INCREASING font size \"%s\" :text \"%s\" height %.1f - point size %.2f - h: %.2f\e[0m\n",
                //         widget()->objectName().toLatin1().constData(), text.toLatin1().constData(), txtHeight, f.pointSizeF(), borderH2);
                QFontMetricsF tmpFm(f);
                txtHeight = linecnt * tmpFm.lineSpacing();
            }
        }

    }
    return f.pointSizeF();
}

void FontScalingWidget::rescaleFont(const QString& text, const QSize &size)
{
    double fontSize;
    if((d_scaleMode != 1 && d_scaleMode != 2)  || size.width() < 8  || size.height() < 4) return;
    if(d_vertical) {
        fontSize = calculateVertFontPointSizeF(text, size);
    } else {
        fontSize = calculateFontPointSizeF(text, size);
    }
    if(fontSize < MIN_FONT_SIZE) fontSize = MIN_FONT_SIZE;
    QFont f = d_widget->font();
    f.setPointSizeF(fontSize);
    d_widget->setFont(f);
}





