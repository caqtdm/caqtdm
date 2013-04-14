#ifndef CADOUBLETABWIDGETEXTENSION_H
#define CADOUBLETABWIDGETEXTENSION_H

#include <QtDesigner/QDesignerContainerExtension>

class QExtensionManager;
class caDoubleTabWidget;

class caDoubleTabWidgetExtension: public QObject,
        public QDesignerContainerExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerContainerExtension)

public:
    caDoubleTabWidgetExtension(caDoubleTabWidget *widget, QObject *parent);

    void addWidget(QWidget *widget);
    int count() const;
    int currentIndex() const;
    void insertWidget(int index, QWidget *widget);
    void remove(int index);
    void setCurrentIndex(int index);
    QWidget *widget(int index) const;

private:
    caDoubleTabWidget *myWidget;
};

#endif
