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
 *  Copyright (c) 2010 - 2026
 *
 *  Author:
 *    Julian Houba
 *  Contact details:
 *    julian.houba@psi.ch
 */

#include "tst_caimage.h"

#include <caimage.h>

#include <QImage>
#include <QLabel>
#include <QMetaProperty>
#include <QSet>
#include <QTemporaryDir>
#include <QTest>

#include <cstring>

namespace {

QString createScalingFixture(QTemporaryDir &directory, QSet<QRgb> &palette)
{
    QImage image(2, 2, QImage::Format_ARGB32);
    image.setPixel(0, 0, qRgba(255, 0, 0, 255));
    image.setPixel(1, 0, qRgba(0, 255, 0, 255));
    image.setPixel(0, 1, qRgba(0, 0, 255, 255));
    image.setPixel(1, 1, qRgba(255, 255, 255, 255));

    for(int y = 0; y < image.height(); ++y) {
        for(int x = 0; x < image.width(); ++x) {
            palette.insert(image.pixel(x, y));
        }
    }

    const QString fileName = directory.filePath("scaling.png");
    if(!image.save(fileName)) return QString();
    return fileName;
}

bool usesOnlyPalette(const QImage &image, const QSet<QRgb> &palette)
{
    for(int y = 0; y < image.height(); ++y) {
        for(int x = 0; x < image.width(); ++x) {
            if(!palette.contains(image.pixel(x, y))) return false;
        }
    }
    return true;
}

bool containsInterpolatedPixel(const QImage &image, const QSet<QRgb> &palette)
{
    for(int y = 0; y < image.height(); ++y) {
        for(int x = 0; x < image.width(); ++x) {
            if(!palette.contains(image.pixel(x, y))) return true;
        }
    }
    return false;
}

QImage renderedImage(caImage &image)
{
    QLabel *label = image.findChild<QLabel *>();
    if(label == Q_NULLPTR) return QImage();
    return label->grab().toImage().convertToFormat(QImage::Format_ARGB32);
}

QPixmap labelPixmap(const QLabel *label)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const QPixmap *pixmap = label->pixmap();
    return pixmap == Q_NULLPTR ? QPixmap() : *pixmap;
#else
    return label->pixmap(Qt::ReturnByValue);
#endif
}

bool hasVisiblePixels(const QImage &image)
{
    for(int y = 0; y < image.height(); ++y) {
        for(int x = 0; x < image.width(); ++x) {
            if(qAlpha(image.pixel(x, y)) != 0) return true;
        }
    }
    return false;
}

void verifyScalingMode(const QImage &image, const QSet<QRgb> &palette, bool smoothScaling)
{
    if(smoothScaling) QVERIFY(containsInterpolatedPixel(image, palette));
    else QVERIFY(usesOnlyPalette(image, palette));
}

}

void TestCaImage::smoothScalingPropertyAndPixels()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QSet<QRgb> palette;
    const QString fileName = createScalingFixture(directory, palette);
    QVERIFY(!fileName.isEmpty());

    caImage image;
    const int propertyIndex = image.metaObject()->indexOfProperty("smoothScaling");
    QVERIFY(propertyIndex >= 0);
    const QMetaProperty property = image.metaObject()->property(propertyIndex);
    QVERIFY(property.isWritable());
    QCOMPARE(std::strcmp(property.typeName(), "bool"), 0);
    QVERIFY(!image.getSmoothScaling());

    image.resize(7, 5);
    image.show();
    image.setFileName(fileName, false);
    QCoreApplication::processEvents();

    QLabel *label = image.findChild<QLabel *>();
    QVERIFY(label != Q_NULLPTR);
    QCOMPARE(label->size(), image.contentsRect().size());
    const QImage nearestImage = renderedImage(image);
    QCOMPARE(nearestImage.size(), image.contentsRect().size());
    QVERIFY(usesOnlyPalette(nearestImage, palette));

    image.setSmoothScaling(true);
    QCoreApplication::processEvents();
    const QImage smoothImage = renderedImage(image);
    QVERIFY(containsInterpolatedPixel(smoothImage, palette));

    image.setSmoothScaling(false);
    QCoreApplication::processEvents();
    QVERIFY(usesOnlyPalette(renderedImage(image), palette));
}

void TestCaImage::smoothScalingBeforeLoad()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QSet<QRgb> palette;
    const QString fileName = createScalingFixture(directory, palette);
    QVERIFY(!fileName.isEmpty());

    caImage image;
    image.setSmoothScaling(true);
    image.resize(7, 5);
    image.show();
    image.setFileName(fileName, false);
    QCoreApplication::processEvents();

    QVERIFY(image.getSmoothScaling());
    QVERIFY(containsInterpolatedPixel(renderedImage(image), palette));
}

