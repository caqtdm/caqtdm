#include <qlayout.h>
#include <qsizepolicy.h>
#include <qpixmap.h>
#include "splashscreen.h"
#include <QStyleOptionProgressBarV2>

SplashScreen::SplashScreen(QWidget *parent) : QSplashScreen(parent), m_progress(0)

{
    m_maximum = 100;
    pixmap.load(":caQtDM.png");
    this->setPixmap(pixmap);
    this->setCursor(Qt::BusyCursor);
    this->showMessage("loading include ui files", Qt::AlignBottom, QColor("white"));
    this->resize(pixmap.size().width()+200, pixmap.size().height()+100);
}

void SplashScreen::setMaximum(int max)
{
    m_maximum = max;
}

void SplashScreen::drawContents(QPainter *painter)
    {
      QSplashScreen::drawContents(painter);
      QStyleOptionProgressBarV2 pbstyle;
      pbstyle.initFrom(this);
      pbstyle.state = QStyle::State_Enabled;
      pbstyle.textVisible = false;
      pbstyle.minimum = 0;
      pbstyle.maximum = m_maximum;
      pbstyle.progress = m_progress;
      pbstyle.invertedAppearance = false;
      pbstyle.text = "loading";
      pbstyle.textVisible = true;
      pbstyle.rect = QRect(0, pixmap.size().height()+50, pixmap.size().width()+200, 25);
      style()->drawControl(QStyle::CE_ProgressBar, &pbstyle, painter, this);

}
