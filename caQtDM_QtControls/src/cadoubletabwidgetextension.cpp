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

#include "cadoubletabwidgetextension.h"
#include "cadoubletabwidget.h"

caDoubleTabWidgetExtension::caDoubleTabWidgetExtension(caDoubleTabWidget *widget,
                                                                         QObject *parent)
    :QObject(parent)
{
    myWidget = widget;
}

void caDoubleTabWidgetExtension::addWidget(QWidget *widget)
{
    myWidget->addPage(widget);
}

int caDoubleTabWidgetExtension::count() const
{
    return myWidget->count();
}

int caDoubleTabWidgetExtension::currentIndex() const
{
    return myWidget->currentIndex();
}

void caDoubleTabWidgetExtension::insertWidget(int index, QWidget *widget)
{
    myWidget->insertPage(index, widget);
}

void caDoubleTabWidgetExtension::remove(int index)
{
    myWidget->removePage(index);
}

void caDoubleTabWidgetExtension::setCurrentIndex(int index)
{
    myWidget->setCurrentIndex(index);
}

QWidget* caDoubleTabWidgetExtension::widget(int index) const
{
    return myWidget->widget(index);
}
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
bool caDoubleTabWidgetExtension::canAddWidget() const{
    return true;

}
bool caDoubleTabWidgetExtension::canRemove(int index) const{
    return true;
}
#endif
