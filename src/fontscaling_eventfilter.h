#ifndef FONTSCALING_EVENT_FILTER_H
#define FONTSCALING_EVENT_FILTER_H

#include <QObject>

class SimpleFontScalingWidget;

class FontScalingEventFilter : public QObject
{
  Q_OBJECT
  public:
	FontScalingEventFilter(SimpleFontScalingWidget *scalingWidget);
	
  protected:
	virtual bool eventFilter(QObject *obj, QEvent *event);
	
  private:
	SimpleFontScalingWidget* d_scalingWidget;
};

#endif
