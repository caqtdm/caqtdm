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
