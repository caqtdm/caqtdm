#include "espinboxes.h"
#include <QLineEdit>
#include <QStyleOptionFrame>
#include <QStyle>
#include <QtDebug>

ECommonSpinBox::ECommonSpinBox(QWidget *parent)
{
  d_spin = parent;
}

/* since we have scalable fonts, we do not worry too much about sizeHint. 
 * Inside a layout spin box can shrink up to a very small size. However,
 * fix a size hint equal to the size requested by the text() if the font is
 * 10 + a reasonable width for the spin button. For the height, take 26, inspired 
 * by QDoubleSpinBox default size.
 */
QSize ECommonSpinBox::d_sizeHint(const QString& text) const
{
  QFont f = d_spin->font();
  f.setPointSize(10);
  QFontMetrics fm(f);
  int w = fm.width(text);
  w += 10; /* extra space for spin buttons */
  return QSize(w, 26); 
}

QSize ECommonSpinBox::d_minimumSizeHint(const QString& text) const
{
	return d_sizeHint(text);
}

ESpinBox::ESpinBox(QWidget *parent) : QSpinBox(parent), FontScalingWidget(this), ECommonSpinBox(this)
{
  connect(this, SIGNAL(valueChanged(const QString&)),this, SLOT(textChanged(const QString&)));
}

bool ESpinBox::event(QEvent *e)
{
   /* first process QDoubleSpinBox event (resize, show.. ) */
  bool ret = QSpinBox::event(e);
  /* then rescaleFont if type is Resize or Show */
  if(e->type() == QEvent::Resize || e->type() == QEvent::Show)
	rescaleFont(text(), calculateTextSpace()); /* calculateTextSpace() saves size */
  return ret;
}

void ESpinBox::textChanged(const QString& s)
{
  rescaleFont(s, d_savedTextSpace);
}

QSize ESpinBox::calculateTextSpace()
{
  d_savedTextSpace = lineEdit()->size();
  return d_savedTextSpace;
}


QSize ESpinBox::sizeHint() const
{
  if(!fontScaleEnabled())
	return QSpinBox::sizeHint();
  return d_sizeHint(text());
}

QSize ESpinBox::minimumSizeHint() const
{
  if(!fontScaleEnabled())
	return QSpinBox::minimumSizeHint();
  return d_minimumSizeHint(text());
}


EDoubleSpinBox::EDoubleSpinBox(QWidget *parent) : QDoubleSpinBox(parent), FontScalingWidget(this), ECommonSpinBox(this)
{
  connect(this, SIGNAL(valueChanged(const QString&)),this, SLOT(textChanged(const QString&)));
}

bool EDoubleSpinBox::event(QEvent *e)
{
  /* first process QDoubleSpinBox event (resize, show.. ) */
  bool ret = QDoubleSpinBox::event(e);
  /* then rescaleFont if type is Resize or Show */
  if(e->type() == QEvent::Resize || e->type() == QEvent::Show)
	rescaleFont(text(), calculateTextSpace());
  return ret;
}

void EDoubleSpinBox::textChanged(const QString& s)
{
  rescaleFont(s,  d_savedTextSpace);
}

QSize EDoubleSpinBox::calculateTextSpace()
{
  d_savedTextSpace = lineEdit()->size();
  return d_savedTextSpace;
}

QSize EDoubleSpinBox::sizeHint() const
{
  if(!fontScaleEnabled())
	return QDoubleSpinBox::sizeHint();
  return d_sizeHint(text());
}

QSize EDoubleSpinBox::minimumSizeHint() const
{
  if(!fontScaleEnabled())
	return QDoubleSpinBox::minimumSizeHint();
  return d_minimumSizeHint(text());
}

