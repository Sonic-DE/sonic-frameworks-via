// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: SonicDE

#include <via/viadiscovery.h>

#include <QtTest>

class TestViaDiscovery : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testStubReturnsEmpty();
};

void TestViaDiscovery::testStubReturnsEmpty()
{
    const auto keyboards = ViaDiscovery::discoverViaKeyboards();
#ifndef HAVE_VIA_SUPPORT
    QVERIFY(keyboards.isEmpty());
#else
    Q_UNUSED(keyboards)
#endif
}

QTEST_APPLESS_MAIN(TestViaDiscovery)
#include "test_discovery.moc"
