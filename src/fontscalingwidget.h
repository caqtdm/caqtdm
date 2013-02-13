#ifndef FONTSCALING_WIDGET_H
#define FONTSCALING_WIDGET_H

#include <QWidget>

class FontScalingWidget
{ 
  public:
	
	FontScalingWidget(QWidget *parent);
	
        virtual ~FontScalingWidget() {}
	
	void setLateralBorderWidth(double pixValue);
	
	double lateralBorderWidth() { return d_lateralBorderWidth; }
	
	double fontScaleFactor() { return d_fontScaleFactor; }
	
	void setFontScaleFactor(double factor) { d_fontScaleFactor = factor; }
	
	bool fontScaleEnabled() const { return d_scaleMode != 0; }
	
	void setBotTopBorderWidth(double pixValue);
	double botTopBorderWidth() { return d_botTopBorderWidth; }
	
	void saveTextSpace(const QSize& size) { d_savedTextSpace = size; }
	QSize savedTextSpace() { return d_savedTextSpace; }
	
	QFont& savedFont() { return d_savedFont; }

	QWidget *widget() { return d_widget; }
	
	double calculateFontPointSizeF(const QString& text, const QSize & size);
	
	virtual QSize calculateTextSpace() = 0;
	
	virtual QString text() const = 0;
	
	int scaleMode() { return d_scaleMode; }
	/* passed by subclasses as enum from 0 (None) to 4 */
	void setScaleMode(int m);
	
  protected:
	
	void rescaleFont(const QString& text, const QSize &);
	
	/* let this accessible by subclasses to improve performance */
	QSize d_savedTextSpace;
	
	int d_scaleMode;
	
  private:
	double d_lateralBorderWidth;
	double d_botTopBorderWidth;
	double d_fontScaleFactor;
	
	QWidget *d_widget;
	
	/* the font saved before setFontScaleEnabled() was called with a true parameter,
	 * to be able to restore it when setFontScaleEnabled() is called again with false.
	 */
	QFont d_savedFont;
	
	static bool longerThan(const QString& s1, const QString& s2) { return s1.length() > s2.length(); }
};

#endif
