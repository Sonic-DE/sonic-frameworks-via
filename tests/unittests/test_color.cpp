// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: SonicDE

#include <via/viaprotocol.h>

#include <QColor>
#include <QtTest>

class TestViaColor : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRed();
    void testAchromaticGray();
    void testInvalidColor();
};

void TestViaColor::testRed()
{
    // Red: HSV (0°, 100%, 100%) -> hue 0, saturation 255, brightness 255.
    const ViaHsv hsv = ViaProtocol::hsvFromColor(QColor(0xFF, 0x00, 0x00));
    QCOMPARE(hsv.hue, quint8(0));
    QCOMPARE(hsv.saturation, quint8(255));
    QCOMPARE(hsv.brightness, quint8(255));
}

void TestViaColor::testAchromaticGray()
{
    // Gray is achromatic: Qt reports hue -1.0; expect hue 0, saturation 0.
    const ViaHsv hsv = ViaProtocol::hsvFromColor(QColor(128, 128, 128));
    QCOMPARE(hsv.hue, quint8(0));
    QCOMPARE(hsv.saturation, quint8(0));
}

void TestViaColor::testInvalidColor()
{
    const ViaHsv hsv = ViaProtocol::hsvFromColor(QColor());
    QCOMPARE(hsv.hue, quint8(0));
    QCOMPARE(hsv.saturation, quint8(0));
    QCOMPARE(hsv.brightness, quint8(0));
}

QTEST_APPLESS_MAIN(TestViaColor)
#include "test_color.moc"
