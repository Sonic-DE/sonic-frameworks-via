// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: SonicDE

#include <via/viacommand.h>

#include <QtTest>

class TestViaCommand : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testReportIs33BytesAndStartsWithReportId();
    void testFactoryMethods();
    void testPayloadTruncation();
};

void TestViaCommand::testReportIs33BytesAndStartsWithReportId()
{
    ViaCommand cmd(ViaCommandId::CustomSetValue);
    const QByteArray report = cmd.toReport();
    QCOMPARE(report.size(), 33);
    QCOMPARE(static_cast<unsigned char>(report[0]), 0x00); // report id
    QCOMPARE(static_cast<unsigned char>(report[1]), static_cast<unsigned char>(ViaCommandId::CustomSetValue));
}

void TestViaCommand::testFactoryMethods()
{
    QCOMPARE(int(ViaCommand::getProtocolVersion().commandId()), int(ViaCommandId::GetProtocolVersion));

    const auto set = ViaCommand::customSetValue(LightingChannel::QmkRgbMatrix, RgbValueId::Color, QByteArray::fromRawData("\x10\x20", 2));
    QCOMPARE(int(set.commandId()), int(ViaCommandId::CustomSetValue));
    // payload = [channel(3), valueId(4), hue(0x10), sat(0x20)]
    QCOMPARE(set.payload().size(), 4);
    QCOMPARE(static_cast<unsigned char>(set.payload()[0]), 0x03);
    QCOMPARE(static_cast<unsigned char>(set.payload()[1]), 0x04);

    QCOMPARE(int(ViaCommand::customGetValue(LightingChannel::QmkRgbMatrix, RgbValueId::Color).commandId()), int(ViaCommandId::CustomGetValue));
    QCOMPARE(int(ViaCommand::customSave().commandId()), int(ViaCommandId::CustomSave));
}

void TestViaCommand::testPayloadTruncation()
{
    ViaCommand cmd(ViaCommandId::CustomSetValue, QByteArray(50, 'A'));
    QCOMPARE(cmd.payload().size(), 31); // 33 - report id - command id
}

QTEST_APPLESS_MAIN(TestViaCommand)
#include "test_command.moc"
