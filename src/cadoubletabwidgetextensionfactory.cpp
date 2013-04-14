#include "cadoubletabwidgetextensionfactory.h"
#include "cadoubletabwidgetextension.h"
#include "cadoubletabwidget.h"

caDoubleTabWidgetExtensionFactory::caDoubleTabWidgetExtensionFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{}

QObject *caDoubleTabWidgetExtensionFactory::createExtension(QObject *object,
                                                            const QString &iid,
                                                            QObject *parent) const
{
    caDoubleTabWidget *widget = qobject_cast<caDoubleTabWidget*>(object);

    if (widget && (iid == Q_TYPEID(QDesignerContainerExtension))) {
        return new caDoubleTabWidgetExtension(widget, parent);
    } else {
        return 0;
    }
}

