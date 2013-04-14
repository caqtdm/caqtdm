#ifndef CADOUBLETABWIDGETEXTENSIONFACTORY_H
#define CADOUBLETABWIDGETEXTENSIONFACTORY_H

#include <QtDesigner/QExtensionFactory>

class QExtensionManager;

class caDoubleTabWidgetExtensionFactory: public QExtensionFactory
{
    Q_OBJECT

public:
    caDoubleTabWidgetExtensionFactory(QExtensionManager *parent = 0);

protected:
    QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

#endif
