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

#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H


#include <QSplashScreen>
#include <QApplication>

#include "caQtDM_Lib_global.h"

class CAQTDM_LIBSHARED_EXPORT  SplashScreen : public QSplashScreen
{
    Q_OBJECT
public:
    explicit SplashScreen( QWidget *parent = 0);
    int m_progress;
    void setMaximum(int max);


public slots:
    void setProgress(int value)
    {
      m_progress = value;
      if (m_progress > m_maximum) m_progress = m_maximum;
      if (m_progress < 0)         m_progress = 0;
      update();
      QApplication::processEvents();
    }

protected:
    void drawContents(QPainter *painter);

private:
        QPixmap pixmap;
        QPixmap pixmapLoad;
        int m_maximum;

};

#endif
