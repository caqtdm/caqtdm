#include "fontscalingwidget.h"
#include <QEvent>
#include <QtDebug>

#define qslisttoc(x) 			do {}while(0)

#define FONT_SIZE_TOLERANCE_MARGIN 	3 /* pixel */
#define MIN_FONT_SIZE 			6

FontScalingWidget::FontScalingWidget(QWidget *parent)
{
    Q_ASSERT(parent != NULL);
    d_widget = parent;
#ifdef SCALABLE_FONTS_ENABLED_BY_DEFAULT
    d_scaleMode = 1;
#else
    d_scaleMode = 0;
#endif
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
    double borderH2 = borderH1 - d_botTopBorderWidth;

     /* scale according to width and height */
    if(d_scaleMode == 2) {

        txtWidth = fm.width(longestLine);
        double borderW1 = size.width() - d_lateralBorderWidth;
        double borderW2 = borderW1 - d_lateralBorderWidth;

/* original algorithme; personally I prefer to scale according to height and then verify if width is ok
        if( (txtWidth < borderW1 && txtWidth > borderW2) && (txtHeight < borderH1 && txtHeight > borderH2)) {
            printf("\e[1;32m good: <%s> text w %.2f h %.2f\e[0m | borderW1: %.2f borderW2: %.2f borderH1: %.2f borderH2: %.2f\n",
                            text.toAscii().constData(), txtWidth, txtHeight, borderW1, borderW2, borderH1,borderH2 );

        } else {

            while(txtWidth < borderW2 && txtHeight < borderH2) {
                f.setPointSizeF(f.pointSizeF() + 0.5);
                printf(" \e[1;35m ++ INCREASING font size \"%s\" :text \"%s\" width %.1f height %.1f - point size %.2f \e[0m\n",
                         widget()->objectName().toAscii().constData(), text.toAscii().constData(),  txtWidth, txtHeight, f.pointSizeF());
                QFontMetricsF tmpFm(f);
                txtWidth = tmpFm.width(longestLine);
                txtHeight = linecnt * tmpFm.lineSpacing();
            }

            while((txtWidth >= borderW1 || txtHeight >= borderH1) && f.pointSizeF() > MIN_FONT_SIZE) {
                f.setPointSizeF(f.pointSizeF() - 0.5);
                printf(" \e[1;36m -- DECREASING font size \"%s\" :text \"%s\" width %.1f height %.1f - point size %.2f \e[0m\n",
                         widget()->objectName().toAscii().constData(), text.toAscii().constData(),  txtWidth, txtHeight, f.pointSizeF());
                QFontMetricsF tmpFm(f);
                txtWidth = tmpFm.width(longestLine);
                txtHeight = linecnt * tmpFm.lineSpacing();
            }
        }
*/

         /* first scale according to height (same algorithme as below) and then verify width */
        borderH2 = borderH1;
        if(txtHeight == (borderH1 + 1) || txtHeight == (borderH1)) {
            //printf("good: text for <%s> :text \"%s\" %.2f\e[0m | borderH1: %.2f borderH2: %.2f pointSizeF %.2f, h: %.2f\n",
            //widget()->objectName().toAscii().constData(), text.toAscii().constData(), txtHeight, borderH1,borderH2, f.pointSizeF(), borderH1 );

        } else {
            while((txtHeight > borderH1) && f.pointSizeF() > MIN_FONT_SIZE) {
                f.setPointSizeF(f.pointSizeF() - 0.5);
                //printf(" \e[1;36m -- DECREASING font size for object \"%s\" :text \"%s\"  height %.1f - point size %.2f - h: %.2f\e[0m\n",
                //         widget()->objectName().toAscii().constData(), text.toAscii().constData(),  txtHeight, f.pointSizeF(), borderH1);
                QFontMetricsF tmpFm(f);
                txtHeight = linecnt * tmpFm.lineSpacing();
            }

            while(txtHeight < borderH2) {
                f.setPointSizeF(f.pointSizeF() + 0.5);
                //printf(" \e[1;35m ++ INCREASING font size for object\"%s\" :text \"%s\" height %.1f - point size %.2f - h: %.2f\e[0m\n",
                //         widget()->objectName().toAscii().constData(), text.toAscii().constData(), txtHeight, f.pointSizeF(), borderH2);
                QFontMetricsF tmpFm(f);
                txtHeight = linecnt * tmpFm.lineSpacing();
            }
        }

        // check if width does not go outside
        QFontMetricsF tmpFm(f);
        txtWidth = tmpFm.width(longestLine);
        while((txtWidth > borderW2) && f.pointSizeF() > MIN_FONT_SIZE) {
            f.setPointSizeF(f.pointSizeF() - 0.5);
            //printf(" \e[1;36m -- next DECREASING font size \"%s\" :text \"%s\" width %.1f height %.1f - point size %.2f - w: %.2f\e[0m\n",
            //         widget()->objectName().toAscii().constData(), text.toAscii().constData(),  txtWidth, txtHeight, f.pointSizeF(), borderW2);
            QFontMetricsF tmpFm(f);
            txtWidth = tmpFm.width(longestLine);
            txtHeight = linecnt * tmpFm.lineSpacing();
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
                f.setPointSizeF(f.pointSizeF() - 0.5);
                //printf(" \e[1;36m -- DECREASING font size \"%s\" :text \"%s\"  height %.1f - point size %.2f - h: %.2f\e[0m\n",
                //         widget()->objectName().toAscii().constData(), text.toAscii().constData(),  txtHeight, f.pointSizeF(), borderH1);
                QFontMetricsF tmpFm(f);
                txtHeight = linecnt * tmpFm.lineSpacing();
            }

            while(txtHeight < borderH2) {
                f.setPointSizeF(f.pointSizeF() + 0.5);
                //printf(" \e[1;35m ++ INCREASING font size \"%s\" :text \"%s\" height %.1f - point size %.2f - h: %.2f\e[0m\n",
                //         widget()->objectName().toAscii().constData(), text.toAscii().constData(), txtHeight, f.pointSizeF(), borderH2);
                QFontMetricsF tmpFm(f);
                txtHeight = linecnt * tmpFm.lineSpacing();
            }
        }

    }
    return f.pointSizeF();
}

void FontScalingWidget::rescaleFont(const QString& text, const QSize &size)
{
    if((d_scaleMode != 1 && d_scaleMode != 2)  || size.width() < 8  || size.height() < 4) return;
    double fontSize = calculateFontPointSizeF(text, size);
    QFont f = d_widget->font();
    f.setPointSizeF(fontSize);
    d_widget->setFont(f);
}





