#include <QResizeEvent>
#include <QMap>
#include "ewidget.h"
#include "fontscalingwidget.h"
#include <QApplication>
#define qstoc(x) ""

EWidget::EWidget(QWidget *parent) : QWidget(parent)
{
  d_appFont = QApplication::font();
}

void EWidget::resizeEvent(QResizeEvent *e)
{
  QList<double> pointSizes;
  QSize oldSize = e->oldSize();
  QSize size = e->size();
  QMap<QWidget *, double> scalingFactorForWidgetMap;
  double wRatio = (double) e->size().width() / (double)e->oldSize().width();
  double hRatio = (double) e->size().height() / (double)e->oldSize().height();
  double fontScaleFactor;
  
  foreach(QWidget *w, findChildren<QWidget *>())
  {
	FontScalingWidget *fsw = dynamic_cast<FontScalingWidget *>(w);
	if(fsw && (fsw->scaleMode() == 3  || fsw->scaleMode() == 4))
	{
          printf("app font point size: %.1f widget \"%s\" point size %.1f\n", d_appFont.pointSizeF(), qstoc(w->objectName()),
		w->font().pointSizeF());
	  fontScaleFactor = fsw->fontScaleFactor();
	  QSizeF widgetSize = QSizeF(fsw->calculateTextSpace());
	  widgetSize.scale(wRatio * w->width(), hRatio * w->height(), Qt::IgnoreAspectRatio);
	  double pointSize = fsw->calculateFontPointSizeF("", widgetSize.toSize());
	  pointSize /= fontScaleFactor;
	  pointSizes << pointSize;
	  scalingFactorForWidgetMap.insert(w, fontScaleFactor);
          printf("- point size optimal for \"%s\" : %.1f\n", qstoc(w->objectName()), pointSizes.last());
	}
  }
  if(pointSizes.size())
  {
	qSort(pointSizes.begin(), pointSizes.end());
	foreach(QWidget *w, findChildren<QWidget *>())
	{
	  FontScalingWidget *fsw = dynamic_cast<FontScalingWidget *>(w);
	  if(fsw && (fsw->scaleMode() == 3 || fsw->scaleMode() == 4))
	  {
		QFont font = w->font();
		fontScaleFactor = scalingFactorForWidgetMap.value(w);
		font.setPointSizeF(pointSizes.first() * fontScaleFactor);
                printf("setting minimal common font on \"%s\": %.1f (factor %.1f)\n", qstoc(w->objectName()), font.pointSizeF(), fontScaleFactor);
		w->setFont(font);
	  }
	}
  }
  QWidget::resizeEvent(e);
}

