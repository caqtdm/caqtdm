#include "tst_qenum.h"

#include <QMetaEnum>
#include <QMetaProperty>
#include <QTest>
#include <QVariant>

#include "caalarmtree.h"
#include "cabitnames.h"
#include "cagraphics.h"
#include "caimage.h"
#include "cainclude.h"
#include "calabel.h"
#include "calabelvertical.h"
#include "capolyline.h"
#include "eflag.h"
#include "esimplelabel.h"
#include "camultilinestring.h"

namespace {

void checkEnumProperty(const QMetaObject &metaObject, const char *propertyName,
                       const char *keys[], int values[], int count)
{
    const int index = metaObject.indexOfProperty(propertyName);
    QVERIFY2(index >= 0, propertyName);

    const QMetaProperty property = metaObject.property(index);
    QVERIFY2(property.isEnumType(), propertyName);
    const QMetaEnum enumerator = property.enumerator();
    QVERIFY2(enumerator.isValid(), propertyName);
    QCOMPARE(enumerator.keyCount(), count);

    for (int i = 0; i < count; ++i) {
        QCOMPARE(QByteArray(enumerator.key(i)), QByteArray(keys[i]));
        QCOMPARE(enumerator.value(i), values[i]);
    }
}

} // namespace

void TestQEnum::registeredEnumTypes()
{
    const QMetaEnum scaleMode = QMetaEnum::fromType<ESimpleLabel::ScaleMode>();
    QVERIFY(scaleMode.isValid());
    QCOMPARE(scaleMode.keyCount(), 3);
    QCOMPARE(QByteArray(scaleMode.key(0)), QByteArray("None"));
    QCOMPARE(scaleMode.value(0), int(ESimpleLabel::None));
    QCOMPARE(QByteArray(scaleMode.key(1)), QByteArray("Height"));
    QCOMPARE(scaleMode.value(1), int(ESimpleLabel::Height));
    QCOMPARE(QByteArray(scaleMode.key(2)), QByteArray("WidthAndHeight"));
    QCOMPARE(scaleMode.value(2), int(ESimpleLabel::WidthAndHeight));

    const QMetaEnum shape = QMetaEnum::fromType<caMultiLineString::Shape>();
    const QMetaEnum shadow = QMetaEnum::fromType<caMultiLineString::Shadow>();
    const QMetaEnum wrapMode = QMetaEnum::fromType<caMultiLineString::WrapMode>();
    QVERIFY(shape.isValid());
    QVERIFY(shadow.isValid());
    QVERIFY(wrapMode.isValid());
}

void TestQEnum::enumProperties()
{
    const char *visibilityKeys[] = {"StaticV", "IfNotZero", "IfZero", "Calc"};
    int visibilityValues[] = {0, 1, 2, 3};

    checkEnumProperty(caGraphics::staticMetaObject, "visibility", visibilityKeys,
                      visibilityValues, 4);
    checkEnumProperty(caImage::staticMetaObject, "visibility", visibilityKeys,
                      visibilityValues, 4);
    checkEnumProperty(caInclude::staticMetaObject, "visibility", visibilityKeys,
                      visibilityValues, 4);
    checkEnumProperty(caLabel::staticMetaObject, "visibility", visibilityKeys,
                      visibilityValues, 4);
    checkEnumProperty(caLabelVertical::staticMetaObject, "visibility", visibilityKeys,
                      visibilityValues, 4);
    checkEnumProperty(caPolyLine::staticMetaObject, "visibility", visibilityKeys,
                      visibilityValues, 4);

    const char *shapeKeys[] = {"NoFrame"};
    int shapeValues[] = {0};
    const char *shadowKeys[] = {"Plain"};
    int shadowValues[] = {0x0010};
    const char *wrapKeys[] = {"noWrap"};
    int wrapValues[] = {0};
    checkEnumProperty(caMultiLineString::staticMetaObject, "frameShape", shapeKeys,
                      shapeValues, 1);
    checkEnumProperty(caMultiLineString::staticMetaObject, "frameShadow", shadowKeys,
                      shadowValues, 1);
    checkEnumProperty(caMultiLineString::staticMetaObject, "lineWrapMode", wrapKeys,
                      wrapValues, 1);

    const char *commandKeys[] = {"Shell", "Script", "Ask"};
    int commandValues[] = {0, 1, 2};
    checkEnumProperty(caAlarmTree::staticMetaObject, "commandMode", commandKeys,
                      commandValues, 3);
}

void TestQEnum::unrelatedEnumPropertyWrites()
{
    EFlag flag(nullptr);
    flag.setFontScaleMode(ESimpleLabel::None);
    const int flagIndex = flag.metaObject()->indexOfProperty("fontScaleMode");
    QVERIFY(flagIndex >= 0);
    const QMetaProperty flagProperty = flag.metaObject()->property(flagIndex);
    QVERIFY(flagProperty.write(&flag, QVariant(QStringLiteral("WidthAndHeight"))));
    QCOMPARE(flag.fontScaleMode(), ESimpleLabel::WidthAndHeight);

    caBitnames bitnames(nullptr);
    bitnames.setFontScaleModeL(ESimpleLabel::None);
    const int bitnamesIndex = bitnames.metaObject()->indexOfProperty("fontScaleMode");
    QVERIFY(bitnamesIndex >= 0);
    const QMetaProperty bitnamesProperty = bitnames.metaObject()->property(bitnamesIndex);
    QVERIFY(bitnamesProperty.write(&bitnames, QVariant(QStringLiteral("WidthAndHeight"))));
    QCOMPARE(bitnames.fontScaleModeL(), ESimpleLabel::WidthAndHeight);
}
