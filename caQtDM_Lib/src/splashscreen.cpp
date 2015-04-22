/*
 *  This file is part of the caQtDM Framework, developed at the Paul Scherrer Institut,
 *  Villigen, Switzerland
 *
 *  The caQtDM Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The caQtDM Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the caQtDM Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2010 - 2014
 *
 *  Author:
 *    Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch
 */


#include <QSizePolicy>
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QLabel>
#include <QWidget>
#include "splashscreen.h"
#include <QStyleOptionProgressBarV2>
#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>

SplashScreen::SplashScreen(QWidget *parent) : QSplashScreen(parent), m_progress(0)

{
    Qt::WindowFlags flags = 0;
    flags |= Qt::WindowStaysOnTopHint | Qt::SplashScreen ;
    setWindowFlags(flags);

    m_maximum = 100;

#if defined(MOBILE_IOS)
    pixmapLoad.load(":caQtDM-logos.png");
    QSize size = qApp->desktop()->size();
    if(size.height() < 500) {
       pixmap = pixmapLoad.scaled(pixmapLoad.size().width()/2, pixmapLoad.size().height()/2);
    } else {
       pixmap = pixmapLoad.scaled(pixmapLoad.size().width(), pixmapLoad.size().height());
    }
#elif defined(MOBILE_ANDROID)
    pixmapLoad.load(":caQtDM-logos.png");
    pixmap = pixmapLoad.scaled(pixmapLoad.size().width()*1.5, pixmapLoad.size().height()*1.5); // probably wrong
#else
    pixmap.load(":caQtDM-logos.png");
#endif

    this->resize(pixmap.size().width()+200, pixmap.size().height()+100);

    // in order to have a pseudo-transparent image, I load the background (22.4.2015 do not do this anymore, while for Qt5 not ok anyhow
//#ifndef MOBILE
//    QPixmap desktopBackground= QPixmap::grabWindow(QApplication::desktop()->winId(), x()- width()/2, y()-height()/2, width(),height());
//#else
    QPixmap desktopBackground( width(),height());
    desktopBackground.fill(Qt::gray);
//#endif

    // and merge the two pixmaps
    QPainter p;
    p.begin(&desktopBackground);
    QPixmap scaledPixmap = pixmap.scaled(pixmap.size().width()+200,  pixmap.size().height()+200);
    p.drawPixmap(0, 0, scaledPixmap);

    p.setPen(QPen(QColor(200,200,200), 2));
    p.drawRect(2,2, width()-4,height()-4);

    QBrush brush(QColor(200,200,200,255), Qt::SolidPattern);
    p.setBrush(brush);
    p.drawRect(2, height()-70, width()-4, 68);

    p.end();

    this->setPixmap(desktopBackground);
    this->setCursor(Qt::BusyCursor);
    this->showMessage("loading include ui files", Qt::AlignBottom, QColor(Qt::black));
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
