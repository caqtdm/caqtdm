#ifndef SIMPLE_FONTSCALING_WIDGET_H
#define SIMPLE_FONTSCALING_WIDGET_H

#include "fontscalingwidget.h"

class FontScalingEventFilter;

/** \brief this class provides Resize and Show event interception to automatically resize fonts
 *         of the QWidget that subclasses SimpleFontScalingWidget.
 *
 * This class can be used by widgets whose available size for drawing text strings is equal to the
 * size returned by the QWidget::size() method.
 * For instance, QLabels, QLineEdits can use this class, while SpinBoxes cannot, since the text area
 * available for the text on spin box is not equivalent to the total widget area returned bu size().
 * Actually, part of the area od a spin box is occupied by the arrows. 
 */
class SimpleFontScalingWidget : public FontScalingWidget
{
  public:
	SimpleFontScalingWidget(QWidget *parent);
	
	virtual void setFontScaleEnabled(bool en);
	
	/** \brief must be reimplemented in subclasses to provide the text displayed.
	 * 
	 * Used in the show and resize events to obtain the string displayed by the widget.
	 */
	virtual QString currentText() const = 0;
	
	/** \brief gets the text string via currentText() and then calls FontScalingWidget::rescaleFont
	 *         with the provided size.
	 */
	virtual void rescaleFont(const QSize &containerSize);
	
	/** \brief gets the text string via currentText() and the size from FontScalingWidget::widget() and
	 *         calls FontScalingWidget::rescaleFont()
	 */
	virtual void rescaleFont();
	
	virtual QSize calculateTextSpace();
	
  private:
	FontScalingEventFilter *d_eventFilter;
};


#endif
