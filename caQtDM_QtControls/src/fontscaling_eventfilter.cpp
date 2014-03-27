#include "fontscaling_eventfilter.h"
#include "simplefontscalingwidget.h"

#include <QResizeEvent>
#include <QtDebug>

FontScalingEventFilter::FontScalingEventFilter(SimpleFontScalingWidget *scalingWidget)  : QObject()
{
  d_scalingWidget = scalingWidget;
//   d_previousFontAdjustment = false;
}

bool FontScalingEventFilter::eventFilter(QObject *obj, QEvent *event)
{
  if(event->type() == QEvent::Resize)
  {
	QResizeEvent *re = static_cast<QResizeEvent *>(event);
	qDebug() << "resize event for " << obj << "old size " << re->oldSize() << "new size " << re->size();
	qDebug() << "spontaneous " << event->spontaneous();
	d_scalingWidget->rescaleFont(re->size());
  }
  else if(event->type() == QEvent::Show)
  {
	d_scalingWidget->rescaleFont();
  }
  return false;
}

