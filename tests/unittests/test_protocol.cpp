// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: SonicDE

#include <via/viacommand.h>
#include <via/viaprotocol.h>

#include <QtTest>

// Mock device that returns canned responses. Works because ViaDevice::sendReport is virtual.
// Supports either a single static response (m_response) or a per-call sequence (m_responses),
// which is popped front-first; the queue takes priority over m_response. m_lastReport captures
// the most recent report sent so tests can assert on-wire encoding (e.g. legacy vs V3 payloads).
class MockViaDevice : public ViaDevice
{
public:
    explicit MockViaDevice(const QString &path)
        : ViaDevice(path)
    {
    }
    QByteArray m_response;
    QList<QByteArray> m_responses;
    QByteArray m_lastReport;
    QByteArray sendReport(const QByteArray &report) override
    {
        m_lastReport = report;
        if (!m_responses.isEmpty()) {
            return m_responses.takeFirst();
        }
        return m_response;
    }
};

class TestViaProtocol : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSetColorSuccess();
    void testGetColorValue();
    void testUnsupportedReturnsFalse();
    void testMismatchedEchoReturnsFalse();
    void testHasLightingSupportMatrix();
    void testHasLightingSupportRgblightEffectZero();
    void testHasLightingSupportNone();
    void testLegacySetColorEncoding();
    void testLegacyGetColorOffset();
    void testLegacyMatrixRejected();
    void testQueryProtocolVersion();
};

void TestViaProtocol::testSetColorSuccess()
{
    MockViaDevice dev(QStringLiteral("/dev/hidraw0"));
    // Firmware echoes command id 0x07, then channel(3), valueId(4), hue, sat.
    dev.m_response = QByteArray("\x07\x03\x04\x10\x20", 5);

    ViaProtocol p(dev);
    QVERIFY(p.setColor(LightingChannel::QmkRgbMatrix, 0x10, 0x20));
}

void TestViaProtocol::testGetColorValue()
{
    MockViaDevice dev(QStringLiteral("/dev/hidraw0"));
    // response = [0x08, channel(3), valueId(4), hue(0xAA), sat(0xBB)]
    dev.m_response = QByteArray("\x08\x03\x04\xAA\xBB", 5);

    ViaProtocol p(dev);
    QByteArray value;
    QVERIFY(p.getLightingValue(LightingChannel::QmkRgbMatrix, RgbValueId::Color, value));
    QCOMPARE(value.size(), 2); // [hue, sat]
    QCOMPARE(static_cast<unsigned char>(value[0]), 0xAA);
    QCOMPARE(static_cast<unsigned char>(value[1]), 0xBB);
}

void TestViaProtocol::testUnsupportedReturnsFalse()
{
    MockViaDevice dev(QStringLiteral("/dev/hidraw0"));
    dev.m_response = QByteArray("\xFF\x00\x00\x00", 4); // 0xFF => unsupported
    ViaProtocol p(dev);
    QVERIFY(!p.setColor(LightingChannel::QmkRgbMatrix, 0x10, 0x20));
}

void TestViaProtocol::testMismatchedEchoReturnsFalse()
{
    MockViaDevice dev(QStringLiteral("/dev/hidraw0"));
    dev.m_response = QByteArray("\x02\x03\x04\x00", 4); // echoed id != 0x07
    ViaProtocol p(dev);
    QVERIFY(!p.setColor(LightingChannel::QmkRgbMatrix, 0x10, 0x20));
}

// has_devices predicate: matrix channel responds → support true (effect value irrelevant).
void TestViaProtocol::testHasLightingSupportMatrix()
{
    MockViaDevice dev(QStringLiteral("/dev/hidraw0"));
    // getEffect(matrix) response: [0x08, channel(3 matrix), valueId(2 Effect), effect(0x05)]
    dev.m_responses = {QByteArray("\x08\x03\x02\x05", 4)};
    ViaProtocol p(dev);
    QVERIFY(p.hasLightingSupport());
}

// has_devices is more lenient than apply_color: matrix unsupported, but rgblight responds even
// with effect 0 (lighting disabled) → support still true. Rust has_devices uses .is_ok()
// (channel reachable), apply_color uses effect>0 (in use).
void TestViaProtocol::testHasLightingSupportRgblightEffectZero()
{
    MockViaDevice dev(QStringLiteral("/dev/hidraw0"));
    // call 1: matrix unsupported (0xFF); call 2: rgblight getEffect responds effect 0.
    dev.m_responses = {
        QByteArray("\xFF\x00\x00\x00", 4),
        QByteArray("\x08\x02\x02\x00", 4), // [0x08, channel(2 rgblight), Effect(2), effect=0]
    };
    ViaProtocol p(dev);
    QVERIFY(p.hasLightingSupport());
}

