#ifndef EWIDGET_H
#define EWIDGET_H

#include <QWidget>

#include "fontscalingwidget.h"

/** \brief A class supporting uniform font scaling of its children
 *
 * Use this widget in place of the standard QWidget when you want that the 
 * font scaling qtcontrols widget scale their fonts in an uniform way, that 
 * is, their font scale but have all the same point size.
 * During the EWidget resizeEvent, each child implementing FontScalingWidget
 * is asked for its optimal point size according to the string it represents.
 * EWidget then gathers all the point sizes of the children and sets on each 
 * the minimum font size recollected.
 */
class EWidget : public QWidget
{
//   Q_PROPERTY(FontScalingWidget::ScaleMode childrenScaleMode READ childrenScaleMode WRITE setChildrenScaleMode)
  
  Q_OBJECT
  public:
	EWidget(QWidget *parent = NULL);
	
  protected:
	virtual void resizeEvent(QResizeEvent *);
	
  private:
  
	QFont d_appFont;
};



#endif
