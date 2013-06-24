#include "epushbutton.h"
#include <QStyleOptionButton>
#include <QStyle>
#include <QtDebug>

EPushButton::EPushButton(const QString &text, QWidget * parent) : QPushButton(text, parent), FontScalingWidget(this)
{
	
}

EPushButton::EPushButton(QWidget * parent) : QPushButton(parent), FontScalingWidget(this)
{
	
}

EPushButton::EPushButton(const QIcon &icon, const QString &text, QWidget *parent) : 
	QPushButton(icon, text, parent), FontScalingWidget(this)
{
	
}

void EPushButton::setText(const QString& text)
{
  QPushButton::setText(text);
  FontScalingWidget::rescaleFont(text, d_savedLabelSize);
}

void EPushButton::resizeEvent(QResizeEvent *e)
{
  QPushButton::resizeEvent(e);
  rescaleFont(text(), calculateTextSpace());
}

void EPushButton::showEvent(QShowEvent *e)
{
  QPushButton::showEvent(e);
  rescaleFont(text(), calculateTextSpace());
}

QSize EPushButton:: sizeHint() const
{
  if(fontScaleEnabled())
  {
	QFont f = font();
    f.setPointSize(10);
	QFontMetrics fm(f);
	int width = fm.width(text());
        width += 8; /* add some extra space */
	return QSize(width, fm.height() + 4);
  }
  return QPushButton::sizeHint();
}
 
QSize  EPushButton::minimumSizeHint() const
{
  if(fontScaleEnabled())
	return EPushButton::sizeHint();
  return QPushButton::minimumSizeHint();
}

QSize EPushButton::calculateTextSpace()
{
  //qDebug() << "EPushButton::calculateTextSpace(): button size " << size();
  QStyleOptionButton button;
  initStyleOption(&button);
  /* Use QStyle subControlRect() to retrieve the area containing the button label.
   * QStyle::SE_PushButtonContents is the area containing the label (icon with text or pixmap).
   * http://doc.trolltech.com/4.4/qstyle.html#SubElement-enum
   */
  d_savedLabelSize = style()->subElementRect(QStyle::SE_PushButtonContents, &button, this).size();
  return d_savedLabelSize;
}

