#ifndef ESPINBOXES_H
#define ESPINBOXES_H

#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QResizeEvent>
#include "fontscalingwidget.h"

/** \brief helper class that calculates the size hints for the ESpinBox and EDoubleSpinBox
 *
 */
class ECommonSpinBox
{
  protected:
	ECommonSpinBox(QWidget *parent);
	
  protected:
	
	QSize d_sizeHint(const QString& text) const;
	
	QSize d_minimumSizeHint(const QString& text) const;
	
  private:
	QWidget *d_spin;
	
};

class ESpinBox : public QSpinBox, public FontScalingWidget, protected ECommonSpinBox
{
  Q_OBJECT
  Q_ENUMS(ScaleMode)
  /* scalable fonts */
  Q_PROPERTY(ScaleMode fontScaleMode READ fontScaleMode WRITE setFontScaleMode)
  
  public :
	ESpinBox(QWidget *parent);
	
	/** \brief the font scaling modality
	 *
	 * Please refer to the ESimpleLabel::ScaleMode documentation.
	 */
	enum ScaleMode { None, Height, WidthAndHeight, HeightManaged, WidthAndHeightManaged };
	
    void setFontScaleMode(ScaleMode m) { FontScalingWidget::setScaleMode((int) m); }
    ScaleMode fontScaleMode() { return (ScaleMode) d_scaleMode; }
	
	QString text() const { return QSpinBox::text(); }
	
  protected:
	virtual bool event(QEvent *e);
	
	/** \brief provides sizeHint for the widgets inserted into a layout
	 *
	 * The size hint is used by the layout system to resize the widgets according to the 
	 * resize policy. The implementation of size hint returns a size whose width is wide
	 * enough to contain the current text() with the current font with size imposed to 10.
	 * If font scaling is disabled, returns the super class sizeHint.
	 */
	virtual QSize sizeHint() const;
	
	/** \brief returns the minimumSizeHint.
	 *
	 * The implementation returns sizeHint() if font scaling is enabled, super class 
	 * minimumSizeHint() if disabled.
	 */
	virtual QSize minimumSizeHint() const;
	
	QSize calculateTextSpace();
	
  private slots:
	void textChanged(const QString&);
};

class EDoubleSpinBox : public QDoubleSpinBox, public FontScalingWidget, protected ECommonSpinBox
{
  Q_OBJECT
  Q_ENUMS(ScaleMode)
  /* scalable fonts */
  Q_PROPERTY(ScaleMode scaleMode READ scaleMode WRITE setScaleMode)
  public :
	EDoubleSpinBox(QWidget *parent);
	
	/** \brief the font scaling modality
	 *
	 * Please refer to the ESimpleLabel::ScaleMode documentation.
	 */
	enum ScaleMode { None, Height, WidthAndHeight, HeightManaged, WidthAndHeightManaged };
	
    void setScaleMode(ScaleMode m) { FontScalingWidget::setScaleMode((int) m); }
    ScaleMode scaleMode() { return (ScaleMode) d_scaleMode; }
	QString text() const { return QDoubleSpinBox::text(); }
	
  protected:
	virtual bool event(QEvent *e);
	
	/** \brief provides sizeHint for the widgets inserted into a layout
	 *
	 * The size hint is used by the layout system to resize the widgets according to the 
	 * resize policy. The implementation of size hint returns a size whose width is wide
	 * enough to contain the current text() with the current font with size imposed to 10.
	 * If font scaling is disabled, returns the super class sizeHint.
	 */
	virtual QSize sizeHint() const;
	
	/** \brief returns the minimumSizeHint.
	 *
	 * The implementation returns sizeHint() if font scaling is enabled, super class 
	 * minimumSizeHint() if disabled.
	 */
	virtual QSize minimumSizeHint() const;
	
	QSize calculateTextSpace();
	
  private slots:
	void textChanged(const QString&);
};




#endif
