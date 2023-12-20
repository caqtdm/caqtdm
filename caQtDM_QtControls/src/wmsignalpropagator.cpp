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
#include <QMainWindow>
#include "wmsignalpropagator.h"
#include "cainclude.h"


wmSignalPropagator::wmSignalPropagator( QWidget *parent ) : ESimpleLabel(parent)
{
    setForeAndBackground(Qt::black, Qt::lightGray);
    setFontScaleMode(WidthAndHeight);
    setText("wmSignals");
    Parent = parent;
}

void wmSignalPropagator::setResizeFactors(const double factX, const double factY) {
    emit wmFactorX(factX);
    emit wmFactorY(factY);
}

void wmSignalPropagator::closewindow() {emit wmCloseWindow();}
void wmSignalPropagator::reloadwindow() {emit wmReloadWindow();}
void wmSignalPropagator::shownormal() {emit wmShowNormal();}
void wmSignalPropagator::showmaximized() {emit wmShowMaximized();}
void wmSignalPropagator::showminimized() {emit wmShowMinimized();}
void wmSignalPropagator::showfullscreen() {emit wmShowFullScreen();}
void wmSignalPropagator::printwindow() {emit wmPrintWindow();}
void wmSignalPropagator::resizeMainWindow(QRect p){emit wmResizeMainWindow(p);}

void wmSignalPropagator::propagateToParent(QRect p) {

    // move parent to the p.x and p.y
    qDebug() << "propagateToParent";
    printf("propagateToParent\n");
    fflush(stdout);
    QWidget *w = Parent;
    caInclude *includeWidget = (caInclude *) Q_NULLPTR;
    while(w != (QWidget*) Q_NULLPTR) {
        if(w != (QWidget*) Q_NULLPTR) {
            if(caInclude* widget = qobject_cast<caInclude *>(w)) {
                includeWidget = widget;
                if((widget->x() != p.x()) || (widget->y() != p.y())) {
                    widget->move(p.x(), p.y());
                    //printf("propagate:: move parent include to %d %d\n", p.x(), p.y());
                    break;
                } else {

                }
            }
        }
        w = (QWidget*) w->parent();
    }

    // adjust the scroll area, but only when something was moved
    if(includeWidget != (caInclude *) Q_NULLPTR) {
        if(QScrollArea* scrollWidget = qobject_cast<QScrollArea *>(includeWidget->parent()->parent()->parent())) {
            int maxX=300;
            int maxY=200;
            QList<QWidget *> allW = scrollWidget->findChildren<QWidget *>();
            foreach(QWidget* widget, allW) {
                if(widget->x() + widget->width() > maxX) maxX = widget->x() + widget->width();
                if(widget->y() + widget->height() > maxY) maxY = widget->y() + widget->height();
            }
            QWidget *contents = (QWidget*) includeWidget->parent();
            if(contents != (QWidget *) Q_NULLPTR) {
               QSize sizew = contents->minimumSize();
               if(maxX > sizew.width() || maxY > sizew.height()) {
                   //printf("propagate:: resize print area\n");
                   contents->setMinimumSize(maxX, maxY);
               }
            }
        }
    }
}

void wmSignalPropagator::setForeAndBackground(QColor fg, QColor bg)
{
    QString thisStyle = "background-color: rgb(%1, %2, %3); color: rgb(%4, %5, %6);";
    thisStyle = thisStyle.arg(bg.red()).arg(bg.green()).arg(bg.blue()).
            arg(fg.red()).arg(fg.green()).arg(fg.blue());

    setStyleSheet(thisStyle);
}
