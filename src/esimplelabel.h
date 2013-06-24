#ifndef ESIMPLELABEL_H
#define ESIMPLELABEL_H

#include <QLabel>
#include <QStyleOptionFrame>
#include <QStyle>
#include <QtDebug>

#include <qtcontrols_global.h>
#include "fontscalingwidget.h"

/** \brief A QLabel that provides the font scaling feature.
 *
 * This class is a QLabel that scales its fonts when resized.
 * This is a base font scaling class for all the objects that need to implement the 
 * font scaling features, such as ELabel.
 * You might have a look at those classes.
 * If your label is <em>left</em> or <em>right</em> aligned, then consider increasing the
 * lateralBorderWidth if you experience some problems of font scaling: an ESimpleLabel with
 * a thick <em>boxed frame</em> might have its string not perfectly fit the space in some circumstances.
 * <br/>Font scaling <em>labels</em> are tailored for <em>center aligned</em>
 * text only. The ESimpleLabel is constructed with a horizontal centered alignment if scalingMode
 * property is different from ESimpleLabel::None, and a right alignment otherwise.
 * 
 * @see ELabel
 * @see EFlag
 * @see FontScalingWidget
 *
 */
class QTCON_EXPORT ESimpleLabel : public QLabel, public FontScalingWidget
{
  /* scalable fonts */
  Q_PROPERTY(bool fontScaleEnabled READ fontScaleEnabled DESIGNABLE false)
  Q_PROPERTY(double botTopBorderWidth READ botTopBorderWidth WRITE setBotTopBorderWidth DESIGNABLE fontScaleEnabled)
  Q_PROPERTY(double lateralBorderWidth READ lateralBorderWidth WRITE setLateralBorderWidth DESIGNABLE fontScaleEnabled)
  Q_PROPERTY(double fontScaleFactor READ fontScaleFactor WRITE setFontScaleFactor  DESIGNABLE false)
  Q_PROPERTY(ScaleMode fontScaleMode READ fontScaleMode WRITE setFontScaleMode)
  
  Q_ENUMS(ScaleMode)
  Q_OBJECT
  public:
	
        enum ScaleMode { None, Height, WidthAndHeight};
	
	ESimpleLabel(QWidget *parent);
	
	ESimpleLabel(const QString& text, QWidget *parent);
	
	virtual void setText(const QString&);
	
    QString text() const {return QLabel::text(); }
	
	QSize calculateTextSpace();
	
	void setFontScaleMode(ScaleMode m) { FontScalingWidget::setScaleMode((int) m); }
        ScaleMode fontScaleMode() { return (ScaleMode) FontScalingWidget::scaleMode();  }
	
  protected:
	virtual bool event(QEvent *);
	
	virtual QSize sizeHint() const;
	
	virtual QSize minimumSizeHint() const;
};


#endif