void TestCaImage::resizeKeepsSelectedRendering_data()
{
    QTest::addColumn<bool>("smoothScaling");
    QTest::newRow("nearest") << false;
    QTest::newRow("smooth") << true;
}

void TestCaImage::resizeKeepsSelectedRendering()
{
    QFETCH(bool, smoothScaling);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QSet<QRgb> palette;
    const QString fileName = createScalingFixture(directory, palette);
    QVERIFY(!fileName.isEmpty());

    caImage image;
    image.setSmoothScaling(smoothScaling);
    image.resize(7, 5);
    image.show();
    image.setFileName(fileName, false);
    QCoreApplication::processEvents();

    image.resize(13, 11);
    QCoreApplication::processEvents();
    const QImage resizedImage = renderedImage(image);
    QCOMPARE(resizedImage.size(), image.contentsRect().size());
    QCOMPARE(image.getSmoothScaling(), smoothScaling);
    verifyScalingMode(resizedImage, palette, smoothScaling);
}

void TestCaImage::setFrameUpdatesAnimatedImage_data()
{
    QTest::addColumn<bool>("smoothScaling");
    QTest::newRow("nearest") << false;
    QTest::newRow("smooth") << true;
}

void TestCaImage::setFrameUpdatesAnimatedImage()
{
    QFETCH(bool, smoothScaling);
    const QString fileName = QFINDTESTDATA("../../caQtDM_Tests/pacman-eating.gif");
    QVERIFY2(!fileName.isEmpty(), "pacman-eating.gif fixture not found");

    caImage image;
    image.setSmoothScaling(smoothScaling);
    image.resize(70, 70);
    image.show();
    image.setFileName(fileName, false);
    QCoreApplication::processEvents();
    QVERIFY(image.getFrameCount() > 1);

    image.setFrame(0);
    QCoreApplication::processEvents();
    const QImage firstFrame = renderedImage(image);
    QCOMPARE(image.getFrame(), 0);
    QCOMPARE(firstFrame.size(), image.contentsRect().size());

    image.resize(83, 71);
    QCoreApplication::processEvents();
    const QImage resizedFirstFrame = renderedImage(image);
    QCOMPARE(resizedFirstFrame.size(), image.contentsRect().size());

    image.setFrame(1);
    QCoreApplication::processEvents();
    const QImage secondFrame = renderedImage(image);
    QCOMPARE(image.getFrame(), 1);
    QCOMPARE(image.getSmoothScaling(), smoothScaling);
    QCOMPARE(secondFrame.size(), image.contentsRect().size());
    QVERIFY(resizedFirstFrame != secondFrame);
}

void TestCaImage::invalidStateRestoresCurrentFrame_data()
{
    QTest::addColumn<bool>("smoothScaling");
    QTest::newRow("nearest") << false;
    QTest::newRow("smooth") << true;
}

void TestCaImage::invalidStateRestoresCurrentFrame()
{
    QFETCH(bool, smoothScaling);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QSet<QRgb> palette;
    const QString fileName = createScalingFixture(directory, palette);
    QVERIFY(!fileName.isEmpty());

    caImage image;
    image.setSmoothScaling(smoothScaling);
    image.resize(7, 5);
    image.show();
    image.setFileName(fileName, false);
    QCoreApplication::processEvents();

    QLabel *label = image.findChild<QLabel *>();
    QVERIFY(label != Q_NULLPTR);
    const QImage rendered = renderedImage(image);
    verifyScalingMode(rendered, palette, smoothScaling);
    QVERIFY(!labelPixmap(label).isNull());

    image.setInvalid(Qt::magenta);
    QCoreApplication::processEvents();
    QVERIFY(labelPixmap(label).isNull());

    image.setValid();
    QCoreApplication::processEvents();
    QCOMPARE(image.getSmoothScaling(), smoothScaling);
    QCOMPARE(renderedImage(image), rendered);
}

void TestCaImage::tiltAngleRerendersCurrentFrame()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QSet<QRgb> palette;
    const QString fileName = createScalingFixture(directory, palette);
    QVERIFY(!fileName.isEmpty());

    caImage image;
    image.resize(7, 5);
    image.show();
    image.setFileName(fileName, false);
    QCoreApplication::processEvents();
    const QImage unrotated = renderedImage(image);

    image.setAngle(90);
    QCoreApplication::processEvents();
    const QImage rotated = renderedImage(image);
    QCOMPARE(image.getAngle(), 90);
    QVERIFY(hasVisiblePixels(rotated));
    QVERIFY(rotated != unrotated);

    image.setSmoothScaling(true);
    QCoreApplication::processEvents();
    const QImage smoothRotated = renderedImage(image);
    QCOMPARE(image.getAngle(), 90);
    QVERIFY(image.getSmoothScaling());
    QVERIFY(hasVisiblePixels(smoothRotated));
    QVERIFY(smoothRotated != rotated);
}