// Both channels unsupported → no lighting support.
void TestViaProtocol::testHasLightingSupportNone()
{
    MockViaDevice dev(QStringLiteral("/dev/hidraw0"));
    dev.m_responses = {
        QByteArray("\xFF\x00\x00\x00", 4),
        QByteArray("\xFF\x00\x00\x00", 4),
    };
    ViaProtocol p(dev);
    QVERIFY(!p.hasLightingSupport());
}

// Legacy (protocol 9) SET color encoding: payload = [valueId(0x83 Color), hue, sat] — NO channel
// byte, and the value id is 0x83 (legacy) not 0x04 (V3). Verified from qmk-via-api set_rgblight_color.
void TestViaProtocol::testLegacySetColorEncoding()
{
    MockViaDevice dev(QStringLiteral("/dev/hidraw0"));
    dev.m_response = QByteArray("\x07\x83\x10\x20", 4); // firmware echoes cmd 0x07
    ViaProtocol p(dev, kProtocolGamma); // protocol 9 (legacy)
    QVERIFY(p.setColor(LightingChannel::QmkRgblight, 0x10, 0x20));
    // report = [0x00 reportId, 0x07 CustomSetValue, 0x83 Color, 0x10 hue, 0x20 sat, zero-pad]
    QCOMPARE(static_cast<unsigned char>(dev.m_lastReport[0]), 0x00);
    QCOMPARE(static_cast<unsigned char>(dev.m_lastReport[1]), 0x07);
    QCOMPARE(static_cast<unsigned char>(dev.m_lastReport[2]), 0x83); // legacy Color id (no channel byte)
    QCOMPARE(static_cast<unsigned char>(dev.m_lastReport[3]), 0x10);
    QCOMPARE(static_cast<unsigned char>(dev.m_lastReport[4]), 0x20);
}

// Legacy GET color: response value starts at index 2 (cmd, valueId, hue, sat), not index 3.
// Verified from qmk-via-api get_rgblight_color legacy (val[2], val[3]).
void TestViaProtocol::testLegacyGetColorOffset()
{
    MockViaDevice dev(QStringLiteral("/dev/hidraw0"));
    // response = [0x08 cmd, 0x83 Color, 0xAA hue, 0xBB sat]
    dev.m_response = QByteArray("\x08\x83\xAA\xBB", 4);
    ViaProtocol p(dev, kProtocolGamma);
    QByteArray value;
    QVERIFY(p.getLightingValue(LightingChannel::QmkRgblight, RgbValueId::Color, value));
    QCOMPARE(value.size(), 2);
    QCOMPARE(static_cast<unsigned char>(value[0]), 0xAA);
    QCOMPARE(static_cast<unsigned char>(value[1]), 0xBB);
    // and the sent GET payload was [0x83] (no channel byte)
    QCOMPARE(static_cast<unsigned char>(dev.m_lastReport[1]), 0x08); // CustomGetValue
    QCOMPARE(static_cast<unsigned char>(dev.m_lastReport[2]), 0x83); // legacy Color, no channel prefix
}

// Legacy protocol has no RGB Matrix: setColor(matrix) must fail WITHOUT sending HID (no report set).
void TestViaProtocol::testLegacyMatrixRejected()
{
    MockViaDevice dev(QStringLiteral("/dev/hidraw0"));
    ViaProtocol p(dev, kProtocolGamma);
    QVERIFY(!p.setColor(LightingChannel::QmkRgbMatrix, 0x10, 0x20));
    QVERIFY(dev.m_lastReport.isEmpty()); // no HID command sent
}

// queryProtocolVersion parses the big-endian u16 at response[1],[2].
void TestViaProtocol::testQueryProtocolVersion()
{
    MockViaDevice dev(QStringLiteral("/dev/hidraw0"));
    // response = [0x01 cmd, 0x00 hi, 0x0B lo] → protocol 11 (V3)
    dev.m_response = QByteArray("\x01\x00\x0B", 3);
    QCOMPARE(ViaProtocol::queryProtocolVersion(dev), quint16(11));
}

QTEST_APPLESS_MAIN(TestViaProtocol)
#include "test_protocol.moc"
