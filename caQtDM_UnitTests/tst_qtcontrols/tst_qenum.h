#ifndef TST_QENUM_H
#define TST_QENUM_H

#include <QObject>

class TestQEnum : public QObject
{
    Q_OBJECT

private slots:
    void registeredEnumTypes();
    void enumProperties();
    void unrelatedEnumPropertyWrites();
};

#endif // TST_QENUM_H
